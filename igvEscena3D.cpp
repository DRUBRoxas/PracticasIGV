#include <cstdlib>
#include <stdio.h>

#include "igvEscena3D.h"

#include <iostream>
#include <math.h>

// Métodos constructores -----------------------------------

// Constructor: estado inicial de transformaciones
igvEscena3D::igvEscena3D() {
   for (auto & i : objT) {
      i = {0.0f, 0.0f, 0.0f,
                 0.0f, 0.0f, 0.0f,
                 1.0f};
   }
   // Crear malla Godzilla
   generarMallaGodzilla();

   // Generar Textura Ajedrez para el suelo
   generarTexturaAjedrez();

   texturaSuelo[0] = nullptr;
   texturaSuelo[1] = nullptr;
   try {
      // Asegúrate de tener estas imágenes o cambiar el nombre
      texturaSuelo[0] = new igvTextura("../baldosa.png");
   } catch (std::exception& e) { std::cout << "Aviso: No se cargo baldosa.png (usando ajedrez)\n"; }

   try {
      texturaSuelo[1] = new igvTextura("../grass.png");
   } catch (std::exception& e) { std::cout << "Aviso: No se cargo cesped.png (usando ajedrez)\n"; }

   // 3. Definir 3 Materiales
   // Material 0: Gris Mate (Base)
   materialSuelo[0].set(igvColor(0.2, 0.2, 0.2), igvColor(0.6, 0.6, 0.6), igvColor(0.0, 0.0, 0.0), 0);
   // Material 1: Dorado Brillante
   materialSuelo[1].set(igvColor(0.25, 0.2, 0.07), igvColor(0.75, 0.6, 0.23), igvColor(0.63, 0.56, 0.37), 50.0);
   // Material 2: Azul Plástico
   materialSuelo[2].set(igvColor(0.0, 0.1, 0.3), igvColor(0.0, 0.3, 0.8), igvColor(0.5, 0.5, 0.5), 20.0);

   // Inicializar luz puntual con los parámetros especificados
   luzPuntual = igvFuenteLuz(
      GL_LIGHT0,                           // a) Identificador
      igvPunto3D(1.0, 1.0, 1.0),          // b) Posición
      igvColor(0.0, 0.0, 0.0, 1.0),       // c) Color ambiental
      igvColor(1.0, 1.0, 1.0, 1.0),       // d) Color difuso
      igvColor(1.0, 1.0, 1.0, 1.0),       // e) Color especular
      1.0, 0.0, 0.0                       // f) Coeficientes de atenuación radial (a0, a1, a2)
   );

   // Luz ambiental (usada para GL_LIGHT_MODEL_AMBIENT)
   luzAmbiental = igvFuenteLuz(
      GL_LIGHT1,
      igvPunto3D(0.0,0.0,0.0),
      igvColor(0.15, 0.15, 0.15, 1.0),
      igvColor(0.0,0.0,0.0,1.0),
      igvColor(0.0,0.0,0.0,1.0),
      1.0,0.0,0.0
   );
   // No usar posicion para ambiente: we'll use its ambient color via glLightModel in visualizar

   // Luz direccional (simula sol suave desde un ángulo)
   luzDireccional = igvFuenteLuz(
      GL_LIGHT2,
      igvPunto3D(-1.0, -1.0, -0.5), // direction vector (w=0 later)
      igvColor(0.0, 0.0, 0.0, 1.0),
      igvColor(0.8, 0.8, 0.7, 1.0),
      igvColor(0.6, 0.6, 0.6, 1.0),
      1.0, 0.0, 0.0
   );
   luzDireccional.setDirectional(true); // treat as directional (w = 0)

   // Cono / foco: colócalo justo encima del origen mirando al suelo para verlo claro
   luzCono = igvFuenteLuz(
      GL_LIGHT3,
      igvPunto3D(0.0f, 6.0f, 0.0f), // posición elevada sobre el centro
      igvColor(0.0, 0.0, 0.0, 1.0),
      igvColor(1.0, 0.95, 0.9, 1.0),
      igvColor(1.0, 1.0, 1.0, 1.0),
      1.0, 0.0, 0.0,
      igvPunto3D(0.0f, -1.0f, 0.0f), // dirección apuntando hacia abajo
      30.0, // cutoff angle algo más abierto
      10.0  // exponent más suave
   );

   // Guardar la pose inicial del robot (baseline) la primera vez que se construye la escena.
   // Esto asegura que la animación siempre sea relativa a la posición inicial al arrancar el programa.
   robotStateInitial = robotState;
}


// Destructor
igvEscena3D::~igvEscena3D() {
   if (objetoGodzilla) {
      delete objetoGodzilla;
      objetoGodzilla = nullptr;
   }
   // Liberar texturas del suelo
   if (texturaSuelo[0]) {
      delete texturaSuelo[0];
      texturaSuelo[0] = nullptr;
   }
   if (texturaSuelo[1]) {
      delete texturaSuelo[1];
      texturaSuelo[1] = nullptr;
   }
   glDeleteTextures(1, &idTexturaAjedrez);
}

void igvEscena3D::pintarSuelo() {
   // 1. Aplicar Material
   materialSuelo[indiceMaterialActual].aplicar();

   // 2. Configurar Textura
   if (usarTextura) {
      glEnable(GL_TEXTURE_2D);

      // Seleccionar ID según el índice
      unsigned int idToUse = idTexturaAjedrez; // Por defecto
      if (indiceTexturaActual == 1 && texturaSuelo[0]) idToUse = texturaSuelo[0]->getIdTextura();
      if (indiceTexturaActual == 2 && texturaSuelo[1]) idToUse = texturaSuelo[1]->getIdTextura();

      glBindTexture(GL_TEXTURE_2D, idToUse);

      // 3. Aplicar Filtros (MAG y MIN)
      GLint mag = GL_LINEAR;
      GLint min = GL_LINEAR;

      switch (filtroActual) {
         case FILTRO_NN: mag = GL_NEAREST; min = GL_NEAREST; break;
         case FILTRO_NL: mag = GL_NEAREST; min = GL_LINEAR; break;
         case FILTRO_LN: mag = GL_LINEAR;  min = GL_NEAREST; break;
         case FILTRO_LL: mag = GL_LINEAR;  min = GL_LINEAR; break;
      }
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min);
      // Repeticion para que se vea bien el suelo
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

   } else {
      glDisable(GL_TEXTURE_2D);
   }

   // 4. Dibujar Quad (Suelo) subdividido para mejorar la iluminacion
   const float halfSize = 20.0f; // Mantiene el tamano original (40x40)
   const int subdivX = 50;
   const int subdivZ = 50;
   const float repeticiones = 5.0f; // Para que la textura se repita

   glNormal3f(0.0f, 1.0f, 0.0f);
   pintar_quad(subdivX, subdivZ, halfSize, repeticiones);

   glDisable(GL_TEXTURE_2D); // Limpieza
}

void igvEscena3D::pintar_quad(int div_x, int div_z, float halfSize, float texRepeat) {
   if (div_x < 1 || div_z < 1) return;

   const float stepX = (2.0f * halfSize) / static_cast<float>(div_x);
   const float stepZ = (2.0f * halfSize) / static_cast<float>(div_z);
   const float stepS = texRepeat / static_cast<float>(div_x);
   const float stepT = texRepeat / static_cast<float>(div_z);

   glBegin(GL_QUADS);
      glNormal3f(0,1,0);
      for (int ix = 0; ix < div_x; ++ix) {
         const float x0 = -halfSize + stepX * ix;
         const float x1 = x0 + stepX;
         const float s0 = stepS * ix;
         const float s1 = s0 + stepS;

         for (int iz = 0; iz < div_z; ++iz) {
            const float z0 = -halfSize + stepZ * iz;
            const float z1 = z0 + stepZ;
            const float t0 = stepT * iz;
            const float t1 = t0 + stepT;

            glTexCoord2f(s0, t0); glVertex3f(x0, 0.0f, z0);
            glTexCoord2f(s1, t0); glVertex3f(x1, 0.0f, z0);
            glTexCoord2f(s1, t1); glVertex3f(x1, 0.0f, z1);
            glTexCoord2f(s0, t1); glVertex3f(x0, 0.0f, z1);
         }
      }
   glEnd();
}

void igvEscena3D::inicializarSuelo() {
   // Generar Textura Ajedrez para el suelo
   generarTexturaAjedrez();

   // Texturas desde archivo
   texturaSuelo[0] = nullptr;
   texturaSuelo[1] = nullptr;
   try {
      texturaSuelo[0] = new igvTextura("../baldosa.png");
   } catch (std::exception& e) {
      std::cout << "Aviso: No se cargo baldosa.png (usando ajedrez)\n";
   }

   try {
      texturaSuelo[1] = new igvTextura("../grass.png");
   } catch (std::exception& e) {
      std::cout << "Aviso: No se cargo grass.png (usando ajedrez)\n";
   }

   // Material 0: Gris Mate (Base)
   materialSuelo[0].set(
      igvColor(0.2, 0.2, 0.2),
      igvColor(0.6, 0.6, 0.6),
      igvColor(0.0, 0.0, 0.0),
      0.0
   );
   // Material 1: Dorado Brillante
   materialSuelo[1].set(
      igvColor(0.25, 0.2, 0.07),
      igvColor(0.75, 0.6, 0.23),
      igvColor(0.63, 0.56, 0.37),
      50.0
   );
   // Material 2: Azul Plástico
   materialSuelo[2].set(
      igvColor(0.0, 0.1, 0.3),
      igvColor(0.0, 0.3, 0.8),
      igvColor(0.5, 0.5, 0.5),
      20.0
   );
}

void igvEscena3D::generarTexturaAjedrez() {
   GLubyte image[64][64][4];

   for (int i = 0; i < 64; i++) {
      for (int j = 0; j < 64; j++) {
         // Cada 8 píxeles cambiamos de color en x e y
         int c = (((i & 0x8) == 0) ^ ((j & 0x8) == 0)) * 255;

         image[i][j][0] = (GLubyte)c;     // R
         image[i][j][1] = (GLubyte)c;     // G
         image[i][j][2] = (GLubyte)c;     // B
         image[i][j][3] = (GLubyte)255;   // Alpha
      }
   }

   glGenTextures(1, &idTexturaAjedrez);
   glBindTexture(GL_TEXTURE_2D, idTexturaAjedrez);

   // Aseguramos alineación por si acaso
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

   glTexImage2D(
       GL_TEXTURE_2D, 0, GL_RGBA,
       64, 64, 0,
       GL_RGBA, GL_UNSIGNED_BYTE, image
   );

   glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

void igvEscena3D::cambiarSombreado() {
   if (objetoGodzilla) {
      objetoGodzilla->cambiarvis();
   }
}

void igvEscena3D::addTapa(float x, float y, float z, float rx, float ry,
                          int type, float normalDir,
                          int numLados,
                          std::vector<float>& verts, std::vector<float>& norms, std::vector<unsigned int>& inds, int& idx) {

    // 1. Vértice Central
    verts.push_back(x); verts.push_back(y); verts.push_back(z);

    // Normal del centro
    if (type == 0) { // Tapa Vertical -> Normal en Y
        norms.push_back(0); norms.push_back(normalDir); norms.push_back(0);
    } else { // Tapa Horizontal -> Normal en Z
        norms.push_back(0); norms.push_back(0); norms.push_back(normalDir);
    }

    // 2. Vértices del Perímetro
    for (int j = 0; j <= numLados; ++j) {
        float theta = (float)j * 2.0f * M_PI / (float)numLados;
        float c = cos(theta);
        float s = sin(theta);

        float px = x, py = y, pz = z;

        if (type == 0) { // Plano XZ (Tapa de cilindro vertical)
            px += rx * c;
            pz += ry * s;
            norms.push_back(0); norms.push_back(normalDir); norms.push_back(0);
        } else { // Plano XY (Tapa de cilindro horizontal)
            px += rx * c;
            py += ry * s;
            norms.push_back(0); norms.push_back(0); norms.push_back(normalDir);
        }

        verts.push_back(px); verts.push_back(py); verts.push_back(pz);
    }

    // 3. Triángulos (Fan)
    for (int j = 0; j < numLados; ++j) {
        if (normalDir > 0) {
            // Sentido antihorario
            inds.push_back(idx);
            inds.push_back(idx + 1 + j);
            inds.push_back(idx + 1 + j + 1);
        } else {
            // Sentido horario
            inds.push_back(idx);
            inds.push_back(idx + 1 + j + 1);
            inds.push_back(idx + 1 + j);
        }
    }

    idx += (numLados + 2);
}

void igvEscena3D::addTuboHorizontal(float x1, float y1, float z1, float rBaseX, float rBaseY,
                                    float x2, float y2, float z2, float rTopX, float rTopY,
                                    int numLados,
                                    std::vector<float>& verts, std::vector<float>& norms, std::vector<unsigned int>& inds, int& idx) {
   for (int ring = 0; ring < 2; ++ring) {
      float curX = (ring == 0) ? x1 : x2;
      float curY = (ring == 0) ? y1 : y2;
      float curZ = (ring == 0) ? z1 : z2;
      float curRX = (ring == 0) ? rBaseX : rTopX;
      float curRY = (ring == 0) ? rBaseY : rTopY;

      for (int j = 0; j <= numLados; ++j) {
         float theta = (float)j * 2.0f * M_PI / (float)numLados;
         float c = cos(theta);
         float s = sin(theta);


         float px = curX + (curRX * c);
         float py = curY + (curRY * s);
         float pz = curZ;

         verts.push_back(px); verts.push_back(py); verts.push_back(pz);

         // Normales radiales en XY
         norms.push_back(c); norms.push_back(s); norms.push_back(0.0f);
      }
   }

   // Conectar anillos con triángulos
   int offsetTop = numLados + 1;
   for (int j = 0; j < numLados; ++j) {
      int curr = idx + j;
      int next = idx + j + 1;
      int topCurr = idx + offsetTop + j;
      int topNext = idx + offsetTop + j + 1;

      inds.push_back(curr); inds.push_back(next); inds.push_back(topCurr);
      inds.push_back(next); inds.push_back(topNext); inds.push_back(topCurr);
   }
   idx += (numLados + 1) * 2;
}

void igvEscena3D::addTuboToMesh(float x1, float y1, float z1, float rBaseX, float rBaseZ,
                                float x2, float y2, float z2, float rTopX, float rTopZ,
                                int numLados,
                                std::vector<float>& verts, std::vector<float>& norms, std::vector<unsigned int>& inds, int& idx) {

    // Generamos dos anillos de vértices: Base y Top
    for (int ring = 0; ring < 2; ++ring) {
        float curY = (ring == 0) ? y1 : y2;
        float curX = (ring == 0) ? x1 : x2;
        float curZ = (ring == 0) ? z1 : z2;
        float curRX = (ring == 0) ? rBaseX : rTopX;
        float curRZ = (ring == 0) ? rBaseZ : rTopZ;

        for (int j = 0; j <= numLados; ++j) {
            float theta = (float)j * 2.0f * M_PI / (float)numLados;
            float c = cos(theta);
            float s = sin(theta);

            // Posición
            float px = curX + (curRX * c);
            float py = curY;
            float pz = curZ + (curRZ * s);

            verts.push_back(px);
            verts.push_back(py);
            verts.push_back(pz);

            // Normal radial en XZ
            norms.push_back(c);
            norms.push_back(0.0f);
            norms.push_back(s);
        }
    }

    // Conectar anillos con triángulos
    int offsetTop = numLados + 1;

    for (int j = 0; j < numLados; ++j) {
        int current = idx + j;
        int next = idx + j + 1;
        int topCurrent = idx + offsetTop + j;
        int topNext = idx + offsetTop + j + 1;

        // Triángulo 1
        inds.push_back(current);
        inds.push_back(next);
        inds.push_back(topCurrent);

        // Triángulo 2
        inds.push_back(next);
        inds.push_back(topNext);
        inds.push_back(topCurrent);
    }

    // Actualizar índice global
    idx += (numLados + 1) * 2;
}

void igvEscena3D::generarMallaGodzilla() {
    std::vector<float> v, n;
    std::vector<unsigned int> i;
    int idx = 0;
    int res = 12;

    // --- 1. CUERPO ---
    addTuboToMesh(0, -0.2f, 0.0f,  0.9f, 0.9f,   0,  0.8f,  0.2f, 0.8f, 0.8f, res, v, n, i, idx);
    addTuboToMesh(0,  0.8f, 0.2f,  0.8f, 0.8f,   0,  1.3f,  0.4f, 0.55f, 0.55f, res, v, n, i, idx);
    addTapa(0, -0.2f, 0.0f, 0.9f, 0.9f, 0, -1.0f, res, v, n, i, idx);
    addTapa(0, 1.3f, 0.4f, 0.55f, 0.55f, 0, 1.0f, res, v, n, i, idx);

    // --- 2. CUELLO ---
    addTuboToMesh(0, 1.3f, 0.4f, 0.55f, 0.55f,   0, 1.6f, 0.5f, 0.45f, 0.45f, res, v, n, i, idx);

    // --- 3. CABEZA ---
    addTapa(0, 1.7f, 0.4f, 0.45f, 0.5f, 1, -1.0f, res, v, n, i, idx);
    // Cráneo
    addTuboHorizontal(0, 1.7f, 0.4f, 0.45f, 0.5f,   0, 1.7f, 0.8f, 0.42f, 0.45f, res, v, n, i, idx);
    // Hocico
    addTuboHorizontal(0, 1.68f, 0.75f, 0.4f, 0.4f,   0, 1.68f, 1.3f, 0.25f, 0.3f, res, v, n, i, idx);
    // Punta del Hocico
    addTapa(0, 1.68f, 1.3f, 0.25f, 0.3f, 1, 1.0f, res, v, n, i, idx);


    // --- 4. COLA---
    addTuboHorizontal(0, -0.5f, -0.8f, 0.7f, 0.7f,   0, -0.8f, -1.8f, 0.5f, 0.5f, res, v, n, i, idx);
    addTuboHorizontal(0, -0.8f, -1.8f, 0.5f, 0.5f,   0, -1.2f, -2.8f, 0.1f, 0.1f, res, v, n, i, idx);
    // Punta de la cola
    addTapa(0, -1.2f, -2.8f, 0.1f, 0.1f, 1, -1.0f, res, v, n, i, idx);


    // --- 5. PATAS ---
    // Muslos
    addTuboToMesh(-0.6f, -0.1f, 0.1f, 0.5f, 0.7f,  -0.7f, -0.9f, 0.2f, 0.3f, 0.4f, res, v, n, i, idx);
    addTapa(-0.6f, -0.1f, 0.1f, 0.5f, 0.7f, 0, 1.0f, res, v, n, i, idx);  // Arriba
    addTapa(-0.7f, -0.9f, 0.2f, 0.3f, 0.4f, 0, -1.0f, res, v, n, i, idx); // Abajo
    addTuboToMesh( 0.6f, -0.1f, 0.1f, 0.5f, 0.7f,   0.7f, -0.9f, 0.2f, 0.3f, 0.4f, res, v, n, i, idx);
    addTapa( 0.6f, -0.1f, 0.1f, 0.5f, 0.7f, 0, 1.0f, res, v, n, i, idx);  // Arriba
    addTapa( 0.7f, -0.9f, 0.2f, 0.3f, 0.4f, 0, -1.0f, res, v, n, i, idx); // Abajo

    // Pies (Tubos)
    addTuboHorizontal(-0.7f, -1.1f, 0.0f, 0.3f, 0.2f, -0.7f, -1.1f, 0.6f, 0.35f, 0.2f, 8, v, n, i, idx);
    addTuboHorizontal( 0.7f, -1.1f, 0.0f, 0.3f, 0.2f,  0.7f, -1.1f, 0.6f, 0.35f, 0.2f, 8, v, n, i, idx);

    // Tapas de los Pies Dedos
    addTapa(-0.7f, -1.1f, 0.6f, 0.35f, 0.2f, 1, 1.0f, 8, v, n, i, idx);
    addTapa( 0.7f, -1.1f, 0.6f, 0.35f, 0.2f, 1, 1.0f, 8, v, n, i, idx);
    // Tapas traseras de los pies (Talones)
    addTapa(-0.7f, -1.1f, 0.0f, 0.3f, 0.2f, 1, -1.0f, 8, v, n, i, idx);
    addTapa( 0.7f, -1.1f, 0.0f, 0.3f, 0.2f, 1, -1.0f, 8, v, n, i, idx);


    // --- 6. BRAZOS (CERRADOS) ---
    // Brazos Tubos
    addTuboHorizontal(-0.6f, 0.9f, 0.4f, 0.2f, 0.2f,  0.6f, 0.7f, 0.9f, 0.15f, 0.15f, 8, v, n, i, idx);
    addTuboHorizontal( 0.6f, 0.9f, 0.4f, 0.2f, 0.2f,   0.6f, 0.7f, 0.9f, 0.15f, 0.15f, 8, v, n, i, idx);

    // Tapas de las Manos
    addTapa(-0.6f, 0.7f, 0.9f, 0.15f, 0.15f, 1, 1.0f, 8, v, n, i, idx);
    addTapa( 0.6f, 0.7f, 0.9f, 0.15f, 0.15f, 1, 1.0f, 8, v, n, i, idx);


    // --- 7. PINCHOS ---
    // Cola
    addTuboToMesh(0, -0.4f, -1.2f, 0.2f, 0.3f,   0, 0.2f, -1.3f, 0.0f, 0.0f, 4, v, n, i, idx);
    addTuboToMesh(0, -0.8f, -2.0f, 0.15f, 0.25f,  0, -0.3f, -2.1f, 0.0f, 0.0f, 4, v, n, i, idx);
    addTuboToMesh(0, -1.0f, -2.5f, 0.1f, 0.2f,   0, -0.7f, -2.6f, 0.0f, 0.0f, 4, v, n, i, idx);
    // Espalda
    addTuboHorizontal(0, 0.2f, -0.5f, 0.2f, 0.25f,   0, 0.3f, -1.1f, 0.0f, 0.0f, 4, v, n, i, idx);
    addTuboHorizontal(0, 0.8f, -0.2f, 0.2f, 0.3f,    0, 0.9f, -0.8f, 0.0f, 0.0f, 4, v, n, i, idx);
    addTuboHorizontal(0, 1.3f, 0.1f, 0.15f, 0.25f,   0, 1.4f, -0.4f, 0.0f, 0.0f, 4, v, n, i, idx);
    addTuboToMesh(0, 1.6f, 0.3f, 0.1f, 0.2f,   0, 1.9f, 0.2f, 0.0f, 0.0f, 4, v, n, i, idx);

    if (objetoGodzilla) delete objetoGodzilla;
    objetoGodzilla = new igvMallaTriangulos(v.size()/3, v.data(), i.size()/3, i.data());
    objetoGodzilla->set_normales(n.data());
}

// Métodos públicos ----------------------------------------

/**
 * Método para pintar los ejes coordenados llamando a funciones de OpenGL
 */
void igvEscena3D::pintar_ejes ()
{  GLfloat rojo[] = { 1,0,0,1.0 };
   GLfloat verde[] = { 0,1,0,1.0 };
   GLfloat azul[] = { 0,0,1,1.0 };

   glBegin(GL_LINES);
   glMaterialfv(GL_FRONT, GL_EMISSION, rojo);
   glVertex3f(1000, 0, 0);
   glVertex3f(-1000, 0, 0);

   glMaterialfv(GL_FRONT, GL_EMISSION, verde);
   glVertex3f(0, 1000, 0);
   glVertex3f(0, -1000, 0);

   glMaterialfv(GL_FRONT, GL_EMISSION, azul);
   glVertex3f(0, 0, 1000);
   glVertex3f(0, 0, -1000);
   glEnd();
}

/**
 * Método con las llamadas OpenGL para visualizar la escena
 * @param escena Identificador del tipo de escena a dibujar
 * @pre Se asume que el valor del parámetro es correcto
 */
void igvEscena3D::visualizar ( int escena )
{
   // Habilitar iluminación y configurar componente ambiental global
   glEnable(GL_LIGHTING);
   // Usamos la componente ambiental definida en luzAmbiental como ambient global
   igvColor &amb = luzAmbiental.getAmbiental();
   GLfloat globalAmb[] = { (GLfloat)amb[0], (GLfloat)amb[1], (GLfloat)amb[2], 1.0f };
   glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmb);

   // Aplicar luces (la propia clase decide si están activas)
   luzPuntual.aplicar();
   luzDireccional.aplicar();
   luzCono.aplicar();

   glPushMatrix (); // guarda la matriz de modelado
   if (verMalla) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Modo Alambre
      glDisable(GL_LIGHTING); // Desactivamos luz para ver las líneas verdes/blancas mejor
      glColor3f(0.0f, 1.0f, 0.0f); // Forzamos color verde matrix para las líneas
   } else {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Modo Relleno (Normal)
      glEnable(GL_LIGHTING); // Luz activada
   }
   if ( ejes ) {  pintar_ejes (); }
   if ( escena == EscenaA ) {  renderEscenaA (); }
   else { if ( escena == EscenaB ) {  renderEscenaB (); } else { if ( escena == EscenaC ) {  renderEscenaC (); } } }
   glPopMatrix (); // restaura la matriz de modelado
}

/**
 * Pinta la escena A llamando a las funciones de OpenGL
 */
void igvEscena3D::renderEscenaA ()
{

   glPushMatrix();
      glTranslated(0.0f,-3.0f,0.0f);
      pintarSuelo();
   glPopMatrix();

   glDisable(GL_TEXTURE_2D);

   // B) Reiniciar Materiales: Volvemos a valores neutros para que los colores
   // definidos en setColor (glMaterial con GL_EMISSION) se vean puros y no mezclados con el suelo.
   GLfloat defaultAmbient[] = {0.2f, 0.2f, 0.2f, 1.0f};
   GLfloat defaultDiffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
   GLfloat defaultSpecular[] = {0.0f, 0.0f, 0.0f, 1.0f};
   GLfloat defaultShininess[] = {0.0f};

   glMaterialfv(GL_FRONT, GL_AMBIENT, defaultAmbient);
   glMaterialfv(GL_FRONT, GL_DIFFUSE, defaultDiffuse);
   glMaterialfv(GL_FRONT, GL_SPECULAR, defaultSpecular);
   glMaterialfv(GL_FRONT, GL_SHININESS, defaultShininess);

   const GLfloat c_seleccion[] = {1.0f, 0.3f, 0.0f, 1.0f}; // Naranja para selección

   auto setColor = [&](int idParte, const GLfloat* color) {
      if (seleccionando) {
         glColor3ub(idParte, 0,0);
      } else {
         if (idParte == parteActiva) {
            glMaterialfv(GL_FRONT, GL_EMISSION, c_seleccion);
         } else {
            glMaterialfv(GL_FRONT, GL_EMISSION, color);
         }

      }
   };

   auto applyFor = [&](int idx){
      if (mode == RST) {
         const Transform& t = objT[idx];
         glTranslatef(t.tx, t.ty, t.tz);
         glScalef(t.s, t.s, t.s);
         glRotatef(t.rx, 1, 0, 0);
         glRotatef(t.ry, 0, 1, 0);
         glRotatef(t.rz, 0, 0, 1);
      } else {
         for (auto it = objOps[idx].rbegin(); it != objOps[idx].rend(); ++it) {
            const auto& op = *it;
            switch (op.kind) {
            case Op::T:  glTranslatef(op.v1, op.v2, op.v3); break;
            case Op::RX: glRotatef(op.v1, 1, 0, 0); break;
            case Op::RY: glRotatef(op.v1, 0, 1, 0); break;
            case Op::RZ: glRotatef(op.v1, 0, 0, 1); break;
            case Op::S:  glScalef(op.v1, op.v1, op.v1); break;
            }
         }
      }
   };

   // --- PARTE 1: GODZILLA (OBJETO MALLA) ---
   // A la izquierda. Hecho de triángulos.
   glPushMatrix();
      glTranslatef(-2.5f, 0.0f, 0.0f);
      glRotatef(25, 0, 1, 0); // Girarlo para verlo de perfil
      GLfloat colorGodzilla[] = {0.2f, 0.6f, 0.3f, 1.0f}; // Verde oscuro
      applyFor(1); // Aplicar transformaciones interactivas


      setColor(GODZILLA, colorGodzilla);

      if (objetoGodzilla) objetoGodzilla->visualizar();
   glPopMatrix();

   // ===========================================================
   // ROBOT
   // ===========================================================
   {
      glPushMatrix();
         glTranslatef(2.5f, 0.0f, 0.0f); // desplazar a la izquierda
         applyFor(0);

         const GLfloat c_body[]  = {0.15f, 0.35f, 0.65f, 1.0f};
         const GLfloat c_joint[] = {1.0f, 0.0f, 0.0f, 1.0f};
         const GLfloat c_eye[]   = {1.0f, 0.8f, 0.0f, 1.0f};

         // --- TORSO ---
         setColor(ROBOT_COMPLETO, c_body);
         glPushMatrix();
             glScalef(1.0f, 1.5f, 0.6f);
             glutSolidCube(1.0); // <--- PRIMITIVA OPENGL
         glPopMatrix();

         // --- CABEZA (Articulada) ---
         glPushMatrix();
             glTranslatef(0.0f, 0.75f, 0.0f); // Pivote cuello
             glRotatef(robotState.cabeza, 0, 1, 0); // Variable interactiva

             // Cuello
             setColor(CABEZA, c_joint);
             glPushMatrix(); glTranslatef(0.0f, 0.2f, 0.0f); glScalef(0.4f, 0.4f, 0.4f); glutSolidSphere(0.5, 10, 10); glPopMatrix();

             // Cabeza
             setColor(CABEZA, c_body);
             glPushMatrix(); glTranslatef(0.0f, 0.6f, 0.0f); glScalef(0.6f, 0.6f, 0.6f); glutSolidCube(1.0); glPopMatrix();

             // Ojos
             setColor(CABEZA, c_eye);
             glPushMatrix(); glTranslatef(-0.15f, 0.7f, 0.3f); glutSolidSphere(0.1, 8, 8); glPopMatrix();
             glPushMatrix(); glTranslatef( 0.15f, 0.7f, 0.3f); glutSolidSphere(0.1, 8, 8); glPopMatrix();
         glPopMatrix();

         // --- BRAZO IZQ (Articulado) ---
         glPushMatrix();
            // 1. Transformacion del Hombro
             glTranslatef(-0.5f, 0.5f, 0.0f); // Pivote hombro
             glRotatef(robotState.brazoIzqLat, 0, 0, 1); // Rotacion lateral
             glRotatef(robotState.brazoIzq, 1, 0, 0);

             // 2. Hombro
             setColor(BRAZO_IZQ, c_joint);
             glutSolidSphere(0.3, 10, 10);

             // 3. Dibujar Brazo Superior
             setColor(BRAZO_IZQ, c_body);
             glPushMatrix();
                glTranslatef(-0.2f, -0.4f, 0.0f);
                glScalef(0.3f, 0.8f, 0.3f);
                glutSolidCube(1.0);
             glPopMatrix();

             // Nivel 2: Codo y Antebrazo
             glPushMatrix();
               // Trasladar el punto de pivote del codo (relativo al hombro)
               glTranslatef(-0.2f, -0.85f, 0.0f); // Entre brazo y antebrazo
               // 5. Rotacion del codo (Independiente al hombro)
               glRotatef(robotState.codoIzq, 1, 0, 0);
               // 6. Dibujar Codo
               setColor(ANTEBRAZO_IZQ, c_joint);
               glutSolidSphere(0.22, 10, 10);
               // 7. Antebrazo
               setColor(ANTEBRAZO_IZQ, c_body);
               glPushMatrix();
                  glTranslatef(0.0f, -0.35f, 0.0f);
                  glScalef(0.25f, 0.6f, 0.25f);
                  glutSolidCube(1.0);
               glPopMatrix();
             glPopMatrix(); // Fin Nivel 2: Codo y Antebrazo
         glPopMatrix(); // Fin Brazo Izq

         // --- BRAZO DER (Articulado) ---
         glPushMatrix(); // [INICIO HOMBRO DER]
            glTranslatef(0.5f, 0.5f, 0.0f);
            glRotatef(-robotState.brazoDerLat, 0, 0, 1);
            glRotatef(robotState.brazoDer, 1, 0, 0);

            setColor(BRAZO_DER, c_joint);
            glutSolidSphere(0.3, 10, 10);

            setColor(BRAZO_DER, c_body);
            glPushMatrix();
               glTranslatef(0.2f, -0.4f, 0.0f);
               glScalef(0.3f, 0.8f, 0.3f);
               glutSolidCube(1.0);
            glPopMatrix();

            // --- NIVEL 2 ---
            glPushMatrix(); // [INICIO CODO DER]
               glTranslatef(0.2f, -0.85f, 0.0f); // Ir al codo
               glRotatef(robotState.codoDer, 1, 0, 0); // Rotar codo

               setColor(ANTEBRAZO_DER, c_joint);
               glutSolidSphere(0.22, 10, 10);

               setColor(ANTEBRAZO_DER, c_body);
               glPushMatrix();
                  glTranslatef(0.0f, -0.35f, 0.0f);
                  glScalef(0.25f, 0.6f, 0.25f);
                  glutSolidCube(1.0);
               glPopMatrix();
            glPopMatrix(); // [FIN CODO DER]

         glPopMatrix(); // [FIN HOMBRO DER]

         glPushMatrix(); // [INICIO MATRIZ CADERA IZQ]
             // 1. Traslación y Rotación de la CADERA (Muslo)
             glTranslatef(-0.3f, -0.75f, 0.0f);
             glRotatef(robotState.piernaIzq, 1, 0, 0);

             // 2. Dibujar Articulación Cadera (Esfera Roja)
             setColor(PIERNA_IZQ, c_joint);
             glutSolidSphere(0.3, 10, 10);

             // 3. Dibujar Muslo (Azul)
             setColor(PIERNA_IZQ, c_body);
             glPushMatrix();
                 glTranslatef(0.0f, -0.6f, 0.0f);
                 glScalef(0.35f, 1.0f, 0.35f);
                 glutSolidCube(1.0);
             glPopMatrix();

             // --- NIVEL 2: RODILLA Y PANTORRILLA ---
             glPushMatrix(); // [INICIO MATRIZ RODILLA IZQ]
                 // 4. Moverse al pivote de la rodilla
                 // (Bajamos hasta el final del muslo: aprox -1.1 desde la cadera)
                 glTranslatef(0.0f, -1.1f, 0.0f);

                 // 5. ROTACIÓN DE LA RODILLA (Independiente)
                 glRotatef(robotState.pantorrillaIzq, 1, 0, 0);

                 // 6. Dibujar Esfera Rodilla (Roja)
                 setColor(PANTORRILLA_IZQ, c_joint); // ID 8
                 glutSolidSphere(0.25, 10, 10);

                 // 7. Dibujar Pantorrilla/Gemelo (Azul)
                 // Bajamos desde la rodilla (-0.5)
                 setColor(PANTORRILLA_IZQ, c_body);
                 glPushMatrix();
                     glTranslatef(0.0f, -0.5f, 0.0f);
                     glScalef(0.3f, 0.9f, 0.3f);
                     glutSolidCube(1.0);
                 glPopMatrix();

                 // 8. Dibujar Pie (Hijo de la pantorrilla)
                 // Bajamos más desde el centro de la pantorrilla
                 setColor(PANTORRILLA_IZQ, c_eye);
                 glPushMatrix();
                     glTranslatef(0.0f, -1.0f, 0.2f); // Ajuste posición pie
                     glScalef(0.35f, 0.2f, 0.6f);
                     glutSolidCube(1.0);
                 glPopMatrix();

             glPopMatrix(); // [FIN MATRIZ RODILLA IZQ]

         glPopMatrix(); // [FIN MATRIZ CADERA IZQ]


         // --- PIERNA DERECHA ---
         glPushMatrix(); // [INICIO CADERA DER]
             glTranslatef(0.3f, -0.75f, 0.0f);
             glRotatef(robotState.piernaDer, 1, 0, 0);

             setColor(PIERNA_DER, c_joint);
             glutSolidSphere(0.3, 10, 10);

             setColor(PIERNA_DER, c_body);
             glPushMatrix();
                 glTranslatef(0.0f, -0.6f, 0.0f);
                 glScalef(0.35f, 1.0f, 0.35f);
                 glutSolidCube(1.0);
             glPopMatrix();

             // --- NIVEL 2 ---
             glPushMatrix(); // [INICIO RODILLA DER]
                 glTranslatef(0.0f, -1.1f, 0.0f); // Ir a rodilla
                 glRotatef(robotState.pantorrillaDer, 1, 0, 0); // Rotar rodilla

                 setColor(PANTORRILLA_DER, c_joint); // ID 9
                 glutSolidSphere(0.25, 10, 10);

                 setColor(PANTORRILLA_DER, c_body);
                 glPushMatrix();
                     glTranslatef(0.0f, -0.5f, 0.0f);
                     glScalef(0.3f, 0.9f, 0.3f);
                     glutSolidCube(1.0);
                 glPopMatrix();

                 setColor(PANTORRILLA_DER, c_eye);
                 glPushMatrix();
                     glTranslatef(0.0f, -1.0f, 0.2f);
                     glScalef(0.35f, 0.2f, 0.6f);
                     glutSolidCube(1.0);
                 glPopMatrix();
             glPopMatrix(); // [FIN RODILLA DER]

         glPopMatrix(); // [FIN CADERA DER]

      glPopMatrix();
   }
}

void igvEscena3D::pick(int x, int y) {
   // 1. Desactivar luces y texturas para que el color se vea bien
   glDisable(GL_LIGHTING);
   glDisable(GL_DITHER);
   // 2. Dibujar la escena en modo selección
   seleccionando = true;
   // Limpiamos buffer para dibujar
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   renderEscenaA();

   // 3. Leer el píxel bajo el cursor
   GLint viewport[4];
   glGetIntegerv(GL_VIEWPORT, viewport);
   GLubyte pixel[3];
   glReadPixels(x, viewport[3] - y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
   // 4. Interpretar el color leído como ID de parte
   int idParte = (int)pixel[0];

   // 5. Actualizar la parte activa
   parteActiva = idParte;

   if (parteActiva == GODZILLA) {
      objetoSeleccionado = 2;
   } else if (parteActiva == ROBOT_COMPLETO) {
      objetoSeleccionado = 1;
   } else if (parteActiva > 0 && parteActiva < 100) {
      objetoSeleccionado = 1;
   }
   // 6. Reactivar luces y texturas
   glEnable(GL_LIGHTING);
   glEnable(GL_DITHER);
   seleccionando = false;
}

void igvEscena3D::arrastrar(int x, int y) {
   // Calcular cuanto se ha movido el raton
   int dx = x - lastMouseX;
   int dy = y - lastMouseY;

   // Si no se selecciona nada o es el fondo, no hacer nada
   if (parteActiva == 100 || parteActiva == 255) return;

   // Sensibilidad para no moverlo como loco la parte
   float sensibilidad = 1.0f;
   // Mover la articulación según el movimiento del ratón
   if (dx != 0 || dy != 0) {
      moverArticulacion((float)dx * sensibilidad, (float)dy * sensibilidad);
   }
   // Actualizar la última posición del ratón
   lastMouseX = x;
   lastMouseY = y;
}


void igvEscena3D::moverArticulacion(float dx, float dy) {
   switch (parteActiva) {
      case CABEZA:
         robotState.cabeza += dx;
         if (robotState.cabeza > 45.0f) robotState.cabeza = 45.0f;
         if (robotState.cabeza < -45.0f) robotState.cabeza = -45.0f;
         break;
      case BRAZO_IZQ:
         robotState.brazoIzq -= dy;
         robotState.brazoIzqLat += dx;
         if (robotState.brazoIzq > 90.0f) robotState.brazoIzq = 90.0f;
         if (robotState.brazoIzq < -90.0f) robotState.brazoIzq = -90.0f;
         if (robotState.brazoIzqLat > 45.0f) robotState.brazoIzqLat = 45.0f;
         if (robotState.brazoIzqLat < -45.0f) robotState.brazoIzqLat = -45.0f;
         break;
      case BRAZO_DER:
         robotState.brazoDer += dy;
         robotState.brazoDerLat += dx;
         if (robotState.brazoDer > 90.0f) robotState.brazoDer = 90.0f;
         if (robotState.brazoDer < -90.0f) robotState.brazoDer = -90.0f;
         if (robotState.brazoDerLat > 45.0f) robotState.brazoDerLat = 45.0f;
         if (robotState.brazoDerLat < -45.0f) robotState.brazoDerLat = -45.0f;
         break;
      case PIERNA_IZQ:
         robotState.piernaIzq += dy;
         if (robotState.piernaIzq > 45.0f) robotState.piernaIzq = 45.0f;
         if (robotState.piernaIzq < -45.0f) robotState.piernaIzq = -45.0f;
         break;
      case PIERNA_DER:
         robotState.piernaDer += dy;
         if (robotState.piernaDer > 45.0f) robotState.piernaDer = 45.0f;
         if (robotState.piernaDer < -45.0f) robotState.piernaDer = -45.0f;
         break;
      case ANTEBRAZO_IZQ:
         robotState.codoIzq += dy;
         if (robotState.codoIzq > 0.0f) robotState.codoIzq = 0.0f;
         if (robotState.codoIzq < -90.0f) robotState.codoIzq = -90.0f;
         break;
      case ANTEBRAZO_DER:
         robotState.codoDer += dy;
         if (robotState.codoDer > 0.0f) robotState.codoDer = 0.0f;
         if (robotState.codoDer < -90.0f) robotState.codoDer = -90.0f;
         break;
      case PANTORRILLA_IZQ:
         robotState.pantorrillaIzq += dy;
         if (robotState.pantorrillaIzq < 0.0f) robotState.pantorrillaIzq = 0.0f;
         if (robotState.pantorrillaIzq > 90.0f) robotState.pantorrillaIzq = 90.0f;
         break;
      case PANTORRILLA_DER:
         robotState.pantorrillaDer += dy;
         if (robotState.pantorrillaDer < 0.0f) robotState.pantorrillaDer = 0.0f;
         if (robotState.pantorrillaDer > 90.0f) robotState.pantorrillaDer = 90.0f;
         break;
      default: break;
   }
}


/**
 * Pinta la escena B llamando a las funciones de OpenGL
 */
void igvEscena3D::renderEscenaB ()
{  GLfloat color_pieza[] = { 0, 0, 0.5 };

   // TODO: Practica 2a. Parte B.
   glMaterialfv ( GL_FRONT, GL_EMISSION, color_pieza );

   glPushMatrix ();
   glutSolidCube ( 1 );
   glPopMatrix ();
}

/**
 * Pinta la escena C llamando a las funciones de OpenGL
 */
void igvEscena3D::renderEscenaC ()
{  GLfloat color_pieza[] = { 0.5, 0, 0 };

   // TODO: Practica 2a. Parte C.
   glMaterialfv ( GL_FRONT, GL_EMISSION, color_pieza );

   glPushMatrix ();
   glutSolidCube ( 1 );
   glPopMatrix ();
}

/**
 * Método para consultar si hay que dibujar los ejes o no
 * @retval true Si hay que dibujar los ejes
 * @retval false Si no hay que dibujar los ejes
 */
bool igvEscena3D::get_ejes ()
{  return ejes;
}

/**
 * Método para activar o desactivar el dibujado de los ejes
 * @param _ejes Indica si hay que dibujar los ejes (true) o no (false)
 * @post El estado del objeto cambia en lo que respecta al dibujado de ejes,
 *       de acuerdo al valor pasado como parámetro
 */
void igvEscena3D::set_ejes ( bool _ejes )
{  ejes = _ejes;
}

void igvEscena3D::applyTranslation(float dx, float dy, float dz) {
   int idx = objetoSeleccionado - 1;
   if (idx < 0 || idx > 2) return;
   if (mode == RST) {
      objT[idx].tx += dx;
      objT[idx].ty += dy;
      objT[idx].tz += dz;
   } else {
      objOps[idx].push_back({Op::T, dx, dy, dz});
   }
}

void igvEscena3D::applyRotation(float ax, float ay, float az) {
   int idx = objetoSeleccionado - 1;
   if (idx < 0 || idx > 2) return;
   if (mode == RST) {
      objT[idx].rx += ax;
      objT[idx].ry += ay;
      objT[idx].rz += az;
   } else {
      if (ax != 0.0f) objOps[idx].push_back({Op::RX, ax, 0, 0});
      if (ay != 0.0f) objOps[idx].push_back({Op::RY, ay, 0, 0});
      if (az != 0.0f) objOps[idx].push_back({Op::RZ, az, 0, 0});
   }
}

void igvEscena3D::applyScale(float factor) {
   int idx = objetoSeleccionado - 1;
   if (idx < 0 || idx > 2) return;
   if (mode == RST) {
      objT[idx].s *= factor;
      if (objT[idx].s < kMinScale) objT[idx].s = kMinScale;
      if (objT[idx].s > kMaxScale) objT[idx].s = kMaxScale;
   } else {
      objOps[idx].push_back({Op::S, factor, 0, 0});
   }
}

void igvEscena3D::toggleMode() {
   mode = (mode == RST) ? SEQ : RST;
}

void igvEscena3D::startAnimacion() {
   robotState = robotStateInitial;
   // No sobrescribimos la baseline (se tomó en el constructor).
   // Solo (re)iniciamos el tiempo de animación y activamos la animación.
   animTime = 0.0f;
   animacionActiva = true;
}

void igvEscena3D::stopAnimacion() {
   // Restaurar pose inicial guardada y desactivar animación
   robotState = robotStateInitial;
   animacionActiva = false;
   animTime = 0.0f;
}

void igvEscena3D::animarRobot() {
   if (!animacionActiva) return; // nada que hacer si no está activa

   // --- tiempo de animación ---
   float &t = animTime;

   // --- AJUSTE DE VELOCIDAD ---
   const float velocidad = 0.15f;

   // Avanzar el tiempo
   t += velocidad;

   // Amplitud del movimiento
   float ampBrazos = 45.0f; // Más amplio para que se note
   float ampPiernas = 45.0f;

   // --- CICLO DE MARCHA (WALK CYCLE) ---
   // Aplicar animación relativa a la pose inicial guardada
   robotState.brazoIzq = robotStateInitial.brazoIzq + ampBrazos * sin(t);
   robotState.brazoDer = robotStateInitial.brazoDer + ampBrazos * sin(t + M_PI); // Desfasado 180 grados
   robotState.piernaIzq = robotStateInitial.piernaIzq + ampPiernas * sin(t + M_PI);
   robotState.piernaDer = robotStateInitial.piernaDer + ampPiernas * sin(t);

   // --- DETALLES PARA FLUIDEZ (ARTICULACIONES SECUNDARIAS) ---

   // CODOS: relativos a la pose inicial
   robotState.codoIzq = robotStateInitial.codoIzq + (-30.0f * fabs(sin(t + 0.5f)));
   robotState.codoDer = robotStateInitial.codoDer + (-30.0f * fabs(sin(t + M_PI + 0.5f)));

   // RODILLAS: relativas a la pose inicial
   robotState.pantorrillaIzq = robotStateInitial.pantorrillaIzq + 40.0f * std::max(0.0f, (float)sin(t + M_PI));
   robotState.pantorrillaDer = robotStateInitial.pantorrillaDer + 40.0f * std::max(0.0f, (float)sin(t));

   // CABEZA: relativo a la pose inicial
   robotState.cabeza = robotStateInitial.cabeza + 5.0f * sin(t * 0.5f);
}

// -------------------------
// Métodos públicos para control de luces (menu/teclado)
// -------------------------
void igvEscena3D::toggleLight(LuzSeleccionada l) {
    switch (l) {
        case LUZ_DIRECCIONAL:
            if (luzDireccional.esta_encendida()) luzDireccional.apagar(); else luzDireccional.encender();
            break;
        case LUZ_PUNTUAL:
            if (luzPuntual.esta_encendida()) luzPuntual.apagar(); else luzPuntual.encender();
            break;
        case LUZ_CONO:
            if (luzCono.esta_encendida()) luzCono.apagar(); else luzCono.encender();
            break;
        default: break;
    }
}

void igvEscena3D::selectLight(LuzSeleccionada l) {
    luzSeleccionada = l;
}

void igvEscena3D::enterMoveLightMode(bool enter) {
    modoMoverLuz = enter;
}

void igvEscena3D::moveSelectedLight(float dx, float dy, float dz) {
    if (!modoMoverLuz) return;
    switch (luzSeleccionada) {
        case LUZ_PUNTUAL:
            luzPuntual.mover(dx, dy, dz);
            break;
        case LUZ_CONO:
            luzCono.mover(dx, dy, dz);
            break;
        case LUZ_DIRECCIONAL:
            // Para direccional, mover la dirección vector (posicion usado como dir)
            luzDireccional.mover(dx, dy, dz);
            break;
        default: break;
    }
}


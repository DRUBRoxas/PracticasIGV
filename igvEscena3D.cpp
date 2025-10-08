#include <cstdlib>
#include <stdio.h>

#include "igvEscena3D.h"

// Métodos constructores -----------------------------------

// Constructor: estado inicial de transformaciones
igvEscena3D::igvEscena3D() {
   for (auto & i : objT) {
      i = {0.0f, 0.0f, 0.0f,
                 0.0f, 0.0f, 0.0f,
                 1.0f};
   }
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
{  // borra la ventana y el Z-buffer
   glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   //Luces
   GLfloat light0[] = { 10, 8, 9, 1 }; // point light source
   glLightfv ( GL_LIGHT0, GL_POSITION, light0 );
   glEnable ( GL_LIGHT0 );

   glPushMatrix (); // guarda la matriz de modelado

   // se pintan los ejes
   if ( ejes )
   {  pintar_ejes ();
   }

   // Escena seleccionada a través del menú (clic botón derecho)
   if ( escena == EscenaA )
   {  renderEscenaA ();
   }
   else
   {  if ( escena == EscenaB )
      {  renderEscenaB ();
      }
      else
      {  if ( escena == EscenaC )
         {  renderEscenaC ();
         }
      }
   }

   glPopMatrix (); // restaura la matriz de modelado
   glutSwapBuffers (); // se utiliza, en vez de glFlush(), para evitar el parpadeo
}

/**
 * Pinta la escena A llamando a las funciones de OpenGL
 */
void igvEscena3D::renderEscenaA ()
{
   auto applyFor = [&](int idx){
      if (mode == RST) {
         const Transform& t = objT[idx];
         // Call order T -> S -> R (effect R -> S -> T)
         glTranslatef(t.tx, t.ty, t.tz);
         glScalef(t.s, t.s, t.s);
         glRotatef(t.rx, 1, 0, 0);
         glRotatef(t.ry, 0, 1, 0);
         glRotatef(t.rz, 0, 0, 1);
      } else { // SEQ: world-space composition (about origin)
         // Apply operations in reverse press order to emulate left-multiplication
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
   // Objeto 1: Cubo
   {
      const GLfloat color_pieza[] = {0,0.25f,0,1.0f};
      glMaterialfv(GL_FRONT, GL_EMISSION, color_pieza);
      glPushMatrix();
      applyFor(0);
      glScalef(1.2f, 0.6f, 2.6f);
      glutSolidCube(1.0);
      glPopMatrix();
   }

   // Objeto 2: Esfera
   {
      const GLfloat color_pieza2[] = {0.25f,0,0.25f,1.0f};
      glMaterialfv(GL_FRONT, GL_EMISSION, color_pieza2);
      glPushMatrix();
      applyFor(1);
      glScaled(1.2f, 1.2f, 1.2f);
      glutSolidSphere(0.5, 20, 20);
      glPopMatrix();
   }

   // Objeto 3: Cono
   {
      const GLfloat color_pieza3[] = {0.25f,0.25f,0,1.0f};
      glMaterialfv(GL_FRONT, GL_EMISSION, color_pieza3);
      glPushMatrix();
      applyFor(2);
      glRotatef(-90, 1, 0, 0);
      glutSolidCone(0.5, 1.0, 20, 20);
      glPopMatrix();
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

// Mutadores: en RST acumulan; en SEQ apilan operaciones en orden de pulsación
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



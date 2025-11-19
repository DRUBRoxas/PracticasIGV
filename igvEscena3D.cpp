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
{
   //Luces
   GLfloat light0[] = { 10, 8, 9, 1 }; // point light source
   glLightfv ( GL_LIGHT0, GL_POSITION, light0 );
   glEnable ( GL_LIGHT0 );

   glPushMatrix (); // guarda la matriz de modelado
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

   // ===========================================================
   // ROBOT
   // ===========================================================
   {
      glPushMatrix();
      glTranslatef(-2.5f, 0.0f, 0.0f); // desplazar a la izquierda
      applyFor(0);

      const GLfloat c_body[]  = {0.15f, 0.35f, 0.65f, 1.0f};
      const GLfloat c_joint[] = {0.7f, 0.7f, 0.7f, 1.0f};
      const GLfloat c_eye[]   = {1.0f, 0.7f, 0.2f, 1.0f};

      // Cuerpo
      glMaterialfv(GL_FRONT, GL_EMISSION, c_body);
      glPushMatrix();
         glScalef(0.9f, 1.2f, 0.5f);
         glutSolidCube(1.0);
      glPopMatrix();

      // Cabeza
      glPushMatrix();
         glTranslatef(0.0f, 1.0f, 0.0f);
         glScalef(0.6f, 0.6f, 0.6f);
         glutSolidCube(1.0);
      glPopMatrix();

      // Ojos
      glMaterialfv(GL_FRONT, GL_EMISSION, c_eye);
      glPushMatrix(); glTranslatef(-0.18f, 1.05f, 0.31f); glutSolidSphere(0.06, 14, 14); glPopMatrix();
      glPushMatrix(); glTranslatef( 0.18f, 1.05f, 0.31f); glutSolidSphere(0.06, 14, 14); glPopMatrix();

      // Brazos
      glMaterialfv(GL_FRONT, GL_EMISSION, c_body);
      glPushMatrix(); glTranslatef(-0.95f, 0.45f, 0.0f); glScalef(0.5f, 0.2f, 0.2f); glutSolidCube(1.0); glPopMatrix();
      glPushMatrix(); glTranslatef( 0.95f, 0.45f, 0.0f); glScalef(0.5f, 0.2f, 0.2f); glutSolidCube(1.0); glPopMatrix();

      // Caderas y piernas
      glMaterialfv(GL_FRONT, GL_EMISSION, c_joint);
      glPushMatrix(); glTranslatef(0.0f, 0.0f, 0.0f); glutSolidSphere(0.12, 16, 16); glPopMatrix();

      glMaterialfv(GL_FRONT, GL_EMISSION, c_body);
      glPushMatrix(); glTranslatef(-0.25f, -0.75f, 0.0f); glScalef(0.25f, 0.8f, 0.25f); glutSolidCube(1.0); glPopMatrix();
      glPushMatrix(); glTranslatef( 0.25f, -0.75f, 0.0f); glScalef(0.25f, 0.8f, 0.25f); glutSolidCube(1.0); glPopMatrix();

      glPopMatrix();
   }

   // ===========================================================
   // COCHE
   // ===========================================================
   {
      glPushMatrix();
      glTranslatef(0.0f, -0.1f, 0.0f); // un poco hacia abajo
      applyFor(1);

      const GLfloat c_chassis[] = {0.7f, 0.05f, 0.05f, 1.0f};
      const GLfloat c_cabin[]   = {0.25f, 0.25f, 0.35f, 1.0f};
      const GLfloat c_light[]   = {0.9f, 0.8f, 0.2f, 1.0f};

      // Chasis
      glMaterialfv(GL_FRONT, GL_EMISSION, c_chassis);
      glPushMatrix();
         glScalef(1.8f, 0.3f, 1.0f);
         glutSolidCube(1.0);
      glPopMatrix();

      // Cabina
      glMaterialfv(GL_FRONT, GL_EMISSION, c_cabin);
      glPushMatrix();
         glTranslatef(0.1f, 0.25f, 0.0f);
         glScalef(0.9f, 0.4f, 0.9f);
         glutSolidCube(1.0);
      glPopMatrix();

      // Faros
      glMaterialfv(GL_FRONT, GL_EMISSION, c_light);
      glPushMatrix(); glTranslatef( 0.95f, 0.05f,  0.3f); glutSolidSphere(0.06, 14, 14); glPopMatrix();
      glPushMatrix(); glTranslatef( 0.95f, 0.05f, -0.3f); glutSolidSphere(0.06, 14, 14); glPopMatrix();
      glPopMatrix();
   }

   // ===========================================================
   // CASA
   // ===========================================================
   {
      glPushMatrix();
      glTranslatef(2.5f, 0.0f, 0.0f); // desplazar a la derecha
      applyFor(2);

      const GLfloat c_wall[]   = {0.45f, 0.25f, 0.15f, 1.0f};  // paredes marrón oscuro
      const GLfloat c_roof[]   = {0.35f, 0.05f, 0.05f, 1.0f};  // techo granate
      const GLfloat c_door[]   = {0.25f, 0.15f, 0.05f, 1.0f};  // puerta madera oscura
      const GLfloat c_window[] = {0.05f, 0.25f, 0.55f, 1.0f};  // ventanas azul profundo
      const GLfloat c_chim[]   = {0.2f, 0.2f, 0.2f, 1.0f};     // chimenea gris oscuro

      // Base de la casa
      glMaterialfv(GL_FRONT, GL_EMISSION, c_wall);
      glPushMatrix();
      glScalef(1.6f, 1.0f, 1.2f);
      glutSolidCube(1.0);
      glPopMatrix();

      // Techo
      glMaterialfv(GL_FRONT, GL_EMISSION, c_roof);
      glPushMatrix();
      glTranslatef(0.0f, 0.75f, 0.0f);
      glRotatef(-90.0f, 1, 0, 0);
      glutSolidCone(0.95f, 0.9f, 24, 16);
      glPopMatrix();

      // Chimenea
      glMaterialfv(GL_FRONT, GL_EMISSION, c_chim);
      glPushMatrix();
      glTranslatef(-0.35f, 1.0f, 0.1f);
      glScalef(0.25f, 0.45f, 0.25f);
      glutSolidCube(1.0);
      glPopMatrix();

      // Puerta
      glMaterialfv(GL_FRONT, GL_EMISSION, c_door);
      glPushMatrix();
      glTranslatef(0.0f, -0.2f, 0.62f);
      glScalef(0.35f, 0.45f, 0.08f);
      glutSolidCube(1.0);
      glPopMatrix();

      // Ventanas
      glMaterialfv(GL_FRONT, GL_EMISSION, c_window);
      glPushMatrix();
      glTranslatef(-0.45f, 0.15f, 0.62f);
      glScalef(0.3f, 0.25f, 0.06f);
      glutSolidCube(1.0);
      glPopMatrix();
      glPushMatrix();
      glTranslatef( 0.45f, 0.15f, 0.62f);
      glScalef(0.3f, 0.25f, 0.06f);
      glutSolidCube(1.0);
      glPopMatrix();

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

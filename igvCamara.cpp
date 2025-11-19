#include <math.h>
#include <cmath>

#include "igvCamara.h"

// M�todos constructores

/**
 * Constructor parametrizado
 * @param _tipo Tipo de c�mara (IGV_PARALELA, IGV_FRUSTUM o IGV_PERSPECTIVA)
 * @param _P0 Posici�n de la c�mara (punto de visi�n)
 * @param _r Punto al que mira la c�mara (punto de referencia)
 * @param _V Vector que indica la vertical
 * @pre Se asume que todos los par�metros tienen valores v�lidos
 * @post Los atributos de la nueva c�mara ser�n iguales a los par�metros que se
 *       le pasan
 */
igvCamara::igvCamara ( tipoCamara _tipo, igvPunto3D _P0, igvPunto3D _r
   , igvPunto3D _V ): P0 ( _P0 ), r ( _r ), V ( _V )
                      , tipo ( _tipo )
{ }

// M�todos p�blicos
/**
 * Define la posici�n de la c�mara
 * @param _P0 Posici�n de la c�mara (punto de visi�n)
 * @param _r Punto al que mira la c�mara (punto de referencia)
 * @param _V Vector que indica la vertical
 * @pre Se asume que todos los par�metros tienen valores v�lidos
 * @post Los atributos de la c�mara cambian a los valores pasados como par�metro
 */
void igvCamara::set ( igvPunto3D _P0, igvPunto3D _r, igvPunto3D _V )
{  P0 = _P0;
   r  = _r;
   V  = _V;
}

/**
 * Define una c�mara de tipo paralela o frustum
 * @param _tipo Tipo de la c�mara (IGV_PARALELA o IGV_FRUSTUM)
 * @param _P0 Posici�n de la c�mara
 * @param _r Punto al que mira la c�mara
 * @param _V Vector que indica la vertical
 * @param _xwmin Coordenada X m�nima del frustum
 * @param _xwmax Coordenada X m�xima del frustum
 * @param _ywmin Coordenada Y m�nima del frustum
 * @param _ywmax Coordenada Y m�xima del frustum
 * @param _znear Distancia de la c�mara al plano Z near
 * @param _zfar Distancia de la c�mara al plano Z far
 * @pre Se asume que todos los par�metros tienen valores v�lidos
 * @post Los atributos de la c�mara cambian a los valores pasados como par�metro
 */
void igvCamara::set ( tipoCamara _tipo, igvPunto3D _P0, igvPunto3D _r
                      , igvPunto3D _V, double _xwmin, double _xwmax, double _ywmin
                      , double _ywmax, double _znear, double _zfar )
{  tipo = _tipo;

   P0 = _P0;
   r = _r;
   V = _V;

   xwmin = _xwmin;
   xwmax = _xwmax;
   ywmin = _ywmin;
   ywmax = _ywmax;
   znear = _znear;
   zfar = _zfar;
}

/**
 * Define una c�mara de tipo perspectiva
 * @param _tipo Tipo de la c�mara (IGV_PERSPECTIVA)
 * @param _P0 Posici�n de la c�mara
 * @param _r Punto al que mira la c�mara
 * @param _V Vector que indica la vertical
 * @param _angulo �ngulo de apertura
 * @param _raspecto Raz�n de aspecto
 * @param _znear Distancia de la c�mara al plano Z near
 * @param _zfar Distancia de la c�mara al plano Z far
 * @pre Se asume que todos los par�metros tienen valores v�lidos
 * @post Los atributos de la c�mara cambian a los valores que se pasan como
 *       par�metros
 */
void igvCamara::set ( tipoCamara _tipo, igvPunto3D _P0, igvPunto3D _r
                      , igvPunto3D _V, double _angulo, double _raspecto
                      , double _znear, double _zfar )
{  tipo = _tipo;

   P0 = _P0;
   r = _r;
   V = _V;

   angulo = _angulo;
   raspecto = _raspecto;
   znear = _znear;
   zfar = _zfar;
}

/**
 * Aplica a los objetos de la escena la transformaci�n de visi�n y la
 * transformaci�n de proyecci�n asociadas a los par�metros de la c�mara
 */
void igvCamara::aplicar ()
{  glMatrixMode ( GL_PROJECTION );
   glLoadIdentity ();

   if ( tipo == IGV_PARALELA )
   {
      glOrtho ( xwmin, xwmax, ywmin, ywmax, znear, zfar );
   }
   if ( tipo == IGV_FRUSTUM )
   {
      glFrustum ( xwmin, xwmax, ywmin, ywmax, znear, zfar );
   }
   if ( tipo == IGV_PERSPECTIVA )
   {
      gluPerspective ( angulo, raspecto, znear, zfar );
   }

   glMatrixMode ( GL_MODELVIEW );
   glLoadIdentity ();
   gluLookAt ( P0[X], P0[Y], P0[Z], r[X], r[Y], r[Z], V[X], V[Y], V[Z] );
}

/**
 * Realiza un zoom sobre la c�mara
 * @param factor Factor (en tanto por 100) que se aplica al zoom. Si el valor es
 *        positivo, se aumenta el zoom. Si es negativo, se reduce.
 * @pre Se asume que el par�metro tiene un valor v�lido
 */
void igvCamara::zoom ( double factor )
{  // factor expresado en tanto por ciento (p.e. 10 = 10%)
   // factor > 0 => acercar (zoom in), factor < 0 => alejar (zoom out)
   double scale = 1.0 - (factor / 100.0); // para perspectiva escala del ángulo, para ortho/frustum escala de extents

   if ( tipo == IGV_PERSPECTIVA )
   {  if ( scale <= 0.0 ) return; // evita valores no válidos
      angulo *= scale;
      if ( angulo < 5.0 ) angulo = 5.0;        // límite mínimo de FOV
      if ( angulo > 170.0 ) angulo = 170.0;    // límite máximo de FOV
   }
   else // IGV_PARALELA o IGV_FRUSTUM
   {  if ( scale <= 0.0 ) return;
      // Centro actual
      double cx = ( xwmin + xwmax ) / 2.0;
      double cy = ( ywmin + ywmax ) / 2.0;
      double hw = ( xwmax - xwmin ) / 2.0; // half width
      double hh = ( ywmax - ywmin ) / 2.0; // half height

      hw *= scale;
      hh *= scale;

      const double kMin = 1e-4; // evita colapso
      if ( hw < kMin ) hw = kMin;
      if ( hh < kMin ) hh = kMin;

      xwmin = cx - hw; xwmax = cx + hw;
      ywmin = cy - hh; ywmax = cy + hh;
   }
}

// Utilidades internas
static void normalize(double &x, double &y, double &z) {
   double n = std::sqrt(x*x + y*y + z*z);
   if (n > 1e-12) { x/=n; y/=n; z/=n; }
}

void igvCamara::orbitY(double grados) {
   // orbita posición alrededor del origen (0,0,0) manteniendo el radio horizontal y altura
   double rad = grados * M_PI / 180.0;
   double x = P0[X];
   double z = P0[Z];
   double cosA = std::cos(rad);
   double sinA = std::sin(rad);
   double nx = x * cosA + z * sinA;
   double nz = -x * sinA + z * cosA;
   P0[X] = nx;
   P0[Z] = nz;
   r.set(0,0,0); // siempre mira al origen
}

void igvCamara::pitch(double grados) {
   // rotar vector de visión alrededor del eje right local
   // dir = r - P0
   double dirx = r[X] - P0[X];
   double diry = r[Y] - P0[Y];
   double dirz = r[Z] - P0[Z];
   // up vector V
   double upx = V[X]; double upy = V[Y]; double upz = V[Z];
   // right = dir x up
   double rx = diry*upz - dirz*upy;
   double ry = dirz*upx - dirx*upz;
   double rz = dirx*upy - diry*upx;
   normalize(rx,ry,rz);
   double rad = grados * M_PI / 180.0;
   double c = std::cos(rad); double s = std::sin(rad);
   // Rotar dir alrededor de right (Rodrigues)
   double ndirx = dirx*c + (ry*dirz - rz*diry)*s + rx*(rx*dirx+ry*diry+rz*dirz)*(1-c);
   double ndiry = diry*c + (rz*dirx - rx*dirz)*s + ry*(rx*dirx+ry*diry+rz*dirz)*(1-c);
   double ndirz = dirz*c + (rx*diry - ry*dirx)*s + rz*(rx*dirx+ry*diry+rz*dirz)*(1-c);
   // Rotar up también
   double nupx = upx*c + (ry*upz - rz*upy)*s + rx*(rx*upx+ry*upy+rz*upz)*(1-c);
   double nupy = upy*c + (rz*upx - rx*upz)*s + ry*(rx*upx+ry*upy+rz*upz)*(1-c);
   double nupz = upz*c + (rx*upy - ry*upx)*s + rz*(rx*upx+ry*upy+rz*upz)*(1-c);
   // actualizar
   r.set(P0[X]+ndirx, P0[Y]+ndiry, P0[Z]+ndirz);
   V.set(nupx,nupy,nupz);
}

void igvCamara::yaw(double grados) {
   // rotación alrededor del eje Y local (up vector V normalizado) manteniendo posición
   double dirx = r[X] - P0[X];
   double diry = r[Y] - P0[Y];
   double dirz = r[Z] - P0[Z];
   double upx = V[X]; double upy = V[Y]; double upz = V[Z];
   normalize(upx,upy,upz);
   double rad = grados * M_PI / 180.0; double c = std::cos(rad); double s = std::sin(rad);
   double dot = upx*dirx + upy*diry + upz*dirz;
   double crossx = upy*dirz - upz*diry;
   double crossy = upz*dirx - upx*dirz;
   double crossz = upx*diry - upy*dirx;
   double ndirx = dirx*c + crossx*s + upx*dot*(1-c);
   double ndiry = diry*c + crossy*s + upy*dot*(1-c);
   double ndirz = dirz*c + crossz*s + upz*dot*(1-c);
   r.set(P0[X]+ndirx, P0[Y]+ndiry, P0[Z]+ndirz);
}

void igvCamara::adjustNear(double delta) {
   znear += delta;
   if (znear < 0.01) znear = 0.01;
   if (znear > zfar - 0.1) znear = zfar - 0.1;

   aplicar();
   glutPostRedisplay();
}

void igvCamara::adjustFar(double delta) {
   zfar += delta;
   if (zfar < znear + 0.1) zfar = znear + 0.1;

   aplicar();
   glutPostRedisplay();
}
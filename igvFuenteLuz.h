//
// Created by msanchez on 12/12/25.
//

#ifndef PR1A_IGVFUENTELUZ_H
#define PR1A_IGVFUENTELUZ_H

#include <GL/glut.h>
#include "igvColor.h"
#include "igvPunto3D.h"

class igvFuenteLuz
{
    private:
      unsigned int idLuz = 0; ///< Identificador de la luz (GL_LIGHT0 a GL_LIGHT7)
      igvPunto3D posicion = { 0, 0, 0 }; ///< Posición de la luz
      igvColor colorAmbiente = { 0, 0, 0 }; ///< Color ambiental de la luz
      igvColor colorDifuso = { 0, 0, 0 }; ///< Color difuso de la luz
      igvColor colorEspecular = { 0, 0, 0 }; ///< Color especular de la luz

      double aten_a0 = 0; ///< Coeficiente de atenuación a0
      double aten_a1 = 0; ///< Coeficiente de atenuación a1
      double aten_a2 = 0; ///< Coeficiente de atenuación a2

      // parámetros para definir un foco
      igvPunto3D direccion_foco = { 0, 0, 0 };   ///< Vector que indica la direcci�n hacia la que apunta el foco
      double angulo_foco = 0;   ///< �ngulo de apertura del foco
      double exponente_foco = 0;   ///< Exponente para el c�lculo de la atenuaci�n del foco

      bool encendida = false; ///< Indica si la luz est� encendida o no

      // NEW: indicar si la fuente se debe tratar como direccional (w=0) o posicional (w=1)
      bool direccional = false;

   public:
      // Constructores por defecto y destructor
      igvFuenteLuz () = default;
      ~igvFuenteLuz() = default;

      igvFuenteLuz ( const unsigned int _idLuz, const igvPunto3D &_posicion
                     , const igvColor &cAmb, const igvColor &cDif
                     , const igvColor &cEsp, const double a0, const double a1
                     , const double a2 );

      igvFuenteLuz ( const unsigned int _idLuz, const igvPunto3D &_posicion
                     , const igvColor &cAmb, const igvColor &cDif
                     , const igvColor &cEsp, const double a0, const double a1
                     , const double a2, const igvPunto3D &dir_foco
                     , const double ang_foco, const double exp_foco );

      // M�todos
      igvPunto3D &getPosicion (); // devuelve la posici�n de la luz
      void setPosicion ( igvPunto3D pos ); // establece la posici�n de la luz

      void set ( const igvColor &cAmb, const igvColor &cDif, const igvColor &cEsp );
      void setAmbiental ( const igvColor &cAmb );
      void setDifuso ( const igvColor &cDif );
      void setEspecular ( const igvColor &cEsp );
      igvColor &getAmbiental ();
      igvColor &getDifuso ();
      igvColor &getEspecular ();

      void setAtenuacion ( double a0, double a1, double a2 );
      void getAtenuacion ( double &a0, double &a1, double &a2 );

      void encender ();
      void apagar ();
      bool esta_encendida ();

      // NEW: setters for directionality and spot parameters
      void setDirectional(bool isDirectional);
      void setDireccionFoco(const igvPunto3D &dir);
      void setSpotParams(double ang_foco, double exp_foco);
      bool isDireccional() const { return direccional; }

      void aplicar ();
      void mover(float dx, float dy, float dz);
};


#endif //PR1A_IGVFUENTELUZ_H
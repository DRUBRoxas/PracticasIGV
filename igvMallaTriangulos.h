#ifndef __IGVMALLATRIANGULOS
#define __IGVMALLATRIANGULOS

#if defined(__APPLE__) && defined(__MACH__)
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else

#include <GL/glut.h>

#endif   // defined(__APPLE__) && defined(__MACH__)

#include <string>

/**
 * Los objetos de esta clase representan mallas de tri�ngulos
 */
class igvMallaTriangulos
{
protected:
    // Atributos
    long int num_vertices = 0; ///< N�mero de v�rtices de la malla de tri�ngulos
    float* vertices = nullptr; ///< Array con las (num_vertices * 3) coordenadas de los v�rtices
    float* normales = nullptr;
    ///< Array con las (num_vertices * 3) coordenadas de la normal en cada v�rtice (s�lo para la generaci�n de la esfera)

    long int num_triangulos = 0; ///< N�mero de tri�ngulos de la malla de tri�ngulos
    unsigned int* triangulos = nullptr; ///< Array con los (num_triangulos * 3) �ndices a los v�rtices de cada tri�ngulo
    bool usarnormales;
    bool gouraud;

public:
    // Constructor y destructor
    /// Constructor por defecto
    igvMallaTriangulos() = default;

    igvMallaTriangulos(long int _num_vertices, float* _vertices
                       , long int _num_triangulos, unsigned int* _triangulos);

    ~igvMallaTriangulos();

    // M�todo con las llamadas OpenGL para visualizar la malla de tri�ngulos
    void visualizar();

    void set_normales(float *_normales) {
        if (normales) delete[] normales; // Borrar anteriores si existen
        normales = new float[num_vertices * 3];
        for (int i = 0; i < num_vertices * 3; ++i) {
            normales[i] = _normales[i];
        }
        usarnormales = true;
    }

    void cambiarnormales() { usarnormales = !usarnormales; }
    void cambiarvis() { gouraud = !gouraud; }
};

#endif   // __IGVMALLATRIANGULOS

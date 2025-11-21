#include <cstdlib>
#include <stdio.h>
#include <math.h>
#include "igvMallaTriangulos.h"

/**
 * Constructor parametrizado de una malla de tri�ngulos sin normales en los
 * v�rtices
 * @param _num_vertices N�mero de v�rtices de la nueva malla
 * @param _vertices Direcci�n de memoria donde se encuentran almacenadas las
 *        coordenadas (formato X,Y,Z) de los v�rtices. Esta informaci�n se
 *        copia en la nueva malla
 * @param _num_triangulos N�mero de tri�ngulos que forman la malla
 * @param _triangulos �ndices (formato v1, v2, v3) a los v�rtices que forman
 *        cada tri�ngulo. Esta informaci�n se copia en el nuevo objeto
 * @pre Se asume que todos los par�metros tienen valores v�lidos
 * @post La nueva malla almacenar� copias de la informaci�n que se le pasa como
 *       par�metro
 */
igvMallaTriangulos::igvMallaTriangulos(long int _num_vertices, float* _vertices
                                       , long int _num_triangulos
                                       , unsigned int* _triangulos) :
    num_vertices(_num_vertices)
    , num_triangulos(_num_triangulos)
{
    num_vertices = _num_vertices;
    vertices = new float[num_vertices * 3];
    for (long int i = 0; i < (num_vertices * 3); ++i)
    {
        vertices[i] = _vertices[i];
    }

    normales = nullptr;

    num_triangulos = _num_triangulos;
    triangulos = new unsigned int[num_triangulos * 3];
    for (long int i = 0; i < (num_triangulos * 3); ++i)
    {
        triangulos[i] = _triangulos[i];
    }
}

/**
 * Destructor
 */
igvMallaTriangulos::~igvMallaTriangulos()
{
    if (vertices)
    {
        delete []vertices;
        vertices = nullptr;
    }

    if (normales)
    {
        delete []normales;
        normales = nullptr;
    }

    if (triangulos)
    {
        delete []triangulos;
        triangulos = nullptr;
    }
}

/**
 * M�todo con las llamadas OpenGL para visualizar la malla de tri�ngulos
 */
void igvMallaTriangulos::visualizar()
{
    if (!gouraud)
    {
        glShadeModel(GL_FLAT);
    }
    else
    {
        glShadeModel(GL_SMOOTH);
    }

    glEnable(GL_NORMALIZE);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vertices);

    if (usarnormales)
    {
        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(GL_FLOAT, 0, normales);
    }

    glDrawElements(GL_TRIANGLES, num_triangulos * 3, GL_UNSIGNED_INT, triangulos);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
}

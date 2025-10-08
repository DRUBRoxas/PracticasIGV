#ifndef __IGVESCENA3D
#define __IGVESCENA3D
#include <vector>

#if defined(__APPLE__) && defined(__MACH__)
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/glut.h>
#endif   // defined(__APPLE__) && defined(__MACH__)

/**
 * Los objetos de esta clase representan escenas 3D para su visualización
 */
class igvEscena3D
{
public:
    const int EscenaA = 1; ///< Identificador interno de la escena A
    const int EscenaB = 2; ///< Identificador interno de la escena B
    const int EscenaC = 3; ///< Identificador interno de la escena C

    const char* Nombre_EscenaA = "Escena A"; ///< Etiqueta de la escena A
    const char* Nombre_EscenaB = "Escena B"; ///< Etiqueta de la escena B
    const char* Nombre_EscenaC = "Escena C"; ///< Etiqueta de la escena C
    int objetoSeleccionado = 1; ///< Identificador del objeto seleccionado en la escena A
    enum Mode { RST, SEQ };

private:
    // Atributos
    bool ejes = true; ///< Indica si hay que dibujar los ejes coordenados o no

    struct Transform
    {
        float tx, ty, tz;
        float rx, ry, rz; // Grados
        float s; // Escala homogénea
    };
    Transform objT[3]{}; // 0:cubo, 1:esfera, 2:cono

    struct Op {
        enum Kind { T, RX, RY, RZ, S } kind;
        float v1, v2, v3; // T: dx,dy,dz | RX/RY/RZ: angle in v1 | S: factor in v1
    };
    std::vector<Op> objOps[3];

    Mode mode = RST;
    static constexpr float kMinScale = 0.05f;
    static constexpr float kMaxScale = 50.0f;

public:
    // Constructores por defecto y destructor
    /// Constructor por defecto
    igvEscena3D();
    /// Destructor
    ~igvEscena3D() = default;

    // Métodos
    // método con las llamadas OpenGL para visualizar la escena
    void visualizar(int escena);

    bool get_ejes();
    void set_ejes(bool _ejes);

    // Transformaciones sobre el objeto seleccionado
    void applyTranslation(float dx, float dy, float dz);
    void applyRotation(float ax, float ay, float az);
    void applyScale(float factor);

    void toggleMode();
private:
    void renderEscenaA();
    void renderEscenaB();
    void renderEscenaC();
    void pintar_ejes();
};

#endif   // __IGVESCENA3D

#ifndef __IGVESCENA3D
#define __IGVESCENA3D
#include <vector>
#include "igvMallaTriangulos.h"

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
    // Seleccion de la parte del robot
    enum ParteSeleccionada {
        ROBOT_COMPLETO = 0,
        CABEZA = 1,
        BRAZO_IZQ = 2,
        BRAZO_DER = 3,
        PIERNA_IZQ = 4,
        PIERNA_DER = 5,
        ANTEBRAZO_IZQ = 6,
        ANTEBRAZO_DER = 7,
        PANTORRILLA_IZQ = 8,
        PANTORRILLA_DER = 9,

        GODZILLA = 100
    };


    int parteActiva = 0; // 0: Cuerpo Completo, 1:cabeza, 2:bIzq, 3:bDer, 4:pIzq, 5:pDer

private:
    // Atributos
    bool seleccionando = false; ///< Indica si se está en modo selección de objeto
    bool ejes = true; ///< Indica si hay que dibujar los ejes coordenados o no
    bool verMalla = false;
    int lastMouseX = 0; ///< Última posición X del ratón
    int lastMouseY = 0; ///< Última posición Y del ratón
    struct Transform
    {
        float tx, ty, tz;
        float rx, ry, rz; // Grados
        float s; // Escala homogénea
    };
    Transform objT[3]{}; // 0:cubo, 1:esfera, 2:cono

    struct Robot {
        float cabeza = 0.0f;
        float brazoIzq = 0.0f;
        float brazoIzqLat = 0.0f;
        float brazoDer = 0.0f;
        float brazoDerLat = 0.0f;
        float piernaIzq = 0.0f;
        float piernaDer = 0.0f;
        float codoIzq = 0.0f;
        float codoDer = 0.0f;
        float pantorrillaIzq = 0.0f;
        float pantorrillaDer = 0.0f;
    } robotState;

    struct Op {
        enum Kind { T, RX, RY, RZ, S } kind;
        float v1, v2, v3; // T: dx,dy,dz | RX/RY/RZ: angle in v1 | S: factor in v1
    };
    std::vector<Op> objOps[3];

    Mode mode = RST;
    static constexpr float kMinScale = 0.05f;
    static constexpr float kMaxScale = 50.0f;

    //Puntero a malla de triangulos
    igvMallaTriangulos *objetoGodzilla = nullptr;

public:
    // Constructores por defecto y destructor
    /// Constructor por defecto
    igvEscena3D();
    /// Destructor
    ~igvEscena3D();

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
    void toggleMalla(){verMalla = !verMalla;};

    void moverArticulacion(float dx, float dy);

    // Cambiar entre sombreado plano y suave
    void cambiarSombreado();

    // Metodo para saber que has clicado
    void pick(int x, int y);
    // Metodo para darle una animación automática al robot
    void animarRobot();

    // Metodo para mover el objeto con el raton
    void arrastrar(int x, int y);
    // Guardar la última posición del ratón
    void setLastMouse(int x, int y) {
        lastMouseX = x;
        lastMouseY = y;
    }

    void setSeleccionando(bool sel) {
        seleccionando = sel;
    }
private:
    void renderEscenaA();
    void renderEscenaB();
    void renderEscenaC();
    void pintar_ejes();

    // Funcion para crear al Godzilla
    void generarMallaGodzilla();
    // Funciones auxiliares para crear la malla de Godzilla
    /** Genera un tubo vertical entre dos puntos con radios diferentes en cada extremo
     *  y lo añade a la malla pasada como parámetro
     */
    void addTuboToMesh(float x1, float y1, float z1, float rBaseX, float rBaseZ,
                       float x2, float y2, float z2, float rTopX, float rTopZ,
                       int numLados,
                       std::vector<float>& v, std::vector<float>& n, std::vector<unsigned int>& i, int& idxOffset);
    /** Genera un tubo horizontal entre dos puntos con radios diferentes en cada extremo
     *  y lo añade a la malla pasada como parámetro
     */
    void addTuboHorizontal(float x1, float y1, float z1, float rBaseX, float rBaseY,
                           float x2, float y2, float z2, float rTopX, float rTopY,
                           int numLados,
                           std::vector<float>& v, std::vector<float>& n, std::vector<unsigned int>& i, int& idxOffset);
    /** Genera una tapa (base o top) en la posición y orientación indicadas
     *  y la añade a la malla pasada como parámetro
     */
    void addTapa(float x, float y, float z, float rx, float ry,
                 int type, float normalDir,
                 int numLados,
                 std::vector<float>& v, std::vector<float>& n, std::vector<unsigned int>& i, int& idxOffset);

};

#endif   // __IGVESCENA3D

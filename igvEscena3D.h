#ifndef __IGVESCENA3D
#define __IGVESCENA3D
#include <vector>
#include "igvMallaTriangulos.h"
#include "igvMaterial.h"
#include "igvTextura.h"
#include "igvFuenteLuz.h"

#if defined(__APPLE__) && defined(__MACH__)
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/glut.h>
#endif   // defined(__APPLE__) && defined(__MACH__)

/**
 * Los objetos de esta clase representan escenas 3D para su visualizaci?n
 */
class igvEscena3D
{
public:
    // Control de selección/movimiento de luces desde menú
    enum LuzSeleccionada {
        LUZNINGUNA = 0,
        LUZ_DIRECCIONAL = 1,
        LUZ_PUNTUAL = 2,
        LUZ_CONO = 3
    };
    LuzSeleccionada luzSeleccionada = LUZNINGUNA;
    bool modoMoverLuz = false; // si true, las llamadas a moveSelectedLight mueven la luz seleccionada

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

    enum FiltroTextura{
        FILTRO_NN,
        FILTRO_NL,
        FILTRO_LN,
        FILTRO_LL
    };

private:
    // Atributos
    bool seleccionando = false; ///< Indica si se est? en modo selecci?n de objeto
    bool ejes = true; ///< Indica si hay que dibujar los ejes coordenados o no
    bool verMalla = false;
    int lastMouseX = 0; ///< ?ltima posici?n X del rat?n
    int lastMouseY = 0; ///< ?ltima posici?n Y del rat?n
    struct Transform
    {
        float tx, ty, tz;
        float rx, ry, rz; // Grados
        float s; // Escala homog?nea
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

    // Pose inicial guardada al arrancar la animaci?n (baseline)
    Robot robotStateInitial;
    bool animacionActiva = false;
    float animTime = 0.0f; // tiempo de animaci?n en segundos

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

    // Atributos para el suelo
    igvTextura* texturaSuelo[2]; // Dos texturas para el suelo
    unsigned int idTexturaAjedrez; // ID de la textura de ajedrez

    igvMaterial materialSuelo[3]; // Materiales para el suelo

    igvFuenteLuz luzPuntual; // Luz puntual de la escena

    // Nuevas luces: ambiente global, direccional y foco (cono)
    igvFuenteLuz luzAmbiental;
    igvFuenteLuz luzDireccional;
    igvFuenteLuz luzCono;

    int indiceTexturaActual = 0; // Indice de la textura del suelo
    int indiceMaterialActual = 0; // Indice del material actual del suelo
    bool usarTextura = true; // Indica si se usa textura o no en el suelo activar/desactivar
    FiltroTextura filtroActual = FILTRO_LL; // Filtro de la textura del suelo

public:
    // Constructores por defecto y destructor
    /// Constructor por defecto
    igvEscena3D();
    /// Destructor
    ~igvEscena3D();

    // M?todos
    // m?todo con las llamadas OpenGL para visualizar la escena
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
    // Metodo para darle una animaci?n autom?tica al robot
    void animarRobot();

    // Control de ciclo de animaci?n: inicio / parada (restauran pose)
    void startAnimacion();
    void stopAnimacion();
    bool isAnimando() const { return animacionActiva; }

    // Metodo para mover el objeto con el raton
    void arrastrar(int x, int y);
    // Guardar la ?ltima posici?n del rat?n
    void setLastMouse(int x, int y) {
        lastMouseX = x;
        lastMouseY = y;
    }

    void setSeleccionando(bool sel) {
        seleccionando = sel;
    }

    void setMaterialSuelo(int indice) { if(indice >= 0 && indice < 3) indiceMaterialActual = indice; }
    void setTexturaSuelo(int indice) { if(indice >= 0 && indice < 3) indiceTexturaActual = indice; }
    void toggleTexturaSuelo(bool activar) { usarTextura = activar; }
    void setFiltroTextura(FiltroTextura filtro) { filtroActual = filtro; }
    void inicializarSuelo();

    // Control de luces desde la interfaz/menu
    void toggleLight(LuzSeleccionada l); // activa/desactiva la luz indicada
    void selectLight(LuzSeleccionada l); // seleccionar luz para activar/desactivar o mover
    void enterMoveLightMode(bool enter); // alternar estado de movimiento de la luz seleccionada
    void moveSelectedLight(float dx, float dy, float dz); // mover la luz seleccionada (puntual o cono)
    LuzSeleccionada getSelectedLight() const { return luzSeleccionada; }
    bool isModoMoverLuz() const { return modoMoverLuz; }

private:
    void renderEscenaA();
    void renderEscenaB();
    void renderEscenaC();
    void pintar_ejes();

    // Funcion para crear al Godzilla
    void generarMallaGodzilla();
    // Funciones auxiliares para crear la malla de Godzilla
    /** Genera un tubo vertical entre dos puntos con radios diferentes en cada extremo
     *  y lo a?ade a la malla pasada como par?metro
     */
    void addTuboToMesh(float x1, float y1, float z1, float rBaseX, float rBaseZ,
                       float x2, float y2, float z2, float rTopX, float rTopZ,
                       int numLados,
                       std::vector<float>& v, std::vector<float>& n, std::vector<unsigned int>& i, int& idxOffset);
    /** Genera un tubo horizontal entre dos puntos con radios diferentes en cada extremo
     *  y lo a?ade a la malla pasada como par?metro
     */
    void addTuboHorizontal(float x1, float y1, float z1, float rBaseX, float rBaseY,
                           float x2, float y2, float z2, float rTopX, float rTopY,
                           int numLados,
                           std::vector<float>& v, std::vector<float>& n, std::vector<unsigned int>& i, int& idxOffset);
    /** Genera una tapa (base o top) en la posici?n y orientaci?n indicadas
     *  y la a?ade a la malla pasada como par?metro
     */
    void addTapa(float x, float y, float z, float rx, float ry,
                 int type, float normalDir,
                 int numLados,
                 std::vector<float>& v, std::vector<float>& n, std::vector<unsigned int>& i, int& idxOffset);
    /** M?todo para pintar el suelo con la textura seleccionada
     */
    void pintarSuelo();
    // Genera una malla regular de quads sobre el plano XZ
    void pintar_quad(int div_x, int div_z, float halfSize, float texRepeat);
    /** M?todo para inicializar las texturas del suelo
     */
    void generarTexturaAjedrez(); // Auxiliar para crear el ajedrez
};

#endif   // __IGVESCENA3D

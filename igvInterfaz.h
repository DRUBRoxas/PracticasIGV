#ifndef __IGVINTERFAZ
#define __IGVINTERFAZ

#if defined(__APPLE__) && defined(__MACH__)

#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>

#else
#include <GL/glut.h>
#endif   // defined(__APPLE__) && defined(__MACH__)

#include <string>
#include "igvEscena3D.h"
#include "igvCamara.h"
#include "igvPunto3D.h"

/**
 * Los objetos de esta clase encapsulan la interfaz y el estado de la aplicación
 */

/// Vistas canónicas
enum Vista
{
    PLANTA ///< Vista de planta (cámara en el eje Y)
    , ALZADO ///< Vista de alzado (cámara en el eje Z)
    , PERFIL ///< Vista de perfil (cámara en el eje X)
    , OTRA ///< Otro tipo de vista
};

class igvInterfaz
{
private:
    // Atributos
    int ancho_ventana = 0; ///< Ancho de la ventana de visualización
    int alto_ventana = 0; ///< Alto de la ventana de visualización

    igvEscena3D escena; ///< Escena que se visualiza en la ventana definida por igvInterfaz
    igvCamara camara;   ///< Cámara activa
    Vista vistaActual = OTRA; ///< Vista canónica actual
    int menuSelection = 0; ///< Última opción de menú seleccionada
    bool multiViewport = false; ///< Muestra varios viewports simultáneos

    // Aplicación del patrón Singleton
    static igvInterfaz* _instancia; ///< Puntero al objeto único de la clase
    igvInterfaz();

    void aplicarVista(Vista v); // ajusta posición/orientación cámara
public:
    static igvInterfaz& getInstancia();

    /// Destructor
    ~igvInterfaz() = default;

    // Métodos estáticos
    // callbacks de eventos
    static void keyboardFunc(unsigned char key, int x, int y); // método para control de eventos del teclado
    static void reshapeFunc(int w, int h); // método que define la cámara de vision y el viewport
    // se llama automáticamente cuando se cambia el tamaño de la ventana
    static void displayFunc(); // método para visualizar la escena
    static void menuHandle(int value); // método para gestionar la selección de opciones de menú
    static void specialFunc(int key, int x, int y);


    // Métodos
    // inicializa todos los parámetros para crear una ventana de visualización
    void configura_entorno(int argc, char** argv // parámetros del main
                           , int _ancho_ventana, int _alto_ventana // ancho y alto de la ventana de visualización
                           , int _pos_X, int _pos_Y // posición inicial de la ventana de visualización
                           , std::string _titulo // título de la ventana de visualización
    );
    void create_menu(); // Creates a menu that is handled with the right button of the mouse.

    void inicializa_callbacks(); // inicializa todos los callbacks

    void inicia_bucle_visualizacion(); // visualiza la escena y espera a eventos sobre la interfaz

    // métodos get_ y set_ de acceso a los atributos
    int get_ancho_ventana();
    int get_alto_ventana();

    void set_ancho_ventana(int _ancho_ventana);
    void set_alto_ventana(int _alto_ventana);
};

#endif   // __IGVINTERFAZ

#include <cstdlib>

#include "igvInterfaz.h"

// Aplicaci?n del patr?n Singleton
igvInterfaz* igvInterfaz::_instancia = nullptr;

// M?todos constructores -----------------------------------

/**
 * Constructor por defecto
 */
igvInterfaz::igvInterfaz() : menuSelection(escena.EscenaA)
{
}

// M?todos p?blicos ----------------------------------------

/**
 * M?todo para acceder al objeto ?nico de la clase, en aplicaci?n del patr?n de
 * dise?o Singleton
 * @return Una referencia al objeto ?nico de la clase
 */
igvInterfaz& igvInterfaz::getInstancia()
{
   if (!_instancia)
   {
      _instancia = new igvInterfaz;
   }

   return *_instancia;
}

/**
 * Inicializa todos los par?metros para crear una ventana de visualizaci?n
 * @param argc N?mero de par?metros por l?nea de comandos al ejecutar la
 *             aplicaci?n
 * @param argv Par?metros por l?nea de comandos al ejecutar la aplicaci?n
 * @param _ancho_ventana Ancho inicial de la ventana de visualizaci?n
 * @param _alto_ventana Alto inicial de la ventana de visualizaci?n
 * @param _pos_X Coordenada X de la posici?n inicial de la ventana de
 *               visualizaci?n
 * @param _pos_Y Coordenada Y de la posici?n inicial de la ventana de
 *               visualizaci?n
 * @param _titulo T?tulo de la ventana de visualizaci?n
 * @pre Se asume que todos los par?metros tienen valores v?lidos
 * @post Cambia el alto y ancho de ventana almacenado en el objeto
 */
void igvInterfaz::configura_entorno(int argc, char** argv
                                    , int _ancho_ventana, int _alto_ventana
                                    , int _pos_X, int _pos_Y
                                    , std::string _titulo)
{
   // inicializaci?n de los atributos de la interfaz
   ancho_ventana = _ancho_ventana;
   alto_ventana = _alto_ventana;

   // inicializaci?n de la ventana de visualizaci?n
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
   glutInitWindowSize(_ancho_ventana, _alto_ventana);
   glutInitWindowPosition(_pos_X, _pos_Y);
   glutCreateWindow(_titulo.c_str());

   create_menu();

   glEnable(GL_DEPTH_TEST); // activa el ocultamiento de superficies por z-buffer
   glClearColor(1.0, 1.0, 1.0, 0.0); // establece el color de fondo de la ventana

   glEnable(GL_LIGHTING); // activa la iluminaci?n de la escena
   glEnable(GL_NORMALIZE); // normaliza los vectores normales para c?lculo de iluminaci?n
}

/**
 * Crea un men? asociado al bot?n derecho del rat?n
 */
void igvInterfaz::create_menu()
{
   int menu_id = glutCreateMenu(menuHandle);
   glutAddMenuEntry(_instancia->escena.Nombre_EscenaA
                    , _instancia->escena.EscenaA);
   glutAddMenuEntry(_instancia->escena.Nombre_EscenaB
                    , _instancia->escena.EscenaB);
   glutAddMenuEntry(_instancia->escena.Nombre_EscenaC
                    , _instancia->escena.EscenaC);

   glutAttachMenu(GLUT_RIGHT_BUTTON);
}

/**
 * M?todo para visualizar la escena y esperar a eventos sobre la interfaz
 */
void igvInterfaz::inicia_bucle_visualizacion()
{
   glutMainLoop(); // inicia el bucle de visualizaci?n de GLUT
}

/**
 * M?todo para control de eventos del teclado
 * @param key C?digo de la tecla pulsada
 * @param x Coordenada X de la posici?n del cursor del rat?n en el momento del
 *          evento de teclado
 * @param y Coordenada Y de la posici?n del cursor del rat?n en el momento del
 *          evento de teclado
 * @pre Se asume que todos los par?metros tienen valores v?lidos
 * @post La escena puede cambiar dependiendo de la tecla pulsada
 */
void igvInterfaz::keyboardFunc(unsigned char key, int x, int y)
{
   const float dT = 0.1f; // delta traslación
   const float dA = 5.0f; // delta rotación en grados
   const float kUp = 1.05f; // factor escala +
   const float kDn = 1.0f / kUp; // factor escala -

   switch (key)
   {
   case 'e': // activa/desactiva la visualizaci?n de los ejes
   case 'E':
      _instancia->escena.set_ejes(!_instancia->escena.get_ejes());
      break;

   // Modo de transformaciones (RST <-> SEQ)
   case 'm':
   case 'M':
      _instancia->escena.toggleMode();
      break;

   // Selección de objeto
   case '1': _instancia->escena.objetoSeleccionado = 1;
      break;
   case '2': _instancia->escena.objetoSeleccionado = 2;
      break;
   case '3': _instancia->escena.objetoSeleccionado = 3;
      break;

   // Traslación en Y
   case 'u': _instancia->escena.applyTranslation(0.0f, +dT, 0.0f);
      break;
   case 'U': _instancia->escena.applyTranslation(0.0f, -dT, 0.0f);
      break;

   // Rotaciones
   case 'x': _instancia->escena.applyRotation(+dA, 0.0f, 0.0f);
      break;
   case 'X': _instancia->escena.applyRotation(-dA, 0.0f, 0.0f);
      break;
   case 'y': _instancia->escena.applyRotation(0.0f, +dA, 0.0f);
      break;
   case 'Y': _instancia->escena.applyRotation(0.0f, -dA, 0.0f);
      break;
   case 'z': _instancia->escena.applyRotation(0.0f, 0.0f, +dA);
      break;
   case 'Z': _instancia->escena.applyRotation(0.0f, 0.0f, -dA);
      break;

   // Escalado homogéneo
   case 's': _instancia->escena.applyScale(kUp);
      break;
   case 'S': _instancia->escena.applyScale(kDn);
      break;

   case 27: exit(1);
      break;
   }
   glutPostRedisplay(); // renueva el contenido de la ventana de vision
}

// --- handle arrows in specialFunc ---
void igvInterfaz::specialFunc(int key, int x, int y)
{
   const float dT = 0.1f;
   switch (key)
   {
   case GLUT_KEY_LEFT: _instancia->escena.applyTranslation(-dT, 0.0f, 0.0f);
      break; // +X
   case GLUT_KEY_RIGHT: _instancia->escena.applyTranslation(+dT, 0.0f, 0.0f);
      break; // -X
   case GLUT_KEY_UP: _instancia->escena.applyTranslation(0.0f, 0.0f, +dT);
      break; // +Z
   case GLUT_KEY_DOWN: _instancia->escena.applyTranslation(0.0f, 0.0f, -dT);
      break; // -Z
   }
   glutPostRedisplay();
}

/**
 * M?todo que define la c?mara de visi?n y el viewport. Se llama autom?ticamente
 * cuando se cambia el tama?o de la ventana.
 * @param w Nuevo ancho de la ventana
 * @param h Nuevo alto de la ventana
 * @pre Se asume que todos los par?metros tienen valores v?lidos
 */
void igvInterfaz::reshapeFunc(int w, int h)
{
   // dimensiona el viewport al nuevo ancho y alto de la ventana
   glViewport(0, 0, (GLsizei)w, (GLsizei)h);

   // guardamos valores nuevos de la ventana de visualizacion
   _instancia->set_ancho_ventana(w);
   _instancia->set_alto_ventana(h);

   // establece el tipo de proyeccion a utilizar
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();

   glOrtho(-1 * 5, 1 * 5, -1 * 5, 1 * 5, -1 * 5, 200);

   // se define la camara de vision
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   gluLookAt(1.5, 1.0, 2.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0); // vista en perspectiva
   //gluLookAt(1.5,0,0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0); // vista en planta desde el eje X positivo
}

/**
 * M?todo para visualizar la escena
 */
void igvInterfaz::displayFunc()
{
   _instancia->escena.visualizar(_instancia->menuSelection);
}

/**
 * M?todo para gestionar la selecci?n de opciones de men?
 * @param value Nueva opci?n seleccionada
 * @pre Se asume que el valor del par?metro es correcto
 * @post Se almacena en el objeto la opci?n seleccionada
 */
void igvInterfaz::menuHandle(int value)
{
   _instancia->menuSelection = value;
   glutPostRedisplay(); // renew the content of the window
}

/**
 * M?todo para inicializar los callbacks
 */
void igvInterfaz::inicializa_callbacks()
{
   glutKeyboardFunc(keyboardFunc);
   glutReshapeFunc(reshapeFunc);
   glutDisplayFunc(displayFunc);
   glutSpecialFunc(specialFunc);
}

/**
 * M?todo para consultar el ancho de la ventana de visualizaci?n
 * @return El valor almacenado como ancho de la ventana de visualizaci?n
 */
int igvInterfaz::get_ancho_ventana()
{
   return ancho_ventana;
}

/**
 * M?todo para consultar el alto de la ventana de visualizaci?n
 * @return El valor almacenado como alto de la ventana de visualizaci?n
 */
int igvInterfaz::get_alto_ventana()
{
   return alto_ventana;
}

/**
 * M?todo para cambiar el ancho de la ventana de visualizaci?n
 * @param _ancho_ventana Nuevo valor para el ancho de la ventana de visualizaci?n
 * @pre Se asume que el par?metro tiene un valor v?lido
 * @post El ancho de ventana almacenado en la aplicaci?n cambia al nuevo valor
 */
void igvInterfaz::set_ancho_ventana(int _ancho_ventana)
{
   ancho_ventana = _ancho_ventana;
}

/**
 * M?todo para cambiar el alto de la ventana de visualizaci?n
 * @param _alto_ventana Nuevo valor para el alto de la ventana de visualizaci?n
 * @pre Se asume que el par?metro tiene un valor v?lido
 * @post El alto de ventana almacenado en la aplicaci?n cambia al nuevo valor
 */
void igvInterfaz::set_alto_ventana(int _alto_ventana)
{
   alto_ventana = _alto_ventana;
}

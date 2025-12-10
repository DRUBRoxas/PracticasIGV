#include "igvInterfaz.h"
#include <cstdio>

#include <cstdlib>

// Aplicaci?n del patr?n Singleton
igvInterfaz* igvInterfaz::_instancia = nullptr;

// M?todos constructores -----------------------------------

/**
 * Constructor por defecto
 */
igvInterfaz::igvInterfaz() : menuSelection(escena.EscenaA)
{
   // Configuración inicial de la cámara: perspectiva
   camara.set(IGV_PERSPECTIVA,
              igvPunto3D(3,2,4), // posición
              igvPunto3D(0,0,0), // referencia
              igvPunto3D(0,1,0), // up
              60.0, 1.0, 0.1, 200.0);
   vistaActual = OTRA;
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

   escena.inicializarSuelo();

   create_menu();

   glEnable(GL_DEPTH_TEST); // activa el ocultamiento de superficies por z-buffer
   glClearColor(1.0, 1.0, 1.0, 0.0); // establece el color de fondo de la ventana

   glEnable(GL_LIGHTING); // activa la iluminación de la escena
   glEnable(GL_NORMALIZE); // normaliza los vectores normales para cálculo de iluminación

   // Ajustar aspecto inicial
   if (_alto_ventana != 0)
      camara.setAspecto(static_cast<double>(_ancho_ventana) / static_cast<double>(_alto_ventana));
}

/**
 * Crea un men? asociado al bot?n derecho del rat?n
 */
void igvInterfaz::create_menu()
{
   // 1. Submenú Materiales
   int menuMateriales = glutCreateMenu(menuHandle);
   glutAddMenuEntry("Material 1 (Gris Mate)", 100);
   glutAddMenuEntry("Material 2 (Dorado)", 101);
   glutAddMenuEntry("Material 3 (Azul Plastico)", 102);

   // 2. Submenú Texturas
   int menuTexturas = glutCreateMenu(menuHandle);
   glutAddMenuEntry("Sin Textura", 200); // Desactivar
   glutAddMenuEntry("Textura 1 (Ajedrez)", 201);
   glutAddMenuEntry("Textura 2 (Baldosa)", 202);
   glutAddMenuEntry("Textura 3 (Cesped)", 203);

   // 3. Submenú Filtros
   int menuFiltros = glutCreateMenu(menuHandle);
   glutAddMenuEntry("MAG: Nearest / MIN: Nearest", 300);
   glutAddMenuEntry("MAG: Nearest / MIN: Linear", 301);
   glutAddMenuEntry("MAG: Linear / MIN: Nearest", 302);
   glutAddMenuEntry("MAG: Linear / MIN: Linear", 303);

   // 4. Menú Principal
   int menuPrincipal = glutCreateMenu(menuHandle);
   glutAddSubMenu("Material Suelo", menuMateriales);
   glutAddSubMenu("Textura Suelo", menuTexturas);
   glutAddSubMenu("Filtros Textura", menuFiltros);
   glutAddMenuEntry("-----------------", -1);
   glutAddMenuEntry("Animar Robot (On/Off)", 998);
   glutAddMenuEntry("Animar Camara (On/Off)", 999);

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
   const double dOrbit = 5.0; // grados órbita
   const double dPitch = 5.0; // grados cabeceo
   const double dYaw = 5.0;   // grados yaw
   const double dNear = 0.1;  // incremento near
   const double dFar = 1.0;   // incremento far

   switch (key)
   {
   case 'c': case 'C': // activar/desactivar control de cámara
      _instancia->camara.toggleControl();
      break;
   case 'e': case 'E': // ejes
      _instancia->escena.set_ejes(!_instancia->escena.get_ejes());
      break;
   case 'm': case 'M': // modo transformaciones objetos
      _instancia->escena.toggleMode();
      break;
   case 'v': case 'V': { // CICLAR vistas: OTRA -> PLANTA -> PERFIL -> ALZADO -> OTRA
         Vista next = OTRA;
         switch (_instancia->vistaActual) {
         case OTRA:   next = PLANTA; break;
         case PLANTA: next = PERFIL; break;
         case PERFIL: next = ALZADO; break;
         case ALZADO: next = OTRA;   break;
         }
         _instancia->aplicarVista(next);
   } break;

   case 'o': case 'O': // activar/desactivar multi-viewport
      _instancia->multiViewport = !_instancia->multiViewport;
      break;

   // Selección de objeto (solo si no estamos en control de cámara)
      case '0': if(!_instancia->camara.isControlActivo()) _instancia->escena.parteActiva = igvEscena3D::ROBOT_COMPLETO; _instancia->escena.objetoSeleccionado = 1; break;
      case '1': if(!_instancia->camara.isControlActivo()) _instancia->escena.parteActiva = igvEscena3D::CABEZA; break;
      case '2': if(!_instancia->camara.isControlActivo()) _instancia->escena.parteActiva = igvEscena3D::BRAZO_IZQ; break;
      case '3' : if(!_instancia->camara.isControlActivo()) _instancia->escena.parteActiva = igvEscena3D::BRAZO_DER; break;
      case '4': if(!_instancia->camara.isControlActivo()) _instancia->escena.parteActiva = igvEscena3D::PIERNA_IZQ; break;
      case '5': if(!_instancia->camara.isControlActivo()) _instancia->escena.parteActiva = igvEscena3D::PIERNA_DER; break;
      case '6': if(!_instancia->camara.isControlActivo()) _instancia->escena.parteActiva = igvEscena3D::ANTEBRAZO_IZQ; break;
      case '7': if(!_instancia->camara.isControlActivo()) _instancia->escena.parteActiva = igvEscena3D::ANTEBRAZO_DER; break;
      case '8': if(!_instancia->camara.isControlActivo()) _instancia->escena.parteActiva = igvEscena3D::PANTORRILLA_IZQ; break;
      case '9': if(!_instancia->camara.isControlActivo()) _instancia->escena.parteActiva = igvEscena3D::PANTORRILLA_DER; break;
      case 'h': case 'H': if(!_instancia->camara.isControlActivo()) _instancia->escena.parteActiva = igvEscena3D::GODZILLA; _instancia->escena.objetoSeleccionado = 2; break; // fondo
   // Traslación en Y (objetos)
   case 'u': if(!_instancia->camara.isControlActivo()) _instancia->escena.applyTranslation(0.0f, +dT, 0.0f); break;
   case 'U': if(!_instancia->camara.isControlActivo()) _instancia->escena.applyTranslation(0.0f, -dT, 0.0f); break;

   // Rotaciones objetos o yaw cámara
   case 'y':
      if (_instancia->camara.isControlActivo()) _instancia->camara.yaw(+dYaw);
      else _instancia->escena.applyRotation(0.0f, +dA, 0.0f);
      break;
   case 'Y':
      if (_instancia->camara.isControlActivo()) _instancia->camara.yaw(-dYaw);
      else _instancia->escena.applyRotation(0.0f, -dA, 0.0f);
      break;
   case 'x': if(!_instancia->camara.isControlActivo()) _instancia->escena.applyRotation(+dA, 0.0f, 0.0f); break;
   case 'X': if(!_instancia->camara.isControlActivo()) _instancia->escena.applyRotation(-dA, 0.0f, 0.0f); break;
   case 'z': if(!_instancia->camara.isControlActivo()) _instancia->escena.applyRotation(0.0f, 0.0f, +dA); break;
   case 'Z': if(!_instancia->camara.isControlActivo()) _instancia->escena.applyRotation(0.0f, 0.0f, -dA); break;

   // Escalado homogéneo objetos
   case 's': if(!_instancia->camara.isControlActivo()) _instancia->escena.applyScale(kUp); break;
   case 'S': if(!_instancia->camara.isControlActivo()) _instancia->escena.applyScale(kDn); break;

   // Toggle proyección p/P
   case 'p': case 'P': {
      if (_instancia->camara.getTipo() == IGV_PERSPECTIVA) _instancia->camara.setTipo(IGV_PARALELA);
      else _instancia->camara.setTipo(IGV_PERSPECTIVA);
   } break;

   // Ajuste planos near/far
   case 'f': _instancia->camara.adjustNear(+dNear); break; // mover near hacia delante (aumenta znear)
   case 'F': _instancia->camara.adjustNear(-dNear); break; // mover near hacia atrás (disminuye znear)
   case 'b': _instancia->camara.adjustFar(+dFar); break;   // alejar far
   case 'B': _instancia->camara.adjustFar(-dFar); break;   // acercar far

   // Zoom cámara
   case '+': _instancia->camara.zoom(10.0); break;   // acercar
   case '-': _instancia->camara.zoom(-10.0); break;  // alejar
   case 'g': case 'G': _instancia->animacionCamara = !_instancia->animacionCamara; break; // activar/desactivar animación cámara
   case 'a': case 'A':
      _instancia->animacionRobot = !_instancia->animacionRobot;
      if (_instancia->animacionRobot) _instancia->escena.startAnimacion();
      else _instancia->escena.stopAnimacion();
      break;

   case 'w': case 'W': _instancia->escena.toggleMalla(); break; // vermalla alámbrica (Para ver como se hace el moñeco con las mallas triangulares)
   case 'j': case 'J': _instancia->escena.cambiarSombreado(); break; // cambiar entre sombreado plano y suave
   case 27: exit(1); break;
   }
   glutPostRedisplay();
}

// --- handle arrows in specialFunc ---
void igvInterfaz::specialFunc(int key, int x, int y)
{
   const float dT = 0.1f;
   const float factor = 5.0f;
   const double dOrbit = 5.0; // grados
   const double dPitch = 5.0; // grados
   if (_instancia->camara.isControlActivo()) {
      switch (key) {
      case GLUT_KEY_LEFT: _instancia->camara.orbitY(-dOrbit); break;
      case GLUT_KEY_RIGHT: _instancia->camara.orbitY(+dOrbit); break;
      case GLUT_KEY_UP: _instancia->camara.pitch(+dPitch); break;
      case GLUT_KEY_DOWN: _instancia->camara.pitch(-dPitch); break;
      }
   } else {
      // 1. Articulaciones del robot (IDs 1 a 9)
      if (_instancia->escena.parteActiva > 0 && _instancia->escena.parteActiva <= 9) {
         switch (key)
         {
            case GLUT_KEY_LEFT: _instancia->escena.moverArticulacion(-factor, 0.0f); break;
            case GLUT_KEY_RIGHT: _instancia->escena.moverArticulacion(+factor, 0.0f); break;
            case GLUT_KEY_UP: _instancia->escena.moverArticulacion(0.0f, +factor); break;
            case GLUT_KEY_DOWN: _instancia->escena.moverArticulacion(0.0f, -factor); break;
         }
      } else { // 2. Traslaciones del objeto completo (ID 0) o Godzilla (ID 100)
         switch (key){
            case GLUT_KEY_LEFT: _instancia->escena.applyTranslation(-dT, 0.0f, 0.0f); break;
            case GLUT_KEY_RIGHT: _instancia->escena.applyTranslation(+dT, 0.0f, 0.0f); break;
            case GLUT_KEY_UP: _instancia->escena.applyTranslation(0.0f, 0.0f, +dT); break;
            case GLUT_KEY_DOWN: _instancia->escena.applyTranslation(0.0f, 0.0f, -dT); break;
         }
      }
   }
   glutPostRedisplay();
}

void igvInterfaz::mouseFunc(int button, int state, int x, int y) {
   if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
      _instancia->escena.setLastMouse(x, y);
      _instancia->camara.aplicar();
      _instancia->escena.pick(x, y);
   }
}

void igvInterfaz::motionFunc(int x, int y) {
   _instancia->escena.arrastrar(x,y);
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
   glViewport(0, 0, (GLsizei)w, (GLsizei)h);
   _instancia->set_ancho_ventana(w);
   _instancia->set_alto_ventana(h);
   if (h != 0)
      _instancia->camara.setAspecto(static_cast<double>(w)/static_cast<double>(h));
   // Reaplicamos cámara para que la nueva proyección se ajuste
   _instancia->camara.aplicar();
}

/**
 * M?todo para visualizar la escena
 */
void igvInterfaz::displayFunc()
{
   glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
   if (!_instancia->multiViewport) {
      _instancia->camara.aplicar();
      _instancia->escena.visualizar(_instancia->menuSelection);
   } else {
      int W = _instancia->ancho_ventana;
      int H = _instancia->alto_ventana;
      int w2 = W/2; int h2 = H/2;

      auto drawViewport = [&](int x, int y, int w, int h, Vista v, bool perspective){
         glViewport(x,y,w,h);
         igvCamara camLocal;
         auto zn = _instancia->camara.getNear();
         auto zf = _instancia->camara.getFar();

         if (v == PLANTA)
            camLocal.set(perspective? IGV_PERSPECTIVA:IGV_PARALELA,
                         igvPunto3D(0,10,0), igvPunto3D(0,0,0), igvPunto3D(0,0,-1),
                         60.0, (double)w/h, zn, zf);
         else if (v == ALZADO)
            camLocal.set(perspective? IGV_PERSPECTIVA:IGV_PARALELA,
                         igvPunto3D(0,0,10), igvPunto3D(0,0,0), igvPunto3D(0,1,0),
                         60.0, (double)w/h, zn, zf);
         else if (v == PERFIL)
            camLocal.set(perspective? IGV_PERSPECTIVA:IGV_PARALELA,
                         igvPunto3D(10,0,0), igvPunto3D(0,0,0), igvPunto3D(0,1,0),
                         60.0, (double)w/h, zn, zf);
         camLocal.setAspecto((double)w/h);
         camLocal.aplicar();
         _instancia->escena.visualizar(_instancia->menuSelection);
      };
      // Superior izquierda: cámara libre actual
      drawViewport(0,h2,w2,h2,OTRA,true);
      // Superior derecha: planta
      drawViewport(w2,h2,w2,h2,PLANTA,false);
      // Inferior izquierda: alzado
      drawViewport(0,0,w2,h2,ALZADO,false);
      // Inferior derecha: perfil
      drawViewport(w2,0,w2,h2,PERFIL,false);
   }
   glutSwapBuffers();
}

/**
 * M?todo para gestionar la selecci?n de opciones de men?
 * @param value Nueva opci?n seleccionada
 * @pre Se asume que el valor del par?metro es correcto
 * @post Se almacena en el objeto la opci?n seleccionada
 */
void igvInterfaz::menuHandle(int value)
{
   // Gestionar Materiales
    if (value >= 100 && value <= 102) {
       _instancia->escena.setMaterialSuelo(value - 100);
    }
   // Gestionar Texturas
    else if (value == 200) {
       _instancia->escena.toggleTexturaSuelo(false);
    }
    else if (value >= 201 && value <= 203) {
       _instancia->escena.toggleTexturaSuelo(true);
       _instancia->escena.setTexturaSuelo(value - 201); // 0, 1, 2
    }
   // Gestionar Filtros
    else if (value >= 300 && value <= 303) {
       switch (value) {
          case 300: _instancia->escena.setFiltroTextura(igvEscena3D::FILTRO_NN); break;
          case 301: _instancia->escena.setFiltroTextura(igvEscena3D::FILTRO_NL); break;
          case 302: _instancia->escena.setFiltroTextura(igvEscena3D::FILTRO_LN); break;
          case 303: _instancia->escena.setFiltroTextura(igvEscena3D::FILTRO_LL); break;
       }
    }
   // Opciones anteriores
    else if (value == 998) {
       _instancia->animacionRobot = !_instancia->animacionRobot;
       if (_instancia->animacionRobot) _instancia->escena.startAnimacion();
       else _instancia->escena.stopAnimacion();
    }
    else if (value == 999) {
       _instancia->animacionCamara = !_instancia->animacionCamara;
    }

   glutPostRedisplay();
}

void igvInterfaz::idleFunc()
{
   bool huboCambios = false;
   if (_instancia->animacionCamara) {
      _instancia->camara.orbitY(0.18);
      huboCambios = true;

   }
   if (_instancia->animacionRobot) {
      _instancia->escena.animarRobot();
      huboCambios = true;
   }

   if (huboCambios) {
      glutPostRedisplay();
   }
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
   glutMouseFunc(mouseFunc);
   glutMotionFunc(motionFunc);
   glutIdleFunc(idleFunc);
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

/**
 * M?todo para aplicar una vista c?nica predefinida
 * @param v Tipo de vista c?nica a aplicar
 */
void igvInterfaz::aplicarVista(Vista v)
{
   vistaActual = v;
   switch (v)
   {
   case PLANTA: // desde arriba
      camara.set(IGV_PARALELA,
                 igvPunto3D(0,10,0), igvPunto3D(0,0,0), igvPunto3D(0,0,-1),
                 -5,5,-5,5, 0.1, 200.0);
      break;
   case ALZADO: // frontal
      camara.set(IGV_PARALELA,
                 igvPunto3D(0,0,10), igvPunto3D(0,0,0), igvPunto3D(0,1,0),
                 -5,5,-5,5, 0.1, 200.0);
      break;
   case PERFIL: // lateral
      camara.set(IGV_PARALELA,
                 igvPunto3D(10,0,0), igvPunto3D(0,0,0), igvPunto3D(0,1,0),
                 -5,5,-5,5, 0.1, 200.0);
      break;
   case OTRA:
      // vuelve a perspectiva por defecto
      camara.set(IGV_PERSPECTIVA,
                 igvPunto3D(3,2,4), igvPunto3D(0,0,0), igvPunto3D(0,1,0),
                 60.0, (alto_ventana? (double)ancho_ventana/alto_ventana : 1.0), 0.1, 200.0);
      break;
   }
}

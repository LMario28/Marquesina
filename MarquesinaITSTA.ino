// NOTAS:

// 1) 1 pixel = 2 LEDs

// 2) Margenes: Arriba: 1 LED, Abajo: 1 LED, Izquierda: 1 LED, Derecha: 1 LED

// 3) Los LED's de La columna izquierda de lo pixeles siempre suben

//# 4) El orden en los datos del tiempo (tupla) es:
//#    Año, mes (1-12), día del mes (1-31), hora (0-23), minuto (0-59), segundo (0-59), día de
//#    la semana (0-6 de lunes a domingo), día de año (1-366)

// 5) Pasos para actualizar el sketch en el ESP32

//    Requisitos: 1) Debe estar en donde está la aplicación el sketch ota.py;
//                2) En GitHub, repositorio LetreroInteractivo debe existir  un arhivo con nombre version.json con las lineas:
//                   {
//                     "version":2
//                   }
//                   El número de la versión debe ser mayor que el mismo archivo en el ESP32

//    a) Abrir www.github.com
//    b) lmmsegura@hotmail.com / le...24
//    c) Copiar la nueva versión del sketch a GitHub, repositorio Aro

// 6) Los timers no se borran bien. Por ese motivo al crear inicialmente los timers, el timer "Telegrama"
// se cambió de posición (se puso al final) De esa manera todo funionó bien. Este no fue una solución:
// fue un parche

// 7) Se cambio el orden de los colores en la línea:
//    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, LEDS_TIRA_TOTAL);
//    de RGB A GRB para que los colores en la tira correspondieran al orden en que se manejan en el
//    programa (rojo,verde,azul)

//#define CASA                                                                  //
#define OTA                                                                   //

#define BLYNK_TEMPLATE_ID "TMPL2VYVHE864"                                     // Estas tres líneas deben ir al inicio
#define BLYNK_TEMPLATE_NAME "Marquesina"                                      //
#if defined(CASA)                                                             //
  #define BLYNK_AUTH_TOKEN "42sf0EnY7AUO0ImoF1v7QpoCR-ls6pxL"                 //
#else                                                                         //
  #define BLYNK_AUTH_TOKEN "VeGakFgN63Yi23wrwAUIDuctKaAjZgUC"                 //
#endif                                                                        //
                                                                              //
////////////////////////////////////////////////////////////////////////////////
//                                INCLUDEs                                    //
////////////////////////////////////////////////////////////////////////////////
#include <WiFi.h>                                                             //
#include <WiFiClient.h>                                                       //
#include <BlynkSimpleEsp32.h>                                                 //
#if defined(OTA)                                                              //
  #include "ota_manager_classic.h"                                            //
#endif                                                                        //
#include <Preferences.h>                                                      //
#include <WidgetRTC.h>                                                        //
#include <TimeLib.h>                                                          //
#include <Arduino_JSON.h>                                                     //
#include <FastLED.h>                                                          //
#if defined(CASA)                                                             //
  #include "Alfabeto_7x5_5x3.h"                                               //
#else                                                                         //
  #include "Alfabeto_11x7_7x5.h"                                              //
#endif                                                                        //

////////////////////////////////////////////////////////////////////////////////
//                               CONSTANTES                                   //
////////////////////////////////////////////////////////////////////////////////
#if defined(CASA)                                                             //
  #define NUMERO_COLUMNAS_TOTAL 83                                            //
  #define LEDs_COLUMNA_TOTAL 16                                               //
  #define NUMERO_COLUMNAS_MENSAJE_MARQUESINA 81                               //
  #define LEDs_COLUMNA_MENSAJE 14                                             //
  #define TAMANO_PIXEL 2                                                      // Un pixel = 2 LEDs
  #define RESOLUCION_COLUMNA 7                                                //
  #define DATA_PIN 5                                                          //
#else                                                                         //
  #define NUMERO_COLUMNAS_TOTAL 132                                           //
  #define LEDs_COLUMNA_TOTAL 24                                               //
  #define NUMERO_COLUMNAS_MENSAJE_MARQUESINA 130                              //
  #define LEDs_COLUMNA_MENSAJE 22                                             //
  #define TAMANO_PIXEL 2                                                      // Un pixel = 2 LEDs
  #define RESOLUCION_COLUMNA 11                                               //
  #define DATA_PIN 47                                                         //
#endif                                                                        //
                                                                              //
const int LEDS_TIRA_TOTAL=NUMERO_COLUMNAS_TOTAL*LEDs_COLUMNA_TOTAL;           //
const int NUMERO_COLUMNAS_PIXELS=NUMERO_COLUMNAS_MENSAJE_MARQUESINA/TAMANO_PIXEL;//
#define TAMANO_ESPACIO_BLANCO 1                                               // 3 Pixeles
const int NUMERO_CARACTERES_ALFABETO = sizeof(alfabeto) / sizeof(alfabeto[0]);// Del caracter 32 al 248
                                                                              //
#define MODO_TODO 0                                                           //
#define MODO_TELEGRAMA 1                                                      //
#define MODO_RELOJ 2                                                          //
#define MODO_DATOS_AMBIENTALES 3                                              //
#define MODO_APAGAR 4                                                         //

#define MINIMA_VELOCIDAD_TEXTO 1.5                                            // Segundos
                                                                              //
#define NUMERO_LINEAS_TELEGRAMA 5                                             //
#define LONGITUD_MAXIMA_LINEA_TELEGRAMA 50                                    //
                                                                              //
const char *WIFI_SSID[]={"TP-Link_LMario_DHCP","INFINITUM2426_2.4"};          //
const char *WIFI_PASS[]={"lmario28","CNnC917MDE"};                            //
#define NUMERO_REDES_WIFI_REGISTRADAS (sizeof(WIFI_SSID)/sizeof(WIFI_SSID[0]))//
#define MAXIMA_LONGITUD_NOMBRE_RED 25                                         //
#define MAXIMA_LONGITUD_CLAVE_ACCESO_RED 15                                   //
char SSID[MAXIMA_LONGITUD_NOMBRE_RED];                                        //
char PASSWD[MAXIMA_LONGITUD_CLAVE_ACCESO_RED];                                //
#define MAXIMA_LONGITUD_CADENA 500                                            //
#define MAXIMA_LONGITUD_MENSAJE_CONSOLA 600                                   //
                                                                              //
#define TIEMPO_ESPERAR_CONEXION_WIFI 25                                       //
                                                                              //
#define ZONA_MEXICO -6                                                        //
                                                                              //
#define WATCHDOG False                                                        //
                                                                              //
#if defined(OTA)                                                              //
  #if defined(CASA)                                                           //
    #define VERSION_URL "https://raw.githubusercontent.com/LMario28/Marquesina/main/version.json"
    #define FIRMWARE_URL "https://raw.githubusercontent.com/LMario28/Marquesina/main/MarquesinaCasa.ino.esp32.bin"
  #else                                                                       //
    #define VERSION_URL "https://raw.githubusercontent.com/LMario28/Marquesina/main/version.json"
    #define FIRMWARE_URL "https://raw.githubusercontent.com/LMario28/Marquesina/main/MarquesinaITSTA.ino.lolin_s3_mini.bin"
  #endif                                                                      //
#endif                                                                        //

////////////////////////////////////////////////////////////////////////////////
//                                OBJETOS                                     //
////////////////////////////////////////////////////////////////////////////////
BlynkTimer timer;                                                             // Master timer
WidgetRTC rtc;                                                                //
CRGB leds[LEDS_TIRA_TOTAL];                                                   //
#if defined(OTA)                                                              //
  OTAManagerClassic otaManager(VERSION_URL, FIRMWARE_URL);                    //
#endif                                                                        //

////////////////////////////////////////////////////////////////////////////////
//                               VARIABLES                                    //
////////////////////////////////////////////////////////////////////////////////
boolean banderaPuertoSerialAbierto = false;                                   //
char mensajeConsola[MAXIMA_LONGITUD_MENSAJE_CONSOLA];                         // 600 para mensajes muy grandes
int contadorIntentosConectarWiFi;                                             //
boolean banderaConectadoBlynk=false;                                          //
                                                                              //
int timerHora=-1;                                                             //
int timerCorrerTextoMarquesina=-1;                                            //
                                                                              //
int modo=0,modoAnterior=-1;                                                   //
int modoTodo=0,modoTodoAnterior=-1;                                                   //
float velocidadTexto=0;                                                       //
int nivelColorRojo=0,nivelColorVerde=0,nivelColorAzul=0;                      //
int intensidadColorRojo=0,intensidadColorVerde=0,intensidadColorAzul=0;       //
float intensidadColorTexto=0.0;                                               //
char telegramaLinea[NUMERO_LINEAS_TELEGRAMA][LONGITUD_MAXIMA_LINEA_TELEGRAMA]=//
     {{""},{""},{""},{""},{""}};                                              //
char telegramaTexto[NUMERO_LINEAS_TELEGRAMA*LONGITUD_MAXIMA_LINEA_TELEGRAMA+4];//     +4: Separador ('|') de líneas de telegrama
char mensajeMarquesina[NUMERO_LINEAS_TELEGRAMA*LONGITUD_MAXIMA_LINEA_TELEGRAMA+4];//  +4: Separador ('|') de líneas de telegrama
char mensajeDatosAmbientales[50]="N.D.";                                      //
bool banderaIniciandoEntorno=true;                                            //
bool todoBanderaTelegramaActivo=false,todoBanderaRelojActivo=false,           //
     todoBanderaDatosAmbientalesActivos=false,                                //
     todoBanderaDiasPatriosFestividadesFinAnoActiva=false,                    //
     todoBanderaAnimacionesActivas=false;                                     //
int todoDuracionTelegrama=0,todoDuracionReloj=0,                              //
    todoDuracionDatosAmbientales=0,todoDuracionDiasPatriosFestividadesFinAno=0,//
    todoDuracionAnimaciones=0;                                                //
int tipLlaDesplegarMensajeMarquesina=1;                                       //
                                                                              //
int longitudMensajeColumnasMarquesina=0;                                      //
int *caracterEncabezadoColumnaLetreroVirtual,**letreroVirtual;                //
int columnaInicioCaracterLetreroVirtual;                                      //
bool banderaMensajeMovimiento=false;                                          //
bool banderaCambioModoPermitido = true;                                       //
int margenPixeles=0;                                                          //
int indiceBaseTiraPixel=0;                                                    //
int columnaBaseLetreroVirtual;                                                //
long tiempoInicialTarea=millis(),tiempoInicialTareaModoTodo=millis();         //
Preferences preferences;                                                      //
                                                                              //
int i,j;                                                                      //
////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
void setup()
//------------------------------------------------------------------------------
{
  Serial.begin(115200);
  delay(300);
  tiempoInicialTarea=millis();
  while (millis()-tiempoInicialTarea<=10000L)
  {
    if (Serial)                                                                 // Check if serial port is open
    {
       sprintf(mensajeConsola, "Puerto serie abierto");
       desplegarMensajeConsola();
       banderaPuertoSerialAbierto = true;
      break;
    }
    delay(25);
  }
  if (millis()-tiempoInicialTarea>10000L)
    banderaPuertoSerialAbierto = false;

  sprintf(mensajeConsola, "=== INICIANDO SISTEMA ===");
  desplegarMensajeConsola();
  sprintf(mensajeConsola, "Caracteres en el alfabeto: %d",sizeof(alfabeto)/sizeof(alfabeto[0]));
  desplegarMensajeConsola();

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, LEDS_TIRA_TOTAL);               // GRB ordering is typical

  // Set WiFi to station mode and disconnect from an AP if it was previously connected.
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  seleccionarMejorRedWiFiDisponible();

  // PASO 1: Conectar a WiFi
  sprintf(mensajeConsola,"Conectando a WiFi: %s",SSID);
  desplegarMensajeConsola();
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID,PASSWD);
  // Esperar conexión WiFi con indicador visual
  contadorIntentosConectarWiFi = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(2000);
    contadorIntentosConectarWiFi++;
    //desplegarMensajeVisualMarquesina(1);
    Serial.print(".");
    // Timeout después de NUMERO_COLUMNAS_MENSAJE_MARQUESINA/10*2 segundos
    if (contadorIntentosConectarWiFi>NUMERO_COLUMNAS_MENSAJE_MARQUESINA/10)
    {
      sprintf(mensajeConsola,"\n¡Error! No se pudo conectar al WiFi");
      desplegarMensajeConsola();
      sprintf(mensajeConsola,"Reiniciando el ESP32 en 5 segundos...");
      desplegarMensajeConsola();
      delay(5000);
      ESP.restart();
    }
  }
  sprintf(mensajeConsola,"\nConectado a la red: %s",SSID);
  desplegarMensajeConsola();
  desplegarMensajeVisualMarquesina(2);

  #if defined(OTA)
    if (otaManager.checkAndUpdate())
    {
      return;                                                          // Reiniciará si actualiza
    }
  #endif

  // PASO 2: Conectar a Blynk DESPUÉS
  sprintf(mensajeConsola,"\nConectando a Blynk...");
  desplegarMensajeConsola();
  // Configurar Blynk con el token (NO conecta automáticamente)
  Blynk.config(BLYNK_AUTH_TOKEN);
  // Intentar conexión con timeout
  if (Blynk.connect(30000))                                                      // Timeout de 5 segundos
  {
    sprintf(mensajeConsola,"¡Conectado a Blynk!");
    desplegarMensajeConsola();
  }
  else
  {
    sprintf(mensajeConsola,"Error conectando a Blynk. Se reintentará en segundo plano.");
    desplegarMensajeConsola();
  }

  // Crear timers
  timer.setTimeout(3600000L, [] () {} ); // dummy/sacrificial Function
  timer.setInterval(300010L,solicitarDatosAmbientalesInternet);                 // Timer indice 1
  timerHora = timer.setInterval(1000L,desplegarHora);
  timer.disable(timerHora);
  timerCorrerTextoMarquesina = timer.setInterval(100L,correrTextoMarquesina);
  timer.disable(timerCorrerTextoMarquesina);
}
////////////////////////////////////////////////////////////////////////////////
//                                  PROCESO                                   //
////////////////////////////////////////////////////////////////////////////////

void loop()
{
  timer.run();
  Blynk.run();

  if(modo!=modoAnterior)
  {
    detenerTimers();

    if(modo==MODO_TODO)                                                       // Todo
    {
      strcpy(mensajeMarquesina,telegramaTexto);
      modoTodo = 1;
      modoTodoAnterior = -1;
      tiempoInicialTareaModoTodo = millis();
      todo();
    }
    else if(modo==MODO_TELEGRAMA)                                                // Telegrama
    {
      desplegarTelegrama();
    }
    else if(modo==MODO_RELOJ)                                                      // Hora
    {
      timer.enable(timerHora);
      sprintf(mensajeConsola, "Desplegando la hora (timerHora activado)");
      desplegarMensajeConsola();
    }
    else if(modo==MODO_DATOS_AMBIENTALES)                                     // Datos Ambientales
    {
      desplegarDatosAmbientales();
    }
    modoAnterior = modo;
  }
  else
  {
    if(modo==MODO_TODO)
      todo();
  }
}

////////////////////////////////////////////////////////////////////////////////
//                                 FUNCIONES                                  //
////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
void seleccionarMejorRedWiFiDisponible()
//------------------------------------------------------------------------------
{
  // Buscar las redes disponibles
  sprintf(mensajeConsola, "\n-------------------------");
  desplegarMensajeConsola();
  sprintf(mensajeConsola, "** Buscando redes WiFi **");
  desplegarMensajeConsola();
  sprintf(mensajeConsola, "-------------------------");
  desplegarMensajeConsola();
 
  int numeroTotalRedesWiFi = WiFi.scanNetworks();
  int rssi,rssiMasFuerte=-999;
  int j;

  if (numeroTotalRedesWiFi == -1)
  {
    sprintf(mensajeConsola, "\nEn encontrarMejorRedWiFi(): NO HAY REDES WIFI DISPONIBLES");
    desplegarMensajeConsola();
    return;
  }
  sprintf(mensajeConsola, "Se encontraron %d redes WiFi",numeroTotalRedesWiFi);
  desplegarMensajeConsola();

  for (int i=0;i<numeroTotalRedesWiFi;i++)
  {
    rssi = WiFi.RSSI(i);
    for (j=0;j<NUMERO_REDES_WIFI_REGISTRADAS;j++)
      if(strcmp((WiFi.SSID(i)).c_str(),WIFI_SSID[j])==0)
        break;
    if(j<NUMERO_REDES_WIFI_REGISTRADAS)
    {
      if(rssi>rssiMasFuerte)
      {
        strcpy(SSID,WIFI_SSID[j]);
        strcpy(PASSWD,WIFI_PASS[j]);
        rssiMasFuerte = rssi;
      }
    }
  }
  sprintf(mensajeConsola, "La mejor es la red: %s",SSID);
  desplegarMensajeConsola();
}

//------------------------------------------------------------------------------
void solicitarDatosAmbientalesInternet()
//------------------------------------------------------------------------------
{
  Blynk.virtualWrite(V81,1);

  sprintf(mensajeConsola, "Solicitando datos ambientales");
  desplegarMensajeConsola();
}

//------------------------------------------------------------------------------
void detenerTimers()
//------------------------------------------------------------------------------
{
  timer.disable(timerHora);
  timer.disable(timerCorrerTextoMarquesina);

  sprintf(mensajeConsola, "Timers detenidos");
  desplegarMensajeConsola();
}

//------------------------------------------------------------------------------
void todo()
//------------------------------------------------------------------------------
{
  if(modoTodo!=modoTodoAnterior)
  {
    detenerTimers();
    // sprintf(mensajeConsola,"modoTodo = %d; todoBanderaTelegramaActivo= %d; todoBanderaRelojActivo = %d;todoBanderaDatosAmbientalesActivos = %d",
             // modoTodo,todoBanderaTelegramaActivo,todoBanderaRelojActivo,todoBanderaDatosAmbientalesActivos);
    //desplegarMensajeConsola();
    if(modoTodo==1)                                                             // Telegrama
    {
      if(todoBanderaTelegramaActivo)
      {
        desplegarTelegrama();
        modoTodoAnterior = 1;
      }
      else
        modoTodo = 2;
    }
    else if(modoTodo==2)                                                        // Hora
    {
      if(todoBanderaRelojActivo)
      {
        timer.enable(timerHora);
        sprintf(mensajeConsola, "Desplegando la hora (timerHora activado)");
        desplegarMensajeConsola();
        modoTodoAnterior = 2;
      }
      else
        modoTodo = 3;
    }
    else if(modoTodo==3)                                                        // Datos Aambientales
    {
      if(todoBanderaDatosAmbientalesActivos)
      {
        desplegarDatosAmbientales();
        modoTodoAnterior = 3;
      }
      else
        modoTodo = 1;
    }
    //modoTodoAnterior = modoTodo;
  }
  else
  {
    if(modoTodo==1)                                                             // Telegrama
    {
      if(millis()-tiempoInicialTareaModoTodo>todoDuracionTelegrama*1000&&banderaCambioModoPermitido)
      {
//sprintf(mensajeConsola, "modo = %d; modoTodo = %d; millis()-tiempoInicialTareaModoTodos = %d s", modo, modoTodo, (millis()-tiempoInicialTareaModoTodo)/1000);
//desplegarMensajeConsola();
        if(todoBanderaRelojActivo||todoBanderaDatosAmbientalesActivos)
        {
          modoTodo = 2;
          modoTodoAnterior = 1;
        }
        tiempoInicialTareaModoTodo = millis();
      }
    }
    else if(modoTodo==2)                                                        // Hora
    {
      if(millis()-tiempoInicialTareaModoTodo>todoDuracionReloj*1000)
      {
//sprintf(mensajeConsola, "modo = %d; modoTodo = %d; millis()-tiempoInicialTareaModoTodos = %d s",modo, modoTodo, (millis()-tiempoInicialTareaModoTodo)/1000);
//desplegarMensajeConsola();
        if(todoBanderaTelegramaActivo||todoBanderaDatosAmbientalesActivos)
        {
          modoTodo = 3;
          modoTodoAnterior = 2;
        }
        tiempoInicialTareaModoTodo = millis();
      }
    }
    else if(modoTodo==3)                                                        // Datos Ambientales
    {
      if(millis()-tiempoInicialTareaModoTodo>todoDuracionDatosAmbientales*1000&&banderaCambioModoPermitido)
      {
//sprintf(mensajeConsola, "Columnas letrero virtual = 267; tiempo desplegar datos ambientales = ",(millis()-tiempoInicialTareaModoTodo)/1000);
//desplegarMensajeConsola();
//sprintf(mensajeConsola, "modo = %d; modoTodo = %d; millis()-tiempoInicialTareaModoTodos = %d s",modo, modoTodo, (millis()-tiempoInicialTareaModoTodo)/1000);
//desplegarMensajeConsola();
        if(todoBanderaTelegramaActivo||todoBanderaRelojActivo)
        {
          modoTodo = 1;
          modoTodoAnterior = 3;
        }
        tiempoInicialTareaModoTodo = millis();
      }
    }
  }
}

//------------------------------------------------------------------------------
void desplegarTelegrama()
//------------------------------------------------------------------------------
{
  sprintf(mensajeConsola, "Desplegando el telegrama: %s",telegramaTexto);
  desplegarMensajeConsola();

  strcpy(mensajeMarquesina,telegramaTexto);
  tipLlaDesplegarMensajeMarquesina = 1;
  desplegarMensajeMarquesina();
}

//------------------------------------------------------------------------------
void desplegarHora()
//------------------------------------------------------------------------------
{
  sprintf(mensajeMarquesina, "%02d:%02d:%02d",hour(),minute(),second());

  tipLlaDesplegarMensajeMarquesina = 1;
  desplegarMensajeMarquesina();
}


//------------------------------------------------------------------------------
void desplegarDatosAmbientales()
//------------------------------------------------------------------------------
{
  strcpy(mensajeMarquesina,mensajeDatosAmbientales);

  sprintf(mensajeConsola, "Desplegando los datos ambientales: %s",mensajeMarquesina);
  desplegarMensajeConsola();

  tipLlaDesplegarMensajeMarquesina = 1;
  desplegarMensajeMarquesina();
}

//------------------------------------------------------------------------------
void desplegarMensajeMarquesina()
//------------------------------------------------------------------------------
{
  if(tipLlaDesplegarMensajeMarquesina==1)
    desplegarMensajeMarquesinaInicial();
}

//------------------------------------------------------------------------------
void desplegarMensajeMarquesinaInicial()
//------------------------------------------------------------------------------
{
  int i,j;

//  sprintf(mensajeConsola, "Desplegando en la marquesina: %s",mensajeMarquesina);
//  desplegarMensajeConsola();

  // IMPORTANTE: Si existe memoria asignada de un mensaje anterior, liberarla
  if(letreroVirtual!=NULL)
  {
    free(letreroVirtual);
    for(i=0;i<RESOLUCION_COLUMNA;i++)
      free(letreroVirtual[i]);
    free(caracterEncabezadoColumnaLetreroVirtual);
  }

  longitudMensajeColumnasMarquesina = 0;
  for (i=0;i<strlen(mensajeMarquesina);)
  {
    unsigned char caracterASCII=mensajeMarquesina[i];
    char caracterUTF8[4];
    int longitudCaracterUTF8Bytes=1;
      
    // Determinar longitud del carácter UTF-8
    if ((caracterASCII & 0xE0) == 0xC0) longitudCaracterUTF8Bytes = 2;
    else if ((caracterASCII & 0xF0) == 0xE0) longitudCaracterUTF8Bytes = 3;
    else if ((caracterASCII & 0xF8) == 0xF0) longitudCaracterUTF8Bytes = 4;
      
    // Imprimir el carácter completo
    for (j = 0; j < longitudCaracterUTF8Bytes && (i + j) < strlen(mensajeMarquesina); j++)
      caracterUTF8[j] = mensajeMarquesina[i + j];
    caracterUTF8[j] = '\0';
    //Serial.println(caracterUTF8);
    // Calcular la longitud del telegrama en columnas de la marquesina
    calcularLongitudTelegramaColumnasMarquesina(caracterUTF8);
    longitudMensajeColumnasMarquesina++;     // Espacio entre caracteres

    //sprintf(mensajeConsola, "Longitud del telegrama en columnas de la marquesina (parcial): %d",longitudMensajeColumnasMarquesina);
    //desplegarMensajeConsola();

    i += longitudCaracterUTF8Bytes;
  }
  longitudMensajeColumnasMarquesina--;     // Restar espacio asignado al último caracter

  //sprintf(mensajeConsola, "Longitud del telegrama en columnas de la marquesina: %d",longitudMensajeColumnasMarquesina);
  //desplegarMensajeConsola();

  // Crear el letrero virtual
  caracterEncabezadoColumnaLetreroVirtual = (int*)malloc(longitudMensajeColumnasMarquesina*sizeof(int));
  letreroVirtual = (int**)malloc(RESOLUCION_COLUMNA * sizeof(int*));
  for(i=0;i<RESOLUCION_COLUMNA;i++)
    letreroVirtual[i] = (int*)malloc(longitudMensajeColumnasMarquesina*sizeof(int));

  // Inicializar con ceros (opcional, ya que malloc no inicializa)
  for(i=0;i<longitudMensajeColumnasMarquesina;i++)
    caracterEncabezadoColumnaLetreroVirtual[i] = 0;
  for(i=0;i<RESOLUCION_COLUMNA;i++)
  {
    for(j=0;j<longitudMensajeColumnasMarquesina;j++)
      letreroVirtual[i][j] = 0;
  }

  // Llenar letrero virtual
  columnaInicioCaracterLetreroVirtual = 0;
  for (i=0;i<strlen(mensajeMarquesina);)
  {
    unsigned char caracterASCII=mensajeMarquesina[i];
    char caracterUTF8[4];
    int longitudCaracterUTF8Bytes=1;
      
    // Determinar longitud del carácter UTF-8
    if ((caracterASCII & 0xE0) == 0xC0) longitudCaracterUTF8Bytes = 2;
    else if ((caracterASCII & 0xF0) == 0xE0) longitudCaracterUTF8Bytes = 3;
    else if ((caracterASCII & 0xF8) == 0xF0) longitudCaracterUTF8Bytes = 4;
  
    // Formar caracter UTF8
    for (j = 0; j < longitudCaracterUTF8Bytes && (i + j) < strlen(mensajeMarquesina); j++)
      caracterUTF8[j] = mensajeMarquesina[i + j];
    caracterUTF8[j] = '\0';

    if(strcmp(caracterUTF8,"|")==0)
      caracterEncabezadoColumnaLetreroVirtual[columnaInicioCaracterLetreroVirtual] = 1;

    // Incorporar el patron del caracter al letrero virtual
    incorporarPatronLetreroVirtual(caracterUTF8);

    i += longitudCaracterUTF8Bytes;
  }

//  // Imprimir letrero virtual
//  for(i=0;i<longitudMensajeColumnasMarquesina;i++)
//    Serial.print(caracterEncabezadoColumnaLetreroVirtual[i]);
//  Serial.println();
//  for(i=0;i<RESOLUCION_COLUMNA;i++)
//  {
//    for(j=0;j<longitudMensajeColumnasMarquesina;j++)
//      Serial.print(letreroVirtual[i][j]);
//    Serial.println();
//  }

  // DESPLEGAR EL MENSAJE EN LA MARQUESINA
  if(longitudMensajeColumnasMarquesina<=NUMERO_COLUMNAS_PIXELS)
  {
    margenPixeles = (NUMERO_COLUMNAS_MENSAJE_MARQUESINA - longitudMensajeColumnasMarquesina * TAMANO_PIXEL) / 2 / TAMANO_PIXEL;
    banderaMensajeMovimiento = false;
    banderaCambioModoPermitido = true;
  }
  else
  {
    margenPixeles = 0;
    timer.enable(timerCorrerTextoMarquesina);
    sprintf(mensajeConsola, "timerCorrerTextoMarquesina activado");
    desplegarMensajeConsola();
    banderaMensajeMovimiento = true;
    banderaCambioModoPermitido = false;
  }
//  sprintf(mensajeConsola, "longitudMensajeColumnasMarquesina: %d; margen en pixeles: %d; banderaMensajeMovimiento: %d",
//           longitudMensajeColumnasMarquesina,margenPixeles,banderaMensajeMovimiento);
//  desplegarMensajeConsola();

  // DESPLEGAR MENSAJE
  fill_solid(leds,LEDS_TIRA_TOTAL,CRGB(0,0,0));
  //senalarEstadoAplicacion()
  for(j=0;j<longitudMensajeColumnasMarquesina;j++)
  {
    indiceBaseTiraPixel = (1 + TAMANO_PIXEL * (margenPixeles + j)) * LEDs_COLUMNA_TOTAL  + 1;     // Los 1 son para saltar la primera columna y la primera fila de LEDs

    if(indiceBaseTiraPixel>=LEDS_TIRA_TOTAL)
      break;

//    sprintf(mensajeConsola, "indiceBaseTiraPixel: %d ",indiceBaseTiraPixel);
//    desplegarMensajeConsola();

    for(i=0;i<RESOLUCION_COLUMNA;i++)
    {
      //sprintf(mensajeConsola, "indiceBaseTiraPixel+2*i: %d ",indiceBaseTiraPixel+2*i);
      //desplegarMensajeConsola();
      if(caracterEncabezadoColumnaLetreroVirtual[j]!=1)
      {
         if(letreroVirtual[i][j]==1)
         {
            // LED izquierda arriba
            leds[indiceBaseTiraPixel+2*i] = CRGB(intensidadColorRojo,intensidadColorVerde,intensidadColorAzul);
            if(TAMANO_PIXEL*(margenPixeles+j)<NUMERO_COLUMNAS_MENSAJE_MARQUESINA)
              // LED derecha arriba
              leds[indiceBaseTiraPixel+2*(LEDs_COLUMNA_TOTAL-1-i)-1] = CRGB(intensidadColorRojo,intensidadColorVerde,intensidadColorAzul);
            // LED izquierda abajo
            leds[indiceBaseTiraPixel+2*i+1] = CRGB(intensidadColorRojo,intensidadColorVerde,intensidadColorAzul);
            if(TAMANO_PIXEL*(margenPixeles+j)+1<NUMERO_COLUMNAS_MENSAJE_MARQUESINA)
              // LED derecha abajo
              leds[indiceBaseTiraPixel+2*(LEDs_COLUMNA_TOTAL-1-i)-2] = CRGB(intensidadColorRojo,intensidadColorVerde,intensidadColorAzul);
         }
      }
      else
      {
         // LED izquierda arriba
         leds[indiceBaseTiraPixel+2*i] = CRGB(10,10,0);
         if(TAMANO_PIXEL*(margenPixeles+j)<NUMERO_COLUMNAS_MENSAJE_MARQUESINA)
           // LED derecha arriba
           leds[indiceBaseTiraPixel+2*(LEDs_COLUMNA_TOTAL-1-i)-1] = CRGB(10,10,0);
         // LED izquierda abajo
         leds[indiceBaseTiraPixel+2*i+1] = CRGB(10,10,0);
         if(TAMANO_PIXEL*(margenPixeles+j)+1<NUMERO_COLUMNAS_MENSAJE_MARQUESINA)
           // LED derecha abajo
           leds[indiceBaseTiraPixel+2*(LEDs_COLUMNA_TOTAL-1-i)-2] = CRGB(10,10,0);
         if(indiceBaseTiraPixel>=LEDS_TIRA_TOTAL)
           break;
      }
    }
  }
  FastLED.show();

  if(banderaMensajeMovimiento)
    columnaBaseLetreroVirtual = 1;

  tiempoInicialTarea = millis();
}

//------------------------------------------------------------------------------
void correrTextoMarquesina()
//------------------------------------------------------------------------------
{
  if(!banderaMensajeMovimiento)
    return;
  if(millis()-tiempoInicialTarea<velocidadTexto*1000)
    return;

  int i,j;

  //senalarEstadoAplicacion()
  fill_solid(leds,LEDS_TIRA_TOTAL,CRGB(0,0,0));
  for(j=1;j<NUMERO_COLUMNAS_MENSAJE_MARQUESINA+1;j=j+TAMANO_PIXEL)              // Empieza en j=1 para saltarse la primera columna
  {
    if(columnaBaseLetreroVirtual+j/TAMANO_PIXEL+1>longitudMensajeColumnasMarquesina)          // +1 ¿?
      break;

    indiceBaseTiraPixel = j * LEDs_COLUMNA_TOTAL + 1;                            // +1 saltar la primera fila

    if(indiceBaseTiraPixel>=LEDS_TIRA_TOTAL)
      break;
    //if(j==1)
    //{
      //sprintf(mensajeConsola, "columnaBaseLetreroVirtual: %d; indiceBaseTiraPixel: %d ",columnaBaseLetreroVirtual,indiceBaseTiraPixel);
      //desplegarMensajeConsola();
    //}

    for(i=0;i<LEDs_COLUMNA_MENSAJE;i+=TAMANO_PIXEL)
    {
      //sprintf(mensajeConsola, "i= %d; j =%d",i,j);
      //desplegarMensajeConsola();
      if(caracterEncabezadoColumnaLetreroVirtual[columnaBaseLetreroVirtual+j/TAMANO_PIXEL]!=1)
      {
        if(letreroVirtual[i/TAMANO_PIXEL][columnaBaseLetreroVirtual+j/TAMANO_PIXEL]==1)
        {
          // LED izquierda arriba
          leds[indiceBaseTiraPixel+i] = CRGB(intensidadColorRojo,intensidadColorVerde,intensidadColorAzul);
          // LED derecha arriba
          if(j+1<NUMERO_COLUMNAS_MENSAJE_MARQUESINA)
            leds[indiceBaseTiraPixel+TAMANO_PIXEL*LEDs_COLUMNA_TOTAL-2-i-1] = CRGB(intensidadColorRojo,intensidadColorVerde,intensidadColorAzul);
          // LED izquierda abajo
          leds[indiceBaseTiraPixel+i+1] = CRGB(intensidadColorRojo,intensidadColorVerde,intensidadColorAzul);
          // LED derecha abajo
          if(j+1<NUMERO_COLUMNAS_MENSAJE_MARQUESINA)
            leds[indiceBaseTiraPixel+TAMANO_PIXEL*LEDs_COLUMNA_TOTAL-2-i-2] = CRGB(intensidadColorRojo,intensidadColorVerde,intensidadColorAzul);

          //if(i==0||i==2||
             //indiceBaseTiraPixel+TAMANO_PIXEL*(LEDs_COLUMNA_TOTAL-1-i)-1>1320||
             //indiceBaseTiraPixel+TAMANO_PIXEL*(LEDs_COLUMNA_TOTAL-1-i)-2>1320)
          //{
            //sprintf(mensajeConsola, "j = %d; i = %d; LED izquierda arriba: %d; LED derecha arriba: %d LED izquierda abajo: %d; LED derecha abajo: %d",j,i,
                     //indiceBaseTiraPixel+i,indiceBaseTiraPixel+TAMANO_PIXEL*(LEDs_COLUMNA_TOTAL-1-i)-1,
                     //indiceBaseTiraPixel+i+1,indiceBaseTiraPixel+TAMANO_PIXEL*(LEDs_COLUMNA_TOTAL-1-i)-2);
            //desplegarMensajeConsola();
          //}
        }
      }
      else
      {
        // LED izquierda arriba
        leds[indiceBaseTiraPixel+i] = CRGB(10,10,0);
        // LED derecha arriba
        if(j+1<NUMERO_COLUMNAS_MENSAJE_MARQUESINA)
          leds[indiceBaseTiraPixel+TAMANO_PIXEL*LEDs_COLUMNA_TOTAL-2-i-1] = CRGB(10,10,0);
        // LED izquierda abajo
        leds[indiceBaseTiraPixel+i+1] = CRGB(10,10,0);
        // LED derecha abajo
        if(j+1<NUMERO_COLUMNAS_MENSAJE_MARQUESINA)
          leds[indiceBaseTiraPixel+TAMANO_PIXEL*LEDs_COLUMNA_TOTAL-2-i-2] = CRGB(10,10,0);
      }
    }
  }

  FastLED.show();

  columnaBaseLetreroVirtual += 1;
  if(columnaBaseLetreroVirtual==1)
  {
    tiempoInicialTarea = millis();
    banderaCambioModoPermitido = false;
  }
  if(columnaBaseLetreroVirtual>=longitudMensajeColumnasMarquesina)
  {
    columnaBaseLetreroVirtual = 0;
    banderaCambioModoPermitido = true;
  }
//  sprintf(mensajeConsola, "columnaBaseLetreroVirtual: %d",columnaBaseLetreroVirtual);
//  desplegarMensajeConsola();
}

// Función para calcular la longitud del telegrama en columnas marquesina
//------------------------------------------------------------------------------
void calcularLongitudTelegramaColumnasMarquesina(char *c)
//------------------------------------------------------------------------------
{
  CaracterFuente *caracter = buscarCaracter(c);
  longitudMensajeColumnasMarquesina += caracter->anchoCaracter;
  return;
}

// Función para obtener el patrón de un carácter
//------------------------------------------------------------------------------
void incorporarPatronLetreroVirtual(char *c)
//------------------------------------------------------------------------------
{
  int i,j;
  CaracterFuente *caracter = buscarCaracter(c);
  int anchoCaracterVariable;

  for(i=0;i<RESOLUCION_COLUMNA;i++)
  {
    anchoCaracterVariable = caracter->anchoCaracter;
    //if(i==0)
    //{
      //Serial.print("Incorporando patrón en el letreroVirtual: Ancho del caracter: ");    Serial.println(anchoCaracterVariable);
      //Serial.print("Incorporando patron en el letreroVirtual: Patrón del caracter: ");   Serial.println(c);
      //Serial.print("Columna inicio caracter en el letrero virtual: ");                   Serial.println(columnaInicioCaracterLetreroVirtual);
    //}
    //Serial.println(caracter->patron[i],BIN);
    for(j=0;j<anchoCaracterVariable;j++)
      letreroVirtual[i][columnaInicioCaracterLetreroVirtual+j] = bitRead(caracter->patron[i],7-j);
  }
  columnaInicioCaracterLetreroVirtual += anchoCaracterVariable;
  columnaInicioCaracterLetreroVirtual++;     // Espacio entre caracteres
}

// Función para buscar un carácter en la fuente
CaracterFuente *buscarCaracter(char *c)
{
  int i;
  for (i = 0; i < NUMERO_CARACTERES_ALFABETO; i++)
  {
    //Serial.print(c);   Serial.print(" || ");   Serial.println(fuente[i].caracter);
    if (strcmp(c,alfabeto[i].caracter)==0)
    {
      //Serial.print("\nHallado ");   Serial.println(c);
      return &alfabeto[i];
    }
  }
  // Si no se encuentra, devolver espacio
  //Serial.print("\nNO hallado ");   Serial.println(c);
  return &alfabeto[0];
}

////////////////////////////////////////////////////////////////////////////////
//                             FIN DE FUNCIONES                               //
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//                            FUNCIONES BLYNK                                 //
////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
BLYNK_CONNECTED()
//------------------------------------------------------------------------------
{
  sprintf(mensajeConsola, "Conectado a Blynk");
  desplegarMensajeConsola();
  desplegarMensajeVisualMarquesina(3);

  Blynk.syncVirtual(V0,V1,V2,V3,V4,V5,V6,   V21,V22,V23,V24,V25,V26,   V41,V42,V43,V44,V45,V46);

  rtc.begin();

  solicitarDatosAmbientalesInternet();

  banderaConectadoBlynk=true;
}

//------------------------------------------------------------------------------
BLYNK_DISCONNECTED()
//------------------------------------------------------------------------------
{
  sprintf(mensajeConsola, "Desconectado de Blynk");
  desplegarMensajeConsola();

  banderaConectadoBlynk=false;
}

//------------------------------------------------------------------------------
BLYNK_WRITE(V0)     // MODO
//------------------------------------------------------------------------------
{
  modo = param.asInt();

  sprintf(mensajeConsola, "Nuevo valor para la variable modo (V0): %d",modo);
  desplegarMensajeConsola();

  // NOTA: El cambio de despliegue se realiza en loop()
}

//------------------------------------------------------------------------------
BLYNK_WRITE(V1)     // VELOCIDAD TEXTO
//------------------------------------------------------------------------------
{
  velocidadTexto = param.asFloat();

  sprintf(mensajeConsola, "Nuevo velocidad del Texto (V1): %.1f",velocidadTexto);
  desplegarMensajeConsola();

  timer.deleteTimer(timerCorrerTextoMarquesina);
  timerCorrerTextoMarquesina = timer.setInterval(long(velocidadTexto*1000),correrTextoMarquesina);

  sprintf(mensajeConsola, "Nuevo timerCorrerTextoMarquesina recreado");
  desplegarMensajeConsola();
}

//------------------------------------------------------------------------------
BLYNK_WRITE(V2)     // NIVEL COLOR ROJO (0-255)
//------------------------------------------------------------------------------
{
  nivelColorRojo = param.asInt();

  sprintf(mensajeConsola, "Nuevo nivel color rojo (0-255) (V2): %d",nivelColorRojo);
  desplegarMensajeConsola();

  // NOTA: El cambio se realiza cuando se oprime el botón OK en la App
}

//------------------------------------------------------------------------------
BLYNK_WRITE(V3)     // NIVEL COLOR VERDE (0-255)
//------------------------------------------------------------------------------
{
  nivelColorVerde = param.asInt();

  sprintf(mensajeConsola, "Nuevo nivel color verde (0-255) (V3): %d",nivelColorVerde);
  desplegarMensajeConsola();

  // NOTA: El cambio se realiza cuando se oprime el botón OK en la App
}

//------------------------------------------------------------------------------
BLYNK_WRITE(V4)     // NIVEL COLOR AZUL (0-255)
//------------------------------------------------------------------------------
{
  nivelColorAzul = param.asInt();

  sprintf(mensajeConsola, "Nuevo nivel color azul (0-255) (V4): %d",nivelColorAzul);
  desplegarMensajeConsola();

  // NOTA: El cambio se realiza cuando se oprime el botón OK en la App
}

//------------------------------------------------------------------------------
BLYNK_WRITE(V6)     // ACTUALIZAR COLOR TEXTO
//------------------------------------------------------------------------------
{
  if(param.asInt()==1)
  {
    intensidadColorRojo = intensidadColorTexto * nivelColorRojo;
    intensidadColorVerde = intensidadColorTexto * nivelColorVerde;
    intensidadColorAzul = intensidadColorTexto * nivelColorAzul;

    sprintf(mensajeConsola, "Nuevo color texto (V6): (%d,%d,%d)",intensidadColorRojo,intensidadColorVerde,
             intensidadColorAzul);
    desplegarMensajeConsola();

    if(!banderaIniciandoEntorno)
    {
      tipLlaDesplegarMensajeMarquesina = 1;
      desplegarMensajeMarquesina();
    }
  }
}

//------------------------------------------------------------------------------
BLYNK_WRITE(V5)     // INTENSIDAD COLOR TEXTO (0-100)
//------------------------------------------------------------------------------
{
  intensidadColorTexto = param.asFloat() / 100;

  sprintf(mensajeConsola, "Nuevo nivel intensidad color texto (0-1) (V5): %.3f",intensidadColorTexto);
  desplegarMensajeConsola();

  intensidadColorRojo = intensidadColorTexto * nivelColorRojo;
  intensidadColorVerde = intensidadColorTexto * nivelColorVerde;
  intensidadColorAzul = intensidadColorTexto * nivelColorAzul;

  if(!banderaIniciandoEntorno)
  {
    tipLlaDesplegarMensajeMarquesina = 1;
    desplegarMensajeMarquesina();
  }
}

//------------------------------------------------------------------------------
BLYNK_WRITE(V21)     // ACTUALIZAR PARTE 1 DEL TELEGRAMA
//------------------------------------------------------------------------------
{
  strcpy(telegramaLinea[0],param.asStr());

  sprintf(mensajeConsola, "Nuevo valor para la primera línea del telegrama (V21): %s",telegramaLinea[0]);
  desplegarMensajeConsola();
}

//------------------------------------------------------------------------------
BLYNK_WRITE(V22)     // ACTUALIZAR PARTE 2 DEL TELEGRAMA
//------------------------------------------------------------------------------
{
  strcpy(telegramaLinea[1],param.asStr());

  sprintf(mensajeConsola, "Nuevo valor para la segunda línea del telegrama (V22): %s",telegramaLinea[1]);
  desplegarMensajeConsola();
}

//------------------------------------------------------------------------------
BLYNK_WRITE(V23)     // ACTUALIZAR PARTE 3 DEL TELEGRAMA
//------------------------------------------------------------------------------
{
  strcpy(telegramaLinea[2],param.asStr());

  sprintf(mensajeConsola, "Nuevo valor para la tercera línea del telegrama (V23): %s",telegramaLinea[2]);
  desplegarMensajeConsola();
}
//------------------------------------------------------------------------------
BLYNK_WRITE(V24)     // ACTUALIZAR PARTE 4 DEL TELEGRAMA
//------------------------------------------------------------------------------
{
  strcpy(telegramaLinea[3],param.asStr());

  sprintf(mensajeConsola, "Nuevo valor para la cuarta línea del telegrama (V24): %s",telegramaLinea[3]);
  desplegarMensajeConsola();
}

//------------------------------------------------------------------------------
BLYNK_WRITE(V25)     // ACTUALIZAR PARTE 5 DEL TELEGRAMA
//------------------------------------------------------------------------------
{
  strcpy(telegramaLinea[4],param.asStr());

  sprintf(mensajeConsola, "Nuevo valor para la quinta línea del telegrama (V25): %s",telegramaLinea[4]);
  desplegarMensajeConsola();
}

//------------------------------------------------------------------------------
BLYNK_WRITE(V26)     // FORMAR EL TELEGRAMA
//------------------------------------------------------------------------------
{
  formarTelegrama();

  if(!banderaIniciandoEntorno)
  {
    tipLlaDesplegarMensajeMarquesina = 1;
    desplegarMensajeMarquesina();
  }
  else
    banderaIniciandoEntorno = false;
}

//------------------------------------------------------------------------------
void formarTelegrama()
//------------------------------------------------------------------------------
{
   telegramaTexto[0] = '\0';
  for(int i=0;i<NUMERO_LINEAS_TELEGRAMA;i++)
  {
    if(strlen(telegramaLinea[i])>0&&strcmp(telegramaLinea[i],"-"))
    {
      if(i>0)
        strcat(telegramaTexto,"|");
      strcat(telegramaTexto,telegramaLinea[i]);
    }
  }

  sprintf(mensajeConsola, "Nuevo telegrama (V26): %s",telegramaTexto);
  desplegarMensajeConsola();

  if(modo==MODO_TELEGRAMA)
    strcpy(mensajeMarquesina,telegramaTexto);

  if(!banderaIniciandoEntorno)
  {
    tipLlaDesplegarMensajeMarquesina = 1;
    desplegarMensajeMarquesina();
  }
}

//------------------------------------------------------------------------------
BLYNK_WRITE(V41)     // PARA TODO, ¿ESTA EL TELEGRAMA ACTIVO?
//------------------------------------------------------------------------------
{
  todoBanderaTelegramaActivo = param.asInt() == 1?true:false;

  sprintf(mensajeConsola, "¿En TODO, telegrama activo?: %d",todoBanderaTelegramaActivo);
  desplegarMensajeConsola();
}

//------------------------------------------------------------------------------
BLYNK_WRITE(V42)     // PARA TODO, DURACION DEL DESPLEGADO DEL TELEGRAMA
//------------------------------------------------------------------------------
{
  todoDuracionTelegrama = param.asInt();

  sprintf(mensajeConsola, "Duración del desplegado del telegrama: %d",todoDuracionTelegrama);
  desplegarMensajeConsola();
}

//------------------------------------------------------------------------------
BLYNK_WRITE(V43)     // PARA TODO, ¿ESTA EL RELOJ ACTIVO?
//------------------------------------------------------------------------------
{
  todoBanderaRelojActivo = param.asInt()==1?true:false;

  sprintf(mensajeConsola, "¿En TODO, reloj activo?: %d",todoBanderaRelojActivo);
  desplegarMensajeConsola();
}

//------------------------------------------------------------------------------
BLYNK_WRITE(V44)     // PARA TODO, DURACION DEL DESPLEGADO DEL RELOJ
//------------------------------------------------------------------------------
{
  todoDuracionReloj = param.asInt();

  sprintf(mensajeConsola, "Duración del desplegado del reloj: %d",todoDuracionReloj);
  desplegarMensajeConsola();
}

//------------------------------------------------------------------------------
BLYNK_WRITE(V45)     // PARA TODO, ¿ESTAN LOS DATOS AMBIENTALES ACTIVOS?
//------------------------------------------------------------------------------
{
  todoBanderaDatosAmbientalesActivos = param.asInt() == 1?true:false;

  sprintf(mensajeConsola, "¿En TODO, datos ambientales activos?: %d",todoBanderaDatosAmbientalesActivos);
  desplegarMensajeConsola();
}

//------------------------------------------------------------------------------
BLYNK_WRITE(V46)     // PARA TODO, DURACION DEL DESPLEGADO DE LOS DATOS AMBIENTALES
//------------------------------------------------------------------------------
{
  todoDuracionDatosAmbientales = param.asInt();

  sprintf(mensajeConsola, "Duración del desplegado de los datos ambientales: %d",todoDuracionDatosAmbientales);
  desplegarMensajeConsola();
}

////------------------------------------------------------------------------------
//BLYNK_WRITE(V47)     // PARA TODO, ¿ESTAN LOS DIAS PATRIOS Y LAS FESTIVIDADES DE FIN DE AÑO ACTIVOS?
////------------------------------------------------------------------------------
//{
//  todoBanderaDiasPatriosFestividadesFinAnoActiva = param.asInt()==1?true:false;
//
//  sprintf(mensajeConsola, "¿En TODO, días patrios yfestividades fin de año activos?: %d",
//           todoBanderaDiasPatriosFestividadesFinAnoActiva);
//  desplegarMensajeConsola();
//}
//
////------------------------------------------------------------------------------
//BLYNK_WRITE(V48)     // PARA TODO, DURACION DEL DESPLEGADO DEL RELOJ
////------------------------------------------------------------------------------
//{
//  todoDuracionDiasPatriosFestividadesFinAno = param.asInt();
//
//  sprintf(mensajeConsola, "Duración del desplegado del días patrios yfestividades fin de año: %d",
//        todoDuracionDiasPatriosFestividadesFinAno);
//  desplegarMensajeConsola();
//}
//
////------------------------------------------------------------------------------
//BLYNK_WRITE(V49)     // PARA TODO, ¿ESTAN LAS ANIMACIONES ACTIVAS?
////------------------------------------------------------------------------------
//{
//  todoBanderaAnimacionesActivas = param.asInt()==1?1:0;
//
//  sprintf(mensajeConsola, "¿En TODO, animaciones activas?: %d",
//           todoBanderaAnimacionesActivas);
//  desplegarMensajeConsola();
//}
//
////------------------------------------------------------------------------------
//BLYNK_WRITE(V50)     // PARA TODO, DURACION DEL DESPLEGADO DEL AS ANIMACIONES
////------------------------------------------------------------------------------
//{
//  todoDuracionAnimaciones = param.asInt();
//
//  sprintf(mensajeConsola, "Duración del desplegado de las snimaciones: %d",
//        todoDuracionAnimaciones);
//  desplegarMensajeConsola();
//}
//
//------------------------------------------------------------------------------
BLYNK_WRITE(V81)     // RECEPCION DE DATOS AMBIENTALES
//------------------------------------------------------------------------------
{
  char datosAmbientalesRecibidosInternet[MAXIMA_LONGITUD_CADENA];//datosAmbientalesRecibidosInternetJSON[MAXIMA_LONGITUD_CADENA];

  strcpy(datosAmbientalesRecibidosInternet,param.asStr());
//  sprintf(mensajeConsola, "Datos ambientales recibidos CADENA: %s",datosAmbientalesRecibidosInternet);
//  desplegarMensajeConsola();

  JSONVar datosAmbientalesRecibidosInternetJSON = JSON.parse(datosAmbientalesRecibidosInternet);

  sprintf(mensajeDatosAmbientales,"T: %.1f°C; HR: %d%%; PA: %dhPa; VV: %.1fkm/h",
           (double)datosAmbientalesRecibidosInternetJSON["current"]["temperature_2m"],
           (int)   datosAmbientalesRecibidosInternetJSON["current"]["relative_humidity_2m"],
           int((double)datosAmbientalesRecibidosInternetJSON["current"]["surface_pressure"]+0.5),
           (double)datosAmbientalesRecibidosInternetJSON["current"]["wind_speed_10m"]
         );

//  sprintf(mensajeConsola, "Mensaje datos ambientales: %s",mensajeDatosAmbientales);
//  desplegarMensajeConsola();
//  sprintf(mensajeConsola, "Longitud mensaje datos ambientales: %d",strlen(mensajeDatosAmbientales));
//  desplegarMensajeConsola();

  if(modo==MODO_DATOS_AMBIENTALES)
    desplegarDatosAmbientales();
}

////////////////////////////////////////////////////////////////////////////////
//                               UTILERIAS                                    //
////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
void desplegarMensajeVisualMarquesina(int tipLla)
//------------------------------------------------------------------------------
{
  //sprintf(mensajeConsola, "Tipo de llamada a desplegarMensajeVisualMarquesina(): %d",tipLla);
  //desplegarMensajeConsola();
  switch(tipLla)
  {
    // Conexión a red WLAN fallida (un parpadeo corto en rojo)
    case 1:
      fill_solid(leds,LEDS_TIRA_TOTAL,CRGB(0,0,0));
      for(i=0;i<contadorIntentosConectarWiFi*10;i++)
      {
        for(j=0;j<LEDs_COLUMNA_TOTAL;j++)
          leds[LEDs_COLUMNA_TOTAL*(i-1)+j] = CRGB(50,0,0);
      }
      FastLED.show();
      break;
    // Conexión a red WLAN exitosa (un parpadeo corto en verde bajito)
    case 2:
      for(i=0;i<1;i++)
      {
        fill_solid(leds,LEDS_TIRA_TOTAL,CRGB(0,0,25));
        FastLED.show();
        delay(500);
        fill_solid(leds,LEDS_TIRA_TOTAL,CRGB(0,0,0));
        FastLED.show();
        delay(500);
      }
      break;
    // Conexión a Blynk exitosa (un parpadeo corto en verde)
    case 3:
      for(i=0;i<1;i++)
      {
        fill_solid(leds,LEDS_TIRA_TOTAL,CRGB(0,50,0));
        FastLED.show();
        delay(500);
        fill_solid(leds,LEDS_TIRA_TOTAL,CRGB(0,0,0));
        FastLED.show();
        delay(500);
      }
      break;
  }
}

//------------------------------------------------------------------------------
void desplegarMensajeConsola()
//------------------------------------------------------------------------------
{
  if (banderaPuertoSerialAbierto)
    Serial.println(mensajeConsola);
}

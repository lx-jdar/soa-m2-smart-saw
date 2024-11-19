#include <Arduino.h>
#include <SoftwareSerial.h>   // Incluimos la librería  SoftwareSerial
// ------------------------------------------------
// Etiquetas
// ------------------------------------------------
//#define LOG // Comentar esta linea para desactivar logs

// ------------------------------------------------
// Constantes
// ------------------------------------------------
#define ACTIVAR_SIERRA 20
#define BAUD_RATE 9600
#define CANTIDAD_MAXIMA_DE_ENTRADAS 5
#define DEBOUNCE_TIME_MS 200
#define DETENER_SIERRA 21
#define MARGEN_DE_ERROR 1L
#define MODO_PIN_SIERRA FALLING 	// CHANGE, RISING, FALLING
#define MOTOR_SPEED 255
#define VALOR_CONTINUE -1
#define VELOCIDAD_DEL_SONIDO 57
#define MENSAJE_INICIO "Ingrese la cantidad de desplazamiento (en CM) que debera realizar el motor.\nO puede presionar el boton de la sierra para comenzar a cortar."

// ------------------------------------------------
// Umbrales Distancia
// ------------------------------------------------
#define DISTANCIA_MINIMA_V 5
#define DISTANCIA_MAXIMA_H 200
#define DISTANCIA_MINIMA_H 10

// ------------------------------------------------
// Pines Sensores (A = Analógico | D = Digital)
// ------------------------------------------------
#define PIN_D_PULSADOR_SIERRA 2
#define PIN_D_PULSADOR_DERECHA A1
#define PIN_D_PULSADOR_IZQUIERDA A2
#define PIN_D_TRIGGER_H 10
#define PIN_D_ECHO_H 11
#define PIN_D_TRIGGER_V 12
#define PIN_D_ECHO_V A0

// ------------------------------------------------
// Pines Comunicacion Bluetooth
// ------------------------------------------------
#define PIN_BLUETOOTH_RX 3
#define PIN_BLUETOOTH_TX 4
#define BLUETOOTH_IZQUIERDA 'I'
#define BLUETOOTH_DERECHA 'D'
#define BLUETOOTH_SIERRA 'S'

// ------------------------------------------------
// Pines Actuadores (P = PWM | D = Digital)
// ------------------------------------------------
#define PIN_D_LED_SIERRA 9
#define PIN_D_LED_DESPLAZAMIENTO 5
#define PIN_P_MDC_SPEED 6   // Modulacion del PWM M1
#define PIN_D_MDC_T1 7      // Control Motor P2 L293D, Terminal 1
#define PIN_D_MDC_T2 8      // Control Motor P2 L293D, Terminal 2
#define PIN_D_RELE 13

// ------------------------------------------------
// Estados del Embebido
// ------------------------------------------------
enum estadoEnum
{
    ESTADO_EMBEBIDO_IDLE,
    ESTADO_EMBEBIDO_EN_MOVIMIENTO,
    ESTADO_EMBEBIDO_SIERRA_ACTIVA
};

// ------------------------------------------------
// Estados del Coneccion Bluetooth
// ------------------------------------------------
enum bluetoothEnum
{
    PNOK,
	ME_ON,
	ME_OFF,
	SON,
	SOFF
};

// ------------------------------------------------
// Estructura de Evento
// ------------------------------------------------
enum eventoEnum
{
  EVENTO_CONTINUE,
  EVENTO_ACTIVACION_SIERRA,
  EVENTO_DESPLAZAMIENTO_DERECHA,
  EVENTO_DESPLAZAMIENTO_IZQUIERDA,
  EVENTO_INTRODUCCION_DE_DISTANCIA,
  EVENTO_POSICION_FINALIZADA,
  EVENTO_SIERRA_DETENIDA,
  EVENTO_LIMITE_HORIZONTAL_SUPERADO,
  EVENTO_LIMITE_VERTICAL_SUPERADO,
};

typedef struct
{
    eventoEnum tipo;
    int valor;
} Evento;

// ------------------------------------------------
// Estructura del Ultrasonido
// ------------------------------------------------
enum sentidoDesplazamientoEnum
{
	SENTIDO_IZQUIERDA,
	SENTIDO_DERECHA,
	SIN_MOVIMIENTO,
};

enum estadoUltrasonidoEnum
{
	ESTADO_ULT_EN_MOVIMIENTO,
    ESTADO_ULT_DETENIDO,
	ESTADO_ULT_UMBRAL_SUPERADO,
	ESTADO_ULT_LONGITUD_PERMITIDA,
};

typedef struct
{
    int pinTrigger, pinEcho;
    estadoUltrasonidoEnum estado;
    sentidoDesplazamientoEnum sentido;
    unsigned long cm;
    long posicionPartida;
} Ultrasonido;

// ------------------------------------------------
// Estructura de los Motores
// ------------------------------------------------
enum estadoMotorEnum
{
    ESTADO_MOTOR_PRENDIDO,
    ESTADO_MOTOR_APAGADO,
};

typedef struct
{
    int pinTerminal1, pinTerminal2, pinVelocidad;
    estadoMotorEnum estado;
} MotorDesplazamiento;

typedef struct
{
	int pinRele;
	estadoMotorEnum estado;
} MotorSierra;

// ------------------------------------------------
// Estructura de Monitor
// ------------------------------------------------
enum estadoMonitor
{
    ESTADO_MONITOR_IMPRIMIR,
    ESTADO_MONITOR_ESPERANDO_INGRESO,
    ESTADO_MONITOR_ACTUALIZAR_MENSAJE,
};

typedef struct
{
    String mensaje;
    String input;
    estadoMonitor estado;
    long configuracion;
} Monitor;

// ------------------------------------------------
// Estructura del LED
// ------------------------------------------------
typedef struct
{
    int pin;
    bool encendido;
} LedDigital;

// ------------------------------------------------
// Variables globales
// ------------------------------------------------
int entradaActual;
int estadoPulsador;
long posicionActual;
long posicionDePartida;
volatile bool pulsadorSierraActivado;
long startTime;
const int timeThreshold = DEBOUNCE_TIME_MS;
int valorDesplazamiento;

estadoEnum estadoEmbebido;
Evento evento;

Ultrasonido ultrasonidoHorizontal;
Ultrasonido ultrasonidoVertical;

MotorDesplazamiento motorDesplazamiento;
MotorSierra motorSierra;

LedDigital ledSierra;
LedDigital ledDesplazamiento;

Monitor monitor;

SoftwareSerial BTSerial(PIN_BLUETOOTH_RX, PIN_BLUETOOTH_TX);

// ------------------------------------------------
// Logs
// ------------------------------------------------
void log(const char *estado, const char *evento)
{
#ifdef LOG
    Serial.println("------------------------------------------------");
    Serial.println(estado);
    Serial.println(evento);
    Serial.println("------------------------------------------------");
#endif
}

void log(String msg)
{
#ifdef LOG
    Serial.println(msg);
#endif
}

void log(int val)
{
#ifdef LOG
    Serial.println(val);
#endif
}

// ------------------------------------------------
// Firmas de Funciones
// ------------------------------------------------
void setup();
void start();
void inicializarMonitor(Monitor* monitor, long baudRate);
void inicializarMotorDesplazamiento(MotorDesplazamiento* motorDesplazamiento, int pinTerminal1, int pinTerminal2, int pinVelocidad);
void inicializarUltrasonido(Ultrasonido* ultrasonido, int pinTrigger, int pinEcho);
void inicializarLedDigital(LedDigital* ledDigital, int pin);
void inicializarMotorSierra(MotorSierra* motorSierra, int pinRele);
void inicializarPulsadores();
void inicializarVariablesGlobales();

void loop();
void obtenerEvento();
void maquinaEstado();
void ISR_Boton();

void encenderMotorDesplazamiento(unsigned int pinPulsador);
void detenerMotorDesplazamiento();
void encenderMotorSierra();
void detenerMotorSierra();
void encenderLed(LedDigital* ledDigital);
void apagarLed(LedDigital* ledDigital);

void verificarLecturaDesdeMonitorSerial();
void verificarPulsadoresDeDesplazamiento();
void verificarPulsadorDeSierra();
void verificarPulsadores();
void verificarBluetooth();
void verificarPosicionUltrasonidoHorizontal();
void verificarPosicionUltrasonidoVertical();

void actualizarUltrasonido(Ultrasonido* ultrasonido);
bool esPulsadorPresionado(unsigned int pinPulsador);
void setMensajeAImprimir(String mensaje);
void enviarMensajeToApp(const char* data);
const char* EnumToString(bluetoothEnum btCode);

// ------------------------------------------------
// Captura de eventos
// ------------------------------------------------
void (*verificarEntradas[CANTIDAD_MAXIMA_DE_ENTRADAS])() =
{
    verificarLecturaDesdeMonitorSerial,
    verificarPulsadores,
	verificarBluetooth,
    verificarPosicionUltrasonidoHorizontal,
    verificarPosicionUltrasonidoVertical,
};

void obtenerEvento()
{
    verificarEntradas[entradaActual]();
    entradaActual = entradaActual < CANTIDAD_MAXIMA_DE_ENTRADAS - 1? entradaActual + 1 : 0;
}

// ------------------------------------------------
// Implementación máquina de estados
// ------------------------------------------------
void maquinaEstado()
{
    obtenerEvento();
    switch (estadoEmbebido)
    {
        case ESTADO_EMBEBIDO_IDLE:
        {
            switch (evento.tipo)
            {
                case EVENTO_INTRODUCCION_DE_DISTANCIA:
                {
                    int valorIngresado = monitor.input.toInt();
                    if (valorIngresado > 0)
                    {
                        valorDesplazamiento = valorIngresado;
                        enviarMensajeToApp(EnumToString(PNOK));
                        monitor.mensaje = "El motor se desplazara " + String(valorDesplazamiento) + " CM cuando se presione un pulsador.";
                    }
                    else
                    {
                        monitor.mensaje = "El valor ingresado no es valido.\n";
                        monitor.mensaje += MENSAJE_INICIO;
                    }
                    monitor.estado = ESTADO_MONITOR_IMPRIMIR;
                    estadoEmbebido = ESTADO_EMBEBIDO_IDLE;
                    break;
                }

                case EVENTO_LIMITE_HORIZONTAL_SUPERADO:
                {
                    setMensajeAImprimir("El valor ingresado " + String(valorDesplazamiento) + " supera los limites establecidos del desplazamiento.");
                    estadoEmbebido = ESTADO_EMBEBIDO_IDLE;
                    evento.tipo = EVENTO_CONTINUE;
                    break;
                }

                case EVENTO_DESPLAZAMIENTO_IZQUIERDA:
                {
                    log("ESTADO_EMBEBIDO_IDLE", "EVENTO_DESPLAZAMIENTO_IZQUIERDA");
					encenderMotorDesplazamiento(PIN_D_PULSADOR_IZQUIERDA);
					encenderLed(&ledDesplazamiento);
					setMensajeAImprimir("Pulsador izquierdo presionado. Desplazando motor a izquierda.");
					enviarMensajeToApp(EnumToString(ME_ON));
					estadoEmbebido = ESTADO_EMBEBIDO_EN_MOVIMIENTO;
                    break;
                }

                case EVENTO_DESPLAZAMIENTO_DERECHA:
                {
                    log("ESTADO_EMBEBIDO_IDLE", "EVENTO_DESPLAZAMIENTO_DERECHA");
                    encenderMotorDesplazamiento(PIN_D_PULSADOR_DERECHA);
					encenderLed(&ledDesplazamiento);
					setMensajeAImprimir("Pulsador derecho presionado. Desplazando motor a derecha.");
					enviarMensajeToApp(EnumToString(ME_ON));
                    estadoEmbebido = ESTADO_EMBEBIDO_EN_MOVIMIENTO;
                    break;
                }

                case EVENTO_ACTIVACION_SIERRA:
                {
                    log("ESTADO_EMBEBIDO_IDLE", "EVENTO_ACTIVACION_SIERRA");
                    encenderMotorSierra();
                    encenderLed(&ledSierra);
                    setMensajeAImprimir("Pulsador SIERRA encendida. Cortando ...");
                    enviarMensajeToApp(EnumToString(SON));
                    estadoEmbebido = ESTADO_EMBEBIDO_SIERRA_ACTIVA;
                    break;
                }

                case EVENTO_CONTINUE:
                {
                    log("ESTADO_EMBEBIDO_IDLE", "EVENTO_CONTINUE");
                    estadoEmbebido = ESTADO_EMBEBIDO_IDLE;
                    break;
                }

                default:
				{
					log("ESTADO_EMBEBIDO_IDLE", "EVENTO_NO_RECONOCIDO");
					break;
				}
            }
            break;
        }
        break;

        case ESTADO_EMBEBIDO_EN_MOVIMIENTO:
        {
            switch (evento.tipo)
            {
                case EVENTO_POSICION_FINALIZADA:
                {
                    log("ESTADO_EMBEBIDO_EN_MOVIMIENTO", "EVENTO_POSICION_FINALIZADA");
                    detenerMotorDesplazamiento();
                    apagarLed(&ledDesplazamiento);
                    enviarMensajeToApp(EnumToString(ME_OFF));
                    monitor.mensaje = "Posicionamiento finalizado.\n";
                    monitor.mensaje += MENSAJE_INICIO;
                    monitor.estado = ESTADO_MONITOR_IMPRIMIR;
                    estadoEmbebido = ESTADO_EMBEBIDO_IDLE;
                    break;
                }

                case EVENTO_LIMITE_HORIZONTAL_SUPERADO:
                {
                    log("ESTADO_EMBEBIDO_EN_MOVIMIENTO", "EVENTO_LIMITE_HORIZONTAL_SUPERADO");
                    detenerMotorDesplazamiento();
                    apagarLed(&ledDesplazamiento);
                    enviarMensajeToApp(EnumToString(ME_OFF));
                    monitor.mensaje = "Se quiere desplazar mas que el umbral maximo. Motor de desplazamiento apagandose ...\n";
                    monitor.mensaje += MENSAJE_INICIO;
                    monitor.estado = ESTADO_MONITOR_IMPRIMIR;
                    estadoEmbebido = ESTADO_EMBEBIDO_IDLE;
                    break;
                }

                case EVENTO_CONTINUE:
                {
                    log("ESTADO_EMBEBIDO_EN_MOVIMIENTO", "EVENTO_CONTINUE");
                    estadoEmbebido = ESTADO_EMBEBIDO_EN_MOVIMIENTO;
                    break;
                }

                default:
				{
					log("ESTADO_EMBEBIDO_EN_MOVIMIENTO", "EVENTO_NO_RECONOCIDO");
					break;
				}
            }
            break;
        }
        break;

        case ESTADO_EMBEBIDO_SIERRA_ACTIVA:
        {
            switch (evento.tipo)
            {
                case EVENTO_SIERRA_DETENIDA:
                {
                    log("ESTADO_EMBEBIDO_SIERRA_ACTIVA", "EVENTO_SIERRA_DETENIDA");
                    detenerMotorSierra();
                    apagarLed(&ledSierra);
                    enviarMensajeToApp(EnumToString(SOFF));
                    monitor.mensaje = "Deteniendo el motor sierra ...\n";
                    monitor.mensaje += MENSAJE_INICIO;
                    monitor.estado = ESTADO_MONITOR_IMPRIMIR;
                    estadoEmbebido = ESTADO_EMBEBIDO_IDLE;
                    break;
                }

                case EVENTO_LIMITE_VERTICAL_SUPERADO:
                {
                    log("ESTADO_EMBEBIDO_SIERRA_ACTIVA", "EVENTO_LIMITE_VERTICAL_SUPERADO");
                    detenerMotorSierra();
                    apagarLed(&ledSierra);
                    enviarMensajeToApp(EnumToString(SOFF));
                    monitor.mensaje = "Pase el umbral minimo. Deteniendo el motor sierra ...\n";
                    monitor.mensaje += MENSAJE_INICIO;
                    monitor.estado = ESTADO_MONITOR_IMPRIMIR;
                    estadoEmbebido = ESTADO_EMBEBIDO_IDLE;
                    break;
                }

                case EVENTO_CONTINUE:
				{
					log("ESTADO_EMBEBIDO_IDLE", "EVENTO_CONTINUE");
					estadoEmbebido = ESTADO_EMBEBIDO_SIERRA_ACTIVA;
					break;
				}

				default:
				{
					log("ESTADO_EMBEBIDO_SIERRA_ACTIVA", "EVENTO_NO_RECONOCIDO");
					break;
				}
            }
            break;
        }
        break;
        default:
            break;
    }

    // Ya se atendió el evento
    evento.tipo = EVENTO_CONTINUE;
    evento.valor = VALOR_CONTINUE;
}

//----------------------------------------------------
// Interrupciones
//----------------------------------------------------
void ISR_Boton()
{
	if (millis() - startTime > timeThreshold)
	{
	  pulsadorSierraActivado = true;
	  startTime = millis();
	}
}

//----------------------------------------------------
// Arduino Setup
//-----------------------------------------------
void setup()
{
    start();
    attachInterrupt(digitalPinToInterrupt(PIN_D_PULSADOR_SIERRA), ISR_Boton, MODO_PIN_SIERRA);
}

//----------------------------------------------------
// Arduino Loop
//----------------------------------------------------
void loop()
{
    maquinaEstado();
}

//----------------------------------------------------
// Inicialización
//----------------------------------------------------
void inicializarMonitor(Monitor* monitor, long baudRate)
{
    monitor->estado = ESTADO_MONITOR_ACTUALIZAR_MENSAJE;
	monitor->configuracion = baudRate;
  	Serial.begin(monitor->configuracion);
  	BTSerial.begin(monitor->configuracion);      // Inicializamos el puerto serie BT que hemos creado
}

void inicializarMotorDesplazamiento(MotorDesplazamiento* motorDesplazamiento, int pinTerminal1, int pinTerminal2, int pinVelocidad)
{
	motorDesplazamiento->pinTerminal1 = pinTerminal1;
    motorDesplazamiento->pinTerminal2 = pinTerminal2;
    motorDesplazamiento->pinVelocidad = PIN_P_MDC_SPEED;
    motorDesplazamiento->estado = ESTADO_MOTOR_APAGADO;
    pinMode(motorDesplazamiento->pinTerminal1, OUTPUT);
    pinMode(motorDesplazamiento->pinTerminal2, OUTPUT);
    pinMode(motorDesplazamiento->pinVelocidad, OUTPUT);
}

void inicializarUltrasonido(Ultrasonido* ultrasonido, int pinTrigger, int pinEcho)
{
	ultrasonido->pinTrigger = pinTrigger;
    ultrasonido->pinEcho = pinEcho;
    ultrasonido->estado = ESTADO_ULT_DETENIDO;
	ultrasonido->sentido = SIN_MOVIMIENTO;
    pinMode(ultrasonido->pinTrigger, OUTPUT);
    pinMode(ultrasonido->pinEcho, INPUT);
}

void inicializarLedDigital(LedDigital* ledDigital, int pin)
{
	ledDigital->pin = pin;
    ledDigital->encendido = false;
    pinMode(ledDigital->pin, OUTPUT);
}

void inicializarMotorSierra(MotorSierra* motorSierra, int pinRele)
{
	motorSierra->pinRele = pinRele;
	motorSierra->estado = ESTADO_MOTOR_APAGADO;
    pinMode(motorSierra->pinRele, OUTPUT);
    digitalWrite(motorSierra->pinRele, HIGH);
}

void inicializarPulsadores()
{
	pinMode(PIN_D_PULSADOR_IZQUIERDA, INPUT_PULLUP);
    pinMode(PIN_D_PULSADOR_DERECHA, INPUT_PULLUP);
    pinMode(PIN_D_PULSADOR_SIERRA, INPUT_PULLUP);
}

void inicializarVariablesGlobales()
{
	entradaActual = 0;
	estadoEmbebido = ESTADO_EMBEBIDO_IDLE;
	estadoPulsador = ACTIVAR_SIERRA;
	evento.tipo = EVENTO_CONTINUE;
    posicionActual = 0;
    posicionDePartida = 0;
	pulsadorSierraActivado = false;
	startTime = 0;
    valorDesplazamiento = 0;
}

void start()
{
	inicializarLedDigital(&ledSierra, PIN_D_LED_SIERRA);
	inicializarLedDigital(&ledDesplazamiento, PIN_D_LED_DESPLAZAMIENTO);
	inicializarMonitor(&monitor, BAUD_RATE);
	inicializarMotorSierra(&motorSierra, PIN_D_RELE);
	inicializarMotorDesplazamiento(&motorDesplazamiento, PIN_D_MDC_T1, PIN_D_MDC_T2, PIN_P_MDC_SPEED);
	inicializarUltrasonido(&ultrasonidoHorizontal, PIN_D_TRIGGER_H, PIN_D_ECHO_H);
	inicializarUltrasonido(&ultrasonidoVertical, PIN_D_TRIGGER_V, PIN_D_ECHO_V);
	inicializarPulsadores();
    inicializarVariablesGlobales();
}

// ------------------------------------------------
// Comunicación
// ------------------------------------------------
void setMensajeAImprimir(String mensaje)
{
    monitor.mensaje = mensaje;
    monitor.estado = ESTADO_MONITOR_IMPRIMIR;
}

void enviarMensajeToApp(const char* data) {
	BTSerial.write(data);
}

void verificarLecturaDesdeMonitorSerial()
{
    switch (monitor.estado)
    {
        case ESTADO_MONITOR_ACTUALIZAR_MENSAJE:
        {
            monitor.mensaje = MENSAJE_INICIO;
            monitor.estado = ESTADO_MONITOR_IMPRIMIR;
            break;
        }

        case ESTADO_MONITOR_IMPRIMIR:
        {
        	Serial.print(monitor.mensaje);
			monitor.mensaje="\n";
			Serial.print(monitor.mensaje);
            monitor.estado = ESTADO_MONITOR_ESPERANDO_INGRESO;
            break;
        }

        case ESTADO_MONITOR_ESPERANDO_INGRESO:
        {
            if (Serial.available())
            {
                monitor.input = Serial.readStringUntil('\n');
                evento.tipo = EVENTO_INTRODUCCION_DE_DISTANCIA;
            }
            break;
        }

        default:
        	Serial.println("-----------------------------------------------------");
			Serial.println("Estado del Monitor no reconocido...");
			Serial.println(monitor.estado);
			Serial.println(monitor.input);
			Serial.println("-----------------------------------------------------");
            break;
    }
}

// ------------------------------------------------
// Lógica de Sensores
// ------------------------------------------------
void verificarPulsadoresDeDesplazamiento()
{
    bool pulsadorIzquierdaPresionado = esPulsadorPresionado(PIN_D_PULSADOR_IZQUIERDA);
    bool pulsadorDerechaPresionado = esPulsadorPresionado(PIN_D_PULSADOR_DERECHA);
    if (pulsadorIzquierdaPresionado)
    {
        evento.tipo = EVENTO_DESPLAZAMIENTO_IZQUIERDA;
        ultrasonidoHorizontal.estado = ESTADO_ULT_EN_MOVIMIENTO;
        ultrasonidoHorizontal.sentido = SENTIDO_IZQUIERDA;
    }
    else if (pulsadorDerechaPresionado)
    {
        evento.tipo = EVENTO_DESPLAZAMIENTO_DERECHA;
        ultrasonidoHorizontal.estado = ESTADO_ULT_EN_MOVIMIENTO;
        ultrasonidoHorizontal.sentido = SENTIDO_DERECHA;
    }
}

void verificarPulsadorDeSierra()
{
    if (estadoPulsador == ACTIVAR_SIERRA && pulsadorSierraActivado)
    {
    	evento.tipo = EVENTO_ACTIVACION_SIERRA;
        estadoPulsador = DETENER_SIERRA;
        pulsadorSierraActivado = false;
        ultrasonidoVertical.estado = ESTADO_ULT_LONGITUD_PERMITIDA;
    }
    else
    {
        if(estadoPulsador == DETENER_SIERRA && pulsadorSierraActivado)
        {
            evento.tipo = EVENTO_SIERRA_DETENIDA;
            estadoPulsador = ACTIVAR_SIERRA;
            pulsadorSierraActivado = false;
        }
    }
}

void verificarPulsadores()
{
    verificarPulsadoresDeDesplazamiento();
    verificarPulsadorDeSierra();
}

boolean isValidNumber(String str)
{
	for(int i=0; i<str.length();i++)
	{
		if(isDigit(str.charAt(i))) return true;
	}
	return false;
}

const char* EnumToString(bluetoothEnum btCode) {
    switch (btCode) {
        case PNOK: return "PNOK";
        case ME_ON: return "ME_ON";
        case ME_OFF: return "ME_OFF";
        case SON: return "SON";
        case SOFF: return "SOFF";
        default: return "Desconocido";
    }
}


void verificarBluetooth()
{
	if(BTSerial.available())    // Si llega un dato por el puerto BT se envía al monitor serial
	{
        String btInput = BTSerial.readStringUntil('\n');
        if (btInput.indexOf(BLUETOOTH_IZQUIERDA) == 0) {
        	monitor.mensaje = "Se ingresó el caracter: I";
        	evento.tipo = EVENTO_DESPLAZAMIENTO_IZQUIERDA;
			ultrasonidoHorizontal.estado = ESTADO_ULT_EN_MOVIMIENTO;
			ultrasonidoHorizontal.sentido = SENTIDO_IZQUIERDA;
        }
        else if (btInput.indexOf(BLUETOOTH_DERECHA) == 0)
        {
        	monitor.mensaje = "Se ingresó el caracter: D";
        	evento.tipo = EVENTO_DESPLAZAMIENTO_DERECHA;
			ultrasonidoHorizontal.estado = ESTADO_ULT_EN_MOVIMIENTO;
			ultrasonidoHorizontal.sentido = SENTIDO_DERECHA;
        }
        else if (btInput.indexOf(BLUETOOTH_SIERRA) == 0)
		{
			monitor.mensaje = "Se ingresó el caracter: S";
			ISR_Boton();
		}
        else if (btInput.indexOf("T") == 0)
		{
			monitor.mensaje = "Se ingresó el caracter: T";
			ISR_Boton();
		}
        else if (btInput.indexOf('R') == 0)
		{
			monitor.mensaje = "Se ingresó el caracter: R";
			ultrasonidoHorizontal.estado = ESTADO_ULT_DETENIDO;
			ultrasonidoHorizontal.sentido = SIN_MOVIMIENTO;
		}
        else if (isValidNumber(btInput))
        {
        	monitor.input = btInput;
        	monitor.mensaje = "Se recicibió por BT el nro: " + monitor.input;
        	evento.tipo = EVENTO_INTRODUCCION_DE_DISTANCIA;
        }

        monitor.estado = ESTADO_MONITOR_IMPRIMIR;
	}
}

void verificarLimitesHorizontales()
{
    if(valorDesplazamiento)
    {
        actualizarUltrasonido(&ultrasonidoHorizontal);
        posicionActual = ultrasonidoHorizontal.posicionPartida;
        int posibleDesplazamiento = posicionActual + (ultrasonidoHorizontal.sentido == SENTIDO_IZQUIERDA ? valorDesplazamiento : - valorDesplazamiento);
        if ((posibleDesplazamiento <= DISTANCIA_MINIMA_H || posibleDesplazamiento >= DISTANCIA_MAXIMA_H) && ultrasonidoHorizontal.estado == ESTADO_ULT_EN_MOVIMIENTO)
        {
            ultrasonidoHorizontal.estado = ESTADO_ULT_UMBRAL_SUPERADO;
        }
    }
}

void verificarPosicionUltrasonidoHorizontal()
{
    verificarLimitesHorizontales();
    actualizarUltrasonido(&ultrasonidoHorizontal);
    switch (ultrasonidoHorizontal.estado)
    {
        case ESTADO_ULT_UMBRAL_SUPERADO:
        {
			ultrasonidoHorizontal.estado = ESTADO_ULT_DETENIDO;
            ultrasonidoHorizontal.sentido = SIN_MOVIMIENTO;
			if(estadoEmbebido == ESTADO_EMBEBIDO_EN_MOVIMIENTO)
			{
				evento.tipo = EVENTO_LIMITE_HORIZONTAL_SUPERADO;
			}
            break;
        }

        case ESTADO_ULT_EN_MOVIMIENTO:
        {
            posicionActual = ultrasonidoHorizontal.cm;
            log("Posicion actual Ultrasonido Horizontal: " + String(posicionActual) + " CM.");
            int delta = abs(posicionActual - ultrasonidoHorizontal.posicionPartida);
            if (delta >= valorDesplazamiento)
            {
                ultrasonidoHorizontal.estado = ESTADO_ULT_DETENIDO;
                return;
            }
            log("El motor se desplazo horizontalmente hacia la " +
            String(evento.tipo == EVENTO_DESPLAZAMIENTO_IZQUIERDA? "izquierda" : "derecha") + " unos " +
            String(delta) + " CM de " + String(valorDesplazamiento) + " CM.");
            break;
        }

        case ESTADO_ULT_DETENIDO:
        {
            ultrasonidoHorizontal.posicionPartida = ultrasonidoHorizontal.cm;
            if(estadoEmbebido == ESTADO_EMBEBIDO_EN_MOVIMIENTO)
            {
            	evento.tipo = EVENTO_POSICION_FINALIZADA;
            }
            break;
        }

        default:
            break;
    }
}

void verificarPosicionUltrasonidoVertical()
{
	actualizarUltrasonido(&ultrasonidoVertical);	// Actualizar la posicion actual del ultrasonido
    switch (ultrasonidoVertical.estado)
	{

		case ESTADO_ULT_LONGITUD_PERMITIDA:
		{
			posicionActual = ultrasonidoVertical.cm;
			log("Posicion actual ULT V: " + String(posicionActual) + " CM.");
			if (posicionActual <= DISTANCIA_MINIMA_V) ultrasonidoVertical.estado = ESTADO_ULT_UMBRAL_SUPERADO;
			break;
		}

		case ESTADO_ULT_UMBRAL_SUPERADO:
		{
			ultrasonidoVertical.estado = ESTADO_ULT_DETENIDO;
			evento.tipo = EVENTO_LIMITE_VERTICAL_SUPERADO;
			break;
		}

		default:
			break;
	}
}

void actualizarUltrasonido(Ultrasonido* ultrasonido)
{
    digitalWrite(ultrasonido->pinTrigger, LOW);
    delayMicroseconds(2);
    digitalWrite(ultrasonido->pinTrigger, HIGH);
    delayMicroseconds(10);
    digitalWrite(ultrasonido->pinTrigger, LOW);
    unsigned long tiempoPulso = pulseIn(ultrasonido->pinEcho, HIGH);
    if (ultrasonido->estado == ESTADO_ULT_EN_MOVIMIENTO)
    {
        tiempoPulso += ultrasonido->sentido == SENTIDO_IZQUIERDA ? MARGEN_DE_ERROR : - MARGEN_DE_ERROR;
    }
    unsigned long distanciaCm = tiempoPulso / VELOCIDAD_DEL_SONIDO;
    ultrasonido->cm = distanciaCm;
}

bool esPulsadorPresionado(unsigned int pinPulsador)
{
    return digitalRead(pinPulsador) == LOW;
}

//----------------------------------------------------
// Lógica de Actuadores
//----------------------------------------------------
void encenderMotorDesplazamiento(unsigned int pinPulsador)
{
  	if(motorDesplazamiento.estado == ESTADO_MOTOR_APAGADO)
    {
        motorDesplazamiento.estado = ESTADO_MOTOR_PRENDIDO;
        analogWrite(motorDesplazamiento.pinVelocidad, MOTOR_SPEED);
        bool direccionIzquierda = (pinPulsador == PIN_D_PULSADOR_IZQUIERDA);
        digitalWrite(motorDesplazamiento.pinTerminal1, direccionIzquierda ? HIGH : LOW);
        digitalWrite(motorDesplazamiento.pinTerminal2, direccionIzquierda ? LOW : HIGH);
    }
}


void detenerMotorDesplazamiento()
{
    if(motorDesplazamiento.estado == ESTADO_MOTOR_PRENDIDO)
    {
        analogWrite(motorDesplazamiento.pinVelocidad, 0);
        digitalWrite(motorDesplazamiento.pinTerminal1, LOW);
        digitalWrite(motorDesplazamiento.pinTerminal2, LOW);
        motorDesplazamiento.estado = ESTADO_MOTOR_APAGADO;
    }
}

void encenderMotorSierra()
{
    if(motorSierra.estado == ESTADO_MOTOR_APAGADO)
	{
		digitalWrite(motorSierra.pinRele, LOW);
		motorSierra.estado = ESTADO_MOTOR_PRENDIDO;
	}
    Serial.println("Encender Cierra!");
}

void detenerMotorSierra()
{
	if(motorSierra.estado == ESTADO_MOTOR_PRENDIDO)
	{
		digitalWrite(motorSierra.pinRele, HIGH);
		motorSierra.estado = ESTADO_MOTOR_APAGADO;
	}
}

void encenderLed(LedDigital* ledDigital)
{
	if(ledDigital->encendido == false)
	{
		digitalWrite(ledDigital->pin, HIGH);
		ledDigital->encendido = true;
	}
}

void apagarLed(LedDigital* ledDigital)
{
	if(ledDigital->encendido == true)
	{
		digitalWrite(ledDigital->pin, LOW);
		ledDigital->encendido = false;
	}
}

// ------------------------------------------------
// Etiquetas
// ------------------------------------------------
//#define LOG // Comentar esta linea para desactivar logs

// ------------------------------------------------
// Constantes
// ------------------------------------------------
#define ACTIVAR_SIERRA 20
#define BAUD_RATE 9600
#define CANTIDAD_MAXIMA_DE_ENTRADAS 1
#define DEBOUNCE_TIME_MS 150
#define DETENER_SIERRA 21
#define MODO_PIN_SIERRA FALLING
#define VALOR_CONTINUE -1

// ------------------------------------------------
// Pines Sensores (A = Analógico | D = Digital)
// ------------------------------------------------
#define PIN_D_PULSADOR_SIERRA 3

// ------------------------------------------------
// Pines Actuadores (P = PWM | D = Digital)
// ------------------------------------------------
#define PIN_D_LED_SIERRA 9
#define PIN_D_RELE 2

// ------------------------------------------------
// Estados del Embebido
// ------------------------------------------------
enum estadoEnum
{
    ESTADO_EMBEBIDO_IDLE,
    ESTADO_EMBEBIDO_SIERRA_ACTIVA
};

// ------------------------------------------------
// Estructura de Evento
// ------------------------------------------------
enum eventoEnum
{
  EVENTO_CONTINUE,
  EVENTO_ACTIVACION_SIERRA,
  EVENTO_SIERRA_DETENIDA,
};

typedef struct
{
    eventoEnum tipo;
    int valor;
} Evento;

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
	int pinRele;
	estadoMotorEnum estado;
} MotorSierra;

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
long startTime;
const int timeThreshold = DEBOUNCE_TIME_MS;
volatile bool pulsadorSierraActivado;
estadoEnum estadoEmbebido;
Evento evento;
MotorSierra motorSierra;
LedDigital ledSierra;

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
void start();
void inicializarLedDigital(LedDigital* ledDigital, int pin);
void inicializarMotorSierra(MotorSierra* motorSierra, int pinRele);
void inicializarPulsadores();
void inicializarVariablesGlobales();

void obtenerEvento();
void maquinaEstado();
void ISR_Boton();

void verificarPulsadores();

void encenderMotorYLedSierra();
void detenerMotorYLedSierra();

// ------------------------------------------------
// Captura de eventos
// ------------------------------------------------
void (*verificarEntradas[CANTIDAD_MAXIMA_DE_ENTRADAS])() =
{
    verificarPulsadores
};

void obtenerEvento()
{
    verificarEntradas[entradaActual]();
    entradaActual = (entradaActual < CANTIDAD_MAXIMA_DE_ENTRADAS - 1) ? entradaActual + 1 : 0;
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
                case EVENTO_ACTIVACION_SIERRA:
                {
                    log("ESTADO_EMBEBIDO_IDLE", "EVENTO_ACTIVACION_SIERRA");
                    encenderMotorYLedSierra();
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

        case ESTADO_EMBEBIDO_SIERRA_ACTIVA:
        {
            switch (evento.tipo)
            {
                case EVENTO_SIERRA_DETENIDA:
                {
                    log("ESTADO_EMBEBIDO_SIERRA_ACTIVA", "EVENTO_SIERRA_DETENIDA");
                    detenerMotorYLedSierra();
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
}

void inicializarPulsadores()
{
    pinMode(PIN_D_PULSADOR_SIERRA, INPUT_PULLUP);
}

void inicializarVariablesGlobales()
{
	entradaActual = 0;
	estadoEmbebido = ESTADO_EMBEBIDO_IDLE;
	estadoPulsador = ACTIVAR_SIERRA;
	evento.tipo = EVENTO_CONTINUE;
	pulsadorSierraActivado = false;
	startTime = 0;
}

void start()
{
    Serial.begin(BAUD_RATE);
	inicializarLedDigital(&ledSierra, PIN_D_LED_SIERRA);
	inicializarMotorSierra(&motorSierra, PIN_D_RELE);
	inicializarPulsadores();
	inicializarVariablesGlobales();
}

// ------------------------------------------------
// Lógica de Sensores
// ------------------------------------------------
void verificarPulsadores()
{
    if (estadoPulsador == ACTIVAR_SIERRA && pulsadorSierraActivado)
    {
        evento.tipo = EVENTO_ACTIVACION_SIERRA;
        estadoPulsador = DETENER_SIERRA;
        pulsadorSierraActivado = false;
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

//----------------------------------------------------
// Lógica de Actuadores
//----------------------------------------------------
void encenderMotorYLedSierra()
{
    if(motorSierra.estado == ESTADO_MOTOR_APAGADO)
	{
		digitalWrite(motorSierra.pinRele, HIGH);
		motorSierra.estado = ESTADO_MOTOR_PRENDIDO;
		digitalWrite(ledSierra.pin, HIGH);
		ledSierra.encendido = true;
	}
}

void detenerMotorYLedSierra()
{
	if(motorSierra.estado == ESTADO_MOTOR_PRENDIDO)
	{
		digitalWrite(motorSierra.pinRele, LOW);
		motorSierra.estado = ESTADO_MOTOR_APAGADO;
		digitalWrite(ledSierra.pin, LOW);
		ledSierra.encendido = false;
	}
}

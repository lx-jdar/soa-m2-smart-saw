// ------------------------------------------------
// Etiquetas
// ------------------------------------------------
//#define LOG // Comentar esta linea para desactivar logs

// ------------------------------------------------
// Constantes
// ------------------------------------------------
#define BAUD_RATE 9600
#define CANTIDAD_MAXIMA_DE_ENTRADAS 1
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
// Estados del Pulsador
// ------------------------------------------------
enum estadoPulsadorEnum
{
    ACTIVAR_SIERRA,
    DETENER_SIERRA
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
int entradaActual = 0;
estadoEnum estadoEmbebido;
Evento evento;
MotorSierra motorSierra;
LedDigital ledSierra;
bool pulsadorSierraActivado = false;
estadoPulsadorEnum estadoPulsador = ACTIVAR_SIERRA;
long startTime = 0;
const int timeThreshold = 150;

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
void inicializarMotorSierra(MotorSierra* motorSierra, int pinRele);
void inicializarLedDigital(LedDigital* ledDigital, int pin);
void inicializarPulsadores();
void loop();
void obtenerEvento();
void maquinaEstado();
void ISR_Boton();
void encenderMotorYLedSierra();
void detenerMotorYLedSierra();
void verificarPulsadores();
bool esPulsadorPresionado(unsigned int pinPulsador);

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
                    break;
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
					log("ESTADO_EMBEBIDO_IDLE", "EVENTO_CONTINUE");
					estadoEmbebido = ESTADO_EMBEBIDO_SIERRA_ACTIVA;
					break;

				default:
					break;
            }
            break;
        }
        break;

        default:
            break;
    }

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

void start()
{
    Serial.begin(BAUD_RATE);
	inicializarMotorSierra(&motorSierra, PIN_D_RELE);
  	inicializarLedDigital(&ledSierra, PIN_D_LED_SIERRA);
	inicializarPulsadores();
    estadoEmbebido = ESTADO_EMBEBIDO_IDLE;
	evento.tipo = EVENTO_CONTINUE;
}

// ------------------------------------------------
// Lógica de Sensores
// ------------------------------------------------
void verificarPulsadores()
{
    if (pulsadorSierraActivado)
    {
        if (estadoPulsador == ACTIVAR_SIERRA)
        {
            evento.tipo = EVENTO_ACTIVACION_SIERRA;
            estadoPulsador = DETENER_SIERRA;
        }
        else if (estadoPulsador == DETENER_SIERRA)
        {
            evento.tipo = EVENTO_SIERRA_DETENIDA;
            estadoPulsador = ACTIVAR_SIERRA;
        }
        pulsadorSierraActivado = false;
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

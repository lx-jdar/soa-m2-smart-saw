// ------------------------------------------------
// Etiquetas
// ------------------------------------------------
#define LOG // Comentar esta linea para desactivar logs

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
void inicializarLedDigital(LedDigital* ledDigital, int pin);
void inicializarMotorSierra(MotorSierra* motorSierra, int pinRele);
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
                case EVENTO_ACTIVACION_SIERRA:
                {
                    log("ESTADO_EMBEBIDO_IDLE", "EVENTO_ACTIVACION_SIERRA");
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
                case EVENTO_ACTIVACION_SIERRA:
                {
                    log("ESTADO_EMBEBIDO_SIERRA_ACTIVA", "EVENTO_ACTIVACION_SIERRA");
                    encenderMotorYLedSierra();
                    estadoEmbebido = ESTADO_EMBEBIDO_SIERRA_ACTIVA;
                    break;
                }
                
                case EVENTO_SIERRA_DETENIDA:
                {
                    log("ESTADO_EMBEBIDO_SIERRA_ACTIVA", "EVENTO_SIERRA_DETENIDA");
                    detenerMotorYLedSierra();
                    detachInterrupt(digitalPinToInterrupt(PIN_D_PULSADOR_SIERRA));
                    estadoEmbebido = ESTADO_EMBEBIDO_IDLE;
                    break;
                }
                
                case EVENTO_CONTINUE:
                {
                    log("ESTADO_EMBEBIDO_SIERRA_ACTIVA", "EVENTO_CONTINUE");
                    estadoEmbebido = ESTADO_EMBEBIDO_SIERRA_ACTIVA;
                    break;
                }

                default:
                    break;
            }
            break;
        }
        break;

        default:
            break;
    }
    
    // Ya se atendió el evento (Solo sobrescribe el evento si no es EVENTO_SIERRA_DETENIDA ya que es una interrupcion)
    if (evento.tipo != EVENTO_SIERRA_DETENIDA) {
        evento.tipo = EVENTO_CONTINUE;
        evento.valor = VALOR_CONTINUE;
    }
}

//----------------------------------------------------
// Interrupciones
//----------------------------------------------------
void ISR_Boton()
{
    if (estadoEmbebido == ESTADO_EMBEBIDO_SIERRA_ACTIVA && motorSierra.estado == ESTADO_MOTOR_PRENDIDO)
    {
        log("Interrupcion detectada, sierra apagada.");
        evento.tipo = EVENTO_SIERRA_DETENIDA;
    }
}

//----------------------------------------------------
// Arduino Setup
//-----------------------------------------------
void setup()
{
    start();
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
	inicializarLedDigital(&ledSierra, PIN_D_LED_SIERRA);
	inicializarMotorSierra(&motorSierra, PIN_D_RELE);
	inicializarPulsadores();
    estadoEmbebido = ESTADO_EMBEBIDO_IDLE;
	evento.tipo = EVENTO_CONTINUE;
}

// ------------------------------------------------
// Lógica de Sensores
// ------------------------------------------------
void verificarPulsadores()
{
    bool pulsadorSierraPresionado = esPulsadorPresionado(PIN_D_PULSADOR_SIERRA);
    if (pulsadorSierraPresionado && motorSierra.estado == ESTADO_MOTOR_APAGADO)
    {
		evento.tipo = EVENTO_ACTIVACION_SIERRA;
      	attachInterrupt(digitalPinToInterrupt(PIN_D_PULSADOR_SIERRA), ISR_Boton, MODO_PIN_SIERRA);
    }
}

bool esPulsadorPresionado(unsigned int pinPulsador)
{
    return digitalRead(pinPulsador) == LOW;
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

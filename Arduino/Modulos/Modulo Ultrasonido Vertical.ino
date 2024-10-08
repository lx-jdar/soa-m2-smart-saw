// ------------------------------------------------
// Etiquetas
// ------------------------------------------------
//#define LOG // Comentar esta linea para desactivar logs

// ------------------------------------------------
// Constantes
// ------------------------------------------------
#define BAUD_RATE 9600
#define CANTIDAD_MAXIMA_DE_ENTRADAS 2
#define VELOCIDAD_DEL_SONIDO 57

// ------------------------------------------------
// Umbrales Distancia
// ------------------------------------------------
#define DISTANCIA_MINIMA_V 150

// ------------------------------------------------
// Pines Sensores (A = Analógico | D = Digital)
// ------------------------------------------------
#define PIN_D_PULSADOR_SIERRA 3
#define PIN_D_TRIGGER_V 12
#define PIN_D_ECHO_V 13

// ------------------------------------------------
// Pines Actuadores (P = PWM | D = Digital)
// ------------------------------------------------
#define PIN_D_LED_SIERRA 9

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
	SIN_MOVIMIENTO,
};

enum estadoUltrasonidoEnum
{
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
long posicionActual = 0;
int entradaActual = 0;
estadoEnum estadoEmbebido;
Evento evento;
Ultrasonido ultrasonidoVertical;
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
void inicializarUltrasonido(Ultrasonido* ultrasonido, int pinTrigger, int pinEcho);
void inicializarLedDigital(LedDigital* ledDigital, int pin);
void inicializarPulsadores();

void loop();
void obtenerEvento();
void maquinaEstado();

void encenderLed(LedDigital* ledDigital);
void apagarLed(LedDigital* ledDigital);

void verificarPulsadores();
void verificarPosicionUltrasonidoVertical();

void actualizarUltrasonido(Ultrasonido* ultrasonido);
bool esPulsadorPresionado(unsigned int pinPulsador);

// ------------------------------------------------
// Captura de eventos
// ------------------------------------------------
void (*verificarEntradas[CANTIDAD_MAXIMA_DE_ENTRADAS])() =
{ 
    verificarPulsadores,
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
                    encenderLed(&ledSierra);
                    estadoEmbebido = ESTADO_EMBEBIDO_SIERRA_ACTIVA;
                    break;
                }
                
                case EVENTO_LIMITE_VERTICAL_SUPERADO:
                {
                    log("ESTADO_EMBEBIDO_SIERRA_ACTIVA", "EVENTO_LIMITE_VERTICAL_SUPERADO");
                    apagarLed(&ledSierra);
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
    
    // Ya se atendió el evento
    evento.tipo = EVENTO_CONTINUE;
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

void inicializarPulsadores()
{
    pinMode(PIN_D_PULSADOR_SIERRA, INPUT_PULLUP);
}

void start()
{
	Serial.begin(BAUD_RATE);
	inicializarLedDigital(&ledSierra, PIN_D_LED_SIERRA);
	inicializarUltrasonido(&ultrasonidoVertical, PIN_D_TRIGGER_V, PIN_D_ECHO_V);
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
	if (pulsadorSierraPresionado)
    {
		evento.tipo = EVENTO_ACTIVACION_SIERRA;
		ultrasonidoVertical.estado = ESTADO_ULT_LONGITUD_PERMITIDA;
    }
}

void verificarPosicionUltrasonidoVertical()
{
	actualizarUltrasonido(&ultrasonidoVertical);
    switch (ultrasonidoVertical.estado)
	{
		case ESTADO_ULT_LONGITUD_PERMITIDA:
		{
			posicionActual = ultrasonidoVertical.cm;
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

// ------------------------------------------------
// Etiquetas
// ------------------------------------------------
//#define LOG // Comentar esta linea para desactivar logs

// ------------------------------------------------
// Constantes
// ------------------------------------------------
#define BAUD_RATE 9600
#define CANTIDAD_MAXIMA_DE_ENTRADAS 4
#define DESPLAZAMIENTO_DERECHA 51
#define DESPLAZAMIENTO_IZQUIERDA 50
#define MARGEN_DE_ERROR 1L
#define MODO_PIN_SIERRA RISING
#define MOTOR_SPEED 128
#define VALOR_CONTINUE -1
#define VELOCIDAD_DEL_SONIDO 57

// ------------------------------------------------
// Umbrales Distancia
// ------------------------------------------------
#define DISTANCIA_MINIMA_V 150
#define DISTANCIA_MAXIMA_H 200
#define DISTANCIA_MINIMA_H 30

// ------------------------------------------------
// Pines Sensores (A = Analógico | D = Digital)
// ------------------------------------------------
#define PIN_D_PULSADOR_SIERRA 3
#define PIN_D_PULSADOR_DERECHA 4
#define PIN_D_PULSADOR_IZQUIERDA 5
#define PIN_D_TRIGGER_H 10
#define PIN_D_ECHO_H 11
#define PIN_D_TRIGGER_V 12
#define PIN_D_ECHO_V 13

// ------------------------------------------------
// Pines Actuadores (P = PWM | D = Digital)
// ------------------------------------------------
#define PIN_D_LED_SIERRA 9
#define PIN_D_LED_DESPLAZAMIENTO A0
#define PIN_P_MDC_SPEED 6   // Modulacion del PWM M1
#define PIN_D_MDC_T1 7      // Control Motor P2 L293D, Terminal 1
#define PIN_D_MDC_T2 8      // Control Motor P2 L293D, Terminal 2
#define PIN_D_RELE 2

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
// Eventos Posibles
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

// ------------------------------------------------
// Estados del Sensor Distancia
// ------------------------------------------------
enum estadoSensorUltrasonidoEnum
{
    ESTADO_ULT_EN_MOVIMIENTO,
    ESTADO_ULT_DETENIDO,
    ESTADO_ULT_UMBRAL_SUPERADO,
    ESTADO_ULT_LONGITUD_PERMITIDA
};

// ------------------------------------------------
// Estados del Monitor
// ------------------------------------------------
enum estadoMonitor
{
    ESTADO_MONITOR_IMPRIMIR,
    ESTADO_MONITOR_ESPERANDO_INGRESO,
    ESTADO_MONITOR_ACTUALIZAR_MENSAJE,
};

// ------------------------------------------------
// Estados del Puente H
// ------------------------------------------------
enum estadoPuenteH
{
    ESTADO_PUENTE_H_PRENDIDO,
    ESTADO_PUENTE_H_APAGADO,
};

// ------------------------------------------------
// Estructura de Evento
// ------------------------------------------------
typedef struct
{
    eventoEnum tipo;
    int valor;
} Evento;

// ------------------------------------------------
// Estructura de Sensores
// ------------------------------------------------
typedef struct
{
    int pinTrigger, pinEcho;
    estadoSensorUltrasonidoEnum estado;
    int sentido;
    unsigned long cm;
} SensorUltrasonido;

// ------------------------------------------------
// Estructura de Actuadores
// ------------------------------------------------
typedef struct
{
    int pinTerminal1, pinTerminal2, pinVelocidad;
    estadoPuenteH estado;
} PuenteH;

typedef struct
{
    int pin;
    bool encendido;
} LedDigital;

// ------------------------------------------------
// Estructura de Monitor
// ------------------------------------------------
typedef struct
{
    String mensaje;
    String input;
    estadoMonitor estado;
    long configuracion;
   // Monitor() : configuracion(BAUD_RATE) {}
} Monitor;

// ------------------------------------------------
// Variables globales
// ------------------------------------------------
int valorDesplazamiento = 0;
long posicionActual = 0;
int posicionDePartida = 0;
int entradaActual = 0;
estadoEnum estadoActual;
Evento evento;
SensorUltrasonido ultrasonidoHorizontal;
SensorUltrasonido ultrasonidoVertical;
PuenteH puenteH;
LedDigital ledSierra;
LedDigital ledDesplazamiento;
Monitor monitor;

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

void log(const char *msg)
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
void loop();
void obtenerEvento();
void maquinaEstado();
void ISF();

void encenderMotorYLedDesplazamiento(unsigned int pinPulsador);
void detenerMotorYLedDesplazamiento();
void encenderMotorYLedSierra();
void detenerMotorYLedSierra();

void verificarLecturaDesdeMonitorSerial();
void verificarPulsadores();
void verificarPosicionUltrasonidoHorizontal();
void verificarPosicionUltrasonidoVertical();
void verificarLimitesHorizontales();
void actualizarUltrasonido(SensorUltrasonido* sensorUltrasonido);
bool esPulsadorPresionado(unsigned int pinPulsador);
void inicializarMonitor(Monitor* monitor);

// ------------------------------------------------
// Captura de eventos
// ------------------------------------------------
void (*verificarEntradas[CANTIDAD_MAXIMA_DE_ENTRADAS])() =
{ 
    verificarLecturaDesdeMonitorSerial,
    verificarPulsadores,
    verificarPosicionUltrasonidoHorizontal,
    verificarPosicionUltrasonidoVertical,
};

void obtenerEvento()
{
    verificarEntradas[entradaActual]();
    if(entradaActual < CANTIDAD_MAXIMA_DE_ENTRADAS - 1)
    {
        entradaActual++;
    }
    else
    {
        entradaActual = 0;
        evento.tipo = EVENTO_CONTINUE;
        evento.valor = VALOR_CONTINUE;
    }
}

// ------------------------------------------------
// Implementación máquina de estados
// ------------------------------------------------
void maquinaEstado()
{
    obtenerEvento();
    switch (estadoActual)
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
                        monitor.mensaje = "El motor se desplazara " + String(valorDesplazamiento) + " CM cuando se presione un pulsador.";
                    }
                    else
                    {
                        monitor.mensaje = "El valor ingresado no es valido.";
                    }
                    monitor.estado = ESTADO_MONITOR_IMPRIMIR;
                    estadoActual = ESTADO_EMBEBIDO_IDLE;
                    break;
                }

                case EVENTO_LIMITE_HORIZONTAL_SUPERADO:
                {
                    monitor.mensaje = "El valor ingresado " + String(valorDesplazamiento) + " supera los limites establecidos del desplazamiento.";
                    monitor.estado = ESTADO_MONITOR_IMPRIMIR;
                    estadoActual = ESTADO_EMBEBIDO_IDLE;
                    break;
                }

                case EVENTO_DESPLAZAMIENTO_IZQUIERDA:
                {
                    log("ESTADO_EMBEBIDO_IDLE", "EVENTO_DESPLAZAMIENTO_IZQUIERDA");
                    estadoActual = ESTADO_EMBEBIDO_EN_MOVIMIENTO;
                    break;
                }

                case EVENTO_DESPLAZAMIENTO_DERECHA:
                {
                    log("ESTADO_EMBEBIDO_IDLE", "EVENTO_DESPLAZAMIENTO_DERECHA");
                    estadoActual = ESTADO_EMBEBIDO_EN_MOVIMIENTO;
                    break;
                }

                case EVENTO_ACTIVACION_SIERRA:
                {
                    log("ESTADO_EMBEBIDO_IDLE", "EVENTO_ACTIVACION_SIERRA");
                    estadoActual = ESTADO_EMBEBIDO_SIERRA_ACTIVA;
                    break;
                }

                /*case EVENTO_CONTINUE:
                    estadoActual = ESTADO_EMBEBIDO_IDLE;
                    break;*/

                default:
                    break;
            }
            break;
        }
        break;

        case ESTADO_EMBEBIDO_EN_MOVIMIENTO:
        {
            switch (evento.tipo)
            {
                case EVENTO_DESPLAZAMIENTO_IZQUIERDA:
                {
                    encenderMotorYLedDesplazamiento(PIN_D_PULSADOR_IZQUIERDA);
                    monitor.mensaje = "Pulsador izquierdo presionado. Desplazando motor a izquierda.";
                    monitor.estado = ESTADO_MONITOR_IMPRIMIR;
                    log("ESTADO_EMBEBIDO_EN_MOVIMIENTO", "EVENTO_DESPLAZAMIENTO_IZQUIERDA");
                    estadoActual = ESTADO_EMBEBIDO_EN_MOVIMIENTO;
                    break;
                }
                
                case EVENTO_DESPLAZAMIENTO_DERECHA:
                {
                    encenderMotorYLedDesplazamiento(PIN_D_PULSADOR_DERECHA);
                    monitor.mensaje = "Pulsador derecho presionado. Desplazando motor a derecha.";
                    monitor.estado = ESTADO_MONITOR_IMPRIMIR;
                    log("ESTADO_EMBEBIDO_EN_MOVIMIENTO", "EVENTO_DESPLAZAMIENTO_DERECHA");
                    estadoActual = ESTADO_EMBEBIDO_EN_MOVIMIENTO;
                    break;
                }
                
                case EVENTO_POSICION_FINALIZADA:
                {
                    detenerMotorYLedDesplazamiento();
                    monitor.mensaje = "Posicionamiento finalizado.";
                    monitor.estado = ESTADO_MONITOR_IMPRIMIR;
                    log("ESTADO_EMBEBIDO_EN_MOVIMIENTO", "EVENTO_POSICION_FINALIZADA");
                    estadoActual = ESTADO_EMBEBIDO_IDLE;
                    break;
                }
                
                case EVENTO_LIMITE_HORIZONTAL_SUPERADO:
                {
                    detenerMotorYLedDesplazamiento();
                    monitor.mensaje = "Se quiere desplazar mas que el umbral maximo. Motor de desplazamiento apagandose ...";
                    monitor.estado = ESTADO_MONITOR_IMPRIMIR;
                    log("ESTADO_EMBEBIDO_EN_MOVIMIENTO", "EVENTO_LIMITE_HORIZONTAL_SUPERADO");
                    estadoActual = ESTADO_EMBEBIDO_IDLE;
                    break;
                }
                
                /*case EVENTO_CONTINUE:
                    log("ESTADO_EMBEBIDO_EN_MOVIMIENTO", "EVENTO_CONTINUE");
                    estadoActual = ESTADO_EMBEBIDO_EN_MOVIMIENTO;
                    break;*/
                
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
                    encenderMotorYLedSierra();
                    monitor.mensaje = "Pulsador sierrra encendida. Cortando ...";
                    monitor.estado = ESTADO_MONITOR_IMPRIMIR;
                    log("ESTADO_EMBEBIDO_SIERRA_ACTIVA", "EVENTO_ACTIVACION_SIERRA");
                    estadoActual = ESTADO_EMBEBIDO_SIERRA_ACTIVA;
                    break;
                }
                
                case EVENTO_SIERRA_DETENIDA:
                {
                    detenerMotorYLedSierra();
                    monitor.mensaje = "Deteniendo el motor sierra ...";
                    monitor.estado = ESTADO_MONITOR_IMPRIMIR;
                    log("ESTADO_EMBEBIDO_SIERRA_ACTIVA", "EVENTO_SIERRA_DETENIDA");
                    estadoActual = ESTADO_EMBEBIDO_IDLE;
                    break;
                }
                
                case EVENTO_LIMITE_VERTICAL_SUPERADO:
                {
                    detenerMotorYLedSierra();
                    monitor.mensaje = "Pase el umbral minimo. Motor de sierra apagandose ...";
                    monitor.estado = ESTADO_MONITOR_IMPRIMIR;
                    log("ESTADO_EMBEBIDO_SIERRA_ACTIVA", "EVENTO_LIMITE_VERTICAL_SUPERADO");
                    estadoActual = ESTADO_EMBEBIDO_IDLE;
                    break;
                }
                
                /*case EVENTO_CONTINUE:
                    log("ESTADO_EMBEBIDO_SIERRA_ACTIVA", "EVENTO_CONTINUE");
                    estadoActual = ESTADO_EMBEBIDO_SIERRA_ACTIVA;
                    break;*/
                
                default:
                    break;
            }
            break;
        }
        break;
    }
    
    // Ya se atendió el evento
    /*evento.tipo = EVENTO_CONTINUE;
    evento.valor = VALOR_CONTINUE;*/
}

//----------------------------------------------------
// Interrupciones
//----------------------------------------------------
void ISF()
{
    if (estadoActual == ESTADO_EMBEBIDO_SIERRA_ACTIVA)
    {
        Serial.println("Interrupcion detectada, sierra apagada.");
        evento.tipo = EVENTO_SIERRA_DETENIDA;
    }
}

//----------------------------------------------------
// Arduino Setup
//-----------------------------------------------
void setup()
{
    start();
    attachInterrupt(digitalPinToInterrupt(PIN_D_PULSADOR_SIERRA), ISF, MODO_PIN_SIERRA);
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
void start()
{
    // Monitor Serial
    monitor.estado = ESTADO_MONITOR_ACTUALIZAR_MENSAJE;
    inicializarMonitor(&monitor);

    // Relé
    pinMode(PIN_D_RELE, OUTPUT);

    // Puente H
    puenteH.pinTerminal1 = PIN_D_MDC_T1;
    puenteH.pinTerminal2 = PIN_D_MDC_T2;
    puenteH.pinVelocidad = PIN_P_MDC_SPEED;
    puenteH.estado = ESTADO_PUENTE_H_APAGADO;
    pinMode(puenteH.pinTerminal1, OUTPUT);
    pinMode(puenteH.pinTerminal2, OUTPUT);
    pinMode(puenteH.pinVelocidad, OUTPUT);

    // Pulsadores
    pinMode(PIN_D_PULSADOR_IZQUIERDA, INPUT);
    pinMode(PIN_D_PULSADOR_DERECHA, INPUT);
    pinMode(PIN_D_PULSADOR_SIERRA, INPUT);

    // LEDs
    ledSierra.pin = PIN_D_LED_SIERRA;
    ledDesplazamiento.pin = PIN_D_LED_DESPLAZAMIENTO;
    ledSierra.encendido = false;
    ledSierra.encendido = false;
    pinMode(ledSierra.pin, OUTPUT);
    pinMode(ledDesplazamiento.pin, OUTPUT);

    // Sensor Ultrasonido Horizontal
    ultrasonidoHorizontal.pinTrigger = PIN_D_TRIGGER_H;
    ultrasonidoHorizontal.pinEcho = PIN_D_ECHO_H;
    ultrasonidoHorizontal.estado = ESTADO_ULT_DETENIDO;
    pinMode(ultrasonidoHorizontal.pinTrigger, OUTPUT);
    pinMode(ultrasonidoHorizontal.pinEcho, INPUT);

    // Sensor Ultrasonido Vertical
    ultrasonidoVertical.pinTrigger = PIN_D_TRIGGER_V;
    ultrasonidoVertical.pinEcho = PIN_D_ECHO_V;
    ultrasonidoVertical.estado = ESTADO_ULT_DETENIDO;
    pinMode(PIN_D_TRIGGER_V, OUTPUT);
    pinMode(PIN_D_ECHO_V, INPUT);

    // Inicializar motores
    detenerMotorYLedDesplazamiento();
    detenerMotorYLedSierra();

    // Inicalizar estado embebido
    estadoActual = ESTADO_EMBEBIDO_IDLE;
}

// ------------------------------------------------
// Comunicación
// ------------------------------------------------
void inicializarMonitor(Monitor* monitor)
{
    monitor->configuracion = BAUD_RATE;
}

void verificarLecturaDesdeMonitorSerial()
{
    switch (monitor.estado)
    {
        case ESTADO_MONITOR_ACTUALIZAR_MENSAJE:
        {
            monitor.mensaje = "Ingrese la cantidad de desplazamiento (en CM) que debera realizar el motor:";
            monitor.estado = ESTADO_MONITOR_IMPRIMIR;
            break;
        }
        
        case ESTADO_MONITOR_IMPRIMIR:
        {
            Serial.println(monitor.mensaje);
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
            break;
    }
}

// ------------------------------------------------
// Lógica de Sensores
// ------------------------------------------------
void verificarPulsadores()
{
    bool pulsadorIzquierdaPresionado = esPulsadorPresionado(PIN_D_PULSADOR_IZQUIERDA);
    bool pulsadorDerechaPresionado = esPulsadorPresionado(PIN_D_PULSADOR_DERECHA);
    bool pulsadorSierraPresionado = esPulsadorPresionado(PIN_D_PULSADOR_SIERRA);
    if (pulsadorIzquierdaPresionado)
    {
        evento.tipo = EVENTO_DESPLAZAMIENTO_IZQUIERDA;
        ultrasonidoHorizontal.estado = ESTADO_ULT_EN_MOVIMIENTO;
        ultrasonidoHorizontal.sentido = DESPLAZAMIENTO_IZQUIERDA;
    }
    else if (pulsadorDerechaPresionado)
    {
        evento.tipo = EVENTO_DESPLAZAMIENTO_DERECHA;
        ultrasonidoHorizontal.estado = ESTADO_ULT_EN_MOVIMIENTO;
        ultrasonidoHorizontal.sentido = DESPLAZAMIENTO_DERECHA;
    }
    else if (pulsadorSierraPresionado)
    {
        evento.tipo = EVENTO_ACTIVACION_SIERRA;
        ultrasonidoVertical.estado = ESTADO_ULT_LONGITUD_PERMITIDA;
    }
}

void verificarLimitesHorizontales()
{
    if(valorDesplazamiento)
    {
        actualizarUltrasonido(&ultrasonidoHorizontal);
        posicionActual = ultrasonidoHorizontal.cm;
        int posibleDesplazamiento = posicionActual + (ultrasonidoHorizontal.sentido == DESPLAZAMIENTO_IZQUIERDA ? valorDesplazamiento : - valorDesplazamiento);
        if (posibleDesplazamiento <= DISTANCIA_MINIMA_H || posibleDesplazamiento >= DISTANCIA_MAXIMA_H)
        {
            ultrasonidoHorizontal.estado = ESTADO_ULT_UMBRAL_SUPERADO;
        }
    }
}

void verificarPosicionUltrasonidoHorizontal()
{
    verificarLimitesHorizontales();
    switch (ultrasonidoHorizontal.estado)
    {
        case ESTADO_ULT_UMBRAL_SUPERADO:
        {
            evento.tipo = EVENTO_LIMITE_HORIZONTAL_SUPERADO;
            break;
        }
        
        case ESTADO_ULT_EN_MOVIMIENTO:
        {
            actualizarUltrasonido(&ultrasonidoHorizontal);
            posicionActual = ultrasonidoHorizontal.cm;
            Serial.println("Posicion actual Ultrasonido Horizontal: " + String(posicionActual) + " CM.");
            if (posicionActual >= DISTANCIA_MAXIMA_H)
            {
                evento.tipo = EVENTO_LIMITE_HORIZONTAL_SUPERADO;
            }
            else
            {
                int delta = abs(posicionActual - posicionDePartida);
                if (delta >= valorDesplazamiento)
                {  
                    ultrasonidoHorizontal.estado = ESTADO_ULT_DETENIDO;
                    return;
                }
                Serial.println("El motor se desplazo horizontalmente hacia la " +
                    String(evento.tipo == EVENTO_DESPLAZAMIENTO_IZQUIERDA? "izquierda" : "derecha") + " unos " +
                    String(delta) + " CM de " + String(valorDesplazamiento) + " CM.");
            }
            break;
        }
        
        case ESTADO_ULT_DETENIDO:
        {
            evento.tipo = EVENTO_POSICION_FINALIZADA;
            break;
        }
        
        default:
            break;
    }
}

void verificarPosicionUltrasonidoVertical()
{
    actualizarUltrasonido (&ultrasonidoVertical);
    posicionActual = ultrasonidoVertical.cm;
    Serial.println("Posicion actual ULT V: " + String(posicionActual) + " CM.");
    if (posicionActual <= DISTANCIA_MINIMA_V)
    {
        evento.tipo = EVENTO_LIMITE_VERTICAL_SUPERADO;
        ultrasonidoVertical.estado = ESTADO_ULT_UMBRAL_SUPERADO;
    }
}

void actualizarUltrasonido(SensorUltrasonido* sensorUltrasonido)
{
    digitalWrite(sensorUltrasonido->pinTrigger, LOW);
    delayMicroseconds(2);
    digitalWrite(sensorUltrasonido->pinTrigger, HIGH);
    delayMicroseconds(10);
    digitalWrite(sensorUltrasonido->pinTrigger, LOW);
    unsigned long tiempoPulso = pulseIn(sensorUltrasonido->pinEcho, HIGH);
    if (sensorUltrasonido->estado == ESTADO_ULT_EN_MOVIMIENTO)
    {
        tiempoPulso += sensorUltrasonido->sentido == DESPLAZAMIENTO_IZQUIERDA ? MARGEN_DE_ERROR : - MARGEN_DE_ERROR;
    }
    unsigned long distanciaCm = tiempoPulso / VELOCIDAD_DEL_SONIDO;
    sensorUltrasonido->cm = distanciaCm;
}

bool esPulsadorPresionado(unsigned int pinPulsador)
{
    return digitalRead(pinPulsador) == HIGH;
}

//----------------------------------------------------
// Lógica de Actuadores
//----------------------------------------------------
void encenderMotorYLedDesplazamiento(unsigned int pinPulsador)
{
    analogWrite(puenteH.pinVelocidad, MOTOR_SPEED);
    if (pinPulsador == PIN_D_PULSADOR_IZQUIERDA)
    {
        digitalWrite(puenteH.pinTerminal1, HIGH);
        digitalWrite(puenteH.pinTerminal2, LOW);
    }
    else if (pinPulsador == PIN_D_PULSADOR_DERECHA)
    {
        digitalWrite(puenteH.pinTerminal1, LOW);
        digitalWrite(puenteH.pinTerminal2, HIGH);
    }
    digitalWrite(ledDesplazamiento.pin, HIGH);
    ledDesplazamiento.encendido = true;
}

void detenerMotorYLedDesplazamiento()
{
    if(puenteH.estado != ESTADO_PUENTE_H_APAGADO)
    {
        analogWrite(puenteH.pinVelocidad, 0);
        digitalWrite(puenteH.pinTerminal1, LOW);
        digitalWrite(puenteH.pinTerminal2, LOW);
        puenteH.estado = ESTADO_PUENTE_H_APAGADO;

        digitalWrite(ledDesplazamiento.pin, LOW);
        ledDesplazamiento.encendido = false;
    }
}

void encenderMotorYLedSierra()
{
    digitalWrite(PIN_D_RELE, HIGH);
    digitalWrite(ledSierra.pin, HIGH);
    ledSierra.encendido = true;
}

void detenerMotorYLedSierra()
{
    digitalWrite(PIN_D_RELE, LOW);
    digitalWrite(ledSierra.pin, LOW);
    ledSierra.encendido = false;
}

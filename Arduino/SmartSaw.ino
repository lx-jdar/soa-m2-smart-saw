// ------------------------------------------------
// Etiquetas
// ------------------------------------------------
//#define LOG // Comentar esta linea para desactivar logs

// ------------------------------------------------
// Constantes
// ------------------------------------------------
#define VALOR_CONTINUE -1
#define MARGEN_DE_ERROR 1L
#define BAUD_RATE 9600
#define MOTOR_SPEED 128
#define VELOCIDAD_DEL_SONIDO 57
#define MODO_PIN_SIERRA RISING

// ------------------------------------------------
// Umbrales Distancia
// ------------------------------------------------
#define DISTANCIA_MINIMA_V 150
#define DISTANCIA_MAXIMA_H 200  // REVISAR LOGICA (CUANDO SE VA DESPLAZANDO SUMA EL TOPE Y NO DESPLAZA DEL TODO), VER FUNCION checkearPosicionMotor

// ------------------------------------------------
// Pines sensores (A = analógico | D = Digital)
// ------------------------------------------------
#define PIN_D_PULSADOR_SIERRA 3
#define PIN_D_PULSADOR_DERECHA 4
#define PIN_D_PULSADOR_IZQUIERDA 5
#define PIN_D_TRIGGER_H 10
#define PIN_D_ECHO_H 11
#define PIN_D_TRIGGER_V 12
#define PIN_D_ECHO_V 13

// ------------------------------------------------
// Pines actuadores (P = PWM | D = Digital)
// ------------------------------------------------
#define PIN_D_LED_SIERRA 9
#define PIN_D_LED_DESPLAZAMIENTO A0
#define PIN_P_MDC_SPEED 6   // Modulacion del PWM M1
#define PIN_D_MDC_T1 7      // Control Motor P2 L293D, Terminal 1
#define PIN_D_MDC_T2 8      // Control Motor P2 L293D, Terminal 2
#define PIN_D_RELE 2

// ------------------------------------------------
// Estados del embebido
// ------------------------------------------------
enum estadoEnum
{
    ESTADO_IDLE,
    ESTADO_EN_MOVIMIENTO,
    ESTADO_SIERRA_ACTIVA
};

// ------------------------------------------------
// Eventos posibles
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
  EVENTO_UMBRAL_HORIZONTAL_SUPERADO,
  EVENTO_UMBRAL_VERTICAL_SUPERADO,
};

// ------------------------------------------------
// Estados del sensor distancia
// ------------------------------------------------
enum estadoSensorUltrasonidoEnum
{
    ULTRASONIDO_EN_MOVIMIENTO,
    ULTRASONIDO_SIN_MOVIMIENTO
};

// ------------------------------------------------
// Estructura de Actuadores
// ------------------------------------------------
typedef struct
{
    int pinTerminal1, pinTerminal2, pinVelocidad;
} PuenteH;

typedef struct
{
    int pin;
    bool encendido;
} LedDigital;

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
    unsigned long cm;
} SensorUltrasonido;

// ------------------------------------------------
// Variables globales
// ------------------------------------------------
bool imprimirMsg = true;
int valorDesplazamiento = 0;
long posicionActual = 0;
int posicionDePartida = 0;
estadoEnum estadoActual;
Evento evento;
SensorUltrasonido ultrasonidoHorizontal;
SensorUltrasonido ultrasonidoVertical;
PuenteH puenteH;
LedDigital ledSierra;
LedDigital ledDesplazamiento;

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
void getEvento();
void maquinaEstado();
void ISF();
void setup();
void loop();
void start();
void leerTiempoDesdeSerie();
void checkearPosicionMotor(unsigned int pinPulsador);
void encenderMotorYLedDesplazamiento(unsigned int pinPulsador);
void detenerMotorYLedDesplazamiento();
void checkearLongitudDeCorte();
void encenderMotorYLedSierra();
void detenerMotorYLedSierra();
void medirDistanciaUltrasonido(SensorUltrasonido* sensorUltrasonido, unsigned int pinPulsador);
bool esPulsadorPresionado(unsigned int pinPulsador);

// ------------------------------------------------
// Captura de eventos
// ------------------------------------------------
void getEvento()
{
    if (esPulsadorPresionado(PIN_D_PULSADOR_IZQUIERDA))
    {
        evento.tipo = EVENTO_DESPLAZAMIENTO_IZQUIERDA;
    }
    else if (esPulsadorPresionado(PIN_D_PULSADOR_DERECHA))
    {
        evento.tipo = EVENTO_DESPLAZAMIENTO_DERECHA;
    }
    else if (esPulsadorPresionado(PIN_D_PULSADOR_SIERRA))
    {
        if (estadoActual != ESTADO_SIERRA_ACTIVA)
        {
            evento.tipo = EVENTO_ACTIVACION_SIERRA;
        }
        else if (estadoActual == ESTADO_SIERRA_ACTIVA)
        {
            evento.tipo = EVENTO_SIERRA_DETENIDA;
        }
    }
    /*else
    {
      evento.tipo = EVENTO_CONTINUE;
    }*/
}

// ------------------------------------------------
// Implementación máquina de estados
// ------------------------------------------------
void maquinaEstado()
{
    getEvento();
    switch (estadoActual)
    {
        case ESTADO_IDLE:
        {
            leerTiempoDesdeSerie();
            switch (evento.tipo)
            {
                case EVENTO_DESPLAZAMIENTO_IZQUIERDA:
                    ultrasonidoHorizontal.estado = ULTRASONIDO_EN_MOVIMIENTO;
                    medirDistanciaUltrasonido(&ultrasonidoHorizontal, PIN_D_PULSADOR_IZQUIERDA);
                    posicionDePartida = ultrasonidoHorizontal.cm;
                    encenderMotorYLedDesplazamiento(PIN_D_PULSADOR_IZQUIERDA);
                    Serial.println("Pulsador izquierdo presionado. Desplazando motor a izquierda.");
                    log("ESTADO_IDLE", "EVENTO_DESPLAZAMIENTO_IZQUIERDA");
                    imprimirMsg = true;
                    estadoActual = ESTADO_EN_MOVIMIENTO;
                    break;
                case EVENTO_DESPLAZAMIENTO_DERECHA:
                    ultrasonidoHorizontal.estado = ULTRASONIDO_EN_MOVIMIENTO;
                    medirDistanciaUltrasonido(&ultrasonidoHorizontal, PIN_D_PULSADOR_DERECHA);
                    posicionDePartida = ultrasonidoHorizontal.cm;
                    encenderMotorYLedDesplazamiento(PIN_D_PULSADOR_DERECHA);
                    Serial.println("Pulsador derecho presionado. Desplazando motor a derecha.");
                    log("ESTADO_IDLE", "EVENTO_DESPLAZAMIENTO_DERECHA");
                    imprimirMsg = true;
                    estadoActual = ESTADO_EN_MOVIMIENTO;
                    break;
                case EVENTO_ACTIVACION_SIERRA:
                    ultrasonidoVertical.estado = ULTRASONIDO_SIN_MOVIMIENTO;
                    medirDistanciaUltrasonido(&ultrasonidoVertical, PIN_D_PULSADOR_SIERRA);
                    posicionDePartida = ultrasonidoVertical.cm;
                    encenderMotorYLedSierra();
                    Serial.println("Pulsador sierrra encendida. Cortando ...");
                    log("ESTADO_IDLE", "EVENTO_ACTIVACION_SIERRA");
                    estadoActual = ESTADO_SIERRA_ACTIVA;
                break;
                /*case EVENTO_CONTINUE:
                    estadoActual = ESTADO_IDLE;
                    break;*/
                default:
                    break;
            }
            break;
        }
        break;

        case ESTADO_EN_MOVIMIENTO:
        {
            switch (evento.tipo)
            {
                case EVENTO_DESPLAZAMIENTO_IZQUIERDA:
                    checkearPosicionMotor(PIN_D_PULSADOR_IZQUIERDA);
                    log("ESTADO_EN_MOVIMIENTO", "EVENTO_DESPLAZAMIENTO_IZQUIERDA");
                    estadoActual = ESTADO_EN_MOVIMIENTO;
                    break;
                case EVENTO_DESPLAZAMIENTO_DERECHA:
                    checkearPosicionMotor(PIN_D_PULSADOR_DERECHA);
                    log("ESTADO_EN_MOVIMIENTO", "EVENTO_DESPLAZAMIENTO_DERECHA");
                    estadoActual = ESTADO_EN_MOVIMIENTO;
                    break;
                case EVENTO_POSICION_FINALIZADA:
                    detenerMotorYLedDesplazamiento();
                    Serial.println("Posicionamiento finalizado.");
                    valorDesplazamiento = 0;
                    log("ESTADO_EN_MOVIMIENTO", "EVENTO_POSICION_FINALIZADA");
                    estadoActual = ESTADO_IDLE;
                    break;
                case EVENTO_UMBRAL_HORIZONTAL_SUPERADO:
                    detenerMotorYLedDesplazamiento();
                    Serial.println("Se quiere desplazar mas que el umbral maximo. Motor de desplazamiento apagandose ...");
                    valorDesplazamiento = 0;
                    log("ESTADO_EN_MOVIMIENTO", "EVENTO_UMBRAL_HORIZONTAL_SUPERADO");
                    estadoActual = ESTADO_IDLE;
                    break;
                /*case EVENTO_CONTINUE:
                    log("ESTADO_EN_MOVIMIENTO", "EVENTO_CONTINUE");
                    estadoActual = ESTADO_EN_MOVIMIENTO;
                    break;*/
                default:
                    break;
            }
            break;
        }
        break;

        case ESTADO_SIERRA_ACTIVA:
        {
            switch (evento.tipo)
            {
                case EVENTO_ACTIVACION_SIERRA:
                    checkearLongitudDeCorte();
                    log("ESTADO_SIERRA_ACTIVA", "EVENTO_ACTIVACION_SIERRA");
                    estadoActual = ESTADO_SIERRA_ACTIVA;
                    break;
                case EVENTO_SIERRA_DETENIDA:
                    detenerMotorYLedSierra();
                    Serial.println("Deteniendo el motor sierra ...");
                    log("ESTADO_SIERRA_ACTIVA", "EVENTO_SIERRA_DETENIDA");
                    estadoActual = ESTADO_IDLE;
                    break;
                case EVENTO_UMBRAL_VERTICAL_SUPERADO:
                    detenerMotorYLedSierra();
                    Serial.println("Pase el umbral minimo o presione el boton. Motor de sierra apagandose ...");
                    log("ESTADO_SIERRA_ACTIVA", "EVENTO_UMBRAL_VERTICAL_SUPERADO");
                    estadoActual = ESTADO_IDLE;
                    break;
                /*case EVENTO_CONTINUE:
                    log("ESTADO_SIERRA_ACTIVA", "EVENTO_CONTINUE");
                    estadoActual = ESTADO_SIERRA_ACTIVA;
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
    if (estadoActual == ESTADO_SIERRA_ACTIVA)
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
    Serial.begin(BAUD_RATE);

    // Relé
    pinMode(PIN_D_RELE, OUTPUT);

    // Puente H
    puenteH.pinTerminal1 = PIN_D_MDC_T1;
    puenteH.pinTerminal2 = PIN_D_MDC_T2;
    puenteH.pinVelocidad = PIN_P_MDC_SPEED;
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
    ultrasonidoHorizontal.estado = ULTRASONIDO_SIN_MOVIMIENTO;
    pinMode(ultrasonidoHorizontal.pinTrigger, OUTPUT);
    pinMode(ultrasonidoHorizontal.pinEcho, INPUT);

    // Sensor Ultrasonido Vertical
    ultrasonidoVertical.pinTrigger = PIN_D_TRIGGER_V;
    ultrasonidoVertical.pinEcho = PIN_D_ECHO_V;
    ultrasonidoVertical.estado = ULTRASONIDO_SIN_MOVIMIENTO;
    pinMode(PIN_D_TRIGGER_V, OUTPUT);
    pinMode(PIN_D_ECHO_V, INPUT);

    // Inicializar motores
    detenerMotorYLedDesplazamiento();
    detenerMotorYLedSierra();

    // Inicalizar estado embebido
    estadoActual = ESTADO_IDLE;
}

//----------------------------------------------------
// Funcionalidades del Motor Desplazamiento
//----------------------------------------------------
void leerTiempoDesdeSerie()
{
    if (imprimirMsg)
    {
        Serial.println("Ingrese la cantidad de desplazamiento (en CM) que debera realizar el motor:");
        imprimirMsg = false;
    }
    if (Serial.available() > 0)
    {
        String input = Serial.readStringUntil('\n');
        valorDesplazamiento = input.toInt();
        if (valorDesplazamiento > 0)
        {
            Serial.println("El motor se desplazara " + String(valorDesplazamiento) + " CM cuando se presione un pulsador.");
        }
        else
        {
            Serial.println("El valor ingresado no es valido.");
            imprimirMsg = true;
        }
    }
}

void checkearPosicionMotor(unsigned int pinPulsador)
{
    medirDistanciaUltrasonido(&ultrasonidoHorizontal, pinPulsador);
    posicionActual = ultrasonidoHorizontal.cm;
    Serial.println("Posicion actual ULT H: " + String(posicionActual) + " CM.");
    // SE DEBE CALCULAR EN OTRO LADO, QUIZAS NI BIEN APRETADO EL BOTON PARA DONDE SE QUIERA CORRER
    //int posibleDesplazamiento = posicionActual + (pinPulsador == PIN_D_PULSADOR_IZQUIERDA ? -valorDesplazamiento : valorDesplazamiento);
    if (posicionActual >= DISTANCIA_MAXIMA_H)
    {
        evento.tipo = EVENTO_UMBRAL_HORIZONTAL_SUPERADO;
    }
    else
    {
        int delta = abs(posicionActual - posicionDePartida);
        if (delta >= valorDesplazamiento)
        {  
            evento.tipo = EVENTO_POSICION_FINALIZADA;
            return;
        }
        Serial.println("El motor se desplazo horizontalmente hacia la " +
          String(pinPulsador == PIN_D_PULSADOR_IZQUIERDA ? "izquierda" : "derecha") + " unos " +
          String(delta) + " CM de " + String(valorDesplazamiento) + " CM.");
    }
}

void encenderMotorYLedDesplazamiento(unsigned int pinPulsador)
{
    if (pinPulsador == PIN_D_PULSADOR_IZQUIERDA)
    {
        analogWrite(puenteH.pinVelocidad, MOTOR_SPEED);
        digitalWrite(puenteH.pinTerminal1, HIGH);
        digitalWrite(puenteH.pinTerminal2, LOW);
        digitalWrite(ledDesplazamiento.pin, HIGH);
        ledDesplazamiento.encendido = true;
    }
    else if (pinPulsador == PIN_D_PULSADOR_DERECHA)
    {
        analogWrite(puenteH.pinVelocidad, MOTOR_SPEED);
        digitalWrite(puenteH.pinTerminal1, LOW);
        digitalWrite(puenteH.pinTerminal2, HIGH);
        digitalWrite(ledDesplazamiento.pin, HIGH);
        ledDesplazamiento.encendido = true;
    }
}

void detenerMotorYLedDesplazamiento()
{
    analogWrite(puenteH.pinVelocidad, 0);
    digitalWrite(puenteH.pinTerminal1, LOW);
    digitalWrite(puenteH.pinTerminal2, LOW);
    digitalWrite(ledDesplazamiento.pin, LOW);
    ledDesplazamiento.encendido = false;
}

//----------------------------------------------------
// Funcionalidades del Motor Sierra
//----------------------------------------------------
void checkearLongitudDeCorte()
{
    medirDistanciaUltrasonido(&ultrasonidoVertical, PIN_D_PULSADOR_SIERRA);
    posicionActual = ultrasonidoVertical.cm;
    Serial.println("Posicion actual ULT V: " + String(posicionActual) + " CM.");
    if (posicionActual <= DISTANCIA_MINIMA_V)  // esPulsadorPresionado(PIN_D_PULSADOR_SIERRA) ||
    {
        evento.tipo = EVENTO_UMBRAL_VERTICAL_SUPERADO;
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

//----------------------------------------------------
// Otras Funcionalidades
//----------------------------------------------------
void medirDistanciaUltrasonido(SensorUltrasonido* sensorUltrasonido, unsigned int pinPulsador)
{
    digitalWrite(sensorUltrasonido->pinTrigger, LOW);
    delayMicroseconds(2);
    digitalWrite(sensorUltrasonido->pinTrigger, HIGH);
    delayMicroseconds(10);
    digitalWrite(sensorUltrasonido->pinTrigger, LOW);
    unsigned long tiempoPulso = pulseIn(sensorUltrasonido->pinEcho, HIGH);
    if (sensorUltrasonido->estado == ULTRASONIDO_EN_MOVIMIENTO || pinPulsador != PIN_D_PULSADOR_SIERRA)
    {
        tiempoPulso += pinPulsador == PIN_D_PULSADOR_IZQUIERDA ? MARGEN_DE_ERROR : - MARGEN_DE_ERROR;
    }
    unsigned long distanciaCm = tiempoPulso / VELOCIDAD_DEL_SONIDO;
    sensorUltrasonido->cm = distanciaCm;
}

bool esPulsadorPresionado(unsigned int pinPulsador)
{
    return digitalRead(pinPulsador) == HIGH;
}

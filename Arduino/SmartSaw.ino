// Pulsadores
#define PULSADOR_SIERRA 3
#define PULSADOR_DER 4
#define PULSADOR_IZQ 5

// Puente H
#define PM_SPEED 6			// Modulacion del PWM M1 
#define M_T1 7				// Control Motor P2 L293D, Terminal 1
#define M_T2 8				// Control Motor P2 L293D, Terminal 2

// LED's
#define LED_SIERRA 9
#define LED_DESPLAZAMIENTO A0

// Relé
#define RELE 2

// Sensores Ultrasonido
#define TRIGGER_H 10
#define ECHO_H 11
#define TRIGGER_V 12
#define ECHO_V 13

// Estados del embebido
#define IDLE 14
#define EN_MOVIMIENTO 15
#define SIERRA_ACTIVA 16

// Tolerancia de longitud para apagar el motor (REVISAR)
#define DISTANCIA_MINIMA_V 150

int MOTOR_SPEED = 128;

int estadoEmbebido = IDLE;
long MARGEN_DE_ERROR = 1;

bool imprimirMsg;

// Variable para almacenar el valor de desplazamiento del motor en cm
int valorDesplazamiento = 0;

// Estado del motor (encendido/apagado)
bool sierraEncendida = false;

// Para detectar si el botón fue presionado
bool botonSierraPresionado = false;

// Para calcular la posicion actual del ultrasonido horizontal
long posicionActual = 0;

int posicionDePartida;

int pulsadorPresionado;

//----------------------------------------------------
// Arduino Setup
//----------------------------------------------------
void setup()
{
  inicializarHardware();
  imprimirMsg = true;
}

//----------------------------------------------------
// Arduino Loop
//----------------------------------------------------
void loop()
{
  msf();
}

//ISF() {
  //estadoEmbebido = IDLE;
//}

//----------------------------------------------------
// Inicialización
//----------------------------------------------------
void inicializarHardware() 
{
  // Monitor Serial
  Serial.begin(9600);
  
  // Rele
  pinMode(RELE, OUTPUT);
  
  // Puente H
  pinMode(PM_SPEED, OUTPUT);
  pinMode(M_T1, OUTPUT);
  pinMode(M_T2, OUTPUT);
  
  // Pulsadores
  pinMode(PULSADOR_IZQ, INPUT);
  pinMode(PULSADOR_DER, INPUT);
  pinMode(PULSADOR_SIERRA, INPUT);
  
  // LEDs
  pinMode(LED_SIERRA, OUTPUT);
  pinMode(LED_DESPLAZAMIENTO, OUTPUT);
  
  // Sensor Ultrasonido Horizontal
  pinMode(TRIGGER_H, OUTPUT);
  pinMode(ECHO_H, INPUT);
  
  // Sensor Ultrasonido Horizontal
  pinMode(TRIGGER_V, OUTPUT);
  pinMode(ECHO_V, INPUT);
  
  // Inicializar relé y LED apagados
  apagarMotorYLed(RELE, LED_DESPLAZAMIENTO);
  estadoEmbebido = IDLE;
}

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
    // Esto es solo de prueba, se debe modificar con el desplazamiento del motor
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

void msf()
{
  
  if (estadoEmbebido == IDLE) {
    leerTiempoDesdeSerie();
    if (esPulsadorPresionado(PULSADOR_IZQ))
    {
      estadoEmbebido=EN_MOVIMIENTO;
      Serial.println("Pulsador izquierdo presionado. Desplazando motor a izquierda.");
      posicionDePartida = medirDistanciaUltrasonido(TRIGGER_H, ECHO_H, false, MARGEN_DE_ERROR, 1);
      encenderMotorDesplazamiento(PULSADOR_IZQ);
      Serial.println("Posicionamiento finalizado.");
      imprimirMsg = true;
      pulsadorPresionado=PULSADOR_IZQ;
    }
    else if (esPulsadorPresionado(PULSADOR_DER))
    {
      estadoEmbebido=EN_MOVIMIENTO;
      Serial.println("Pulsador derecho presionado. Desplazando motor a derecha.");
      posicionDePartida = medirDistanciaUltrasonido(TRIGGER_H, ECHO_H, false, MARGEN_DE_ERROR, 1);
      encenderMotorDesplazamiento(PULSADOR_DER);
      Serial.println("Posicionamiento finalizado.");
      imprimirMsg = true;
      pulsadorPresionado=PULSADOR_DER;
    } 
    else if (esPulsadorPresionado(PULSADOR_SIERRA))
    {
      estadoEmbebido=SIERRA_ACTIVA;
      Serial.println("Pulsador sierra presionado. Cortando.");
      posicionDePartida = medirDistanciaUltrasonido(TRIGGER_V, ECHO_V, false, MARGEN_DE_ERROR, 1);
      encenderMotorYLed(RELE,LED_SIERRA); // encendemos sierra
      
    } 
    
  } else if (estadoEmbebido == EN_MOVIMIENTO) {
  	checkearPosicionMotor(pulsadorPresionado);
  } else if (estadoEmbebido == SIERRA_ACTIVA) {
    //actualizarEstadoMotor();
  	checkearLongitudDeCorte();
  }
  
}

bool esPulsadorPresionado(int pinPulsador)
{
  return digitalRead(pinPulsador) == HIGH;
}

void encenderMotorDesplazamiento(int pinPulsador)
{
  if (pinPulsador == PULSADOR_IZQ) 
  {
    // Set the speed
    analogWrite(PM_SPEED, MOTOR_SPEED); 
    // Set the rotation direction
    digitalWrite(M_T1, HIGH);
    digitalWrite(M_T2, LOW);
    digitalWrite(LED_DESPLAZAMIENTO, HIGH);
    //desplazarMotor(pinPulsador);
    // ELIMINAR DELAYS
    //delay(valorDesplazamiento * 1000);
  } 
  else if (pinPulsador == PULSADOR_DER)
  {
    // We change the rotation direction
    analogWrite(PM_SPEED, MOTOR_SPEED); 
    digitalWrite(M_T1, LOW);
    digitalWrite(M_T2, HIGH);
    digitalWrite(LED_DESPLAZAMIENTO, HIGH);
    //desplazarMotor(pinPulsador);
    // ELIMINAR DELAYS
    //delay(valorDesplazamiento * 1000);
  }

}

void checkearLongitudDeCorte() 
{
  posicionActual = medirDistanciaUltrasonido(TRIGGER_V, ECHO_V, false, MARGEN_DE_ERROR, 1);
  Serial.println("Posicion actual ULT V: " + String(posicionActual) + " CM.");
  if (esPulsadorPresionado(PULSADOR_SIERRA) || posicionActual <= DISTANCIA_MINIMA_V)
  {
    Serial.println("Pase el humbral min o presione boton");
    apagarMotorYLed(RELE, LED_SIERRA);
    estadoEmbebido = IDLE;
  }
}

void checkearPosicionMotor(int pinPulsador)
{
  posicionActual = medirDistanciaUltrasonido(TRIGGER_H, ECHO_H, EN_MOVIMIENTO, MARGEN_DE_ERROR, pinPulsador);

  Serial.println("Posicion actual ULT H: " + String(posicionActual) + " CM.");

  //posicionActual =  valorDesplazamiento + distancia;
  //posicionActual += distancia;
  //Serial.println("Posicion actual luego del desplazamiento: " + String(posicionActual) + " CM.");

  
  int delta= abs(posicionActual-posicionDePartida);
  if (delta>=valorDesplazamiento) {
    detenerMotorDesplazamiento();
    estadoEmbebido = IDLE;
  }
  Serial.println("El motor se desplazo en horizontal unos " + String(delta) + " CM de " + String(valorDesplazamiento));
  
}

void detenerMotorDesplazamiento()
{
  analogWrite(PM_SPEED, 0); 
  digitalWrite(M_T1, LOW);
  digitalWrite(M_T2, LOW);
  digitalWrite(LED_DESPLAZAMIENTO, LOW);
  // Breve pausa para asegurarse de que el motor se detenga completamente - ELIMINAR DELAYS
  //delay(200);
}

void encenderMotorYLed(int pinMotor, int pinLed)
{
  digitalWrite(pinMotor, HIGH);
  digitalWrite(pinLed, HIGH);
}

void apagarMotorYLed(int pinMotor, int pinLed)
{
  digitalWrite(pinMotor, LOW);
  digitalWrite(pinLed, LOW);
}

long medirDistanciaUltrasonido(int triggerPin, int echoPin, bool estaEnMovimiento, long margenDeError, int pinPulsador)
{
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  unsigned long duracion = pulseIn(echoPin, HIGH);
  if(estaEnMovimiento)
  {
    duracion += pinPulsador == PULSADOR_IZQ ? margenDeError : -margenDeError;
  }
  long distanciaCm = duracion / 57;
  return distanciaCm;
}

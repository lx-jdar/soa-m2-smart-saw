// Pulsadores
#define PULSADOR_SIERRA 3
#define PULSADOR_DER 4
#define PULSADOR_IZQ 5

// Puente H
#define PM_SPEED 6    // Modulacion del PWM M1 
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

// Estados del Embebido
#define ESTADO_IDLE 14
#define ESTADO_EN_MOVIMIENTO 15
#define ESTADO_SIERRA_ACTIVA 16

// Tolerancia de longitud para apagar el motor
#define DISTANCIA_MINIMA_V 150

// --------------------------------------------------------------------------
// REVISAR LOGICA (CUANDO SE VA DESPLAZANDO SUMA EL TOPE Y NO DESPLAZA DEL TODO)
// VER FUNCION checkearPosicionMotor
#define DISTANCIA_MAXIMA_H 200
// --------------------------------------------------------------------------

#define MARGEN_DE_ERROR 1L

#define ULTRASONIDO_EN_MOVIMIENTO true
#define ULTRASONIDO_SIN_MOVIMIENTO false

#define BAUD_RATE 9600

//INTERRUPCIONES

#define INTERRUPCION_SIERRA digitalPinToInterrupt(PULSADOR_SIERRA)

//MOTOR
#define MOTOR_SPEED 128

//ULTRASONIDO

#define CONST_VELOCIDAD_SONIDO 57

//MODO INTERRUPCION

#define MODO_PIN_SIERRA RISING

bool interrupcionDetectada = false;

int estadoEmbebido = ESTADO_IDLE;

struct structEvento
{
  int tipo;
  int valor;
};


bool imprimirMsg = true;
int pulsadorPresionado;
int valorDesplazamiento = 0;    // Variable para almacenar el valor de desplazamiento del motor en cm
int posicionActual = 0;
int posicionDePartida = 0;

//Maquina de estado
int current_state=0;
structEvento evento;
//----------------------------------------------------
// Arduino Loop
//----------------------------------------------------


void getEvento()
{
   if(esPulsadorPresionado(PULSADOR_IZQ))
   {
		evento.tipo=PULSADOR_IZQ;
   }
   if(esPulsadorPresionado(PULSADOR_DER))
   {
		evento.tipo=PULSADOR_DER;
   }
   if(esPulsadorPresionado(PULSADOR_SIERRA))
   {
		evento.tipo=PULSADOR_SIERRA;
   }
   
}
void maquinaEstado()
{
   switch (estadoEmbebido)
    {
      case ESTADO_IDLE:
      {
          leerTiempoDesdeSerie();
          getEvento();
          switch (evento.tipo)
          {
              case PULSADOR_IZQ:
              {
					estadoEmbebido = ESTADO_EN_MOVIMIENTO;
                    Serial.println("Pulsador izquierdo presionado. Desplazando motor a izquierda.");
                    posicionDePartida = medirDistanciaUltrasonido(TRIGGER_H, ECHO_H, ULTRASONIDO_SIN_MOVIMIENTO, PULSADOR_IZQ);
                    encenderMotorDesplazamiento(PULSADOR_IZQ);
                    imprimirMsg = true;
                    pulsadorPresionado = PULSADOR_IZQ;
              }
              break;
            
              case PULSADOR_DER:
              {
                    estadoEmbebido = ESTADO_EN_MOVIMIENTO;
                    Serial.println("Pulsador derecho presionado. Desplazando motor a derecha.");
                    posicionDePartida = medirDistanciaUltrasonido(TRIGGER_H, ECHO_H, ULTRASONIDO_SIN_MOVIMIENTO, PULSADOR_DER);
                    encenderMotorDesplazamiento(PULSADOR_DER);
                    imprimirMsg = true;
                    pulsadorPresionado = PULSADOR_DER;
              }
              break;
            
                 case PULSADOR_SIERRA:
              {
                    estadoEmbebido = ESTADO_SIERRA_ACTIVA;
                    Serial.println("Pulsador sierra presionado. Cortando ...");
                    posicionDePartida = medirDistanciaUltrasonido(TRIGGER_V, ECHO_V, ULTRASONIDO_SIN_MOVIMIENTO, PULSADOR_SIERRA);
                    encenderMotorYLedSierra();
              }
              break;
          }
          break;
      }
      break;
     
      case ESTADO_EN_MOVIMIENTO:
      {
          switch (evento.tipo)
          {
            case PULSADOR_IZQ:
            {
                checkearPosicionMotor(PULSADOR_IZQ);
            }
            break;

            case PULSADOR_DER:
            {
                checkearPosicionMotor(PULSADOR_DER);
            }
            break;
          }
          break;
      }
      break;
     
     
      case ESTADO_SIERRA_ACTIVA:
      {
          switch (evento.tipo)
          {
            case PULSADOR_SIERRA:
            {
                checkearLongitudDeCorte();
            }
            break;
          }
          break;
      }
      break;


   }
 

  
}

void ISF()
{
  Serial.println("Interrupcion detectada, sierra apagada.");
 
  if(estadoEmbebido==ESTADO_SIERRA_ACTIVA)
  {
      Serial.println("TEST.");
      detenerMotorYLedSierra();

  	  estadoEmbebido = ESTADO_IDLE;
      evento.tipo=ESTADO_IDLE;
  //detachInterrupt(digitalPinToInterrupt(PULSADOR_SIERRA));
    
  }
 
 
}
void setup()
{
  inicializarHardware();
  attachInterrupt(digitalPinToInterrupt(PULSADOR_SIERRA), ISF, MODO_PIN_SIERRA);
  
  
}

void loop()
{
 // msf();
  maquinaEstado();
}

//----------------------------------------------------
// Inicialización
//----------------------------------------------------

void inicializarHardware() 
{
  // Monitor Serial
  Serial.begin(BAUD_RATE);
  
  // Relé
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
  
  // Sensor Ultrasonido Vertical
  pinMode(TRIGGER_V, OUTPUT);
  pinMode(ECHO_V, INPUT);
  
  // Inicializar motores
  detenerMotorDesplazamiento();
  detenerMotorYLedSierra();

  // Inicalizar estado embebido
  estadoEmbebido = ESTADO_IDLE;
}

//----------------------------------------------------
// Interrupciones
//----------------------------------------------------

//----------------------------------------------------
// Máquina de Estados
//----------------------------------------------------

void msf()
{
  if (estadoEmbebido == ESTADO_IDLE)
  {
    leerTiempoDesdeSerie();
    if (esPulsadorPresionado(PULSADOR_IZQ))
    {
      estadoEmbebido = ESTADO_EN_MOVIMIENTO;
      Serial.println("Pulsador izquierdo presionado. Desplazando motor a izquierda.");
      posicionDePartida = medirDistanciaUltrasonido(TRIGGER_H, ECHO_H, ULTRASONIDO_SIN_MOVIMIENTO, PULSADOR_IZQ);
      encenderMotorDesplazamiento(PULSADOR_IZQ);
      imprimirMsg = true;
      pulsadorPresionado = PULSADOR_IZQ;
      evento.tipo=PULSADOR_IZQ;
    }
    else if (esPulsadorPresionado(PULSADOR_DER))
    {
      estadoEmbebido = ESTADO_EN_MOVIMIENTO;
      Serial.println("Pulsador derecho presionado. Desplazando motor a derecha.");
      posicionDePartida = medirDistanciaUltrasonido(TRIGGER_H, ECHO_H, ULTRASONIDO_SIN_MOVIMIENTO, PULSADOR_DER);
      encenderMotorDesplazamiento(PULSADOR_DER);
      imprimirMsg = true;
      pulsadorPresionado = PULSADOR_DER;
      evento.tipo=PULSADOR_DER;
    } 
    else if (esPulsadorPresionado(PULSADOR_SIERRA))
    {
      estadoEmbebido = ESTADO_SIERRA_ACTIVA;
      evento.tipo=ESTADO_SIERRA_ACTIVA;
      Serial.println("Pulsador sierra presionado. Cortando ...");
      posicionDePartida = medirDistanciaUltrasonido(TRIGGER_V, ECHO_V, ULTRASONIDO_SIN_MOVIMIENTO, PULSADOR_SIERRA);
      encenderMotorYLedSierra();
    //  attachInterrupt(INTERRUPCION_SIERRA, ISF, RISING);  // Interrupción al cambio en PULSADOR_SIERRA
	
    } 
  }
  else if (estadoEmbebido == ESTADO_EN_MOVIMIENTO)
  {
  	checkearPosicionMotor(pulsadorPresionado);
  }
  else if (estadoEmbebido == ESTADO_SIERRA_ACTIVA)
  {
  	checkearLongitudDeCorte();
  }
}

//----------------------------------------------------
// Funciones Motor Desplazamiento
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
  posicionActual = medirDistanciaUltrasonido(TRIGGER_H, ECHO_H, ULTRASONIDO_EN_MOVIMIENTO, pinPulsador);
  Serial.println("Posicion actual ULT H: " + String(posicionActual) + " CM.");
  int posibleDesplazamiento = posicionActual + (pinPulsador == PULSADOR_IZQ ? -valorDesplazamiento : valorDesplazamiento);
  if(posibleDesplazamiento >= DISTANCIA_MAXIMA_H)
  {
    detenerMotorDesplazamiento();
    Serial.println("Se quiere desplazar mas que el umbral maximo. Motor de desplazamiento apagandose ...");
    estadoEmbebido = ESTADO_IDLE;
    evento.tipo=ESTADO_IDLE;
  }
  else
  {
    int delta = abs(posicionActual - posicionDePartida);
    if (delta >= valorDesplazamiento)
    {
      detenerMotorDesplazamiento();
      Serial.println("Posicionamiento finalizado.");
      estadoEmbebido = ESTADO_IDLE;
      evento.tipo=ESTADO_IDLE;
    }
    Serial.println("El motor se desplazo horizontalmente hacia la " + 
      String(pinPulsador == PULSADOR_IZQ ? "izquierda" : "derecha") + " unos " + 
      String(delta) + " CM de " + String(valorDesplazamiento) + " CM.");
  }
}

void encenderMotorDesplazamiento(unsigned int pinPulsador)
{
  if (pinPulsador == PULSADOR_IZQ) 
  {
    analogWrite(PM_SPEED, MOTOR_SPEED); 
    digitalWrite(M_T1, HIGH);
    digitalWrite(M_T2, LOW);
    digitalWrite(LED_DESPLAZAMIENTO, HIGH);
  } 
  else if (pinPulsador == PULSADOR_DER)
  {
    analogWrite(PM_SPEED, MOTOR_SPEED); 
    digitalWrite(M_T1, LOW);
    digitalWrite(M_T2, HIGH);
    digitalWrite(LED_DESPLAZAMIENTO, HIGH);
  }
}

void detenerMotorDesplazamiento()
{
  analogWrite(PM_SPEED, 0); 
  digitalWrite(M_T1, LOW);
  digitalWrite(M_T2, LOW);
  digitalWrite(LED_DESPLAZAMIENTO, LOW);
  valorDesplazamiento = 0;  // Limpiar la variable desplazamiento
   Serial.println("Deteniendo el motor desplazamiento");
}

//----------------------------------------------------
// Funciones Motor Sierra
//----------------------------------------------------

void checkearLongitudDeCorte() 
{
  posicionActual = medirDistanciaUltrasonido(TRIGGER_V, ECHO_V, ULTRASONIDO_SIN_MOVIMIENTO, PULSADOR_SIERRA);
  Serial.println("Posicion actual ULT V: " + String(posicionActual) + " CM.");
  if ( posicionActual <= DISTANCIA_MINIMA_V) //esPulsadorPresionado(PULSADOR_SIERRA) ||
  {
    Serial.println("Pase el umbral minimo o presione el boton. Motor de sierra apagandose ...");
    detenerMotorYLedSierra();
    estadoEmbebido = ESTADO_IDLE;
    evento.tipo=ESTADO_IDLE;
  }
}

void encenderMotorYLedSierra()
{
  digitalWrite(RELE, HIGH);
  digitalWrite(LED_SIERRA, HIGH);
}

void detenerMotorYLedSierra()
{
  digitalWrite(RELE, LOW);
  digitalWrite(LED_SIERRA, LOW);
   Serial.println("Deteniendo el motor sierra");
}

//----------------------------------------------------
// Otras Funcionalidades
//----------------------------------------------------

unsigned long medirDistanciaUltrasonido(unsigned int triggerPin, unsigned int echoPin, bool estaEnMovimiento, unsigned int pinPulsador)
{
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  unsigned long duracion = pulseIn(echoPin, HIGH);
  if(estaEnMovimiento || pinPulsador != PULSADOR_SIERRA)
  {
    duracion += pinPulsador == PULSADOR_IZQ ? MARGEN_DE_ERROR : -MARGEN_DE_ERROR;
  }
  unsigned long distanciaCm = duracion / CONST_VELOCIDAD_SONIDO;
  return distanciaCm;
}

bool esPulsadorPresionado(unsigned int pinPulsador)
{
  return digitalRead(pinPulsador) == HIGH;
}

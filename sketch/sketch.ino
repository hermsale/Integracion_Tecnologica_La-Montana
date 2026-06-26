#include <Servo.h>

// ------------------------------
// MOTOR PASO A PASO (A4988)
// ------------------------------
const int STEP_PIN = 2;
const int DIR_PIN = 3;
const int EN_PIN = 4;

// ------------------------------
// SENSOR LDR
// ------------------------------
const int sensorLuz = A0;
const int UMBRAL = 100;

// ------------------------------
// VARIABLES DE ESTADO INTERNO
// ------------------------------
bool imprimiendo = false;
bool huboTrabajo = false;
int ultimoValor = -1;

//--------------------------------

// ------------------------------
// VARIABLES DE SERVO
// ------------------------------
const int SERVO_PIN = 5;
Servo barrera;
const int BARRERA_ARRIBA = 90;
const int BARRERA_ABAJO = 0;

// ------------------------------
// LUCES DE ESTADO
// ------------------------------
const int LED_VERDE = 8; // OK - ESPERANDO
const int LED_AMARILLO = 9; // IMPRIMIENDO
const int LED_AZUL = 10; // TRANSPORTANDO
const int LED_ROJO = 11; // ERROR

// ------------------------------
// VARIABLES DE ESTADO VISIBLES
// ------------------------------
enum EstadoSistema {
  ESPERANDO,
  IMPRIMIENDO,
  TRANSPORTANDO,
  ERROR
};

EstadoSistema estado = ESPERANDO;

void setup() {

  Serial.begin(115200);

  barrera.attach(SERVO_PIN);
  // Posición inicial
  barrera.write(BARRERA_ARRIBA);

// motor dc
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);

  // estados led

  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AMARILLO, OUTPUT);
  pinMode(LED_AZUL, OUTPUT);
  pinMode(LED_ROJO, OUTPUT);

// inicializo en esperando trabajo
  cambiarEstado(ESPERANDO);

  digitalWrite(EN_PIN, LOW);

  Serial.println("Sistema iniciado.");
  Serial.println("Barrera en posición inicial.");
  Serial.println("Esperando trabajos...");
}

//--------------------------------

void loop() {

  detectarImpresion();

  if (huboTrabajo) {

    Serial.println("--------------------------------");
    Serial.println("Trabajo detectado.");
    Serial.println("Moviendo bandeja...");
    Serial.println("--------------------------------");
    // cambio de estado al detectar que hubo un trabajo 
    cambiarEstado(TRANSPORTANDO);

    moverMotor();

// volvemos el flag a falso
    huboTrabajo = false;
// preparamos la barrera para el siguiente trabajo
    subirBarrera();

    cambiarEstado(ESPERANDO);
    Serial.println("Sistema listo para un nuevo trabajo.");
    Serial.println();
  }
}

//--------------------------------
// DETECCIÓN DE IMPRESIÓN
//--------------------------------

void detectarImpresion() {
  cambiarEstado(ESPERANDO);
  while (true) {

    int valor = analogRead(sensorLuz);

if (abs(valor - ultimoValor) > 20) {
    Serial.print("Luz: ");
    Serial.println(valor);
    ultimoValor = valor;
}

    // Comenzó la impresión
    if (valor < UMBRAL) {

      
      if (!imprimiendo) {
        cambiarEstado(IMPRIMIENDO);
        Serial.println(">>> Impresión iniciada");
        imprimiendo = true;
        huboTrabajo = true;
      }

    }

    // Terminó la impresión
    else {

      if (imprimiendo) {

        imprimiendo = false;

        Serial.println(">>> Impresión finalizada");
        Serial.println("Esperando 5 segundos...");

        delay(5000);

        Serial.println("Tiempo cumplido.");

        return;
      }
    }

    delay(200);
  }
}

//--------------------------------
// MOVIMIENTO
//--------------------------------

void moverMotor() {

  // Ida
  digitalWrite(DIR_PIN, HIGH);

  for (int i = 0; i < 2000; i++) {

    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(800);

    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(800);
  }

  delay(500);

// bajamos la barrera
  bajarBarrera();

  // Vuelta
  digitalWrite(DIR_PIN, LOW);

  for (int i = 0; i < 2000; i++) {

    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(800);

    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(800);
  }

  Serial.println("Bandeja en posición inicial.");
}

// funcion para bajar la barrera
void bajarBarrera() {

  Serial.println("Bajando barrera...");

  barrera.write(BARRERA_ABAJO);

  delay(700);

}

// funcion para subir la barrera 
void subirBarrera() {

  Serial.println("Subiendo barrera...");

  barrera.write(BARRERA_ARRIBA);

  delay(700);

}

// funcion manejo de estado LEDS
void cambiarEstado(EstadoSistema nuevoEstado){

       if(estado == nuevoEstado)
        return;
    estado = nuevoEstado;

    digitalWrite(LED_VERDE,LOW);
    digitalWrite(LED_AMARILLO,LOW);
    digitalWrite(LED_AZUL,LOW);

    switch(estado){

        case ESPERANDO:

            digitalWrite(LED_VERDE,HIGH);
            Serial.println("[LED] Estado: ESPERANDO");

        break;

        case IMPRIMIENDO:

            digitalWrite(LED_AMARILLO,HIGH);
            Serial.println("[LED] Estado: IMPRIMIENDO");

        break;

        case TRANSPORTANDO:

            digitalWrite(LED_AZUL,HIGH);
            Serial.println("[LED] Estado: TRANSPORTANDO");

        break;

        case ERROR:

            digitalWrite(LED_ROJO,HIGH);
            Serial.println("[LED] ERROR");
        
        break;
    } 
}
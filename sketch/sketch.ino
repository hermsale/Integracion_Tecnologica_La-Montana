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
const int sensorTope = A1;
const int UMBRAL = 100;

// ------------------------------
// VARIABLES DE ESTADO INTERNO
// ------------------------------
bool imprimiendo = false;
bool huboTrabajo = false;
int ultimoValor = -1;
int valorInicialTope  = 0;

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

// ------------------------------
// BOTONES DE FALLA
// ------------------------------
const int BTN_FALLA_SENSOR = 6;
const int BTN_RESET = 13;

// variable estado sistema
bool sistemaEnError=false;

EstadoSistema estado = ESPERANDO;

void setup() {

  Serial.begin(115200);

  barrera.attach(SERVO_PIN);
  // Posición inicial
  barrera.write(BARRERA_ARRIBA);

// botones de fallas
  pinMode(BTN_FALLA_SENSOR, INPUT_PULLUP);
// boton reiniciar sistema
  pinMode(BTN_RESET, INPUT_PULLUP);

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

// cada vez que hay un error. no permite continuar ningun proceso
  if (sistemaEnError) {
    reinicioSistema();
    return;   // Espera el botón RESET
  }

  // calibramos valor del sensor tope
  valorInicialTope = leerSensorTope();

  detectarImpresion();

  if (huboTrabajo) {

    Serial.println("--------------------------------");
    Serial.println("Trabajo detectado.");
    Serial.println("Moviendo bandeja...");
    Serial.println("--------------------------------");
    // cambio de estado al detectar que hubo un trabajo 
    cambiarEstado(TRANSPORTANDO);

    moverMotor();

     // Si ocurrió una falla durante el transporte,
    // detenemos el flujo acá.
    if (sistemaEnError) {
        return;
    }

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

  while (true) {

    if (verificarFallaSensor()) {
        return;
    }

    int valor = leerSensorLuz();

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
          if (verificarFallaSensor()) {
           return;
        }
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
    
    if (verificarFallaSensor()) {
        return;
    }

    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(800);

    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(800);
  }

  delay(500);
  if(esperarSensorTope()){
    // bajamos la barrera
      bajarBarrera();
    
      // Vuelta
      digitalWrite(DIR_PIN, LOW);
    
      for (int i = 0; i < 2000; i++) {  

        if (verificarFallaSensor()) {
        return;
        } 
    
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(800);
    
        digitalWrite(STEP_PIN, LOW);
        delayMicroseconds(800);
      }
    
      Serial.println("Bandeja en posición inicial.");
  }else{
      Serial.println("ERROR: No llegó al destino.");
      cambiarEstado(ERROR);
      sistemaEnError = true;

    return;
  }

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
        
    estado = nuevoEstado;

    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_AMARILLO, LOW);
    digitalWrite(LED_AZUL, LOW);
    digitalWrite(LED_ROJO, LOW);

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

// funcion utilizada para leer sensor
int leerSensorLuz() {
  // simulacion de falla 1. Falla de sensor luz
    digitalRead(BTN_FALLA_SENSOR);
// Falla 1: si no detecta el sensor funcionando correctamente.
    if (digitalRead(BTN_FALLA_SENSOR) == LOW) {
        return -1;   // simulamos un valor anormal
    }

// si no hay falla, devolvemos el valor sensado.
    return analogRead(sensorLuz);
}

// funcion que verifica el estado del sensor.
bool verificarFallaSensor() {

    if (digitalRead(BTN_FALLA_SENSOR) == LOW) {
        if (!sistemaEnError){
            sistemaEnError = true;
            cambiarEstado(ERROR);
            huboTrabajo = false;
            Serial.println("================================");
            Serial.println("ERROR: Sensor LDR desconectado.");
            Serial.println("================================");
        }
        return true;
    }
    return false;
}

// funcion para salir del modo error 
void reinicioSistema() {

  if (digitalRead(BTN_RESET) == LOW) {

    Serial.println();
    Serial.println("================================");
    Serial.println("Reiniciando sistema...");
    Serial.println("================================");

    
    // reiniciamos todos los valores
    sistemaEnError = false;

    imprimiendo = false;
    huboTrabajo = false;
    ultimoValor = -1;

    subirBarrera();              // vuelve la barrera arriba
    delay(500); 

    cambiarEstado(ESPERANDO);    // volvemos al estado de espera

    Serial.println("Sistema listo.");
    Serial.println();

    delay(500);   // anti rebote
  }
}

// lectura del segundo sensor. que marca la llegada de bandeja
int leerSensorTope() {
    int valor = analogRead(sensorTope);
    return valor;
}


// funcion que coteja que si en 10 segundos la bandeja no llega
// algo ocurrio. 
bool esperarSensorTope() {

    Serial.println("Esperando llegada al sensor de tope...");

    unsigned long inicio = millis();

    while (millis() - inicio < 5000) {

        if (verificarFallaSensor()) {
          return false;
        }

        int valor = leerSensorTope();

        // si el valor del sensor tope cambia dentro de los 10 seg. NO hubo falla. 
        if (abs(valor - valorInicialTope)>100) {

            Serial.println("Bandeja llegó al destino.");
            return true;

        }
        delay(50);
    }

    return false;
}

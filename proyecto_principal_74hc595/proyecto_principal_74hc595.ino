///RIOS OVIEDO, PABLO ENRIQUE// MONROY ACOSTA, JOSE IVAN // 2024 // CATAMARCA // ARGENTINA//
//Variables para el motor Brushless
#include <Servo.h>
Servo esc;               // Crear un objeto Servo para controlar el ESC
const int relePin = 7;   // Pin de control del relé
const int escPin = 6;    // Pin PWM conectado al ESC
const int velocidadMinima = 1000; // Señal mínima para detener el motor
const int velocidadMaxima = 1500; // Velocidad máxima permitida
const int incrementoVelocidad = 100; // Incremento de velocidad
int velocidadActual = velocidadMinima; // Velocidad actual del motor
bool motorEncendido = false;       // Variable para rastrear el estado del motor brushless

//Variables para los motores
int retardo1 = 15;          // Velocidad del Motor (NEMA 17)
int retardo2 = 3;          //Velocidad del Motor (28BYJ-48)
int dato_rx1;              // valor recibido en grados para el motor principal (NEMA 17)
int dato_rx2;              // valor recibido en grados para el motor principal (28BYJ-48)
int numero_pasos1 = 0;     // Valor en grados donde se encuentra el motor principal (NEMA 17)
int numero_pasos2 = 0;       // Valor en grados donde se encuentra el motor del disco de Hartl (28BYJ-48)
int stepIndex1 = 0;      // Índice de paso para la secuencia de pasos (NEMA17)
int stepIndex2 = 0;      // Índice de paso para la secuencia de pasos (28BYJ-48)
String leeCadena1;         // Almacena la cadena de datos recibida (NEMA 17)
String leeCadena2;         // Almacena la cadena de datos recibida (28BYJ-48)
bool motor1_seleccionado = false;  // Flag para indicar si se ha seleccionado el motor (NEMA17)
bool motor2_seleccionado = false;  // Flag para indicar si se ha seleccionado el motor (28BYJ-48)
bool motor1_giro_completo = false; //Indica cuando el motor NEMA17  termino de girar
const int pasosPorRevolucion1 = 200; // Número de pasos por revolución para el NEMA17
const int pasosPorRevolucion2 = 2048; // Número de pasos por revolución para el 28BYJ-48

//Otras Variables
bool encendido = false;

//Variables para los laseres
int led1 = 3;
int led2 = 2;
int led3 = 4;

// Variables para la verificación continua de la posicion del eje
bool enPosicionCorrecta = true;

//Variables para el fin de carrera
const int Microswitch = 5; //pin del switch
int estadoMicroswitch = 0; //estado del switch

//Variables para el boton
const int buttonPin = 13;
int buttonState = 0;  //Estado del boton
int lastButtonState = 0; //Estado anterior del boton

// Pines para el 74HC595
const int latchPin = 9;  // Pin ST_CP (Latch) del 74HC595
const int clockPin = 10;   // Pin SH_CP (Clock) del 74HC595
const int dataPin = 8;    // Pin DS (Data) del 74HC595

// Variable para almacenar el tiempo de la última actividad
unsigned long lastActivityMillis = 0; 
unsigned long lastActivityMillis2 = 0; //Para la proteccion del motor si falla el microswitch

//Variables para el CD4051
int A = 11; //Pin A del Multiplexor al pin Digital 11
int B = 12; //Pin B del Multiplexor al pin Digital 12

void setup() {                
    Serial.begin(9600);     // Inicializamos el puerto serie a 9600 baudios

    pinMode(relePin, OUTPUT);
    digitalWrite(relePin, LOW); // Relé  apagado al inicio
    esc.attach(escPin);
    esc.writeMicroseconds(velocidadMinima); // Señal mínima para inicializar el ESC
  
    pinMode(led1, OUTPUT);
    pinMode(led2, OUTPUT);
    pinMode(led3, OUTPUT);

    pinMode(A, OUTPUT);
    pinMode(B, OUTPUT);
    
    pinMode(dataPin, OUTPUT);
    pinMode(clockPin, OUTPUT);
    pinMode(latchPin, OUTPUT);
    
    pinMode(buttonPin, INPUT);

    pinMode(Microswitch, INPUT);
    
     // Inicializa lastActivityMillis al tiempo actual
     lastActivityMillis = millis();
     lastActivityMillis2 = millis();
}

void loop() {

    unsigned long currentMillis = millis();
      if (currentMillis - lastActivityMillis >= 300000) {
        // Realiza los mismos pasos que cuando se envía 'z' o 'Z'
        Serial.println("Inactividad detectada...");
        digitalWrite(led1, LOW);
        digitalWrite(led2, LOW);
        digitalWrite(led3, LOW);
        digitalWrite(A, LOW);
        digitalWrite(B, LOW);
        reiniciar_motor1();
        dato_rx1 = 0;
        numero_pasos1 = 0;
        leeCadena1 = "";
        motor1_seleccionado = false;        
        reiniciar_motor2();
        dato_rx2 = 0;
        numero_pasos2 = 0;
        leeCadena2 = "";
        motor2_seleccionado = false;

        encendido = false;
        // Actualiza lastActivityMillis al tiempo actual
        lastActivityMillis = currentMillis;
      }
      
      //Leer estado del boton
       buttonState = digitalRead(buttonPin);
       
      // Si el botón ha sido presionado
      if (buttonState != lastButtonState) {
        if (buttonState == HIGH) {
                    digitalWrite(led1, LOW);
                    digitalWrite(led2, LOW);
                    digitalWrite(led3, LOW);
                    digitalWrite(A, LOW);
                    digitalWrite(B, LOW);
                    apagarBrushless(); // Llamar a la función para apagar el motor brushless

                    dato_rx1 = 0;
                    numero_pasos1 = 0;
                    leeCadena1 = "";
                    motor1_seleccionado = false;    
                    
                    reiniciar_motor2();
                    dato_rx2 = 0;
                    numero_pasos2 = 0;
                    leeCadena2 = "";    
                    motor2_seleccionado = false;

                    encender_proyecto();
        }
        delay(50);  // Delay para debounce
      }

  lastButtonState = buttonState;

  if (!motor1_seleccionado && !motor2_seleccionado && !encendido){
    Serial.println("Pulse 'E', para encender el proyecto");

    while(!Serial.available()) {}
    char c = Serial.read();

    Serial.print("Recibido: ");
    Serial.println(c);

    switch(c){
      case 'e':
      case 'E':
              encender_proyecto();
      break;
    }
  }

if (Serial.available()) {
    lastActivityMillis = currentMillis; // Reinicia el contador al recibir nueva actividad
 
  if (!motor1_seleccionado && !motor2_seleccionado && !motorEncendido) {
    // Mensaje para seleccionar el motor
    Serial.println("Seleccione el proyecto a usar");
    Serial.println("A: Proyecto A");
    Serial.println("B: Proyecto B");
    Serial.println("C: Proyecto C");
    Serial.println("D: Proyecto D");

    while (!Serial.available()) {}  // Esperar hasta que esté disponible la entrada serial
    char c = Serial.read();          // Leer la entrada serial
    
    Serial.print("Recibido: ");
    Serial.println(c);
    
        switch (c){
            case 'A':
            case 'a':
                  activar_motor1();
                  leeCadena1 = "0";
                  digitalWrite(led1, LOW);
                  digitalWrite(led2, LOW);
                  digitalWrite(led3, LOW);      
                  digitalWrite(A, LOW);
                  digitalWrite(B, LOW);
                  Serial.println("Canal 0");
                  encenderBrushless();
                  break;
           case 'B':
           case 'b':
                  activar_motor1();
                  leeCadena1 = "210";
                  digitalWrite(led1, HIGH);
                  digitalWrite(led2, HIGH);
                  digitalWrite(led3, HIGH);      
                  digitalWrite(A, HIGH);
                  digitalWrite(B, LOW);
                  Serial.println("Canal 1");
                  break;
            case 'C':
            case 'c':
                  activar_motor1();
                  leeCadena1 = "390";
                  digitalWrite(led1, HIGH);
                  digitalWrite(led2, HIGH);
                  digitalWrite(led3, HIGH);
                  digitalWrite(A, LOW);
                  digitalWrite(B, HIGH);
                  Serial.println("Canal 2");      
                  break;
            case 'D':
            case 'd':
                  activar_motorchico();
                  leeCadena1 = "560";
                  digitalWrite(led1, LOW);
                  digitalWrite(led2, HIGH);
                  digitalWrite(led3, LOW);
                  digitalWrite(A, HIGH);
                  digitalWrite(B, HIGH);
                  Serial.println("Canal 3");      
                  break;
        
         case 'r':
         case 'R':
            //Esto es por si se inicio un proyecto, se envio una "X", y apartir de alli se envio una "R" para resetear
            Serial.println("Comando 'r' recibido.");
            digitalWrite(led1, LOW);
            digitalWrite(led2, LOW);
            digitalWrite(led3, LOW);
            digitalWrite(A, LOW);
            digitalWrite(B, LOW);
            apagarBrushless(); // Llamar a la función para apagar el motor brushless  
            
                    dato_rx1 = 0;
                    numero_pasos1 = 0;
                    leeCadena1 = "";
                    motor1_seleccionado = false;    
                    
                    reiniciar_motor2();
                    dato_rx2 = 0;
                    numero_pasos2 = 0;
                    leeCadena2 = "";    
                    motor2_seleccionado = false;

                    encender_proyecto();
            break;
            
          default:
            Serial.println("Comando erroneo");
            break;       
        }
  }
    
if (motor1_seleccionado && !motor2_seleccionado && motorEncendido){
        while (Serial.available()) {    // Leer el valor enviado por el Puerto serial
        delay(retardo1);
        char c  = Serial.read();     // Lee los caracteres
        Serial.print("Recibido: ");
        Serial.println(c);  
             switch (c) {
                  case 'r':  // Comando de reinicio 'r'
                  case 'R':
                    Serial.println("Comando 'r' recibido.");
                    
                    digitalWrite(led1, LOW);
                    digitalWrite(led2, LOW);
                    digitalWrite(led3, LOW);
                    digitalWrite(A, LOW);
                    digitalWrite(B, LOW);
                    apagarBrushless(); // Llamar a la función para apagar el motor brushless

                    dato_rx1 = 0;
                    numero_pasos1 = 0;
                    leeCadena1 = "";
                    motor1_seleccionado = false;    
                    
                    reiniciar_motor2();
                    dato_rx2 = 0;
                    numero_pasos2 = 0;
                    leeCadena2 = "";    
                    motor2_seleccionado = false;

                    encender_proyecto();
                    break;
            
                  case 'X':  // Comando para apagar motor 'X'
                  case 'x':
                    digitalWrite(led1, LOW);
                    digitalWrite(led2, LOW);
                    digitalWrite(led3, LOW);
                    Serial.println("Motor NEMA 17 encendido. Ingrese el proyecto al que quiere acceder");
                    motor1_seleccionado = false;
                    motor2_seleccionado = false;
                    leeCadena1 = "";                     
                    apagarBrushless(); // Llamar a la función para apagar el motor  
                    break;

                  case 'i':
                  case 'I':
                  motor1_seleccionado = false;
                  incrementarVelocidad(); // Incrementar la velocidad del motor
                  break;
          
                  case 'z':  // Comando de apagado
                  case 'Z':
                    Serial.println("Buenas noches....");
                    
                    digitalWrite(led1, LOW);
                    digitalWrite(led2, LOW);
                    digitalWrite(led3, LOW);
                    digitalWrite(A, LOW);
                    digitalWrite(B, LOW);
                    apagarBrushless(); // Llamar a la función para apagar el motor brushless
                    
                    reiniciar_motor1();  // Llama a la función de reinicio
                    dato_rx1 = 0;
                    numero_pasos1 = 0;
                    leeCadena1 = "";
                    motor1_seleccionado = false;

                    reiniciar_motor2();
                    dato_rx2 = 0;
                    numero_pasos2 = 0;
                    leeCadena2 = "";    
                    motor2_seleccionado = false;    

                    encendido = false;
                    break;
                    
                    default:
                      Serial.println("Comando erroneo");
                      break;       
                }
        }        
}

if (!motor1_seleccionado && !motor2_seleccionado && motorEncendido){
        while (Serial.available()) {    // Leer el valor enviado por el Puerto serial
        delay(retardo1);
        char c  = Serial.read();     // Lee los caracteres
        Serial.print("Recibido: ");
        Serial.println(c);  
             switch (c) {
              
                  case 'X':  // Comando para apagar motor 'X'
                  case 'x':
                    digitalWrite(led1, LOW);
                    digitalWrite(led2, LOW);
                    digitalWrite(led3, LOW);
                    Serial.println("Motor NEMA 17 encendido. Ingrese el proyecto al que quiere acceder");
                    motor1_seleccionado = false;
                    motor2_seleccionado = false;
                    leeCadena1 = "";                     
                    apagarBrushless(); // Llamar a la función para apagar el motor  
                    break;
                    
                  case 'z':  // Comando de apagado
                  case 'Z':
                    Serial.println("Buenas noches....");
                    
                    digitalWrite(led1, LOW);
                    digitalWrite(led2, LOW);
                    digitalWrite(led3, LOW);
                    digitalWrite(A, LOW);
                    digitalWrite(B, LOW);
                    apagarBrushless(); // Llamar a la función para apagar el motor brushless
                    
                    reiniciar_motor1();  // Llama a la función de reinicio
                    dato_rx1 = 0;
                    numero_pasos1 = 0;
                    leeCadena1 = "";
                    motor1_seleccionado = false;

                    reiniciar_motor2();
                    dato_rx2 = 0;
                    numero_pasos2 = 0;
                    leeCadena2 = "";    
                    motor2_seleccionado = false;    

                    encendido = false;
                    break;
                    
                    default:
                    Serial.println("Comando erroneo");
                    break;       
             }        
      }
}
     if (motor1_seleccionado && !motor2_seleccionado && !motorEncendido){
        while (Serial.available()) {    // Leer el valor enviado por el Puerto serial
        delay(retardo1);
        char c  = Serial.read();     // Lee los caracteres
        Serial.print("Recibido: ");
        Serial.println(c);  
             switch (c) {
                  case 'r':  // Comando de reinicio 'r'
                  case 'R':
                    Serial.println("Comando 'r' recibido.");
                    
                    digitalWrite(led1, LOW);
                    digitalWrite(led2, LOW);
                    digitalWrite(led3, LOW);
                    digitalWrite(A, LOW);
                    digitalWrite(B, LOW);
                    apagarBrushless(); // Llamar a la función para apagar el motor brushless

                    dato_rx1 = 0;
                    numero_pasos1 = 0;
                    leeCadena1 = "";
                    motor1_seleccionado = false;    
                    
                    reiniciar_motor2();
                    dato_rx2 = 0;
                    numero_pasos2 = 0;
                    leeCadena2 = "";    
                    motor2_seleccionado = false;

                    encender_proyecto();
                    break;
            
                  case 'X':  // Comando para apagar motor 'X'
                  case 'x':
                    digitalWrite(led1, LOW);
                    digitalWrite(led2, LOW);
                    digitalWrite(led3, LOW);
                    Serial.println("Motor NEMA 17 encendido. Ingrese el proyecto al que quiere acceder");
                    motor1_seleccionado = false;
                    motor2_seleccionado = false;
                    leeCadena1 = "";                     
                    apagarBrushless(); // Llamar a la función para apagar el motor  
                    break;

                  case 'i':
                  case 'I':
                  incrementarVelocidad(); // Incrementar la velocidad del motor
                  break;
          
                  case 'z':  // Comando de apagado
                  case 'Z':
                    Serial.println("Buenas noches....");
                    
                    digitalWrite(led1, LOW);
                    digitalWrite(led2, LOW);
                    digitalWrite(led3, LOW);
                    digitalWrite(A, LOW);
                    digitalWrite(B, LOW);
                    
                    apagarBrushless(); // Llamar a la función para apagar el motor brushless
                    
                    reiniciar_motor1();  // Llama a la función de reinicio
                    dato_rx1 = 0;
                    numero_pasos1 = 0;
                    leeCadena1 = "";
                    motor1_seleccionado = false;

                    reiniciar_motor2();
                    dato_rx2 = 0;
                    numero_pasos2 = 0;
                    leeCadena2 = "";    
                    motor2_seleccionado = false;    

                    encendido = false;
                    break;
                    
                    default:
                      Serial.println("Comando erroneo");
                      break;       
                }
        }        
}
     
      
      if (!motor1_seleccionado && motor2_seleccionado && !motorEncendido){
          // Leer el valor enviado por el Puerto serial
            while (Serial.available()) {
                delay(retardo2);
                char c = Serial.read();
          if ((c >= '0' && c <= '9') || c == '-') {
              leeCadena2 += c;
          }switch (c) {
                  case 'r':  // Comando de reinicio 'r'
                  case 'R':
                    Serial.println("Comando 'r' recibido.");
                    
                    digitalWrite(led1, LOW);
                    digitalWrite(led2, LOW);
                    digitalWrite(led3, LOW);
                    digitalWrite(A, LOW);
                    digitalWrite(B, LOW);
                    apagarBrushless(); // Llamar a la función para apagar el motor  
                    
                    dato_rx1 = 0;
                    numero_pasos1 = 0;
                    leeCadena1 = "";
                    motor1_seleccionado = false;

                    reiniciar_motor2();
                    dato_rx2 = 0;
                    numero_pasos2 = 0;
                    leeCadena2 = "";    
                    motor2_seleccionado = false; 
                    encender_proyecto();
                    break;
            
                  case 'X':  // Comando para salir del proyecto actual 'X'
                  case 'x':
                    digitalWrite(led1, LOW);
                    digitalWrite(led2, LOW);
                    digitalWrite(led3, LOW);
                    Serial.println("Motor NEMA 17 encendido. Ingrese el proyecto al que quiere acceder");
                    motor1_seleccionado = false;
                    motor2_seleccionado = false;
                    leeCadena1 = "";                   
                    apagarBrushless(); // Llamar a la función para apagar el motor
                    break;
                    
                  case 'z':  // Comando de apagado
                  case 'Z':
                    Serial.println("Buenas noches....");
                    
                    digitalWrite(led1, LOW);
                    digitalWrite(led2, LOW);
                    digitalWrite(led3, LOW);
                    digitalWrite(A, LOW);
                    digitalWrite(B, LOW);
                    
                    apagarBrushless(); // Llamar a la función para apagar el motor brushless  
                    
                    reiniciar_motor1();  // Llama a la función de reinicio
                    dato_rx1 = 0;
                    numero_pasos1 = 0;
                    leeCadena1 = "";
                    motor1_seleccionado = false;

                    reiniciar_motor2();
                    dato_rx2 = 0;
                    numero_pasos2 = 0;
                    leeCadena2 = "";    
                    motor2_seleccionado = false;    

                    encendido = false;
                    break;
                     }    
          } 
}

      
    if (leeCadena1.length() > 0) {       
        dato_rx1 = leeCadena1.toInt();   // Convierte Cadena de caracteres a Enteros
        Serial.print(dato_rx1);         // Envía valor en Grados 
        Serial.println(" Grados");
        delay(retardo1);
        // Convertir grados a pasos
        int pasosRequeridos1 = dato_rx1 * (pasosPorRevolucion1 / 360.0);
        // Ajustar el número de pasos necesarios
        dato_rx1 = pasosRequeridos1;
        leeCadena1 = ""; // Limpiar la cadena de caracteres recibidos
    }  

    if (leeCadena2.length() > 0) {
        dato_rx2 = leeCadena2.toInt();   // Convierte Cadena de caracteres a Enteros
        Serial.print(dato_rx2);         // Envía valor en Grados 
        Serial.println(" Grados");
        delay(retardo2);
        // Convertir grados a pasos
        int pasosRequeridos2 = dato_rx2 * (pasosPorRevolucion2 / 360.0);
        // Ajustar el número de pasos necesarios
        dato_rx2 = pasosRequeridos2;
        leeCadena2 = ""; // Limpiar la cadena de caracteres recibidos
  } 

    if (motor1_seleccionado) {
      girar_motor1();
      autocorregirPosicion(); // Corrección automática integrada
    } else if (motor2_seleccionado){
      girar_motor2();
    }

     if (motor1_giro_completo && !motor2_seleccionado){
          Serial.println("Motor 2 activado. Ingrese la cantidad de grados a girar");
          motor1_giro_completo = false;  // Resetear el estado para evitar enviar el comando de activacion  repetidamente
          motor1_seleccionado = false;
          motor2_seleccionado = true;
     }
 }

 /*if (motor1_seleccionado && motorEncendido){
  motor1_seleccionado = false;
 }*/
 
     // Verificación continua de que esté bien posicionado el eje
    if (motor1_seleccionado || motor2_seleccionado) {
    verificarPosicionContinuamente();
  }
  
}  ///////////////////// Fin del Loop ///////////////////////////

//Funcion para activar el motor NEMA 17
void activar_motor1() {
  Serial.println("Motor 1 girando a la posicion de proyecto.");  // Mensaje para ingresar la cantidad de grados
  motor1_seleccionado = true;
    // Limpiar el puerto serie
  while (Serial.available()) {
    Serial.read();
  }
}

//Funcion para el inicio del proyecto
void encender_proyecto() {
    encendido = true;
    motor1_seleccionado = true;

    Serial.println("Iniciando proyecto...");

    unsigned long startTime = millis(); // Registra el tiempo inicial

    while (motor1_seleccionado && encendido) {
        estadoMicroswitch = digitalRead(Microswitch);

        if (estadoMicroswitch == HIGH) {
            paso_izq1(); // Motor sigue girando
            startTime = millis(); // Reinicia el temporizador si el microswitch se activa
        } else if (estadoMicroswitch == LOW) {
            leeCadena1 = "620"; // Guardar 600 en leeCadena1
            int pasosRequeridos1 = leeCadena1.toInt() * (pasosPorRevolucion1 / 360.0);
            Serial.println("Motor llegó al microswitch, volviendo a la posición inicial...");
            leeCadena1 = ""; // Limpiar la cadena
            while (pasosRequeridos1 > 0) {
                paso_der1(); // Volver a la posición inicial
                pasosRequeridos1--;
            }
            motor1_seleccionado = false;
            Serial.println("Motor volvió a la posición inicial.");

            while (Serial.available()) {
                Serial.read();
            }
        }

        // Verifica si el tiempo excede 1 minuto
        if (Serial.available()) {
          lastActivityMillis2 = startTime;
            if (startTime - lastActivityMillis2 > 10000) { // 1 minuto
                Serial.println("Error: Microswitch no se activó. Apagando motor para evitar sobrecalentamiento.");
                motor1_seleccionado = false;
                apagado(); // Apaga el motor
                lastActivityMillis2 = startTime;
            }
        }
    }
    apagado(); // Apaga el motor después de salir del bucle

}

//Funcion para girar el motor NEMA 17
void girar_motor1(){ //gira el motor NEMA17
  while (dato_rx1 > numero_pasos1) {   // Giro hacia la izquierda en pasos
        paso_izq1();
        numero_pasos1++;
    }
    while (dato_rx1 < numero_pasos1) {   // Giro hacia la derecha en pasos
        paso_der1();
        numero_pasos1--;
    }
    apagado();         // Apagado del Motor para que no se caliente
}

//Funcion para reiniciar el motor NEMA 17
void reiniciar_motor1(){
  leeCadena1 = "0";
  dato_rx1 = 0;
  Serial.println("Reiniciando el motor NEMA17...");
    while (dato_rx1 > numero_pasos1) {   // Giro hacia la izquierda en grados
    paso_izq1();
    numero_pasos1 = numero_pasos1 + 1;
  }
  while (dato_rx1 < numero_pasos1) {   // Giro hacia la derecha en grados
    paso_der1();
    numero_pasos1 = numero_pasos1 - 1;
  }
  apagado();         // Apagado del Motor para que no se caliente
}

//Funcion para activar el motor 28BYJ-48
void activar_motorchico(){ //Se activa el motor BYJ28-48 y se desactiva el NEMA 17
    motor1_seleccionado = true;
    motor2_seleccionado = false;
    Serial.println("Posicionando el motor...");
      
while (dato_rx1 > numero_pasos1) {   // Giro hacia la izquierda en grados
    paso_izq1();
    numero_pasos1 = numero_pasos1 + 1;
  }
  while (dato_rx1 < numero_pasos1) {   // Giro hacia la derecha en grados
    paso_der1();
    numero_pasos1 = numero_pasos1 - 1;
  }
  apagado();         // Apagado del Motor para que no se caliente
  motor1_giro_completo = true;
}

//Funcion para girar el motor 28BYJ-48
void girar_motor2() {
   // Girar el motor 28byj28
  while (dato_rx2 > numero_pasos2) {   // Giro hacia la izquierda en grados
    paso_izq2();
    numero_pasos2 = numero_pasos2 + 1;
  }
  while (dato_rx2 < numero_pasos2) {   // Giro hacia la derecha en grados
    paso_der2();
    numero_pasos2 = numero_pasos2 - 1;
  }
  apagado();         // Apagado del Motor para que no se caliente
}

//Funcion para reiniciar el motor 28BYJ-48
void reiniciar_motor2(){
  leeCadena2 = "0";
  dato_rx2 = 0;
  Serial.println("Reiniciando el motor 28BYJ-48...");
    while (dato_rx2 > numero_pasos2) {   // Giro hacia la izquierda en grados
    paso_izq2();
    numero_pasos2 = numero_pasos2 + 1;
  }
  while (dato_rx2 < numero_pasos2) {   // Giro hacia la derecha en grados
    paso_der2();
    numero_pasos2 = numero_pasos2 - 1;
  }
  apagado();         // Apagado del Motor para que no se caliente
}

void paso_der1(){   //Pasos a la derecha (NEMA17)
      byte motor1Bits = (0b0001 << stepIndex1);  //mascara del motor Nema 17
      byte motor2Bits = (0b0001 << (stepIndex2 + 4)); //mascara del motor 28BYJ-48
      shiftOutData(motor1Bits | motor2Bits);
      delay(retardo1);
      stepIndex1 = (stepIndex1 + 1) % 4;
}

void paso_izq1(){   //Pasos a la izq (NEMA17)
      byte motor1Bits = (0b0001 << stepIndex1);  //mascara del motor Nema 17
      byte motor2Bits = (0b0001 << (stepIndex2 + 4)); //mascara del motor 28BYJ-48
      shiftOutData(motor1Bits | motor2Bits);
      delay(retardo1);
      stepIndex1 = (stepIndex1 - 1 +4) % 4;
}

void paso_der2(){   //Pasos a la derecha (28BYJ-48)
      byte motor1Bits = (0b0001 << stepIndex1);  //mascara del motor Nema 17
      byte motor2Bits = (0b0001 << (stepIndex2 + 4)); //mascara del motor 28BYJ-48
      shiftOutData(motor1Bits | motor2Bits);
      delay(retardo2);
      stepIndex2 = (stepIndex2 + 1) % 4;
}

void paso_izq2(){   //Pasos a la izq (28BYJ-48)
      byte motor1Bits = (0b0001 << stepIndex1);  //mascara del motor Nema 17
      byte motor2Bits = (0b0001 << (stepIndex2 + 4)); //mascara del motor 28BYJ-48
      shiftOutData(motor1Bits | motor2Bits);
      delay(retardo2);
      stepIndex2 = (stepIndex2 - 1 +4) % 4;
}

// Función para encender el motor brushless
void encenderBrushless() {
  
  if (!motorEncendido) {
    digitalWrite(relePin, HIGH); // Activar el relé para alimentar el ESC
    delay(1000); // Delay para estabilizar la alimentación

    esc.writeMicroseconds(velocidadMinima); // Señal mínima para inicializar el ESC
    delay(2000); // Esperar 2 segundos para que el ESC se calibre

    motorEncendido = true; // Cambiar el estado del motor a encendido
    velocidadActual = velocidadMinima; // Inicializar la velocidad actual
    
    Serial.println("Motor encendido. Usa 'i' para incrementar la velocidad.");
  } else {
    Serial.println("El motor ya está encendido.");
  }
}

// Función para incrementar la velocidad del motor brushless
void incrementarVelocidad() {
  if (motorEncendido) {
    if (velocidadActual + incrementoVelocidad <= velocidadMaxima) {
      velocidadActual += incrementoVelocidad; // Incrementar la velocidad
      esc.writeMicroseconds(velocidadActual); // Enviar nueva señal al ESC
      Serial.print("Velocidad incrementada a: ");
      Serial.print(velocidadActual);
      Serial.println(" uS.");
    } else {
      Serial.println("Velocidad máxima alcanzada.");
    }
  } else {
    Serial.println("El motor no está encendido. Usa 'a' para encenderlo.");
  }
}

// Función para apagar el motor brushless
void apagarBrushless() {
  if (motorEncendido) {
    esc.writeMicroseconds(velocidadMinima); // Señal mínima para detener el motor
    delay(500); // Esperar un momento antes de cortar la alimentación
    digitalWrite(relePin, LOW); // Apagar el relé para cortar la alimentación del ESC
    motorEncendido = false; // Cambiar el estado del motor a apagado
    velocidadActual = velocidadMinima; // Reiniciar la velocidad actual
    Serial.println("Motor apagado.");
  } else {
    Serial.println("El motor ya está apagado.");
  }
}

// Función para corregir la posición del motor
void autocorregirPosicion() {
    int valorAnalogico = analogRead(A1);  // Leer el valor analógico del receptor infrarrojo
    Serial.print("Valor analógico leído: ");
    Serial.println(valorAnalogico);

    if (valorAnalogico >= 800) {
        Serial.println("El motor ya está en la posición correcta.");
        return;  // No es necesario ajustar la posición
    }

    // Si el valor es menor a 800, iniciar el ajuste automático
    Serial.println("El valor es menor a 800. Corrigiendo posición...");
    bool posicionCorregida = false;
    int pasosMaximos1 = 18;  // Cantidad máxima de pasos antes de cambiar de dirección
    int pasosMaximos2 = 44;  // Cantidad máxima de pasos antes de cambiar de dirección
    int contadorPasos = 0;

    // Girar hacia la derecha primero
    while (!posicionCorregida && contadorPasos < pasosMaximos1) {
        paso_der1();  // Girar un paso a la derecha
        contadorPasos ++;
        delay(retardo1);  // Esperar para estabilizar el sensor

        valorAnalogico = analogRead(A1);  // Leer nuevamente el valor analógico
        Serial.print("Nuevo valor analógico (derecha): ");
        Serial.println(valorAnalogico);

        if (valorAnalogico >= 800) {
            posicionCorregida = true;
            Serial.println("Posición corregida hacia la derecha.");
        }
    }

    // Si no se corrigió la posición girando a la derecha, intentar a la izquierda
    if (!posicionCorregida) {
        Serial.println("No se encontró la posición hacia la derecha. Intentando hacia la izquierda...");
        contadorPasos = 0;  // Reiniciar contador de pasos

        while (!posicionCorregida && contadorPasos < pasosMaximos2) {
            paso_izq1();  // Girar un paso a la izquierda
            contadorPasos ++;
            delay(retardo1);  // Esperar para estabilizar el sensor

            valorAnalogico = analogRead(A1);  // Leer nuevamente el valor analógico
           Serial.print("Nuevo valor analógico (izquierda): ");
           Serial.println(valorAnalogico);

            if (valorAnalogico >= 800) {
                posicionCorregida = true;
           Serial.println("Posición corregida hacia la izquierda.");
            }
        }
    }

    // Finalizar
    if (!posicionCorregida) {
        Serial.println("No se pudo corregir la posición después de intentar en ambas direcciones.");
    } else {
        Serial.println("Posición corregida exitosamente.");
    }

    apagado();  // Apagar el motor
}

// Verifica continuamente si el motor está desalineado y activa la corrección si es necesario
void verificarPosicionContinuamente() {
  int valorAnalogico = analogRead(A1);  // Leer el valor del sensor infrarrojo
  if (valorAnalogico < 800 && enPosicionCorrecta) {
    // Si el valor está fuera del rango aceptable, inicia la corrección
    Serial.println("Desfase detectado. Activando sistema de autocorrección...");
    enPosicionCorrecta = false; // Marca como fuera de posición
    autocorregirPosicion();     // Llama a la función de corrección
  } else if (valorAnalogico >= 800 && !enPosicionCorrecta) {
    // Si el eje está correctamente alineado, actualiza el estado
    Serial.println("El eje se ha corregido y está alineado.");
    enPosicionCorrecta = true; // Marca como en posición correcta
  }
}

void shiftOutData(byte data) {
    digitalWrite(latchPin, LOW);   // Preparar el 74HC595 para recibir datos
    shiftOut(dataPin, clockPin, MSBFIRST, data); // Enviar datos al 74HC595
    digitalWrite(latchPin, HIGH);  // Actualizar la salida del 74HC595
}

void apagado() {         // Apagado del Motor
    shiftOutData(0b0000); // Apagar todos los pines
}

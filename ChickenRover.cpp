#include <IRremote.h> 
//arduino libraries e-install
//IRremote by shirriff, z3t0 (v 2.6.0)
//Angelo M. Morales
//CHICKEN ROVER

//comments-extpected result

// Motor Pins
const int IN1 = 13;
const int IN2 = 12;
const int IN3 = 11;
const int IN4 = 10;
const int ENA = 8;
const int ENB = 9;

// Ultrasonic Sensor
const int trigPin = 4;
const int echoPin = 3;

// IR Remote 
const int IR_RECEIVE_PIN = 2;

// LED Pin
const int ledPin = 7;
const int ledPin2 = 6;  // LED2

// IR Receiver Setup
IRrecv irrecv(IR_RECEIVE_PIN);
decode_results results;

// Variables
long duration;
int distance;
int motorSpeed = 80;
bool isChickenMode = false;
bool isMoving = false;
bool isMovingForward = false;

unsigned long lastMoveTime = 0;
unsigned long moveDuration = 0;
unsigned long pauseDuration = 0;

// Escape Routine variables
unsigned long lastObstacleTime = 0;
int obstacleCount = 0;
const int escapeThreshold = 3;          // # of hits before escape
const unsigned long obstacleWindow = 3000;  // 3 seconds window

// FUNCTIONS
void moveForward();
void moveBackward();
void turnLeft();
void turnRight();
void stopMovement();
void rampMotors();
long measureDistance();
void performRandomMovement();
void escapeRoutine();

void setup() {
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(ledPin, OUTPUT); 
  pinMode(ledPin2, OUTPUT); // LED2

  irrecv.enableIRIn();
  Serial.begin(9600);
  randomSeed(analogRead(A0));
}

void loop() {
  if (irrecv.decode(&results)) {
    long int decCode = results.value;
    Serial.println(decCode);

    if (decCode == 0xFFA25D) {  // cHICKEN MODE (CH-) toggle button 
      isChickenMode = !isChickenMode;
      stopMovement();

      digitalWrite(ledPin, isChickenMode ? HIGH : LOW);   // LED ON chicken mode
      digitalWrite(ledPin2, isChickenMode ? LOW : HIGH);  // LED2 ON manual mode

      Serial.println(isChickenMode ? "Chicken Mode Activated" : "Manual Mode Activated");
      delay(500);
    }

    if (!isChickenMode) {
      switch (decCode) {
        case 0xFF629D: moveForward(); break;     // this Forward  (CH)
        case 0xFFC23D: turnLeft(); break;        // this Left     (|<<)
        case 0xFF02FD: stopMovement(); break;    // this Stop     (>>|)
        case 0xFF22DD: turnRight(); break;       // this Right    (>||)
        case 0xFFA857: moveBackward(); break;    // this Backward (+)
        default: break;
      }
    }

    irrecv.resume();
  }

  distance = measureDistance();

  if (isChickenMode) {
    if (distance < 15) {
      Serial.println("Obstacle detected");
      stopMovement();
      delay(200);

      unsigned long now = millis();

      // Update obstacle count
      if (now - lastObstacleTime <= obstacleWindow) {
        obstacleCount++;
      } else {
        obstacleCount = 1;
      }
      lastObstacleTime = now;

      // ESCAPE ROUTINE if stuck
      if (obstacleCount >= escapeThreshold) {
        Serial.println("Stuck detected! Escaping...");
        escapeRoutine();
        obstacleCount = 0;  // reset after escape
      } else {
        // normal random avoidance
        if (random(0, 2) == 0) {
          turnLeft();
        } else {
          turnRight();
        }

        delay(300);
        moveForward();
        delay(400);
        stopMovement();
      }
    }

    unsigned long currentTime = millis();

    if (!isMoving && currentTime - lastMoveTime >= pauseDuration) {
      performRandomMovement();
      isMoving = true;
      moveDuration = random(300, 800);
      lastMoveTime = currentTime;
    }

    if (isMoving && currentTime - lastMoveTime >= moveDuration) {
      stopMovement();
      isMoving = false;
      pauseDuration = random(300, 600);
      lastMoveTime = currentTime;
    }

  } else {
    if (distance < 15 && isMovingForward) {
      Serial.println("Obstacle detected");
      stopMovement();
      delay(200);
      moveBackward();
      delay(500);
      stopMovement();
    }
  }
}

// ------------------------Movement Functions------------------------//

void moveForward() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  rampMotors();
  isMovingForward = true;
  Serial.println("Moving forward");
}

void moveBackward() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  rampMotors();
  isMovingForward = false;
  Serial.println("Moving backward");
}

void turnLeft() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  rampMotors();
  delay(300);
  stopMovement();
  Serial.println("Turning left");
}

void turnRight() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  rampMotors();
  delay(300);
  stopMovement();
  Serial.println("Turning right");
}

void stopMovement() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  isMovingForward = false;
  Serial.println("Stopped");
}

void rampMotors() {
  for (int i = 0; i <= motorSpeed; i += 5) {
    analogWrite(ENA, i);
    analogWrite(ENB, i);
    delay(10);
  }
}

long measureDistance() {
  digitalWrite(trigPin, LOW); delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  return (duration / 2) / 29.1;
}

void performRandomMovement() {
  int randAction = random(0, 3);  //CHICKEN MODE MOVEMENT RNG
  switch (randAction) {
    case 0: moveForward(); break;
    case 1: turnLeft(); break;
    case 2: turnRight(); break;
  }
}

// ------------------------Escape Routine------------------------//

void escapeRoutine() {
  moveBackward();
  delay(600);
  stopMovement();
  delay(200);
  
  if (random(0, 2) == 0) {
    turnLeft();
    delay(500);
  } else {
    turnRight();
    delay(500);
  }
  stopMovement();
}

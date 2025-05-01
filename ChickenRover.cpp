#include <IRremote.h>

const int IN1 = 13;
const int IN2 = 12;
const int IN3 = 11;
const int IN4 = 10;
const int ENA = 5;
const int ENB = 6;

const int trigPin = 4;
const int echoPin = 3;
const int IR_RECEIVE_PIN = 2;

IRrecv irrecv(IR_RECEIVE_PIN);
decode_results results;

long duration;
int distance;
int motorSpeed = 80;
bool isChickenMode = false;
bool isMoving = false;
bool isMovingForward = false;

unsigned long lastMoveTime = 0;
unsigned long moveDuration = 0;
unsigned long pauseDuration = 0;

void setup() {
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  irrecv.enableIRIn();
  Serial.begin(9600);
  randomSeed(analogRead(A0));
}

void loop() {
  if (irrecv.decode(&results)) {
    long int decCode = results.value;
    Serial.println(decCode);

    if (decCode == 0xFFA25D) {  // POWER button toggles mode
      isChickenMode = !isChickenMode;
      stopMovement();
      Serial.println(isChickenMode ? "🐔 Chicken Mode Activated" : "👤 Manual Mode Activated");
      delay(500);
    }

    if (!isChickenMode) {
      switch (decCode) {
        case 0xFF629D: moveForward(); break;     // Forward
        case 0xFFC23D: turnLeft(); break;        // Left
        case 0xFF02FD: stopMovement(); break;    // OK / Stop
        case 0xFF22DD: turnRight(); break;       // Right
        case 0xFFA857: moveBackward(); break;    // Backward
        default: break;
      }
    }

    irrecv.resume();
  }

  distance = measureDistance();

  if (isChickenMode) {
    // 🐔 Chicken reacts naturally — no backward motion
    if (distance < 15) {
      Serial.println("🐔 Obstacle detected! Changing direction...");
      stopMovement();
      delay(200);

      // Turn left or right randomly
      if (random(0, 2) == 0) {
        turnLeft();
      } else {
        turnRight();
      }

      delay(300);
      moveForward();
      delay(400); // move forward a bit after turning
      stopMovement();
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
    // 👤 Manual mode obstacle avoidance (only forward blocked)
    if (distance < 15 && isMovingForward) {
      Serial.println("⚠️ Obstacle detected in manual mode!");
      stopMovement();
      delay(200);
      moveBackward();
      delay(500);
      stopMovement();
    }
  }
}

void performRandomMovement() {
  int randAction = random(0, 3);  // 0 to 2: forward, left, right
  switch (randAction) {
    case 0: moveForward(); break;
    case 1: turnLeft(); break;
    case 2: turnRight(); break;
  }
}

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

// 🌀 Turn left using right wheel only
void turnLeft() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);  // left wheel stop
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW); // right wheel forward
  rampMotors();
  delay(300);
  stopMovement();
  Serial.println("Turning left");
}

// 🌀 Turn right using left wheel only
void turnRight() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW); // left wheel forward
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);  // right wheel stop
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

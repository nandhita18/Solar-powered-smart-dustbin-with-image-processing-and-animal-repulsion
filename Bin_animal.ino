#include <LiquidCrystal.h>
#include <TinyGPS++.h>

LiquidCrystal lcd(8, 9, 10, 11, 12, 13);

const int trigPin = A1;
const int echoPin = A0;

TinyGPSPlus gps;

int binLevel;
float latitude = 13.032608892244347;
float longitude = 80.17787275573419;

unsigned long lastMeasurementTime = 0;
const unsigned long measurementInterval = 1000;

bool messageSent = false;

// Motor pins
const int doorMotor1 = 47;
const int doorMotor2 = 49;
const int pumpMotor1 = 51;
const int pumpMotor2 = 53;

void setup() {
  Serial.begin(9600);
  Serial3.begin(9600); // GSM
  Serial2.begin(9600); // GPS
  
  lcd.begin(16, 2);
  
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  pinMode(doorMotor1, OUTPUT);
  pinMode(doorMotor2, OUTPUT);
  pinMode(pumpMotor1, OUTPUT);
  pinMode(pumpMotor2, OUTPUT);
  
  Serial3.println("AT+CMGF=1"); // Set SMS text mode
  delay(100);
}

void loop() {
  unsigned long currentTime = millis();
  
  if (currentTime - lastMeasurementTime >= measurementInterval) {
    measureBinLevel();
    updateGPS();
    displayInfo();
    
    if (binLevel >= 70 && !messageSent) {
      sendSMS();
      messageSent = true;
      displaySendingMessage();
    }
    
    lastMeasurementTime = currentTime;
  }
  
  if (Serial.available() > 0) {
    char command = Serial.read();
    
    if (command == 'o') { // 'o' for open_door
      openDoor();
    } else if (command == 'a') { // 'a' for activate_pump
      activatePump();
    }
  }
}

void measureBinLevel() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH, 30000);
  int distance = duration * 0.034 / 2;
  
  binLevel = map(distance, 100, 0, 0, 100);
  binLevel = constrain(binLevel, 0, 100);
}

void updateGPS() {
  bool validLocation = false;
  while (Serial2.available() > 0) {
    if (gps.encode(Serial2.read())) {
      if (gps.location.isValid()) {
        latitude = gps.location.lat();
        longitude = gps.location.lng();
        validLocation = true;
      }
    }
  }
  if (!validLocation) {
    latitude = 13.032608892244347;
    longitude = 80.17787275573419;
  }
}

void displayInfo() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Bin Level: ");
  lcd.print(binLevel);
  lcd.print("%");
}

void sendSMS() {
  Serial3.println("AT+CMGS=\"09080192655\"");
  delay(100);
  
  Serial3.print("Bin is ");
  Serial3.print(binLevel);
  Serial3.print("% full. Location: ");
  Serial3.print("http://maps.google.com/maps?q=");
  Serial3.print(latitude, 6);
  Serial3.print(",");
  Serial3.print(longitude, 6);
  
  Serial3.write(26); // End SMS
}

void displaySendingMessage() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sending message");
  lcd.setCursor(0, 1);
  lcd.print("to driver...");
  delay(10000);
  displayInfo();
}

void openDoor() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Person detected");
  lcd.setCursor(0, 1);
  lcd.print("Door opening...");
  
  digitalWrite(doorMotor1, HIGH);
  digitalWrite(doorMotor2, LOW);
  delay(2000); // Open door for 5 seconds
  digitalWrite(doorMotor1, LOW);
  digitalWrite(doorMotor2, LOW);
  lcd.clear();
  lcd.setCursor(0, 0);
  delay(5000);
  lcd.print("Door closing...");
  digitalWrite(doorMotor1, LOW);
  digitalWrite(doorMotor2, HIGH);
    delay(2000); // Open door for 5 seconds
  digitalWrite(doorMotor1, LOW);
  digitalWrite(doorMotor2, LOW);
  delay(1000); // Wait for 1 second
  
  lcd.clear();
}

void activatePump() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Animal detected");
  lcd.setCursor(0, 1);
  lcd.print("Pump activating...");
  
  digitalWrite(pumpMotor1, HIGH);
  digitalWrite(pumpMotor2, LOW);
  delay(5000); // Activate pump for 5 seconds
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Pump deactivating...");
  digitalWrite(pumpMotor1, LOW);
  digitalWrite(pumpMotor2, LOW);
  delay(1000); // Wait for 1 second
  
  lcd.clear();
}

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

void setup() {
  Serial.begin(9600);
  Serial3.begin(115200); // GSM
  Serial2.begin(9600); // GPS
  
  lcd.begin(16, 2);
  
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
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
  Serial3.println("AT+CMGS=\"09344638753\"");
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

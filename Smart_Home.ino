#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h>

#define trig 2  
#define in_led1 3
#define in_led2 4
#define in_side_led1 5
#define in_side_led2 6
#define out_led_gate 7
#define out_led1 8
#define out_led2 9
#define out_led3 10
#define out_led4 11
#define out_led5 12
#define out_led6 13
#define take_picture 14 //pin to send signal to the ESP32 CAM and triggering it to take a picture of the thing in the gate
#define is_authorized_plate 22
#define servo_pin 15
#define echo 16


// LED & Buzzer Pin

#define rfid_green 24  // Green LED (Scanning)
#define rfid_red 25    // Red LED (Access Denied)
#define buzzer_pin 23  



// RFID Pins (Arduino Mega)
#define SS_PIN 53  //SDA pin 
#define RST_PIN 49   

MFRC522 rfid(SS_PIN, RST_PIN);
Servo myServo;

// ✅ Allowed UID (E3 EE 37 0E)
byte authorizedUID[] = {0xB5, 0xB1, 0x4F, 0x00}; 

bool checkRFID();
void open_close_gate();

void setup() {
    Serial.begin(9600);
    SPI.begin();  
    rfid.PCD_Init();
    myServo.attach(servo_pin);


    pinMode(rfid_green, OUTPUT);
    pinMode(rfid_red, OUTPUT);
    pinMode(buzzer_pin, OUTPUT);
    pinMode(in_led1, OUTPUT);
    pinMode(in_led2, OUTPUT);
    pinMode(in_side_led2,OUTPUT);
    pinMode(in_side_led1,OUTPUT);
    pinMode(out_led_gate, OUTPUT);
    pinMode(out_led1, OUTPUT);
    pinMode(out_led2, OUTPUT);
    pinMode(out_led3,OUTPUT);
    pinMode(out_led4, OUTPUT);
    pinMode(out_led5, OUTPUT);
    pinMode(out_led6, OUTPUT);
    pinMode(take_picture, OUTPUT);
    pinMode(is_authorized_plate, INPUT);

    //Ultrasound pins
    pinMode(trig, OUTPUT);
    pinMode(echo, INPUT);

    //myServo.write(0);
    digitalWrite(is_authorized_plate, LOW);

    Serial.println("System Ready! Scan your RFID card...");
}

void loop() {
  //initialize variables to calculate the distances
  long duration;
  float distance;

  //calculate the time it takes to get an echo
  digitalWrite(trig, LOW);
  delay(3);
  digitalWrite(trig, HIGH);
  delay(10);
  digitalWrite(trig, LOW);

  duration = pulseIn(echo, HIGH);

  //calculate distance 
  distance = duration * 0.0343 / 2;  //in cm
  Serial.println(distance);
    //If car approaches the gate
    
  if(distance <=20){
    digitalWrite(take_picture, HIGH);
    Serial.println("Picture taken");
    if(is_authorized_plate == HIGH){
      open_close_gate();
      Serial.println("CAR plate KCA 123A authorized");
    }
    bool myRFID = checkRFID(); 
    if (myRFID){
      digitalWrite(in_led1, HIGH);
      digitalWrite(in_led2, HIGH);
      digitalWrite(in_side_led1, HIGH);
      digitalWrite(in_side_led2, HIGH);
      digitalWrite(out_led_gate, HIGH);
      digitalWrite(out_led_gate, HIGH);
      digitalWrite(out_led1, HIGH);
      digitalWrite(out_led2, HIGH);
      digitalWrite(out_led3, HIGH);
      digitalWrite(out_led4, HIGH);
      digitalWrite(out_led5, HIGH);
      digitalWrite(out_led6, HIGH);
      //digitalWrite(out_led7, HIGH);
      delay(10000);
      digitalWrite(in_led1, LOW);
      digitalWrite(in_led2, LOW );
      digitalWrite(in_side_led1, LOW);
      digitalWrite(in_side_led2, LOW);
      digitalWrite(out_led_gate, LOW);
      digitalWrite(out_led_gate, LOW);
      digitalWrite(out_led1, LOW);
      digitalWrite(out_led2, LOW);
      digitalWrite(out_led3, LOW);
      digitalWrite(out_led4, LOW);
      digitalWrite(out_led5, LOW);
      digitalWrite(out_led6, LOW);
      //digitalWrite(out_led7, LOW);
    }
         
  }
    digitalWrite(take_picture, LOW);


    //We want when a car approaches the gate, the PIR/ultrasonic sensor detects that and waits for authentication
      //Once the car is inside the compund, Security lights turn on until the person goes into the house
}
bool checkRFID(){
   bool authorized = false;
   if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
        digitalWrite(buzzer_pin, HIGH); // ✅ RFID green LED ON when scanning

        Serial.print("Card UID: ");
        authorized = true;
        if (rfid.uid.size != sizeof(authorizedUID)) {
            authorized = false; // UID size mismatch
        } else {
            for (byte i = 0; i < rfid.uid.size; i++) {
                Serial.print(rfid.uid.uidByte[i], HEX);
                Serial.print(" ");
                if (rfid.uid.uidByte[i] != authorizedUID[i]) {
                    authorized = false;
                }
            }
        }
         if (authorized) {
          open_close_gate();
          digitalWrite(in_led1, HIGH);
          digitalWrite(in_led2, HIGH);
          digitalWrite(in_side_led1, HIGH);
          digitalWrite(in_side_led2, HIGH);
          digitalWrite(out_led_gate, HIGH);
          digitalWrite(out_led1, HIGH);
          digitalWrite(out_led2, HIGH);
          digitalWrite(out_led3, HIGH);
          digitalWrite(out_led4, HIGH);
          digitalWrite(out_led5, HIGH);
          digitalWrite(out_led6, HIGH);
          //digitalWrite(out_led7, HIGH);
          }
          else{
            
            digitalWrite(in_led1, LOW);
            digitalWrite(in_led2, LOW);
            digitalWrite(in_side_led1, LOW);
            digitalWrite(in_side_led2, LOW);
            //checkCamera()
          }
          Serial.println("✅ Access Granted!");
          digitalWrite(buzzer_pin, HIGH);
          delay(300);
          digitalWrite(buzzer_pin, LOW);
          return true;
        } 
        else {
          Serial.println("❌ Access Denied!"); 
          delay(500);
          digitalWrite(rfid_green, LOW);
          rfid.PICC_HaltA();
          authorized = false; // Stop reading
          return false;     
        }
  }


void open_close_gate(){
  myServo.write(180);  // Full speed clockwise
  delay(1000);        // Run for 10 seconds

  // Stop for 1 second
  myServo.write(90);   // Stop (neutral position)
  delay(2500);

  // Rotate counterclockwise
  myServo.write(0);    // Full speed counterclockwise
  delay(1000);        // Run for 10 seconds

  // Stop for 1 second
  myServo.write(90);   // Stop (neutral position)
  delay(1000);
}


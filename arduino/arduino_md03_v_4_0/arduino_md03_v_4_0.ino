/***************************************************
           ArcEyes MD03 I2C sandbox
             by David Wilman        
   MD03 over I2C code by By James Henderson 2012       
****************************************************/

#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>

 //int LCD pins on I2C bus
 #define I2C_ADDR 0x27
 #define BACKLIGHT_PIN 3
 #define En_pin  2
 #define Rw_pin  1
 #define Rs_pin  0
 #define D4_pin  4
 #define D5_pin  5
 #define D6_pin  6
 #define D7_pin  7
 LiquidCrystal_I2C lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);

#define address_1           0x58                     // Address of MD03 1
#define address_2           0x59                     // Address of MD03 2
#define address_3           0x5A                     // Address of MD03 3
#define SOFTREG             0x07                    // Byte to read software
#define CMDBYTE             0x00                    // Command byte
#define SPEEDBYTE           0x02                    // Byte to write to speed register
#define TEMPREG             0x04                    // Byte to read temprature
#define CURRENTREG          0x05                    // Byte to read motor current

const int potPin1 = A0;
const int potPin2 = A1;
const int potPin3 = A2;
const int rlyPin1 = 7; // Relay Pin for break
const int rlyPin2 = 8; // Relay Pin for break
const int rlyPin3 = 9; // Relay Pin for break
byte direct1 = 1; // Stores what direction the motor should run in
byte direct2 = 1; // Stores what direction the motor should run in
byte direct3 = 1; // Stores what direction the motor should run in
//int temp = 0;
int temp1 = 0; // store the temp read from MD03
int temp2 = 0; // store the temp read from MD03
int temp3 = 0; // store the temp read from MD03
//float current = 0; // store the current read from MD03
float current1 = 0; // store the current read from MD03
float current2 = 0; // store the current read from MD03
float current3 = 0; // store the current read from MD03
//float speedControl = 0; //store the current speed
float speedControl1 = 0; //store the current speed
float speedControl2 = 0; //store the current speed
float speedControl3 = 0; //store the current speed
long previousMillis = 0;  // store the time LCD was last updated
long interval = 50; // time setween lcd updates


//serial
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete



void setup(){
  Wire.begin(); //start I2C bus
  delay(100); //wait for bus to stabalise
  pinMode(rlyPin1,OUTPUT); // Setup pins
  pinMode(rlyPin2,OUTPUT); // Setup pins
  pinMode(rlyPin3,OUTPUT); // Setup pins
  lcd.begin (16,2); // 2 rows x 16 char
  // Switch on the backlight
  lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("ArcEyes MD03 I2C");
  lcd.setCursor(0,1);
  lcd.print("SandBox");
  delay(30);
  //lcd.clear();
  
  
  //serial
  
    Serial.begin(9600);
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);
}

void loop(){
  direct1 = manualSpeedControl(address_1, potPin1, rlyPin1, direct1, speedControl1);
  direct2 = manualSpeedControl(address_2, potPin2, rlyPin2, direct2, speedControl2);
  direct3 = manualSpeedControl(address_3, potPin3, rlyPin3, direct3, speedControl3);
  
    // print the string when a newline arrives:
  if (stringComplete) {
    Serial.println(inputString); 
    // clear the string:
    inputString = "";
    stringComplete = false;
  }
  
 
}


byte manualSpeedControl(int address, byte potPin, int rlyPin, byte direct, int speedControl){
    int potVal = analogRead(potPin);
    if(potVal < 450){               //Sets reverse speed from Pot1
      digitalWrite(rlyPin, LOW);
      speedControl = map(potVal, 450, 0, 0, 250);
      direct = 1;
    }
    else if(potVal < 573){          // sets zero speed from Pot1
      digitalWrite(rlyPin, HIGH);
      speedControl = 0;
    }
    else if (potVal >= 574){        // sets forward speed from Pot1
      digitalWrite(rlyPin, LOW);
      speedControl = map(potVal, 574, 1023, 0, 250);
      direct = 2;
    }
    sendData(address, SPEEDBYTE, speedControl);  
    sendData(address, CMDBYTE, direct);          
    int temp = getData(address, TEMPREG);            // Gets temperature
    delay(50);
    int current = map(getData(address, CURRENTREG), 0, 186, 0 ,20000);      // Gets motor current, scale to give output in mA
    current = (current/1000) - 0.32; //convert to A
    delay(50);   
    return direct;
}

byte getData(int address, byte reg){                   // function for getting data from MD03
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.endTransmission();
  
  byte data = 0;
  //for (int i; i<100; i++){
    Wire.requestFrom(address, 1);         // Requests byte from MD03
    while(Wire.available() < 1);          // Waits for byte to become availble
    data = Wire.read();
    //}
  
  return(data);
}

void sendData(int address, byte reg, byte val){         // Function for sending data to MD03
  Wire.beginTransmission(address);         // Send data to MD03
    Wire.write(reg);
    Wire.write(val);
  Wire.endTransmission();
}

/*
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(inChar);
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    } 
  }
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(inputString);

}

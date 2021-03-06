/***Blind Spot Detection for TI DIY Intern Edition***/

/*See Schematic for setup of project*/
/** MCU used
For demo: MSP-EXP432P401R

SCL: P6.5
SDA: P6.4

Driver used: DRV2625EVM-mini  
DRV2625 Slave Address: 0x5A (7-bit)  see section 8.6.3.3 of DRV2625 datasheet

SCL/SDA Pullups used:
3.3V source
5.6K resistor (yet to be installed) 
Set up between I2C output of controller and DRV2625EVM-mini I2C inputs

Format: [read/write] [slaveAddress(Hex)] [dataRegister(Hex)] [Data to Write(Hex)]
Note: Registers were found using the Haptics Control Console and the i2c sniffer and Register Maps on it

DRV2625 Boot code
w 58 01 00   Register name: Status (removes device from standby))
w 58 07 84   Register name: Control1 (sets broadcast mode on, trig pin function to external enable, and RTP mode)
w 58 08 48   Register name: Control2 (sets ERM mode, Open Loop)
w 58 e0 7f   Register name: RTPStrength (sets RTP strength to 100%)
w 58 1f 5b   Register name: RatedVoltage (sets the rated voltage to 2.0V)  //change this!
w 58 20 7b   Register name: OverDriveVoltage (sets the OverDriveVoltage to 2.5V)  //change this!

note: Rated voltage and overdrive voltage should be changed according to what actuator you are using. 
In this case, our actuator is the Z7AL2B1692082 from Jinlong Machinary (ERM rated at 3V but we run it at 2.5V)

Now the DRV2625 will run the actuator at 2.5V when a signal is sent to the TRIG pin on the driver. 

**/

//Wire.h allows us to write to the registers in the ERM Driver
#include <Wire.h>
#define ERM_TriggPIN 8   //PIN 4.6: Controlls the Trigger for the ERM Driver
#define DRV2625 (0x5A)   //Energia must use a 7 bit address 
#define LED RED_LED      //most launchpads have one
#define button 11        //PIN 3.6: Controls button that acts like "car blinker"
#define echoPin 18       //PIN 3.0: Recieves echo reponse from Ultrasonic Sensor
#define trigPin 19       //PIN 2.5: Sends Trigger to Ultrasonic Sensor
#define Blinker_LED 17   //PIN 5.7: Blinks to simulate a car "blinker"

boolean toggle = true;

void setup() {
  pinMode(LED, OUTPUT);     //Intialize LED
  Wire.begin();             //Join i2c bus
  digitalWrite(LED, LOW);   //Turn LED off
  Serial.begin (9600);
  delay(100);               //Wait at least 1 us before device accepts transmission
  
  pinMode(ERM_TriggPIN, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(button,INPUT);
  pinMode(Blinker_LED, OUTPUT);
  
  //w 58 01 00
  Wire.beginTransmission(DRV2625);   //Slave address
  Wire.write(0x01);                  //Register to write to 
  Wire.write(0x00);                  //Data to write to register
  Wire.endTransmission();            //Each individual command must be begun and then ended before doing the next one
 
  
  //w 58 07 84
  Wire.beginTransmission(DRV2625);   //Slave address
  Wire.write(0x07);                  //Register to write to 
  Wire.write(0x84);                  //Data to write to register
  Wire.endTransmission();            //Each individual command must be begun and then ended before doing the next one
 
  //w 58 08 48
  Wire.beginTransmission(DRV2625);   //Slave address
  Wire.write(0x08);                  //Register to write to 
  Wire.write(0x48);                  //Data to write to register
  Wire.endTransmission();            //Each individual command must be begun and then ended before doing the next one
  
  //w 58 e0 7f   RTP Strength (change to 0x40 for 50%) 
  Wire.beginTransmission(DRV2625);   //Slave address
  Wire.write(0xe0);                  //Register to write to 
  Wire.write(0x7f);                  //Data to write to register
  Wire.endTransmission();            //Each individual command must be begun and then ended before doing the next one
  
  //w 58 1f 5b
  Wire.beginTransmission(DRV2625);   //Slave address
  Wire.write(0x1f);                  //Register to write to 
  Wire.write(0x5b);                  //Data to write to register
  Wire.endTransmission();            //Each individual command must be begun and then ended before doing the next one
  
  //w 58 20 7b
  Wire.beginTransmission(DRV2625);   //Slave address
  Wire.write(0x20);                  //Register to write to 
  Wire.write(0x2e);                  // 1V
  Wire.endTransmission();            //Each individual command must be begun and then ended before doing the next one
  
  // 1V - 2e
  // 1.5V - 45
  // 2V - 5c
  // 2.5V -73
}

//Main Loop
void loop() {
  int val = digitalRead(button);   //Holds Button State (Pressed or Unpressed)
  if(val == LOW){                  //Checks for Button press; else, skip sensor
    long duration, distance;
    
    if(toggle){
      toggle = false;
      digitalWrite(Blinker_LED,LOW); 
    }
    else{
      toggle = true;
      digitalWrite(Blinker_LED,HIGH); 
    }
    
    
    /* Protocol for Ultrasonic Sensor */
    digitalWrite(trigPin, LOW);  
    delayMicroseconds(2); 
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    /* Calculation */
    duration = pulseIn(echoPin, HIGH);
    distance = (duration/2) / 29.1;
    
    //Vibration strength depends on distance
    if (distance < 25) {
      digitalWrite(ERM_TriggPIN, HIGH);  //Send signal to Trigger the Driver
      Wire.beginTransmission(DRV2625);   //Slave Address
      Wire.write(0x20);                  //Register to write to
      Wire.write(0x73);                  //Strength 
      Wire.endTransmission();            
      delay(50);                         //Delay to wait for register to take value
      digitalWrite(ERM_TriggPIN, HIGH);
    }
    else if (distance < 50) {  
      digitalWrite(ERM_TriggPIN, HIGH);
      Wire.beginTransmission(DRV2625); 
      Wire.write(0x20);
      Wire.write(0x5c);
      Wire.endTransmission();
      delay(50);
      digitalWrite(ERM_TriggPIN, HIGH);
    }
    else if (distance < 75) {  
      digitalWrite(ERM_TriggPIN, HIGH);
      Wire.beginTransmission(DRV2625);
      Wire.write(0x20);
      Wire.write(0x45);
      Wire.endTransmission();
      delay(50);
    }
    else if (distance < 100) {  
      Wire.beginTransmission(DRV2625);
      Wire.write(0x20);
      Wire.write(0x2e);
      Wire.endTransmission();
      delay(50);
      digitalWrite(ERM_TriggPIN, HIGH);
    }
    else {
      digitalWrite(ERM_TriggPIN, LOW);
    }
    
    //Filter out any distance over 2m
    if (!(distance >= 200 || distance <= 0)){
     Serial.print(distance);
     Serial.println(" cm");
    }
  }
  else{ /* Condition if button is not pressed */
    Serial.println("Disconnected");
    digitalWrite(ERM_TriggPIN, LOW);
    digitalWrite(Blinker_LED, LOW);
  }
  delay(100);
}

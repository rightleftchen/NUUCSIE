#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <I2Cdev.h>
#include <ArduinoJson.h>  // biblioteca JSON para sistemas embarcados
#include "MPU6050_6Axis_MotionApps20.h"
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

//Global Variable

WiFiClient client;
MPU6050 mpu;
Quaternion q;    // [w, x, y, z] quaternion container
#define OUTPUT_READABLE_QUATERNION
#define INTERRUPT_PIN D5  // use pin 2 on Arduino Uno & most boards
bool blinkState = false;// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer
String w,x,y,z,str;
const char* ssid = "TOTOLINK N500RD";    //define WIFI SSID & PASSWORDS
const char* passwords = "";
volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high

void setup() {
    Serial.begin(115200);
    Serial.println();
    while (!Serial); // wait for Leonardo enumeration, others continue immediately    
    WiFi.begin("TOTOLINK N500RD","79461382"); // Connect to wifi
    Serial.println("");
    while(WiFi.status() != WL_CONNECTED){
      delay(100);
      Serial.print(".");
      }
      Serial.println("");
      Serial.println("WiFi Connected!!");
          
    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin(D2,D1);
        Wire.setClock(400000); // 400kHz I2C clock. Comment this line if having compilation difficulties
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
    #endif
    
    Serial.println(F("Initializing I2C devices..."));         // initialize device
    mpu.initialize();
    pinMode(INTERRUPT_PIN, INPUT);
                                             
    Serial.println(F("Testing device connections..."));       // verify connection
    Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));
    Serial.println(F("Initializing DMP..."));                 // load and configure the DMP
    devStatus = mpu.dmpInitialize();
    
    mpu.setXGyroOffset(220);                                 // supply your own gyro offsets here, scaled for min sensitivity
    mpu.setYGyroOffset(76);
    mpu.setZGyroOffset(-85);
    mpu.setZAccelOffset(1788);                                // 1688 factory default for my test chip

    // make sure it worked (returns 0 if so)
    if (devStatus == 0) {                                           
        Serial.println(F("Enabling DMP..."));                  // turn on the DMP, now that it's ready
        mpu.setDMPEnabled(true);      
        Serial.println(F("Enabling interrupt detection (Arduino external interrupt 0)..."));  // enable Arduino interrupt detection
        attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), dmpDataReady, RISING);
        mpuIntStatus = mpu.getIntStatus();
      
        // set our DMP Ready flag so the main loop() function knows it's okay to use it
        Serial.println(F("DMP ready! Waiting for first interrupt..."));
        dmpReady = true;

        // get expected DMP packet size for later comparison
        packetSize = mpu.dmpGetFIFOPacketSize();
    } 
    else {
        Serial.print(F("DMP Initialization failed (code "));
        Serial.print(devStatus);
        Serial.println(F(")"));
    }
}

void loop() {    
    if (!dmpReady) return;                                // if programming failed, don't try to do anything   
    while (!mpuInterrupt && fifoCount < packetSize) {     // wait for MPU interrupt or extra packet(s) available  
    }   
    mpuInterrupt = false;                                 // reset interrupt flag and get INT_STATUS byte
    mpuIntStatus = mpu.getIntStatus();    
    fifoCount = mpu.getFIFOCount();                       // get current FIFO count   
    if ((mpuIntStatus & 0x10) || fifoCount == 1024) {     // check for overflow (this should never happen unless our code is too inefficient)        
        mpu.resetFIFO();                                  // reset so we can continue cleanly
                                                          
    } else if (mpuIntStatus & 0x02) {                     // otherwise, check for DMP data ready interrupt (this should happen frequently)        
        while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();// wait for correct available data length, should be a VERY short wait        
        mpu.getFIFOBytes(fifoBuffer, packetSize);         // read a packet from FIFO        
        fifoCount -= packetSize;                          // track FIFO count here in case there is > 1 packet available
                                                          // (this lets us immediately read more without waiting for an interrupt) 
        #ifdef OUTPUT_READABLE_QUATERNION                 // display quaternion values in easy matrix form: w x y z            
            mpu.dmpGetQuaternion(&q, fifoBuffer);
            w = String(q.w);
            x = String(q.x);
            y = String(q.y);
            z = String(q.z);
            str = "&w="+w+"&x="+x+"&y="+y+"&z="+z;
        #endif
        makePOST();
    }
}
void dmpDataReady() {
    mpuInterrupt = true;
}
void makePOST(){
  if (client.connect("120.105.129.111",80)) {       // REPLACE WITH YOUR SERVER ADDRESS
    client.println("POST /QQ.php HTTP/1.1"); 
    client.println("Host: 120.105.129.111");      // SERVER ADDRESS HERE TOO
    client.println("Content-Type: application/x-www-form-urlencoded"); 
    client.print("Content-Length: "); 
    client.println(str.length());
    client.println(); 
    client.print(str); 
  }
  else Serial.println("Connect Fail!"); 
  
  if(client.connected()){
    client.stop();
  }  
}

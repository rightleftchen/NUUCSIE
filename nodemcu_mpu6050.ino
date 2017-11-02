#include <Wire.h>
 
const int MPU_ADDR =      0x68; // Address setting sensor MPU6050 (0x68)
const int WHO_AM_I =      0x75; // Device identification record
const int PWR_MGMT_1 =    0x6B; // Power management configuration log
const int GYRO_CONFIG =   0x1B; // Gyro configuration record
const int ACCEL_CONFIG =  0x1C; // Accelerometer setup log
const int ACCEL_XOUT =    0x3B; // Accelerometer X-axis read record

const int sda_pin = D2; // Pin definition I2C SDA
const int scl_pin = D1; // Pin definition I2C SCL
 
bool led_state = false;
 
// Variables to store the raw data of the accelerometer
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ; 
 

void setup(){
  
  pinMode(LED_BUILTIN, OUTPUT);
  
  Serial.begin(115200);
  Serial.println("\nStarting the MPU6050\n");
  
  initI2C();
  initMPU();
 
  checkMPU(MPU_ADDR); 
}
 
void loop() {
  readRawMPU();// Reads sensor data
  delay(500);
}

void initI2C(){
  Wire.begin(sda_pin, scl_pin);
}

void initMPU(){
  setSleepOff();
  setGyroScale();
  setAccelScale();
}

void writeRegMPU(int reg, int val){     // Accepts a record and a value as parameter
  Wire.beginTransmission(MPU_ADDR);     // Initiates communication with the MPU6050 address
  Wire.write(reg);                      // Sends the record with which you want to work
  Wire.write(val);                      // Write the value in the registry
  Wire.endTransmission(true);           // The transmission ends
}

uint8_t readRegMPU(uint8_t reg){        // Accepts a record as a parameter
  uint8_t data;
  Wire.beginTransmission(MPU_ADDR);     // Initiates communication with the MPU6050 address
  Wire.write(reg);                      // Sends the record with which you want to work
  Wire.endTransmission(false);          // Ends transmission but continues with I2C open (sends STOP and START)
  Wire.requestFrom(MPU_ADDR, 1);        // Configures to receive 1 byte from the record chosen above.
  data = Wire.read();                   // Reads the byte and saves in 'date'
  return data;                          // Returns 'date'
}

void findMPU(int mpu_addr){
  Wire.beginTransmission(MPU_ADDR);
  int data = Wire.endTransmission(true);
  if(data == 0){
    Serial.print("Device found at address: 0x");
    Serial.println(MPU_ADDR, HEX);
  }
  else{
    Serial.println("Device not found!");
  }
}

void checkMPU(int mpu_addr){
  findMPU(MPU_ADDR);
     
  int data = readRegMPU(WHO_AM_I); // Register 117 – Who Am I - 0x75
   
  if(data == 104){
    Serial.println("MPU6050 Device responded OK! (104)");
    data = readRegMPU(PWR_MGMT_1); // Register 107 – Power Management 1-0x6B
    if(data == 64) Serial.println("MPU6050 in SLEEP mode! (64)");
    else Serial.println("MPU6050 in ACTIVE mode!"); 
  }
  else Serial.println("Check Device - MPU6050 NOT AVAILABLE!");
}

void setSleepOff(){
  writeRegMPU(PWR_MGMT_1, 0); 
}
/*------------------------------------------------------------------------------- 
    Function to configure the gyro scales
    Gyroscope scale register: 0x1B [4: 3]
    0 is 250 °/s
 
    FS_SEL  Full Scale Range
      0        ± 250 °/s      0b00000000
      1        ± 500 °/s      0b00001000
      2        ± 1000 °/s     0b00010000
      3        ± 2000 °/s     0b00011000
*/
void setGyroScale(){
  writeRegMPU(GYRO_CONFIG, 0);
}
/*------------------------------------------------------------------------------ 
    Function to configure the accelerometer scales
    Accelerometer scale record: 0x1C [4: 3]
    0 is 250 °/s
 
    AFS_SEL   Full Scale Range
      0           ± 2g            0b00000000
      1           ± 4g            0b00001000
      2           ± 8g            0b00010000
      3           ± 16g           0b00011000
*/
void setAccelScale(){
  writeRegMPU(ACCEL_CONFIG, 0);
}
/*------------------------------------------------------------------------------
    Function that reads the raw data of the sensor
    Are 14 bytes in total being they 2 bytes for each axis and 2 bytes for temperature:
 
    0x3B 59 ACCEL_XOUT[15:8]
    0x3C 60 ACCEL_XOUT[7:0]
    0x3D 61 ACCEL_YOUT[15:8]
    0x3E 62 ACCEL_YOUT[7:0]
    0x3F 63 ACCEL_ZOUT[15:8]
    0x40 64 ACCEL_ZOUT[7:0]
 
    0x41 65 TEMP_OUT[15:8]
    0x42 66 TEMP_OUT[7:0]
 
    0x43 67 GYRO_XOUT[15:8]
    0x44 68 GYRO_XOUT[7:0]
    0x45 69 GYRO_YOUT[15:8]
    0x46 70 GYRO_YOUT[7:0]
    0x47 71 GYRO_ZOUT[15:8]
    0x48 72 GYRO_ZOUT[7:0]
    
*/
void readRawMPU(){  
  Wire.beginTransmission(MPU_ADDR);       // Initiates communication with the MPU6050 address
  Wire.write(ACCEL_XOUT);                 // Sends the record with which you want to work, starting with record 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);            // Ends transmission but continues with I2C open (sends STOP and START)
  Wire.requestFrom(MPU_ADDR, 14);         // Configures to receive 14 bytes starting from the record chosen above (0x3B)
 
  AcX = Wire.read() << 8;                 // Reads the most significant byte first
  AcX |= Wire.read();                     // Then reads the least significant bit
  AcY = Wire.read() << 8;
  AcY |= Wire.read();
  AcZ = Wire.read() << 8;
  AcZ |= Wire.read();
 
  Tmp = Wire.read() << 8;
  Tmp |= Wire.read();
 
  GyX = Wire.read() << 8;
  GyX |= Wire.read();
  GyY = Wire.read() << 8;
  GyY |= Wire.read();
  GyZ = Wire.read() << 8;
  GyZ |= Wire.read(); 
 
  Serial.print("AcX = "); Serial.print(AcX);
  Serial.print(" | AcY = "); Serial.print(AcY);
  Serial.print(" | AcZ = "); Serial.print(AcZ);
  Serial.print(" | Tmp = "); Serial.print(Tmp/340.00+36.53);
  Serial.print(" | GyX = "); Serial.print(GyX);
  Serial.print(" | GyY = "); Serial.print(GyY);
  Serial.print(" | GyZ = "); Serial.println(GyZ);
 
  led_state = !led_state;
  digitalWrite(LED_BUILTIN, led_state);         // Flashes NodeMCU LED at each sensor reading
  delay(10);                                        
}

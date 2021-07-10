#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define PIN_CE 10
#define PIN_CSN 9
#define DATA_LEN 3

// MODULES
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);
RF24 radio(PIN_CE, PIN_CSN);

// GLOBAL VARS
float data[DATA_LEN];

// FUNCTIONS
void displaySensorDetails(void);


void setup(void) 
{
  Serial.begin(9600);
  
  /* Initialise the sensor */
  if(!mag.begin())
  {
    /* There was a problem detecting the HMC5883 ... check your connections */
    Serial.println("Ooops, no HMC5883 detected ... Check your wiring!");
    while(1);
  }

  if (!radio.begin())
  {
    Serial.println("Ooops, no nRF detected ... Check your wiring!");
    // while(1);
  }

  radio.setChannel(5);
  radio.setDataRate(RF24_1MBPS);
  radio.setPALevel(RF24_PA_HIGH);
  radio.openWritingPipe(0x7878787878LL);
  
  /* Display some basic information on this sensor */
  // displaySensorDetails();
}

void loop(void) 
{
  /* Get a new sensor event */ 
  sensors_event_t event; 
  mag.getEvent(&event);
 
  /* Display the results (magnetic vector values are in micro-Tesla (uT)) */
  //Serial.print("X: ");
  Serial.print(event.magnetic.x); Serial.print(";");
  //Serial.print("Y: ");
  Serial.print(event.magnetic.y); Serial.print(";");
  //Serial.print("Z: ");
  Serial.print(event.magnetic.z); Serial.println(";");//Serial.println("uT");

  data[0] = event.magnetic.x;
  data[1] = event.magnetic.y;
  data[2] = event.magnetic.z;
 
  radio.write(&data, sizeof(data));
 
  delay(100);
}


void displaySensorDetails(void)
{
  sensor_t sensor;
  mag.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" uT");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" uT");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" uT");  
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);
}

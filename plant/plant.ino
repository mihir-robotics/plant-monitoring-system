/*Project Date: 16th November 2021

  Project Summary:
  - The following program takes multiple sensor inputs to automate watering the plant.
  - The inputs are Soil Moisture, Temperature, Humidity and Sunlight.

  List of User-defined Functions:
  - checkMoisture()
  - checkValve()
  - setValveTime()
  - getSensorValues()
  - printSerial()

*/

// Adafruit library used to interface with DHT 11 Temperature and Humidity Sensor
#include <Adafruit_Sensor.h>                

// Specific library for DHT sensors
#include <DHT.h>                            

// Light Sensor (Photo-resistor) to be connected to Analog 1
#define LIGHT_SENSOR    A1

// Soil Moisture sensor to be connected to Analog 3
#define MOISTURE_SENSOR A3                  

// DHT Sensor to be connected to Digital Pin 3
#define DHT_SENSOR      3                        

// Relay (for controlling pump) connected to Digital Pin 2
#define RELAY   2                             

// Button for closing valve (override)
#define BUTTON  4                            

// Defining which type of DHT sensor in use
#define DHTTYPE DHT11                         

// Initialise DHT for 16mhz Arduino
DHT dht_sensor = DHT(DHT_SENSOR, DHTTYPE);


// Seperate sensor values 
int temperature, humidity, moisture, light, buttonstatus;     

// Time for which the valve opens, determined by humidity
int valveTime;					    

// Moisture level at which pump will turn on    
int moistureThreshold = 700;   

// State of solenoid valve
bool valveState;

// Current state of valve
bool currentState = LOW;              

// Used for multithreading
unsigned long prevTime = millis();          

// Setup Function
void setup()
{
  // Defining Light sensor pin as INPUT
    pinMode(LIGHT_SENSOR, INPUT);
  // Defining Relay pin as OUTPUT
    pinMode(RELAY, OUTPUT);         
  // Defining the soil moisture pin as INPUT    
    pinMode(MOISTURE_SENSOR, INPUT);    
  // Defining the button as OUTPUT
    pinMode(BUTTON, OUTPUT);            
  // begin() used to begin communication with DHT sensor
    dht_sensor.begin();                 
  // Serial communication started with Baud Rate = 9600  
    Serial.begin(9600);                 
}

// Function to check Soil Moisture
void checkMoisture(int moist)           
{ 
  // Check if soil moisture is low; if low, turn on valve   
    if (moist > moistureThreshold)
    {
        valveState = HIGH;
    }
}

// Function to check the Valve state and change it if necessary
void checkValve(unsigned long elapsedTime)   
{
  // Check if the valve needs to be opened, do so if required
    static unsigned long timeCounter = 0;
    if (valveState == HIGH)
    {
        currentState = HIGH;
        digitalWrite(RELAY, currentState);
        timeCounter += elapsedTime;
        if (timeCounter >= valveTime)
        {
            currentState = LOW;
            digitalWrite(RELAY, currentState);
            timeCounter -= valveTime;
            valveState = LOW;
        }
    }
}

// Function to set the amount of time for which Valve is open
void setValveTime(int humidity) 
{
  // Check humidity level and map the values to time to keep valve open
    valveTime = map(humidity, 1, 100, 15000, 5000);
}

// Function to Open the Valve by using Override Button
void buttonPressed()            
{
  // Open the valve if the button is pressed
    if (digitalRead(BUTTON))
    {
        digitalWrite(RELAY, HIGH);
        valveState = HIGH;
    }
    else
    {
        digitalWrite(RELAY, LOW);
        valveState = LOW;
    }
}

// Function to obtain the sensor values
void getSensorValues()
{
  // Get temperature in Celsius
    temperature = dht_sensor.readTemperature();                   
  // Get relative humidity percent (% in air)
    humidity = dht_sensor.readHumidity();                         
  // Get reading from Light sensor
    light = analogRead(LIGHT_SENSOR);

  // Obtain Soil moisture level (Dry Soil -> Higher value, Wet Soil -> Lower value)
  // For Reference: Sensor Reading in open air ~ 800 , Sensor Reading in freshly watered soil ~ 200 - 265                   
    moisture = analogRead(MOISTURE_SENSOR);                      
                                                              
}

// Function to print the Sensor Values and Valve status on Serial Monitor
void printSerial(unsigned long elapsedTime)     
{
  
    static unsigned long timeCounter = 0;
    int printInterval = 1000;

    timeCounter += elapsedTime;
    if (timeCounter >= printInterval)
    { 
      // Print sensor values to the serial monitor in readable format
        Serial.print("Temperature: ");        
        Serial.print(temperature);
        Serial.print(" | Humidity: ");
        Serial.print(humidity);
        Serial.print(" | Moisture: ");
        Serial.print(moisture);
        Serial.print(" | Light: ");
        Serial.print(light);
        Serial.print(" | Valve State: ");
        Serial.print(valveState ? "Open" : "Closed");
        Serial.print(" | Valve Time: ");
        Serial.print(valveTime / 1000);
        Serial.println(" sec");
        timeCounter -= printInterval;
    }
}

// Loop Function (main)
void loop()
{ 
    unsigned long elapsedTime = millis() - prevTime;
    prevTime = prevTime + elapsedTime;

    getSensorValues();
    setValveTime(humidity);
    checkMoisture(moisture);

    checkValve(elapsedTime);
    printSerial(elapsedTime);
}

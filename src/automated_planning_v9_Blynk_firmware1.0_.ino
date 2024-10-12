
// Fill-in information from your Blynk Template here
#define BLYNK_TEMPLATE_ID           "YOUR_TEMPLATE_ID"
#define BLYNK_DEVICE_NAME           "YOUR_DEVICE_NAME"
#define BLYNK_AUTH_TOKEN            "YOUR_AUTH_TOKEN"
#define BLYNK_FIRMWARE_VERSION      "YOUR_BLYNK_FIRMWARE_VERSION"
#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG
#define APP_DEBUG

#include "secrets.h"
#include <DHT.h>

// Uncomment your board, or configure a custom board in Settings.h
//#define USE_SPARKFUN_BLYNK_BOARD
#define USE_NODE_MCU_BOARD
//#define USE_WITTY_CLOUD_BOARD
//#define USE_WEMOS_D1_MINI

#include "BlynkEdgent.h"
// Code settings:
//#define USE_DHT11_SENSOR                         // Comment this out if you are not using a 1-Wire temperature sensor (DS18B20)
#define USE_SOIL_MOISTURE_SENSOR                // Comment this out if you are not using an analog (3 wire) soil moisture sensor

// Physical settings:
#define PUMP_PIN                        D6       // Physical pin where you connected the pump relay/driver
#define PUMP_LOGIC_INVERTED             false   // Select true if the pump turns ON when the output is low (0V)
#define DHT11_SENSOR_PIN                D5      // Physical pin where you connected the 1-Wire temperature sensor
#define MOISTURE_SENSOR_POWER_PIN       A0       // Physical ping where you connect the soil moisture sensor power (+) pin. Analog is connected to AN0

// Blynk App settings:
#define BLYNK_APP_PUMPTIMERVALUE_VPIN   V0      // Pump timer value (in seconds) tied to a slider (values: from 1 to a reasonable number like 7)
#define BLYNK_APP_PUMPONTIMER_VPIN      V1      // Push button to turn pump ON for the timer value above (values: 0 or 1)
#define BLYNK_APP_PUMPONOFF_VPIN        V2      // ON/OFF button (or timer/scheduler) to turn pump ON/OFF manually (values: 0 or 1)
#define BLYNK_APP_PUMPTIMERSTATUS_VPIN  V3      // Status (duration set) of the pump timer (in seconds): (values: from 1 to a reasonable number)
#define BLYNK_APP_PUMPSTATUS_VPIN       V4      // Status of the pump every time its status changes (values: "OFF" or "ON"). In the app the reading frequency should be set to PUSH
#define BLYNK_APP_TEMP_VPIN             V5      // Value for temperature sensor. In the app the reading frequency should be set to PUSH
#define BLYNK_APP_SOILMOISTURE_VPIN     V6      // Value for the soil humidity. In the app the reading frequency should be set to PUSH
#define BLYNK_APP_HUM_VPIN              V7      // Value for Humidity Sensor
#define Time_Passes_VPIN                V8      //The Time passes from Arduino
#define Analog_Value_VPIN               V10     //analog value for soil moistrute sensor
// You should get Auth Token in the Blynk App. Go to the Project Settings (nut icon).
char auth[] = BLYNK_AUTH_TOKEN;

// Your WiFi credentials. Set password to "" for open networks.
char ssid[] = "YOUR_SSID";
char pass[] = "YOUR_PASSWORD";
// ==================== EDIT YOUR SETTINGS TILL HERE ====================

//#ifdef USE_DHT11_SENSOR
#define DHTTYPE DHT11 
DHT dht(DHT11_SENSOR_PIN , DHTTYPE);
//#endif
BlynkTimer      timerSystem;
bool            readSensors_flag = false;
int             pumpOnTimeDuration = 1;
int             pumpOnTimer_numTimer = -1;
int             soil_moistr = 0;
int             soilMoistrure = 0; //analog value
const unsigned  reading_count = 10;
unsigned int    analogVals[reading_count];
unsigned int    values_avg = 0;
long now = millis();
long lastMeasure = 0;
int SerialValue = 0;
int fadeValue = 0 ;
// Create a delay function compatible with Blynk and timers:
void Blynk_Delay(int milli) {
    int end_time = millis() + milli;
    while(millis() < end_time) {
        Blynk.run();
        Blynk.run();
        timerSystem.run();
        yield();
    }
}

// Called to control the power of the pump relay/driver.
// Reports back to Blynk app as well:
void setPumpPower(bool power = false) {
    if (power) {
        Serial.println("Pump powered ON");
        Blynk.virtualWrite(BLYNK_APP_PUMPSTATUS_VPIN, "ON");
        if (PUMP_LOGIC_INVERTED) analogWrite(PUMP_PIN, 0);
        else analogWrite(PUMP_PIN, 1024);
        Serial.println("pump status 0-100 ON");
        Serial.println(PUMP_PIN);
    }
    else {
        Serial.println("Pump powered OFF ");
        Blynk.virtualWrite(BLYNK_APP_PUMPSTATUS_VPIN, "OFF");
        if (PUMP_LOGIC_INVERTED) analogWrite(PUMP_PIN, 1024);
        else analogWrite(PUMP_PIN, 0);
        Serial.println("pump status 0-100 OFF ");
        Serial.println(PUMP_PIN);
    }
}

void setPumpTimerDuration(int duration) {
    if (duration < 1) duration = 1;
    pumpOnTimeDuration = duration;
    Blynk.virtualWrite(BLYNK_APP_PUMPTIMERSTATUS_VPIN, pumpOnTimeDuration);
    Serial.printf("Pump duration set to %u seconds\r\n", pumpOnTimeDuration);
}

void pumpTimerTimeout() {
    Serial.println("Timer expired");
    timerSystem.disable(pumpOnTimer_numTimer);
    timerSystem.deleteTimer(pumpOnTimer_numTimer);
    pumpOnTimer_numTimer = -1;
    setPumpPower(false);
}

// Called every time we change the duration time of the water pump
// through the app (for example can be tied to a slider):
BLYNK_WRITE(BLYNK_APP_PUMPTIMERVALUE_VPIN) {
    setPumpTimerDuration(param.asInt());
}

// Called every time we press the push button to turn the pump ON
// for the duration of time specified by 'pumpOnTimeDuration':
BLYNK_WRITE(BLYNK_APP_PUMPONTIMER_VPIN) {
    if (param.asInt() == 1) {
        if (pumpOnTimer_numTimer < 0) {
            Serial.printf("Starting timer of %u seconds \r\n", pumpOnTimeDuration);
            setPumpPower(true);
            pumpOnTimer_numTimer = timerSystem.setTimeout(pumpOnTimeDuration*1000L, pumpTimerTimeout);
        }
        else {
            Serial.println("Pump was already on a timer. Ending timer... ");
            pumpTimerTimeout();
        }
    }
}

// Called every time we press the ON/OFF button to turn the pump ON or OFF:
BLYNK_WRITE(BLYNK_APP_PUMPONOFF_VPIN) {
    if (param.asInt() == 0) setPumpPower(false);
    else setPumpPower(true);
}

// Called to measure and average the soil moisture level:
#ifdef USE_SOIL_MOISTURE_SENSOR
int getSoilMoisture() {
    // Turn the sensor ON:
    digitalWrite(MOISTURE_SENSOR_POWER_PIN, HIGH);
    Blynk_Delay(1000);
    
    for (int counter = 0; counter < reading_count; counter++) {
        analogVals[reading_count] = analogRead(A0);
        Blynk_Delay(100);
        values_avg = (values_avg + analogVals[reading_count]);
        
    }
    values_avg = values_avg/reading_count;
    


    // Turn the sensor OFF:
    digitalWrite(MOISTURE_SENSOR_POWER_PIN, LOW);
    return values_avg;
}
#endif

    // ==================== TIME ====================

 void getReadableTime(String &readableTime) {
  unsigned long currentMillis;
  unsigned long seconds;
  unsigned long minutes;
  unsigned long hours;
  unsigned long days;

  currentMillis = millis();
  seconds = currentMillis / 1000;
  minutes = seconds / 60;
  hours = minutes / 60;
  days = hours / 24;
  currentMillis %= 1000;
  seconds %= 60;
  minutes %= 60;
  hours %= 24;
  //String Time = (String(days) + ":" + String(hours) + ":" + String(minutes) + ":" + String(seconds) + ":"  + String(currentMillis));  //Converts to DD:HH:MM:SS:mm string. This can be returned to the calling function.
  //Serial.print("Time Passes: ");Serial.println (Time);
  if (days > 0) {
    readableTime = String(days) + " ";
  }

  if (hours > 0) {
    readableTime += String(hours) + ":";
  }

  if (minutes < 10) {
    readableTime += "0";
  }
  readableTime += String(minutes) + ":";

  if (seconds < 10) {
    readableTime += "0";
  }
  readableTime += String(seconds);
  Blynk.virtualWrite(Time_Passes_VPIN, readableTime);
 }

// Called every time the timer to read sensors expires:
void readSensors() {
    readSensors_flag = true;
    }

void setup()
{

  BlynkEdgent.begin();
  // Debug console
    Serial.begin(115200);
    dht.begin();
    Serial.println("Preparing...");
    
    // Set pins:
    analogWrite(PUMP_PIN, 0);
    pinMode(PUMP_PIN, OUTPUT);
    if (PUMP_LOGIC_INVERTED) analogWrite(PUMP_PIN, 1024);
    else analogWrite(PUMP_PIN, 0);
    
    
    #ifdef USE_SOIL_MOISTURE_SENSOR
    pinMode(MOISTURE_SENSOR_POWER_PIN, OUTPUT);
    digitalWrite(MOISTURE_SENSOR_POWER_PIN, LOW);
    #endif

    // Start connection to WiFi and Blynk system:
    Blynk.begin(auth, ssid, pass);
    
    // Set initial status, which will also report to Blynk:
    setPumpTimerDuration(1);
    setPumpPower(false);

    // Read the sensors every 60 seconds:
    timerSystem.setInterval(60000L, readSensors);
    
    Serial.println("Ready...");
}



void loop() {
  BlynkEdgent.run();
  timerSystem.run();
//      Serial.println("------------------------");
//  Serial.println("1.)Restart ESP");
//  Serial.println("2.)Set water pump speed flow for 3 seconds");
//  Serial.println("------------------------");
  while (Serial.available() > 0) {
    SerialValue =Serial.parseInt();
    Serial.println("SerialValue");
    Serial.println(SerialValue);
    if (Serial.read() !='0') {
      if (SerialValue !=NULL) {       //call SWITCH if input is available

        switch (SerialValue) {            

          case 1:
               Serial.println("ESP is restarted");
               delay(2000);
               ESP.restart();
                
               break;
               
          case 2:
            Serial.println("Give % of water pump speed you want from 0 to 100");
            Serial.println("It will flows for 3 seconds");
            while (!Serial.available()) {
              ;
            }
            SerialValue =Serial.parseInt();
            fadeValue = SerialValue;
            Serial.println("Water pump speed is set to : ");
            fadeValue = map(fadeValue,0,100,0,1024);
            Serial.print(fadeValue);
            Serial.println("%");
            analogWrite(PUMP_PIN, fadeValue);
            Blynk_Delay(3000);
            analogWrite(PUMP_PIN, 0);
               break;
               
          default:
            Serial.println("!WRONG INPUT!");
               break;
            
            }
        
      }
    }
  }
    // Read sensors here as soon as the flag is set:
    // Reads all the sensors and reports back to Blynk App
    if (readSensors_flag) {
        float temperature = dht.readTemperature();
        float humidity = dht.readHumidity();
        if (isnan(humidity) || isnan(temperature)) {
             Serial.println("Failed to read from DHT sensor!");
             return;
         }
        Serial.print("Read temperature: ");
        Serial.println(temperature);
        Blynk.virtualWrite(BLYNK_APP_TEMP_VPIN, temperature);
       
        Serial.print("Read Humidity: ");
        Serial.println(humidity);
        Blynk.virtualWrite(BLYNK_APP_HUM_VPIN, humidity);
        String NTime;
        getReadableTime(NTime);
        Serial.println(F("------------------------"));
        Serial.println("Time Passes!");
        Serial.println(NTime);
        #ifdef USE_SOIL_MOISTURE_SENSOR
        int soilMoisture = getSoilMoisture();
        // You must calculate your Soil Moisture Sensor
        //511 ->0  //395->100 %  //in plant 481 %
        soil_moistr = map(soilMoisture, 511, 395, 0, 100);
        Serial.print("Read soil moisture: ");
        Serial.print(soil_moistr);
        Serial.println(" % ");
        Serial.print("Analog Read moisture: ");
        Serial.println(soilMoisture);
        Blynk.virtualWrite(BLYNK_APP_SOILMOISTURE_VPIN, soil_moistr);
        Blynk.virtualWrite(Analog_Value_VPIN, soilMoistrure);
        #endif     
        
        readSensors_flag = false;
        
    }


    //---------------------  
    //Start the watering if humidity decrease down of 60%
    //Check every 30 minutes
  if (now - lastMeasure > 30*60*1000) {
    if (soil_moistr < 60){
      lastMeasure = now;
      analogWrite(PUMP_PIN, 512);
      Blynk_Delay(3000);  //3 seconds 
      analogWrite(PUMP_PIN, 0);  
    }
    else {
      analogWrite(PUMP_PIN, 0);
   }
  } 
} 

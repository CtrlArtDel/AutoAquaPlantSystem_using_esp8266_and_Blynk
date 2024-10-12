# AutoAquaPlant System using ESP8266 and Blynk

## Overview

**AutoAquaPlant** is an automated plant care system that leverages the ESP8266 microcontroller and Blynk app to monitor soil moisture and control watering. This system ensures your plants receive the right amount of water by providing real-time updates and remote management through the Blynk platform.

## Features

- **Automated Watering**: The system waters plants automatically based on soil moisture levels.
- **Real-Time Monitoring**: Provides real-time updates on soil moisture, temperature, and humidity.
- **Remote Control**: Manage watering schedules and view sensor data from anywhere using the Blynk app.
- **ESP8266 Integration**: Utilizes the ESP8266 microcontroller for wireless connectivity and automation.

## Components

- **ESP8266 Microcontroller**: The brain of the system, handling sensor data and communication with Blynk.
- **Soil Moisture Sensor**: Measures the moisture level in the soil to determine when watering is needed.
- **DHT11 Sensor**: Monitors temperature and humidity to provide additional environmental data.
- **Relay/Pump**: Controls the watering mechanism based on the sensor readings.

## Setup

1. **Clone the Repository**
   ```bash
   git clone https://github.com/CtrlArtDel/AutoAquaPlant_System_using_esp8266_and_Blynk
   cd autoaquaplant

2. **Install Required Libraries**

- **[Blynk Library](https://github.com/blynkkk/blynk-library)**
- **[DHT Sensor Library](https://github.com/adafruit/DHT-sensor-library)**

3.   ## Configure Credentials

Create a `secrets.h` file in the root directory of the project with your Blynk Auth Token, Wi-Fi SSID, and password.

```cpp
// secrets.h
#define BLYNK_TEMPLATE_ID           "YOUR_TEMPLATE_ID"
#define BLYNK_DEVICE_NAME           "YOUR_DEVICE_NAME"
#define BLYNK_AUTH_TOKEN            "YOUR_AUTH_TOKEN"
#define BLYNK_FIRMWARE_VERSION      "YOUR_BLYNK_FIRMWARE_VERSION"
#define WIFI_SSID                   "YOUR_SSID"
#define WIFI_PASS                   "YOUR_PASSWORD"

```

4.   ## Upload to ESP8266
   
-    **Open** the project in the Arduino IDE.
-    **Select** the appropriate board and port.
-    **Upload** the code to your ESP8266.

5.  ## Set Up Blynk App

    **Create** a new project in the Blynk app.
    **Add** widgets for soil moisture, temperature, humidity, and control buttons.
    **Link** the widgets to the virtual pins defined in the code.

  ## Usage

- **Monitoring**: View soil moisture, temperature, and humidity data in real-time on the Blynk app.
- **Control**: Adjust watering schedules and manually control the pump from the app.


## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Contributing

Feel free to submit issues or pull requests. Contributions are welcome!


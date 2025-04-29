#include <HardwareSerial.h>
#include "ESPNOWHandler.h"
#include "SensorData.h"

// Define UART1 for serial communication
HardwareSerial mySerial(1);  // Use UART1 (TX=GPIO4, RX=GPIO5)

// ** Global Variables for Data Storage **  
SensorInfo predefinedSensorArray[MAX_SENSORS] = { // **Start with predefined values**
    {63600001, 400, 2000},
    {63600002, 600, 2000},
    {63600003, 550, 2000},
    {63600004, 650, 2000},
    {63600005, 700, 2000}
};
int predefinedSensorCount = 5;  // **Start with predefined sensors**

ReceivedData receivedSensorArray[MAX_SENSORS];  // Stores received box count data, now with new fields
int receivedSensorCount = 0;

int rackLength = 2000; // Default value before receiving

// ** Buffer Sizes (Calculated Dynamically) **  
const int SENSOR_BUFFER_SIZE = sizeof(ReceivedData) * MAX_SENSORS;
const int SENSOR_ENTRY_SIZE = sizeof(SensorInfo);  // Size of a single sensor struct

// Timer variables for non-blocking execution
unsigned long lastSendTime = 0;
unsigned long lastPrintTime = 0;
const int sendInterval = 2000;  // Send received sensor data every 2 seconds
const int printInterval = 2000; // Print sensor arrays every 2 seconds

void setup() {
    Serial.begin(115200);  // Monitor output
    mySerial.begin(9600, SERIAL_8N1, 4, 5);  // TX=GPIO4, RX=GPIO5

    Serial.println("ESP32 Receiver Ready!");

    // Initialize ESP-NOW
    initESPNow();

    // **Print predefined sensors at startup**
    Serial.println("Predefined sensors added:");
    for (int i = 0; i < predefinedSensorCount; i++) {
        Serial.print("SensorID: ");
        Serial.print(predefinedSensorArray[i].sensorid);
        Serial.print(", BoxSize: ");
        Serial.print(predefinedSensorArray[i].boxsize);
        Serial.print(", RackLength: ");
        Serial.println(predefinedSensorArray[i].racklength);
    }
}

void loop() {
    // ** Check if new sensor data is received (from sender ESP32) **
    if (mySerial.available() >= SENSOR_ENTRY_SIZE) {
        int availableBytes = mySerial.available();

        // ** Calculate the number of received sensors dynamically **
        int receivedCount = availableBytes / SENSOR_ENTRY_SIZE;

        // ** Limit the received count to avoid buffer overflow **
        int newSensorCount = (receivedCount > MAX_SENSORS) ? MAX_SENSORS : receivedCount;

        // ** Read the exact number of received sensors **
        SensorInfo receivedData[MAX_SENSORS];  // Temporary buffer to hold received data
        mySerial.readBytes((char *)&receivedData, newSensorCount * SENSOR_ENTRY_SIZE);

        // ** Merge received data with predefined values**
        for (int i = 0; i < newSensorCount; i++) {
            bool found = false;
            for (int j = 0; j < predefinedSensorCount; j++) {
                if (predefinedSensorArray[j].sensorid == receivedData[i].sensorid) {
                    // **Update only existing sensor values**
                    predefinedSensorArray[j].boxsize = receivedData[i].boxsize;
                    predefinedSensorArray[j].racklength = receivedData[i].racklength;
                    found = true;
                    break;
                }
            }
            if (!found && predefinedSensorCount < MAX_SENSORS) {
                // **Add new sensor if it doesn't exist**
                predefinedSensorArray[predefinedSensorCount] = receivedData[i];
                predefinedSensorCount++;
            }
        }

        // ** Extract RackLength ONLY ONCE from the first valid entry **
        rackLength = predefinedSensorArray[0].racklength;

        Serial.print("Received & Updated Sensor Data (");
        Serial.print(newSensorCount);
        Serial.println(" sensors)");
    }

    // ** Print sensor arrays and rack length every 2 seconds **
    if (millis() - lastPrintTime >= printInterval) {
        lastPrintTime = millis();  // Update timer

        Serial.println("\n--- Updated Predefined Sensor Array ---");
        for (int i = 0; i < predefinedSensorCount; i++) {
            Serial.print("SensorID: ");
            Serial.print(predefinedSensorArray[i].sensorid);
            Serial.print(", BoxSize: ");
            Serial.print(predefinedSensorArray[i].boxsize);
            Serial.print(", RackLength: ");
            Serial.println(predefinedSensorArray[i].racklength);
        }

        Serial.println("\n--- Received Sensor Data (Box Count) ---");
        for (int i = 0; i < receivedSensorCount; i++) {
            Serial.print("SensorID: ");
            Serial.print(receivedSensorArray[i].sensorid);
            Serial.print(", BoxCount: ");
            Serial.print(receivedSensorArray[i].boxcount);
            //// NEW: Print the additional fields
            Serial.print(", AutoMode: ");
            Serial.print(receivedSensorArray[i].auto_mode ? "true" : "false");
            Serial.print(", AutoBoxSize: ");
            Serial.println(receivedSensorArray[i].auto_box_size);
        }
    }

    // ** Send receivedSensorArray back to sender ESP32 every 2 seconds **
    if (millis() - lastSendTime >= sendInterval) {
        lastSendTime = millis();  // Update timer
        
        if (receivedSensorCount > 0) {
            mySerial.write((uint8_t *)&receivedSensorArray, receivedSensorCount * sizeof(ReceivedData));
            Serial.println("Sent Received Sensor Data via Serial");

            // Debug output with new fields
            for (int i = 0; i < receivedSensorCount; i++) {
                Serial.print("Sent -> SensorID: ");
                Serial.print(receivedSensorArray[i].sensorid);
                Serial.print(", BoxCount: ");
                Serial.print(receivedSensorArray[i].boxcount);
                Serial.print(", AutoMode: ");
                Serial.print(receivedSensorArray[i].auto_mode ? "true" : "false");
                Serial.print(", AutoBoxSize: ");
                Serial.println(receivedSensorArray[i].auto_box_size);
            }
        }
    }
}

#include "SensorData.h"

// Function to find a sensor in received data
int findReceivedSensorIndex(long sensorid) {
    for (int i = 0; i < receivedSensorCount; i++) {
        if (receivedSensorArray[i].sensorid == sensorid) {
            return i;
        }
    }
    return -1;
}

// Function to find a sensor in predefined sensor data
int findPredefinedSensorIndex(long sensorid) {
    for (int i = 0; i < predefinedSensorCount; i++) {
        if (predefinedSensorArray[i].sensorid == sensorid) {
            return i; // Return the index if found
        }
    }
    return -1; // Return -1 if not found
}

#line 1 "C:\\Users\\kavee\\Desktop\\MECH3890\\VSCODE\\reciever\\SensorData.h"
#ifndef SENSORDATA_H
#define SENSORDATA_H

// Data structure for predefined sensor information
typedef struct __attribute__((packed)) {
    long sensorid;
    int boxsize;
    int racklength;  // Ensure this is the last field for clarity
} SensorInfo;

// Updated ReceivedData structure: now stores additional fields from the sender
typedef struct {
    long sensorid;
    int boxcount;
    bool auto_mode;      // NEW: Whether the sender is in auto mode
    int auto_box_size;   // NEW: The auto-calculated box size from the sender
} ReceivedData;

// Updated QueryData structure: now includes the extra fields
typedef struct {
    long sensorid;
    int boxcount;
    bool auto_mode;      // NEW: Indicates if the sender is using auto box calculation
    int auto_box_size;   // NEW: The auto-calculated box size from the sender
} QueryData;

// Data structure for response data sent back to the sender
typedef struct {
    long sensorid;
    int boxsize;
    int racklength;
} ResponseData;

#define MAX_SENSORS 20

// Global arrays (extern declarations; defined in your main receiver sketch)
extern SensorInfo predefinedSensorArray[MAX_SENSORS];
extern int predefinedSensorCount;
extern int rackLength;
extern ReceivedData receivedSensorArray[MAX_SENSORS];
extern int receivedSensorCount;

// Function prototypes for sensor lookup
int findReceivedSensorIndex(long sensorid);
int findPredefinedSensorIndex(long sensorid);

#endif


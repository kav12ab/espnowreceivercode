#line 1 "C:\\Users\\kavee\\Desktop\\MECH3890\\VSCODE\\reciever\\ESPNOWHandler.cpp"
#include "ESPNOWHandler.h"
#include "SensorData.h"  // Contains our updated QueryData definition

// Use extern to reference global variables from your main receiver sketch
extern int rackLength;
extern SensorInfo predefinedSensorArray[MAX_SENSORS];
extern int predefinedSensorCount;
extern ReceivedData receivedSensorArray[MAX_SENSORS];
extern int receivedSensorCount;

// Initialize ESP-NOW on the receiver side
void initESPNow() {
    WiFi.mode(WIFI_STA);
    Serial.println("WiFi initialized in Station mode");

    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW initialization failed");
        return;
    }
    Serial.println("ESP-NOW initialized");

    // Register receive callback
    esp_now_register_recv_cb(onReceive);
}

// Function to dynamically add a sender as a peer
void addDynamicPeer(const uint8_t *mac_addr) {
    if (!esp_now_is_peer_exist(mac_addr)) {
        esp_now_peer_info_t peerInfo;
        memset(&peerInfo, 0, sizeof(esp_now_peer_info_t));
        memcpy(peerInfo.peer_addr, mac_addr, 6);
        peerInfo.channel = 0;
        peerInfo.encrypt = false;
        peerInfo.ifidx = WIFI_IF_STA;

        if (esp_now_add_peer(&peerInfo) != ESP_OK) {
            Serial.println("Failed to add sender as a peer");
        } else {
            Serial.print("Sender added as a peer: ");
            for (int i = 0; i < 6; i++) {
                Serial.printf("%02X", mac_addr[i]);
                if (i < 5) Serial.print(":");
            }
            Serial.println();
        }
    }
}

// Function to send response back to the sender
void sendResponse(const uint8_t *mac_addr, int index, long sensorid) {
    ResponseData response;

    if (index != -1) {
        response.sensorid = predefinedSensorArray[index].sensorid;
        response.boxsize = predefinedSensorArray[index].boxsize;
        response.racklength = rackLength;
    } else {
        response.sensorid = sensorid;
        response.boxsize = -1; // auto box calculation mode
        response.racklength = rackLength;
    }

    esp_err_t result = esp_now_send(mac_addr, (uint8_t *)&response, sizeof(response));

    if (result == ESP_OK) {
        Serial.println("Response sent successfully");
        Serial.print("Response struct - SensorID: ");
        Serial.print(response.sensorid);
        Serial.print(", BoxSize: ");
        Serial.print(response.boxsize);
        Serial.print(", RackLength: ");
        Serial.println(response.racklength);
    } else {
        Serial.print("Error sending response: ");
        Serial.println(result);
    }
}

// Callback function for ESP-NOW reception
void onReceive(const esp_now_recv_info_t *recv_info, const uint8_t *data, int data_len) {
    const uint8_t *mac_addr = recv_info->src_addr;

    Serial.print("Received data from MAC: ");
    for (int i = 0; i < 6; i++) {
        Serial.printf("%02X", mac_addr[i]);
        if (i < 5) Serial.print(":");
    }
    Serial.println();

    addDynamicPeer(mac_addr);

    // Use the updated QueryData struct (which now includes auto_mode and auto_box_size)
    QueryData queryData;
    if (data_len == sizeof(QueryData)) {
        memcpy(&queryData, data, sizeof(QueryData));
        Serial.print("Received query - SensorID: ");
        Serial.print(queryData.sensorid);
        Serial.print(", BoxCount: ");
        Serial.print(queryData.boxcount);
        Serial.print(", Auto Mode: ");
        Serial.print(queryData.auto_mode ? "true" : "false");   //// NEW: Print auto_mode
        Serial.print(", Auto Box Size: ");
        Serial.println(queryData.auto_box_size);                //// NEW: Print auto_box_size

        int index = findReceivedSensorIndex(queryData.sensorid);
        if (index != -1) {
            // Update existing sensor data including new fields
            receivedSensorArray[index].boxcount = queryData.boxcount;
            receivedSensorArray[index].auto_mode = queryData.auto_mode;          //// NEW: Update auto_mode
            receivedSensorArray[index].auto_box_size = queryData.auto_box_size;      //// NEW: Update auto_box_size

            Serial.print("Updated SensorID: ");
            Serial.print(receivedSensorArray[index].sensorid);
            Serial.print(", New BoxCount: ");
            Serial.print(receivedSensorArray[index].boxcount);
            Serial.print(", Auto Mode: ");
            Serial.print(receivedSensorArray[index].auto_mode ? "true" : "false");  //// NEW
            Serial.print(", Auto Box Size: ");
            Serial.println(receivedSensorArray[index].auto_box_size);             //// NEW
        } else if (receivedSensorCount < MAX_SENSORS) {
            // Add new sensor entry with new fields
            receivedSensorArray[receivedSensorCount].sensorid = queryData.sensorid;
            receivedSensorArray[receivedSensorCount].boxcount = queryData.boxcount;
            receivedSensorArray[receivedSensorCount].auto_mode = queryData.auto_mode;          //// NEW
            receivedSensorArray[receivedSensorCount].auto_box_size = queryData.auto_box_size;      //// NEW

            Serial.print("Added new SensorID: ");
            Serial.print(receivedSensorArray[receivedSensorCount].sensorid);
            Serial.print(", BoxCount: ");
            Serial.print(receivedSensorArray[receivedSensorCount].boxcount);
            Serial.print(", Auto Mode: ");
            Serial.print(receivedSensorArray[receivedSensorCount].auto_mode ? "true" : "false");  //// NEW
            Serial.print(", Auto Box Size: ");
            Serial.println(receivedSensorArray[receivedSensorCount].auto_box_size);             //// NEW
            receivedSensorCount++;
        } else {
            Serial.println("Sensor array is full. Cannot add new sensor.");
        }

        int predefinedIndex = findPredefinedSensorIndex(queryData.sensorid);
        sendResponse(mac_addr, predefinedIndex, queryData.sensorid);
    } else {
        Serial.println("Received data size mismatch");
    }
}

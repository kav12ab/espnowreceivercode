#ifndef ESPNOWHANDLER_H
#define ESPNOWHANDLER_H

#include <esp_now.h>
#include <WiFi.h>
#include "SensorData.h"

// Function prototypes
void initESPNow();
void onReceive(const esp_now_recv_info_t *recv_info, const uint8_t *data, int data_len);
void sendResponse(const uint8_t *mac_addr, int index, long sensorid);
void addDynamicPeer(const uint8_t *mac_addr);

#endif

#ifndef dashioBLEgap_H
#define dashioBLEgap_H

#pragma once

#if !defined(CONFIG_IDF_TARGET_ESP32) && !defined(CONFIG_IDF_TARGET_ESP32S2)

#include <nvs_flash.h>

#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"

#include "dashioBLEserver.h"
#include "dash_log.h"

#include "esp_bt.h" //???


#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BLE_NOT_AUTH,
    BLE_AUTH_STARTED,
    BLE_AUTH_REQ_CONN,
    BLE_AUTH_FAIL,
    BLE_AUTHENTICATED
} BLEauthState;

struct BLEclientHolder {
    uint16_t connectionHandle;
    uint16_t mtu;
    bool active;
    char *msg;
    uint16_t messageLength;
    BLEauthState authState;
};

extern uint8_t maxBLEclients;
extern uint16_t maxBLEmessageSize;
extern struct BLEclientHolder *bleClients;
struct BLEclientHolder *getConnection(uint16_t conn_handle);

void ble_init(dash_message_received_fn *_on_message_received, const char *dashType);
void ble_begin();
void ble_secure(uint32_t _passKey);
void ble_start_security(uint16_t connHandle);
struct BLEclientHolder *getConnection(uint16_t conn_handle);
void ble_stop();
static void ble_advertise();
void bleNotifyValue(uint16_t conn_handle, const char *message, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif

#endif

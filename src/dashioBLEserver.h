#ifndef dashBLEsever_H
#define dashBLEsever_H

#pragma once

#if !defined(CONFIG_IDF_TARGET_ESP32) && !defined(CONFIG_IDF_TARGET_ESP32S2)

#include <stdbool.h>

#include "nimble/ble.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#include "dash_log.h"


#ifdef __cplusplus
extern "C" {
#endif

// Bluetooth Light (BLE)
static const ble_uuid128_t dash_svc_uuid = //4fafc201-1fb5-459e-8fcc-c5c9c331914b
    BLE_UUID128_INIT(0x4b, 0x91, 0x31, 0xc3, 0xc9, 0xc5, 0xcc, 0x8f,
                     0x9e, 0x45, 0xb5, 0x1f, 0x01, 0xc2, 0xaf, 0x4f);

static const ble_uuid128_t dash_svr_chr_uuid = // beb5483e-36e1-4688-b7f5-ea07361b26a8
    BLE_UUID128_INIT(0xa8, 0x26, 0x1b, 0x36, 0x07, 0xea, 0xf5, 0xb7,
                     0x88, 0x46, 0xe1, 0x36, 0x3e, 0x48, 0xb5, 0xbe);

struct ble_hs_cfg;
struct ble_gatt_register_ctxt;

extern uint16_t dash_svr_chr_val_handle;

int dash_server_init(void);
void dash_server_gatt_register(struct ble_gatt_register_ctxt *ctxt, void *arg);
int dash_server_send_message(uint16_t conn_handle, const char *message, int length);

typedef void dash_message_received_fn(uint16_t conn_handle, char *message, uint16_t length);
extern dash_message_received_fn *on_dash_server_message_received;

#ifdef __cplusplus
}
#endif

#endif

#endif

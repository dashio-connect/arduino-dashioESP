#include "sdkconfig.h"
#if !defined(CONFIG_IDF_TARGET_ESP32) && !defined(CONFIG_IDF_TARGET_ESP32S2)

#include "dashioBLEgap.h"

static const char *tag = "Dash_BLE_GAP";
static uint8_t own_addr_type;
dash_message_received_fn *on_message_received;

#define EVENT_NOTIFY_DONE_BIT BIT0
EventGroupHandle_t ble_event_group;

uint32_t passKey = 0;

struct BLEclientHolder *bleClients = NULL;
uint8_t maxBLEclients = 1;
uint16_t maxBLEmessageSize = 512;

void initialiseClientHolders() {
    if (bleClients == NULL) {
        bleClients = (struct BLEclientHolder*)malloc(maxBLEclients * sizeof(struct BLEclientHolder));
    }

    if (bleClients != NULL) {
        for (int i = 0; i < maxBLEclients; i++) {
            bleClients[i].connectionHandle = 65535;
            bleClients[i].mtu = 23;
            bleClients[i].active = false;
            bleClients[i].messageLength = 0;
            bleClients[i].msg = malloc(maxBLEmessageSize * sizeof(char));
            bleClients[i].authState = BLE_NOT_AUTH;
        }
    }
}

struct BLEclientHolder *getConnection(uint16_t conn_handle) {
    if (bleClients != NULL) {
        for (int i = 0; i < maxBLEclients; i++) {
            if (bleClients[i].connectionHandle == conn_handle) {
                return &bleClients[i];
            }
        }
    }
    return NULL;
}

void setConnectionMTU(uint16_t conn_handle, uint16_t mtu) {
    if (bleClients != NULL) {
        for (int i = 0; i < maxBLEclients; i++) {
            if (bleClients[i].connectionHandle == conn_handle) {
                bleClients[i].mtu = mtu;
            }
        }
    }
}

bool setConnectionActive(uint16_t conn_handle) {
    bool success = false;
    if (bleClients != NULL) {
        for (int i = 0; i < maxBLEclients; i++) { // Just in case it already exists
            if (bleClients[i].connectionHandle == conn_handle) {
                bleClients[i].active = true;
                success = true;
                break;
            }
        }

        if (!success) { // Find an inactive con holder and use it
            for (int i = 0; i < maxBLEclients; i++) {
                if (bleClients[i].active == false) {
                    bleClients[i].connectionHandle = conn_handle;
                    bleClients[i].mtu = 23;
                    bleClients[i].active = true;
                    bleClients[i].messageLength = 0;
                    success = true;
                    break;
                }
            }
        }
    }
    return success;
}

void setConnectionInactive(uint16_t conn_handle) {
    if (bleClients != NULL) {
        for (int i = 0; i < maxBLEclients; i++) {
            if (bleClients[i].connectionHandle == conn_handle) {
                bleClients[i].connectionHandle = -1;
                bleClients[i].mtu = 23;
                bleClients[i].active = false;
                bleClients[i].messageLength = 0;
                bleClients[i].authState = BLE_NOT_AUTH;
            }
        }
    }
}

void setConnectionAuthState(uint16_t conn_handle, BLEauthState authState) {
    if (bleClients != NULL) {
        for (int i = 0; i < maxBLEclients; i++) {
            if (bleClients[i].connectionHandle == conn_handle) {
                bleClients[i].authState = authState;
            }
        }
    }
}

BLEauthState getConnectionAuthState(uint16_t conn_handle) {
    if (bleClients != NULL) {
        for (int i = 0; i < maxBLEclients; i++) {
            if (bleClients[i].connectionHandle == conn_handle) {
                return bleClients[i].authState;
            }
        }
    }
    return BLE_NOT_AUTH;
}

// ------------------------------------
void ble_store_config_init(void); 

static int ble_gap_event(struct ble_gap_event *event, void *arg);

void printAddress(const void *addr) {
    const uint8_t *u8p = addr;
    DASH_LOGI(tag, "Device Address: %02x:%02x:%02x:%02x:%02x:%02x", u8p[5], u8p[4], u8p[3], u8p[2], u8p[1], u8p[0]);
}

static void ble_advertise(void) {
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    const char *name;
    int rc;

    memset(&fields, 0, sizeof fields);

    // Advertise Discoverable and BLE only (BR/EDR unsupported)
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

    name = ble_svc_gap_device_name();
    fields.name = (uint8_t *)name;
    fields.name_len = strlen(name);
    fields.name_is_complete = 1;

    // 128 byte UUID won't fit in 31 byte advertising packet, so use scan response
    struct ble_hs_adv_fields scan_response_fields;
    memset(&scan_response_fields, 0, sizeof scan_response_fields);
    scan_response_fields.uuids128 = (ble_uuid128_t[]) {dash_svc_uuid};
    scan_response_fields.num_uuids128 = 1;
    scan_response_fields.uuids128_is_complete = 1;
    rc = ble_gap_adv_rsp_set_fields(&scan_response_fields);
    if (rc != 0) {
        DASH_LOGE(tag, "error setting advertisement response data; rc=%d", rc);
        return;
    }

    rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0) {
        DASH_LOGE(tag, "error setting advertisement data; rc=%d", rc);
        return;
    }

    // Begin advertising
    memset(&adv_params, 0, sizeof adv_params);
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND; // Unidirectional connection
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN; // Generally discoverable
    rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_gap_event, NULL);
    if (rc != 0) {
        DASH_LOGE(tag, "error enabling advertisement; rc=%d", rc);
        return;
    }
}

#if MYNEWT_VAL(BLE_POWER_CONTROL)
static void ble_power_control(uint16_t conn_handle) {
    int rc;

    rc = ble_gap_read_remote_transmit_power_level(conn_handle, 0x01 );  // Attempting on LE 1M phy
    assert (rc == 0);

    rc = ble_gap_set_transmit_power_reporting_enable(conn_handle, 0x1, 0x1);
    assert (rc == 0);
}
#endif

/**
 * The nimble host executes this callback when a GAP event occurs.
 * The application associates a GAP event callback with each connection that forms.
 *
 * @param event      The type of event being signalled.
 * @param ctxt       Various information pertaining to the event.
 * @param arg        Application-specified argument; unused by bleprph.
 *
 * @return           0 if the application successfully handled the event; nonzero on failure.
 *                   The semantics of the return code is specific to the particular GAP event being signalled.
 */
static int ble_gap_event(struct ble_gap_event *event, void *arg) {
    struct ble_gap_conn_desc desc;
    int rc;

    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT: // A new connection was established or a connection attempt failed.
        DASH_LOGI(tag, "connection %s; status=%d", event->connect.status == 0 ? "established" : "failed", event->connect.status);
        if (event->connect.status == 0) {
            rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
            assert(rc == 0);

            if (!setConnectionActive(event->connect.conn_handle)) {
                DASH_LOGI(tag, "No connections left %d; status=%d", event->connect.conn_handle, event->connect.status);
            }
        }

        int numActiveBLE = 0;
        for (int i = 0; i < maxBLEclients; i++) {
            if (bleClients[i].active == true) {
                numActiveBLE++;
            }
        }
        if (numActiveBLE < maxBLEclients) {
            ble_advertise();
        }
#if MYNEWT_VAL(BLE_POWER_CONTROL)
	    ble_power_control(event->connect.conn_handle);
#endif
        return 0;

    case BLE_GAP_EVENT_DISCONNECT:
        DASH_LOGI(tag, "disconnect; reason=%d ", event->disconnect.reason);
        setConnectionInactive(event->disconnect.conn.conn_handle);
        ble_advertise();

        return 0;

    case BLE_GAP_EVENT_CONN_UPDATE: // The central has updated the connection parameters.
        DASH_LOGI(tag, "connection updated; status=%d", event->conn_update.status);
        rc = ble_gap_conn_find(event->conn_update.conn_handle, &desc);
        assert(rc == 0);

        return 0;

    case BLE_GAP_EVENT_ADV_COMPLETE: // Doesn't get called because BLE_HS_FOREVER is set in ble_gap_adv_start
        DASH_LOGI(tag, "advertise complete; reason=%d", event->adv_complete.reason);
        ble_advertise();

        return 0;

    case BLE_GAP_EVENT_ENC_CHANGE: // Encryption has been enabled or disabled for this connection.
        rc = ble_gap_conn_find(event->enc_change.conn_handle, &desc);
        if (rc != 0) {
            return BLE_ATT_ERR_INVALID_HANDLE;
        }

        DASH_LOGI(tag, "encryption change event; status=%d", event->enc_change.status);
        if (desc.sec_state.authenticated) {
            DASH_LOGI(tag, "BLE Authenticated");
        } else {
            DASH_LOGI(tag, "BLE Authentication FAIL");
            setConnectionAuthState(desc.conn_handle, BLE_AUTH_FAIL);
        }
        if (desc.sec_state.encrypted) {
            DASH_LOGI(tag, "BLE Encrypted");
        }
        if (desc.sec_state.bonded) {
            DASH_LOGI(tag, "BLE Bonded");
            setConnectionAuthState(desc.conn_handle, BLE_AUTH_REQ_CONN);
        }

        return 0;

    case BLE_GAP_EVENT_NOTIFY_TX:
        DASH_LOGI(tag, "notify_tx event; conn_handle=%d attr_handle=%d "
                    "status=%d is_indication=%d",
                    event->notify_tx.conn_handle,
                    event->notify_tx.attr_handle,
                    event->notify_tx.status,
                    event->notify_tx.indication);

        xEventGroupSetBits(ble_event_group, EVENT_NOTIFY_DONE_BIT);

        return 0;

    case BLE_GAP_EVENT_SUBSCRIBE:
        DASH_LOGI(tag, "subscribe event; conn_handle=%d attr_handle=%d "
                    "reason=%d prevn=%d curn=%d previ=%d curi=%d",
                    event->subscribe.conn_handle,
                    event->subscribe.attr_handle,
                    event->subscribe.reason,
                    event->subscribe.prev_notify,
                    event->subscribe.cur_notify,
                    event->subscribe.prev_indicate,
                    event->subscribe.cur_indicate);

        return 0;

    case BLE_GAP_EVENT_MTU:
        DASH_LOGI(tag, "MTU update event; conn_handle=%d cid=%d mtu=%d",
                    event->mtu.conn_handle,
                    event->mtu.channel_id,
                    event->mtu.value);
        setConnectionMTU(event->mtu.conn_handle, event->mtu.value);

        return 0;

    case BLE_GAP_EVENT_REPEAT_PAIRING:
        DASH_LOGI(tag, "Repeat Pairing");
        // We already have a bond with the peer, but it is attempting to establish a new secure link.
        // This app sacrifices security for convenience: just throw away the old bond and accept the new link.

        // Delete the old bond
        rc = ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc);
        if (rc != 0) {
            return BLE_GAP_REPEAT_PAIRING_IGNORE;
        }

        ble_store_util_delete_peer(&desc.peer_id_addr);

        // Return BLE_GAP_REPEAT_PAIRING_RETRY to indicate that the host should continue with the pairing operation.
        return BLE_GAP_REPEAT_PAIRING_RETRY;

    case BLE_GAP_EVENT_PASSKEY_ACTION:
        DASH_LOGI(tag, "PASSKEY_ACTION_EVENT started");

        if (event->passkey.params.action == BLE_SM_IOACT_DISP) {
            struct ble_sm_io pkey = {0};
            pkey.action = event->passkey.params.action;
            pkey.passkey = passKey; // This is the passkey to be entered on peer
            DASH_LOGI(tag, "Enter passkey %" PRIu32 " on the peer side", pkey.passkey);
            rc = ble_sm_inject_io(event->passkey.conn_handle, &pkey);
            DASH_LOGI(tag, "ble_sm_inject_io result: %d", rc);
        }

        return 0;

    case BLE_GAP_EVENT_AUTHORIZE:
        DASH_LOGI(tag, "authorize event: conn_handle=%d attr_handle=%d is_read=%d",
                    event->authorize.conn_handle,
                    event->authorize.attr_handle,
                    event->authorize.is_read);

        // The default behaviour for the event is to reject authorize request
        event->authorize.out_response = BLE_GAP_AUTHORIZE_REJECT;

        return 0;

#if MYNEWT_VAL(BLE_POWER_CONTROL)
    case BLE_GAP_EVENT_TRANSMIT_POWER:
        DASH_LOGI(tag, "Transmit power event : status=%d conn_handle=%d reason=%d "
                           "phy=%d power_level=%x power_level_flag=%d delta=%d",
                     event->transmit_power.status,
                     event->transmit_power.conn_handle,
                     event->transmit_power.reason,
                     event->transmit_power.phy,
                     event->transmit_power.transmit_power_level,
                     event->transmit_power.transmit_power_level_flag,
                     event->transmit_power.delta);

        return 0;

    case BLE_GAP_EVENT_PATHLOSS_THRESHOLD:
        DASH_LOGI(tag, "Pathloss threshold event : conn_handle=%d current path loss=%d "
                           "zone_entered =%d",
                     event->pathloss_threshold.conn_handle,
                     event->pathloss_threshold.current_path_loss,
                     event->pathloss_threshold.zone_entered);

        return 0;
#endif
    }

    return 0;
}

void bleNotifyValue(uint16_t conn_handle, const char *message, uint16_t length) {
    xEventGroupWaitBits(ble_event_group, EVENT_NOTIFY_DONE_BIT, pdTRUE, pdFALSE, 100 / portTICK_PERIOD_MS);
    xEventGroupClearBits(ble_event_group, EVENT_NOTIFY_DONE_BIT); // Need this in case it times out (i.e. on timeout the bit is NOT cleared).

    int rc = dash_server_send_message(conn_handle, message, length);
    if (rc != 0) {

        if (rc == 7) {
            DASH_LOGI(tag, "Send Message Fail - NOT Connected");
            setConnectionInactive(conn_handle);
            ble_advertise(); // Not sure if this is needed here, but probably a good idea if a connection gets screwed up
        } else {
            DASH_LOGI(tag, "Send Message Fail: %d", rc);
        }
        DASH_LOGI(tag, "%s\n", message);
    }
}
    
static void ble_on_reset(int reason) {
    DASH_LOGE(tag, "Resetting state; reason=%d", reason);
}

static void ble_on_sync(void) {
    int rc = ble_hs_util_ensure_addr(0); // Make sure we have proper identity address set (0 = public, which is preferred)
    assert(rc == 0);

    // Figure out address to use while advertising (no privacy for now)
    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    if (rc != 0) {
        DASH_LOGE(tag, "Error determining address type; rc=%d", rc);
        return;
    }

    // Printing ADDR
    uint8_t addr_val[6] = {0};
    rc = ble_hs_id_copy_addr(own_addr_type, addr_val, NULL);

    printAddress(addr_val);

    // Begin advertising.
    ble_advertise();
}

bool updateClientData(struct BLEclientHolder *conn, char *message, uint16_t length) {
    if (conn->messageLength + length <= maxBLEmessageSize) {
        memcpy(conn->msg + conn->messageLength, message, length);
        conn->messageLength += length;
        if (message[length-1] == '\n') {
            on_message_received(conn->connectionHandle, conn->msg, conn->messageLength);
            conn->messageLength = 0;
        }
        return true;
    } else {
        return false;
    }
}

void onBLEmessageReceived(uint16_t conn_handle, char *message, uint16_t length) {
    struct BLEclientHolder *conn = getConnection(conn_handle);
    if (conn != NULL) {
        if (conn->messageLength == 0) {
            if (message[length-1] == '\n') {
                on_message_received(conn_handle, message, length);
            } else {
                updateClientData(conn, message, length);
            }
        } else {
            if (!updateClientData(conn, message, length)) {
                // Loooong message, can't update client data, possibly OTA, so parse client data first
                if (conn->messageLength > 0) {
                    on_message_received(conn_handle, conn->msg, conn->messageLength);
                    conn->messageLength = 0;
                }
                updateClientData(conn, message, length);
            }
        }
    }
}

void ble_secure(uint32_t _passKey) {
    passKey = _passKey;

    if (passKey >= 100000) {
        ble_hs_cfg.sm_io_cap = BLE_SM_IO_CAP_DISP_ONLY; 
        ble_hs_cfg.sm_our_key_dist = BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID;
        ble_hs_cfg.sm_their_key_dist = BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID;
        
        ble_hs_cfg.sm_bonding = 1; // Bonding
        ble_hs_cfg.sm_mitm = 1; // Man In The Middle
        ble_hs_cfg.sm_sc = 0; // 0 = use legacy, 1 = LE Secure Connections for pairing
    } else {
        ble_hs_cfg.sm_io_cap = BLE_SM_IO_CAP_NO_IO;
        ble_hs_cfg.sm_sc = 0;
    }
}

void ble_start_security(uint16_t connHandle) {
    if (getConnectionAuthState(connHandle) == BLE_NOT_AUTH) {
        int rc = ble_gap_security_initiate(connHandle);
        if (rc == 0) {
            DASH_LOGI(tag, "ble_gap_security_initiate done for handle: %d", connHandle);
            setConnectionAuthState(connHandle, BLE_AUTH_STARTED);
        } else {
            DASH_LOGE(tag, "ble_gap_security_initiate fail for handle: %d, rc=%d", connHandle, rc);
        }
    }
}

void ble_init(dash_message_received_fn *_on_message_received, const char *advName) {
    // NVS needs to be initlised before BLE so it can store bonds etc.
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        err = nvs_flash_erase();
        if (err == ESP_OK) {
            err = nvs_flash_init();
        }
    }
    if (err != ESP_OK) {
        DASH_LOGE(tag, "nvs_flash_init() failed; err=%d", err);
    }

    esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);

    initialiseClientHolders();

    // Initialize the NimBLE host configuration.
    ble_hs_cfg.reset_cb = ble_on_reset;
    ble_hs_cfg.sync_cb = ble_on_sync;
    ble_hs_cfg.gatts_register_cb = dash_server_gatt_register;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    ble_hs_cfg.sm_io_cap = BLE_SM_IO_CAP_NO_IO;
    ble_hs_cfg.sm_sc = 0;

    on_message_received = _on_message_received;
    on_dash_server_message_received = &onBLEmessageReceived;

    // Set the advertising device name.
    esp_err_t rc = ble_svc_gap_device_name_set(advName); 
    assert(rc == 0);

    DASH_LOGI(tag, "BLE Advertising as %s ", advName);

    ble_store_config_init();

    ble_event_group = xEventGroupCreate();
    xEventGroupSetBits(ble_event_group, EVENT_NOTIFY_DONE_BIT);
}

void ble_host_task(void *param) {
    nimble_port_run(); // This function will return only when nimble_port_stop() is executed
    nimble_port_freertos_deinit(); // This deletes the task
}

void ble_begin() {
    esp_err_t rc = nimble_port_init();
    if (rc != ESP_OK) {
        DASH_LOGE(tag, "Failed to init nimble %d", rc);
        return;
    }

    rc = dash_server_init();
    assert(rc == 0);

    nimble_port_freertos_init(ble_host_task); // All this does is create a task with the function
}

void ble_stop() {
    for (int i = 0; i < maxBLEclients; i++) {
        bleClients[i].active = false;
        bleClients[i].connectionHandle = -1;
    }

	if (nimble_port_stop() == ESP_OK) {
	    nimble_port_deinit();
    }
}

#endif

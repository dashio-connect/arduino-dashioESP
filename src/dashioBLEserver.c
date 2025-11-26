#include "sdkconfig.h"
#if !defined(CONFIG_IDF_TARGET_ESP32) && !defined(CONFIG_IDF_TARGET_ESP32S2)

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "dashioBLEserver.h"

static const char *tag = "Dash_BLE_SRV";

dash_message_received_fn *on_dash_server_message_received;

// Dash characteristic
static uint8_t dash_svr_chr_val;
uint16_t dash_svr_chr_val_handle;

static int dash_service_access(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);

static const struct ble_gatt_svc_def dash_server_services[] = {
    { // Dash Service
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &dash_svc_uuid.u,
        .characteristics = (struct ble_gatt_chr_def[]) {
            {
                .uuid = &dash_svr_chr_uuid.u,
                .access_cb = dash_service_access,
#if CONFIG_EXAMPLE_ENCRYPTION
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE |
                BLE_GATT_CHR_F_READ_ENC | BLE_GATT_CHR_F_WRITE_ENC |
                BLE_GATT_CHR_F_NOTIFY | BLE_GATT_CHR_F_INDICATE,
#else
                .flags = BLE_GATT_CHR_F_WRITE_NO_RSP | BLE_GATT_CHR_F_NOTIFY,
#endif
                .val_handle = &dash_svr_chr_val_handle,
            }, {
                0, // No more characteristics in this service
            }
        },
    },

    {
        0, // No more services
    },
};

int dash_server_send_message(uint16_t conn_handle, const char *message, int length) {
    struct os_mbuf *om;
    om = ble_hs_mbuf_from_flat(message, length);
    return ble_gatts_notify_custom(conn_handle, dash_svr_chr_val_handle, om);
}

static int gatt_svr_write(uint16_t conn_handle, struct os_mbuf *om, uint16_t max_len, void *dst, uint16_t *len) {
    uint16_t om_len = OS_MBUF_PKTLEN(om);
    on_dash_server_message_received(conn_handle, (char *)om->om_data, om_len);
    return 0;
}

/**
 * Access callback whenever a characteristic/descriptor is read or written to.
 * ctxt->op tells weather the operation is read or write and weather it is on a characteristic 
 * or descriptor, ctxt->dsc->uuid tells which characteristic/descriptor is accessed.
 * attr_handle give the value handle of the attribute being accessed.
 * Accordingly do:
 *     Append the value to ctxt->om if the operation is READ
 *     Write ctxt->om to the value if the operation is WRITE
 **/
static int dash_service_access(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    int rc;

    switch (ctxt->op) {
    case BLE_GATT_ACCESS_OP_READ_CHR:
        if (conn_handle != BLE_HS_CONN_HANDLE_NONE) {
            DASH_LOGI(tag, "Characteristic read; conn_handle=%d attr_handle=%d",
                        conn_handle, attr_handle);
        } else {
            DASH_LOGI(tag, "Characteristic read by NimBLE stack; attr_handle=%d",
                        attr_handle);
        }
        if (attr_handle == dash_svr_chr_val_handle) {
            rc = os_mbuf_append(ctxt->om,
                                &dash_svr_chr_val,
                                sizeof(dash_svr_chr_val));
            return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
        }
        goto unknown;

    case BLE_GATT_ACCESS_OP_WRITE_CHR:
        if (conn_handle != BLE_HS_CONN_HANDLE_NONE) {
            DASH_LOGI(tag, "Characteristic write; conn_handle=%d attr_handle=%d", conn_handle, attr_handle);
        } else {
            DASH_LOGI(tag, "Characteristic write by NimBLE stack; attr_handle=%d", attr_handle);
        }
        if (attr_handle == dash_svr_chr_val_handle) {
            rc = gatt_svr_write(conn_handle, ctxt->om, sizeof(dash_svr_chr_val), &dash_svr_chr_val, NULL);
            return rc;
        }
        goto unknown;

    default:
        goto unknown;
    }

unknown:
    // Unknown characteristic/descriptor - NimBLE host should not have called this function;
    assert(0);
    return BLE_ATT_ERR_UNLIKELY;
}

void dash_server_gatt_register(struct ble_gatt_register_ctxt *ctxt, void *arg) {
    char buf[BLE_UUID_STR_LEN];

    switch (ctxt->op) {
    case BLE_GATT_REGISTER_OP_SVC:
        DASH_LOGD(tag, "registered service %s with handle=%d",
                    ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
                    ctxt->svc.handle);
        break;

    case BLE_GATT_REGISTER_OP_CHR:
        DASH_LOGD(tag, "registering characteristic %s with "
                    "def_handle=%d val_handle=%d",
                    ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
                    ctxt->chr.def_handle,
                    ctxt->chr.val_handle);
        break;

    case BLE_GATT_REGISTER_OP_DSC:
        DASH_LOGD(tag, "registering descriptor %s with handle=%d",
                    ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
                    ctxt->dsc.handle);
        break;

    default:
        assert(0);
        break;
    }
}

int dash_server_init(void) {
    int rc;

    ble_svc_gap_init();
    ble_svc_gatt_init();

    rc = ble_gatts_count_cfg(dash_server_services);
    if (rc != 0) {
        return rc;
    }

    rc = ble_gatts_add_svcs(dash_server_services);
    if (rc != 0) {
        return rc;
    }

    return 0;
}

#endif

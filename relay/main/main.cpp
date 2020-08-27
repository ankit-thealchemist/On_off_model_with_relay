#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <esp_system.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include "initiation.h"

#define bleTag "bleApp"
#define pinNum1 23

// provisioning callback function
void example_ble_mesh_provisioning_cb(esp_ble_mesh_prov_cb_event_t event, esp_ble_mesh_prov_cb_param_t *param)
{
    switch (event)
    {
    case ESP_BLE_MESH_PROV_REGISTER_COMP_EVT:
        ESP_LOGI(bleTag, "ESP_BLE_MESH_PROV_REGISTER_COMP_EVT, err_code %d", param->prov_register_comp.err_code);
        break;
    case ESP_BLE_MESH_NODE_PROV_ENABLE_COMP_EVT:
        ESP_LOGI(bleTag, "ESP_BLE_MESH_NODE_PROV_ENABLE_COMP_EVT, err_code %d", param->node_prov_enable_comp.err_code);
        break;
    case ESP_BLE_MESH_NODE_PROV_LINK_OPEN_EVT:
        ESP_LOGI(bleTag, "ESP_BLE_MESH_NODE_PROV_LINK_OPEN_EVT, bearer %s",
                 param->node_prov_link_open.bearer == ESP_BLE_MESH_PROV_ADV ? "PB-ADV" : "PB-GATT");
        break;
    case ESP_BLE_MESH_NODE_PROV_LINK_CLOSE_EVT:
        ESP_LOGI(bleTag, "ESP_BLE_MESH_NODE_PROV_LINK_CLOSE_EVT, bearer %s",
                 param->node_prov_link_close.bearer == ESP_BLE_MESH_PROV_ADV ? "PB-ADV" : "PB-GATT");
        break;
    case ESP_BLE_MESH_NODE_PROV_COMPLETE_EVT:
        ESP_LOGI(bleTag, "ESP_BLE_MESH_NODE_PROV_COMPLETE_EVT");
        ESP_LOGI(bleTag, "net index %d\n", param->node_prov_complete.net_idx);
        // ESP_LOGI(bleTag,"net key %d\n", param->node_prov_complete.net_key);
        ESP_LOGI(bleTag, "address %d\n", param->node_prov_complete.addr);
        ESP_LOGI(bleTag, "flags %d\n", param->node_prov_complete.flags);
        ESP_LOGI(bleTag, "IV index%d\n", param->node_prov_complete.iv_index);
        break;
    case ESP_BLE_MESH_NODE_PROV_RESET_EVT:
        ESP_LOGI(bleTag, "ESP_BLE_MESH_NODE_PROV_RESET_EVT");
        break;
    case ESP_BLE_MESH_NODE_SET_UNPROV_DEV_NAME_COMP_EVT:
        ESP_LOGI(bleTag, "ESP_BLE_MESH_NODE_SET_UNPROV_DEV_NAME_COMP_EVT, err_code %d", param->node_set_unprov_dev_name_comp.err_code);
        break;
    case ESP_BLE_MESH_NODE_PROV_OUTPUT_NUMBER_EVT:
        ESP_LOGI(bleTag, " output number is %d ", param->node_prov_output_num.number);
        break;
    case ESP_BLE_MESH_NODE_PROV_OUTPUT_STRING_EVT:
        ESP_LOGI(bleTag, " output string is %s", param->node_prov_output_str.string);
        break;

    case ESP_BLE_MESH_HEARTBEAT_MESSAGE_RECV_EVT:
        ESP_LOGI(bleTag, " heartbeat hops %d feature 0x%04x", param->heartbeat_msg_recv.hops, param->heartbeat_msg_recv.feature);
        break;

    case ESP_BLE_MESH_NODE_PROV_OOB_PUB_KEY_EVT:
        ESP_LOGI(bleTag, "ESP_BLE_MESH_NODE_PROV_OOB_PUB_KEY_EVT");
        break;

    case ESP_BLE_MESH_NODE_PROV_SET_OOB_PUB_KEY_COMP_EVT:
        ESP_LOGI(bleTag, "ESP_BLE_MESH_NODE_PROV_SET_OOB_PUB_KEY_COMP_EVT  error code is %d", param->node_prov_set_oob_pub_key_comp.err_code);
        break;

    default:
        ESP_LOGI(bleTag, "provison cb event number is %d", event);
        break;
    }
}

//configuration server callback function, it is the foundation  model layer
void example_ble_mesh_config_server_cb(esp_ble_mesh_cfg_server_cb_event_t event, esp_ble_mesh_cfg_server_cb_param_t *param)
{
    if (event == ESP_BLE_MESH_CFG_SERVER_STATE_CHANGE_EVT)
    {
        switch (param->ctx.recv_op)
        {
        case ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD:
            ESP_LOGI(bleTag, "ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD");
            ESP_LOGI(bleTag, "net_idx 0x%04x, app_idx 0x%04x",
                     param->value.state_change.appkey_add.net_idx,
                     param->value.state_change.appkey_add.app_idx);
            ESP_LOG_BUFFER_HEX("AppKey", param->value.state_change.appkey_add.app_key, 16);
            ESP_LOGI(bleTag, "opcode for the procedure is %x", param->ctx.recv_op);
            break;
        case ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND:
            ESP_LOGI(bleTag, "ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND");
            ESP_LOGI(bleTag, "elem_addr 0x%04x, app_idx 0x%04x, cid 0x%04x, mod_id 0x%04x",
                     param->value.state_change.mod_app_bind.element_addr,
                     param->value.state_change.mod_app_bind.app_idx,
                     param->value.state_change.mod_app_bind.company_id,
                     param->value.state_change.mod_app_bind.model_id);
            if (param->value.state_change.mod_app_bind.company_id == 0xFFFF &&
                param->value.state_change.mod_app_bind.model_id == ESP_BLE_MESH_MODEL_ID_GEN_ONOFF_CLI)
            {
                node_app_idx = param->value.state_change.mod_app_bind.app_idx;
            }
            break;
        case ESP_BLE_MESH_MODEL_OP_MODEL_SUB_ADD:
            ESP_LOGI(bleTag, "ESP_BLE_MESH_MODEL_OP_MODEL_SUB_ADD");
            ESP_LOGI(bleTag, "elem_addr 0x%04x, sub_addr 0x%04x, cid 0x%04x, mod_id 0x%04x",
                     param->value.state_change.mod_sub_add.element_addr,
                     param->value.state_change.mod_sub_add.sub_addr,
                     param->value.state_change.mod_sub_add.company_id,
                     param->value.state_change.mod_sub_add.model_id);
            break;

        case ESP_BLE_MESH_MODEL_OP_MODEL_PUB_SET:
            ESP_LOGI(bleTag, "ESP_BLE_MESH_MODEL_OP_MODEL_PUB_SET");
            ESP_LOGI(bleTag, "publish address is 0x%04x", param->value.state_change.mod_pub_set.pub_addr);

            break;
        default:
            ESP_LOGI(bleTag, "config server callback value is  0x%04x ", param->ctx.recv_op);
            break;
        }
    }
}



// Generic configuration client callback function

extern "C"
{
    void app_main()
    {

        ESP_LOGI(bleTag, "line : %d ", __LINE__);
        ESP_LOGI(bleTag, "func %s : ", __func__);
        esp_err_t ret; // variable for the error
        ESP_LOGI(bleTag, "Verion of the esp idf is  : %s", esp_get_idf_version());
        ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES)
        {
            ESP_ERROR_CHECK(nvs_flash_erase());
            ret = nvs_flash_init();
        }
        ESP_ERROR_CHECK(ret);
        ret = bluetooth_init();
        ESP_LOGI(bleTag, "bluetooth initialization of nimble ");
        ble_mesh_get_dev_uuid(dev_uuid);
        ret = esp_ble_mesh_register_prov_callback(example_ble_mesh_provisioning_cb);
        ESP_LOGI(bleTag, "provisioning call back set value : %s", esp_err_to_name(ret));
        ret = esp_ble_mesh_register_config_server_callback(example_ble_mesh_config_server_cb);
        ESP_LOGI(bleTag, "config server callback register is %s", esp_err_to_name(ret));
        mesh_intitializaton();
    }
}
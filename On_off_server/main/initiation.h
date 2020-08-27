#ifndef _initaition_h
#define _initaition_h
#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include <esp_ble_mesh_common_api.h>
#include <esp_ble_mesh_provisioning_api.h>
#include <esp_ble_mesh_networking_api.h>
#include <esp_ble_mesh_config_model_api.h>
#include <esp_ble_mesh_generic_model_api.h>
#include <esp_ble_mesh_health_model_api.h>

#define CID_ESP 0x02E5   // cid is the vendor of the manufacturer
extern uint8_t dev_uuid[16];  //setting the uuid of the unprovisioned device to dddd
extern esp_ble_mesh_prov_t provision;
extern esp_ble_mesh_comp_t composition;
extern uint16_t node_net_idx;
extern uint16_t node_app_idx;
esp_err_t bluetooth_init();
void ble_mesh_get_dev_uuid(uint8_t *dev_uuid);
void mesh_intitializaton();


#ifdef __cplusplus
}
#endif



#endif
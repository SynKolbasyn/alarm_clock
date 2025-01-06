#pragma once


#ifndef BLUETOOTH_HPP
#define BLUETOOTH_HPP


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <inttypes.h>

#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"


namespace ble {


enum {
  IDX_SVC,
  
  IDX_CHAR_C,
  IDX_CHAR_VAL_C,
  
  HRS_IDX_NB,
};

struct gatts_profile_inst {
  esp_gatts_cb_t gatts_cb;
  uint16_t gatts_if;
  uint16_t app_id;
  uint16_t conn_id;
  uint16_t service_handle;
  esp_gatt_srvc_id_t service_id;
  uint16_t char_handle;
  esp_bt_uuid_t char_uuid;
  esp_gatt_perm_t perm;
  esp_gatt_char_prop_t property;
  uint16_t descr_handle;
  esp_bt_uuid_t descr_uuid;
};


static std::string esp_key_type_to_str(esp_ble_key_type_t key_type);
static std::string esp_auth_req_to_str(esp_ble_auth_req_t auth_req);
static void show_bonded_devices();
static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t* param);
static void gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param);
static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param);


static constexpr char tag[] = "bluetooth";

static constexpr uint8_t HEART_PROFILE_NUM = 1;
static constexpr uint8_t HEART_PROFILE_APP_IDX = 0;
static constexpr uint8_t ESP_HEART_RATE_APP_ID = 0x55;
static constexpr uint8_t HEART_RATE_SVC_INST_ID = 0;
static constexpr uint8_t EXT_ADV_HANDLE = 0;
static constexpr uint8_t NUM_EXT_ADV_SET = 1;
static constexpr uint8_t EXT_ADV_DURATION = 0;
static constexpr uint8_t EXT_ADV_MAX_EVENTS = 0;
static constexpr uint8_t EXT_ADV_NAME_LEN_OFFSET = 10;
static constexpr uint8_t EXT_ADV_NAME_OFFSET = 12;

static constexpr uint8_t GATTS_DEMO_CHAR_VAL_LEN_MAX = 0x40;

static uint16_t profile_handle_table[HRS_IDX_NB];

static uint8_t ext_adv_raw_data[] = {
  0x02, 0x01, 0x06,
  0x02, 0x0a, 0xeb, 0x03, 0x03, 0xab, 0xcd,
  0x11, 0X09, 'E', 'S', 'P', '_', 'B', 'L', 'E', '5', '0', '_', 'S', 'E', 'R', 'V', 'E', 'R',
};

static esp_ble_gap_ext_adv_t ext_adv[1] = {
  [0] = { EXT_ADV_HANDLE, EXT_ADV_DURATION, EXT_ADV_MAX_EVENTS },
};

esp_ble_gap_ext_adv_params_t ext_adv_params_2M = {
  .type = ESP_BLE_GAP_SET_EXT_ADV_PROP_CONNECTABLE,
  .interval_min = 0x20,
  .interval_max = 0x20,
  .channel_map = ADV_CHNL_ALL,
  .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
  .filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
  .tx_power = EXT_ADV_TX_PWR_NO_PREFERENCE,
  .primary_phy = ESP_BLE_GAP_PHY_1M,
  .max_skip = 0,
  .secondary_phy = ESP_BLE_GAP_PHY_2M,
  .sid = 0,
  .scan_req_notif = false,
};

static struct gatts_profile_inst heart_rate_profile_tab[HEART_PROFILE_NUM] = {
  [HEART_PROFILE_APP_IDX] = {
    .gatts_cb = gatts_profile_event_handler,
    .gatts_if = ESP_GATT_IF_NONE,
  },
};

static constexpr uint16_t GATTS_SERVICE_UUID_TEST = 0x00FF;
static constexpr uint16_t GATTS_CHAR_UUID_TEST_C = 0xFF01;

static constexpr uint16_t primary_service_uuid = ESP_GATT_UUID_PRI_SERVICE;
static constexpr uint16_t character_declaration_uuid = ESP_GATT_UUID_CHAR_DECLARE;
static constexpr uint16_t character_client_config_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
static constexpr uint8_t char_prop_read =  ESP_GATT_CHAR_PROP_BIT_READ;
static constexpr uint8_t char_prop_write = ESP_GATT_CHAR_PROP_BIT_WRITE;
static constexpr uint8_t char_prop_read_write_notify = ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;
static constexpr uint8_t heart_measurement_ccc[2] = { 0x00, 0x00 };
static constexpr uint8_t char_value[4] = { 0x11, 0x22, 0x33, 0x44 };

static const esp_gatts_attr_db_t gatt_db[HRS_IDX_NB] = {
  [IDX_SVC] = {
    { ESP_GATT_AUTO_RSP },
    { ESP_UUID_LEN_16, (uint8_t*)&primary_service_uuid, ESP_GATT_PERM_READ, sizeof(uint16_t), sizeof(GATTS_SERVICE_UUID_TEST), (uint8_t*)&GATTS_SERVICE_UUID_TEST }
  },

  [IDX_CHAR_C] = {
    { ESP_GATT_AUTO_RSP },
    { ESP_UUID_LEN_16, (uint8_t*)&character_declaration_uuid, ESP_GATT_PERM_READ, sizeof(uint8_t), sizeof(uint8_t), (uint8_t*)&char_prop_write }
  },

  [IDX_CHAR_VAL_C] = {
    { ESP_GATT_AUTO_RSP },
    { ESP_UUID_LEN_16, (uint8_t*)&GATTS_CHAR_UUID_TEST_C, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(char_value), (uint8_t*)char_value }
  },
};


void main(void* arg) {
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  ret = esp_bt_controller_init(&bt_cfg);
  if (ret) {
    ESP_LOGE(tag, "%s init controller failed: %s", __func__, esp_err_to_name(ret));
    return;
  }
  ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
  if (ret) {
    ESP_LOGE(tag, "%s enable controller failed: %s", __func__, esp_err_to_name(ret));
    return;
  }

  ESP_LOGI(tag, "%s init bluetooth", __func__);

  ret = esp_bluedroid_init();
  if (ret) {
    ESP_LOGE(tag, "%s init bluetooth failed: %s", __func__, esp_err_to_name(ret));
    return;
  }
  ret = esp_bluedroid_enable();
  if (ret) {
    ESP_LOGE(tag, "%s enable bluetooth failed: %s", __func__, esp_err_to_name(ret));
    return;
  }

  ret = esp_ble_gatts_register_callback(gatts_event_handler);
  if (ret) {
    ESP_LOGE(tag, "gatts register error, error code = %x", ret);
    return;
  }
  ret = esp_ble_gap_register_callback(gap_event_handler);
  if (ret) {
    ESP_LOGE(tag, "gap register error, error code = %x", ret);
    return;
  }
  ret = esp_ble_gatts_app_register(ESP_HEART_RATE_APP_ID);
  if (ret) {
    ESP_LOGE(tag, "gatts app register error, error code = %x", ret);
    return;
  }

  esp_ble_auth_req_t auth_req = ESP_LE_AUTH_REQ_SC_MITM_BOND;
  esp_ble_io_cap_t iocap = ESP_IO_CAP_NONE;
  uint8_t key_size = 16; //the key size should be 7~16 bytes
  uint8_t init_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
  uint8_t rsp_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
  uint32_t passkey = 123456;
  uint8_t auth_option = ESP_BLE_ONLY_ACCEPT_SPECIFIED_AUTH_DISABLE;
  uint8_t oob_support = ESP_BLE_OOB_DISABLE;
  esp_ble_gap_set_security_param(ESP_BLE_SM_SET_STATIC_PASSKEY, &passkey, sizeof(uint32_t));
  esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(uint8_t));
  esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
  esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE, &key_size, sizeof(uint8_t));
  esp_ble_gap_set_security_param(ESP_BLE_SM_ONLY_ACCEPT_SPECIFIED_SEC_AUTH, &auth_option, sizeof(uint8_t));
  esp_ble_gap_set_security_param(ESP_BLE_SM_OOB_SUPPORT, &oob_support, sizeof(uint8_t));
  esp_ble_gap_set_security_param(ESP_BLE_SM_SET_INIT_KEY, &init_key, sizeof(uint8_t));
  esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &rsp_key, sizeof(uint8_t));

  while (true) vTaskDelay(portMAX_DELAY);
}


static std::string esp_key_type_to_str(esp_ble_key_type_t key_type) {
  switch (key_type) {
    case ESP_LE_KEY_NONE:
      return "ESP_LE_KEY_NONE";

    case ESP_LE_KEY_PENC:
      return "ESP_LE_KEY_PENC";

    case ESP_LE_KEY_PID:
      return "ESP_LE_KEY_PID";

    case ESP_LE_KEY_PCSRK:
      return "ESP_LE_KEY_PCSRK";

    case ESP_LE_KEY_PLK:
      return "ESP_LE_KEY_PLK";

    case ESP_LE_KEY_LLK:
      return "ESP_LE_KEY_LLK";

    case ESP_LE_KEY_LENC:
      return "ESP_LE_KEY_LENC";

    case ESP_LE_KEY_LID:
      return "ESP_LE_KEY_LID";

    case ESP_LE_KEY_LCSRK:
      return "ESP_LE_KEY_LCSRK";

    default:
      return "INVALID BLE KEY TYPE";
  }
}


static std::string esp_auth_req_to_str(esp_ble_auth_req_t auth_req) {
  switch (auth_req) {
    case ESP_LE_AUTH_NO_BOND:
      return "ESP_LE_AUTH_NO_BOND";

    case ESP_LE_AUTH_BOND:
      return "ESP_LE_AUTH_BOND";

    case ESP_LE_AUTH_REQ_MITM:
      return "ESP_LE_AUTH_REQ_MITM";

    case ESP_LE_AUTH_REQ_BOND_MITM:
      return "ESP_LE_AUTH_REQ_BOND_MITM";

    case ESP_LE_AUTH_REQ_SC_ONLY:
      return "ESP_LE_AUTH_REQ_SC_ONLY";

    case ESP_LE_AUTH_REQ_SC_BOND:
      return "ESP_LE_AUTH_REQ_SC_BOND";

    case ESP_LE_AUTH_REQ_SC_MITM:
      return "ESP_LE_AUTH_REQ_SC_MITM";

    case ESP_LE_AUTH_REQ_SC_MITM_BOND:
      return "ESP_LE_AUTH_REQ_SC_MITM_BOND";

    default:
      return "INVALID BLE AUTH REQ";
  }
}


static void show_bonded_devices() {
  int dev_num = esp_ble_get_bond_device_num();
  if (dev_num == 0) {
    ESP_LOGI(tag, "Bonded devices number zero\n");
    return;
  }

  esp_ble_bond_dev_t* dev_list = static_cast<esp_ble_bond_dev_t*>(calloc(dev_num, sizeof(esp_ble_bond_dev_t)));
  if (!dev_list) {
    ESP_LOGE(tag, "malloc failed\n");
    return;
  }
  esp_ble_get_bond_device_list(&dev_num, dev_list);
  ESP_LOGI(tag, "Bonded devices number %d", dev_num);
  for (int i = 0; i < dev_num; i++) {
      ESP_LOGI(tag, "[%u] addr_type %u, addr %02x:%02x:%02x:%02x:%02x:%02x", i, dev_list[i].bd_addr_type, ESP_BD_ADDR_HEX(dev_list[i].bd_addr));
  }

  free(dev_list);
}


static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t* param) {
  switch (event) {
    case ESP_GAP_BLE_EXT_ADV_SET_PARAMS_COMPLETE_EVT:
      ESP_LOGI(tag,"Extended advertising params set, status %d",  param->ext_adv_set_params.status);
      esp_ble_gap_config_ext_adv_data_raw(EXT_ADV_HANDLE,  sizeof(ext_adv_raw_data), &ext_adv_raw_data[0]);
      break;

    case ESP_GAP_BLE_EXT_ADV_DATA_SET_COMPLETE_EVT:
      ESP_LOGI(tag,"Extended advertising data set, status %d",  param->ext_adv_data_set.status);
      esp_ble_gap_ext_adv_start(NUM_EXT_ADV_SET, &ext_adv[0]);
      break;

    case ESP_GAP_BLE_EXT_ADV_START_COMPLETE_EVT:
      ESP_LOGI(tag, "Extended advertising start, status %d", param->ext_adv_data_set.status);
      break;

    case ESP_GAP_BLE_ADV_TERMINATED_EVT:
      ESP_LOGI(tag, "Extended advertising terminated, status %d", param->adv_terminate.status);
      if (param->adv_terminate.status == 0x00) {
        ESP_LOGI(tag, "Advertising successfully ended with a connection being created");
      }
      break;

    case ESP_GAP_BLE_PASSKEY_REQ_EVT:
      ESP_LOGI(tag, "Passkey request");
      break;

    case ESP_GAP_BLE_OOB_REQ_EVT: {
      ESP_LOGI(tag, "OOB request");
      uint8_t tk[16] = {1};
      esp_ble_oob_req_reply(param->ble_security.ble_req.bd_addr, tk, sizeof(tk));
      break;
    }

    case ESP_GAP_BLE_LOCAL_IR_EVT:
      ESP_LOGI(tag, "Local identity root");
      break;

    case ESP_GAP_BLE_LOCAL_ER_EVT:
      ESP_LOGI(tag, "Local encryption root");
      break;

    case ESP_GAP_BLE_NC_REQ_EVT:
      esp_ble_confirm_reply(param->ble_security.ble_req.bd_addr, true);
      ESP_LOGI(tag, "Numeric Comparison request, passkey %" PRIu32, param->ble_security.key_notif.passkey);
      break;
        
    case ESP_GAP_BLE_SEC_REQ_EVT:
      esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
      break;

    case ESP_GAP_BLE_PASSKEY_NOTIF_EVT:
      ESP_LOGI(tag, "Passkey notify, passkey %06" PRIu32, param->ble_security.key_notif.passkey);
      break;

    case ESP_GAP_BLE_KEY_EVT:
      ESP_LOGI(tag, "Key exchanged, key_type %s", esp_key_type_to_str(param->ble_security.ble_key.key_type).c_str());
      if (param->ble_security.ble_key.key_type == ESP_LE_KEY_PID) {
        ESP_LOGI(tag, "peer addr %02x:%02x:%02x:%02x:%02x:%02x", ESP_BD_ADDR_HEX(param->ble_security.ble_key.bd_addr));
      }
      break;

    case ESP_GAP_BLE_AUTH_CMPL_EVT: {
      esp_bd_addr_t bd_addr;
      memcpy(bd_addr, param->ble_security.auth_cmpl.bd_addr, sizeof(esp_bd_addr_t));
      ESP_LOGI(tag, "Authentication complete, addr_type %u, addr %02x:%02x:%02x:%02x:%02x:%02x", param->ble_security.auth_cmpl.addr_type, ESP_BD_ADDR_HEX(bd_addr));
      if (!param->ble_security.auth_cmpl.success) {
        ESP_LOGI(tag, "Pairing failed, reason 0x%x",param->ble_security.auth_cmpl.fail_reason);
      }
      else {
        ESP_LOGI(tag, "Pairing successfully, auth_mode %s", esp_auth_req_to_str(param->ble_security.auth_cmpl.auth_mode).c_str());
      }
      show_bonded_devices();
      break;
    }

    case ESP_GAP_BLE_REMOVE_BOND_DEV_COMPLETE_EVT: {
      ESP_LOGD(tag, "Bond device remove, status %d, device %02x:%02x:%02x:%02x:%02x:%02x",
        param->remove_bond_dev_cmpl.status,
        ESP_BD_ADDR_HEX(param->remove_bond_dev_cmpl.bd_addr)
      );
      break;
    }

    case ESP_GAP_BLE_SET_LOCAL_PRIVACY_COMPLETE_EVT:
      ESP_LOGI(tag, "Local privacy config, status 0x%x", param->local_privacy_cmpl.status);
      esp_ble_gap_ext_adv_set_params(EXT_ADV_HANDLE, &ext_adv_params_2M);
      break;

    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
      ESP_LOGI(tag, "Connection params update, status %d, conn_int %d, latency %d, timeout %d",
        param->update_conn_params.status,
        param->update_conn_params.conn_int,
        param->update_conn_params.latency,
        param->update_conn_params.timeout
      );
      break;

    default:
      break;
  }
}

static void gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param) {
  switch (event) {
    case ESP_GATTS_REG_EVT:
      ESP_LOGI(tag, "GATT server register, status %d", param->reg.status);
      esp_ble_gap_config_local_privacy(true);
      esp_ble_gatts_create_attr_tab(gatt_db, gatts_if, HRS_IDX_NB, 0);
      break;

    case ESP_GATTS_READ_EVT:
      ESP_LOGI(tag, "Characteristic read");
      break;

    case ESP_GATTS_WRITE_EVT: {
      ESP_LOGI(tag, "Characteristic write, value ");
      ESP_LOG_BUFFER_HEX(tag, param->write.value, param->write.len);
      // std::string data(param->write.value, param->write.value + param->write.len);
      // ESP_LOGI(tag, "%s", data.c_str());
      break;
    }

    case ESP_GATTS_EXEC_WRITE_EVT:
      break;
    case ESP_GATTS_MTU_EVT:
      break;
    case ESP_GATTS_CONF_EVT:
      break;
    case ESP_GATTS_UNREG_EVT:
      break;
    case ESP_GATTS_DELETE_EVT:
      break;
    case ESP_GATTS_START_EVT:
      break;
    case ESP_GATTS_STOP_EVT:
      break;

    case ESP_GATTS_CONNECT_EVT:
      ESP_LOGI(tag, "Connected, conn_id %u, remote %02x:%02x:%02x:%02x:%02x:%02x", param->connect.conn_id, ESP_BD_ADDR_HEX(param->connect.remote_bda));
      esp_ble_set_encryption(param->connect.remote_bda, ESP_BLE_SEC_ENCRYPT_MITM);
      break;

    case ESP_GATTS_DISCONNECT_EVT:
      ESP_LOGI(tag, "Disconnected, remote %02x:%02x:%02x:%02x:%02x:%02x, reason 0x%x", ESP_BD_ADDR_HEX(param->disconnect.remote_bda), param->disconnect.reason);
      esp_ble_gap_ext_adv_start(NUM_EXT_ADV_SET, &ext_adv[0]);
      break;

    case ESP_GATTS_OPEN_EVT:
      break;
    case ESP_GATTS_CANCEL_OPEN_EVT:
      break;
    case ESP_GATTS_CLOSE_EVT:
      break;
    case ESP_GATTS_LISTEN_EVT:
      break;
    case ESP_GATTS_CONGEST_EVT:
      break;

    case ESP_GATTS_CREAT_ATTR_TAB_EVT: {
      if (param->create.status == ESP_GATT_OK) {
        if (param->add_attr_tab.num_handle == HRS_IDX_NB) {
          ESP_LOGI(tag, "Attribute table create successfully, num_handle 0x%x", param->add_attr_tab.num_handle);
          memcpy(profile_handle_table, param->add_attr_tab.handles,
          sizeof(profile_handle_table));
          esp_ble_gatts_start_service(profile_handle_table[IDX_SVC]);
        }
        else {
          ESP_LOGE(tag, "Attribute table create abnormally, num_handle (%d) doesn't equal to HRS_IDX_NB(%d)", param->add_attr_tab.num_handle, HRS_IDX_NB);
        }
      }
      else {
        ESP_LOGE(tag, "Attribute table create failed, status 0x%x", param->create.status);
      }
      break;
    }

    default:
      break;
  }
}


static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param) {
  if (event == ESP_GATTS_REG_EVT) {
    if (param->reg.status == ESP_GATT_OK) {
      heart_rate_profile_tab[HEART_PROFILE_APP_IDX].gatts_if = gatts_if;
    }
    else {
      ESP_LOGI(tag, "Reg app failed, app_id %04x, status %d", param->reg.app_id, param->reg.status);
      return;
    }
  }

  for (int idx = 0; idx < HEART_PROFILE_NUM; idx++) {
    if (gatts_if == ESP_GATT_IF_NONE || gatts_if == heart_rate_profile_tab[idx].gatts_if) {
      if (heart_rate_profile_tab[idx].gatts_cb) {
        heart_rate_profile_tab[idx].gatts_cb(event, gatts_if, param);
      }
    }
  }
}


} // namespace ble


#endif // !BLUETOOTH_HPP

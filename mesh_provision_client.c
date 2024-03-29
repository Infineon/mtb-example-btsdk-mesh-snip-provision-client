/*
 * Copyright 2016-2023, Cypress Semiconductor Corporation (an Infineon company) or
 * an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
 *
 * This software, including source code, documentation and related
 * materials ("Software") is owned by Cypress Semiconductor Corporation
 * or one of its affiliates ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products.  Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 */

/** @file
 *
 * This file shows how to create a device which implements mesh provisioner client.
 * The main purpose of the app is to process messages coming from the MCU and call Mesh Core
 * Library to perform functionality.
 */
#include "wiced_bt_ble.h"
#include "wiced_bt_gatt.h"
#include "wiced_bt_mesh_app.h"
#include "wiced_bt_mesh_core.h"
#include "wiced_bt_mesh_models.h"
#include "wiced_bt_mesh_model_utils.h"
#include "wiced_bt_mesh_provision.h"
#if ( defined(DIRECTED_FORWARDING_SERVER_SUPPORTED) || defined(NETWORK_FILTER_SERVER_SUPPORTED))
#include "wiced_bt_mesh_mdf.h"
#endif
#ifdef LARGE_COMPOSITION_DATA_SUPPORTED
#include "wiced_bt_mesh_lcd.h"
#endif
#ifdef SAR_CONFIGURATION_SUPPORTED
#include "wiced_bt_mesh_sar.h"
#endif
#ifdef PRIVATE_PROXY_SUPPORTED
#include "wiced_bt_mesh_private_proxy.h"
#endif
#ifdef MESH_DFU_SUPPORTED
#include "wiced_bt_mesh_dfu.h"
#include "wiced_firmware_upgrade.h"
#endif
#ifdef OPCODES_AGGREGATOR_SUPPORTED
#include "wiced_bt_mesh_agg.h"
#endif
#include "wiced_bt_trace.h"
#include "wiced_transport.h"
#include "hci_control_api.h"
#include "wiced_bt_mesh_client.h"
#include "wiced_memory.h"

#include "wiced_bt_cfg.h"
extern wiced_bt_cfg_settings_t wiced_bt_cfg_settings;

uint32_t mesh_default_transition_time_proc_rx_cmd(uint16_t opcode, uint8_t* p_data, uint32_t length);
extern void mesh_default_transition_time_client_message_handler(uint16_t event, wiced_bt_mesh_event_t* p_event, void* p_data);

#ifdef WICED_BT_MESH_MODEL_PROPERTY_CLIENT_INCLUDED
uint32_t mesh_property_client_proc_rx_cmd(uint16_t opcode, uint8_t *p_data, uint32_t length);
extern void mesh_property_client_message_handler(uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);
#endif

#ifdef WICED_BT_MESH_MODEL_BATTERY_CLIENT_INCLUDED
uint32_t mesh_battery_client_proc_rx_cmd(uint16_t opcode, uint8_t *p_data, uint32_t length);
extern void mesh_battery_client_message_handler(uint16_t event, wiced_bt_mesh_event_t *p_event, mesh_battery_event_t *p_data);
#endif

#ifdef WICED_BT_MESH_MODEL_LIGHT_LC_CLIENT_INCLUDED
uint32_t mesh_light_lc_client_proc_rx_cmd(uint16_t opcode, uint8_t *p_data, uint32_t length);
extern void mesh_light_lc_client_message_handler(uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);
#endif

#ifdef WICED_BT_MESH_MODEL_LIGHT_XYL_CLIENT_INCLUDED
uint32_t mesh_light_xyl_client_proc_rx_cmd(uint16_t opcode, uint8_t *p_data, uint32_t length);
extern void mesh_light_xyl_client_message_handler(uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);
#endif

#ifdef WICED_BT_MESH_MODEL_LOCATION_CLIENT_INCLUDED
uint32_t mesh_location_client_proc_rx_cmd(uint16_t opcode, uint8_t *p_data, uint32_t length);
extern void mesh_location_client_message_handler(uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);
#endif

#ifdef WICED_BT_MESH_MODEL_POWER_LEVEL_CLIENT_INCLUDED
uint32_t mesh_power_level_client_proc_rx_cmd(uint16_t opcode, uint8_t *p_data, uint32_t length);
extern void mesh_power_level_client_message_handler(uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);
#endif

#ifdef WICED_BT_MESH_MODEL_POWER_ONOFF_CLIENT_INCLUDED
uint32_t mesh_power_onoff_client_proc_rx_cmd(uint16_t opcode, uint8_t *p_data, uint32_t length);
extern void mesh_power_onoff_client_message_handler(uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);
#endif

#ifdef WICED_BT_MESH_MODEL_SCHEDULER_CLIENT_INCLUDED
uint32_t mesh_scheduler_client_proc_rx_cmd(uint16_t opcode, uint8_t *p_data, uint32_t length);
extern void mesh_scheduler_client_message_handler(uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);
#endif

#ifdef WICED_BT_MESH_MODEL_TIME_CLIENT_INCLUDED
uint32_t mesh_time_client_proc_rx_cmd(uint16_t opcode, uint8_t *p_data, uint32_t length);
extern void mesh_time_client_message_handler(uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);
#endif

#ifdef WICED_BT_MESH_MODEL_ONOFF_CLIENT_INCLUDED
uint32_t mesh_onoff_client_proc_rx_cmd(uint16_t opcode, uint8_t *p_data, uint32_t length);
extern void mesh_onoff_client_message_handler(uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);
#endif

#ifdef WICED_BT_MESH_MODEL_LEVEL_CLIENT_INCLUDED
uint32_t mesh_level_client_proc_rx_cmd(uint16_t opcode, uint8_t *p_data, uint32_t length);
extern void mesh_level_client_message_handler(uint16_t event, wiced_bt_mesh_event_t *p_event, wiced_bt_mesh_level_status_data_t *p_data);
#endif

#ifdef WICED_BT_MESH_MODEL_LIGHT_LIGHTNESS_CLIENT_INCLUDED
uint32_t mesh_light_lightness_client_proc_rx_cmd(uint16_t opcode, uint8_t *p_data, uint32_t length);
extern void mesh_light_lightness_client_message_handler(uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);
#endif

#ifdef WICED_BT_MESH_MODEL_LIGHT_HSL_CLIENT_INCLUDED
uint32_t mesh_light_hsl_client_proc_rx_cmd(uint16_t opcode, uint8_t *p_data, uint32_t length);
extern void mesh_light_hsl_client_message_handler(uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);
#endif

#ifdef WICED_BT_MESH_MODEL_LIGHT_CTL_CLIENT_INCLUDED
uint32_t mesh_light_ctl_client_proc_rx_cmd(uint16_t opcode, uint8_t *p_data, uint32_t length);
extern void mesh_light_ctl_client_message_handler(uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);
#endif

#ifdef WICED_BT_MESH_MODEL_SENSOR_CLIENT_INCLUDED
uint32_t mesh_sensor_client_proc_rx_cmd(uint16_t opcode, uint8_t *p_data, uint32_t length);
extern void mesh_sensor_client_message_handler(uint16_t event, wiced_bt_mesh_event_t* p_event, void* p_data);
#endif

#ifdef WICED_BT_MESH_MODEL_SCENE_CLIENT_INCLUDED
uint32_t mesh_scene_client_proc_rx_cmd(uint16_t opcode, uint8_t *p_data, uint32_t length);
extern void mesh_scene_client_message_handler(uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);
#endif

uint32_t mesh_vendor_client_proc_rx_cmd(uint16_t opcode, uint8_t *p_data, uint32_t length);
wiced_bool_t mesh_gatt_client_local_device_set(wiced_bt_mesh_local_device_set_data_t *p_data);

/******************************************************
 *          Constants
 ******************************************************/
#define MESH_PID                0x301D
#define MESH_VID                0x0002
#define MESH_APP_RPL_DELAY      30        // Value is seconds. Use RPL = 0 to update immediately so that message cannot be replayed
/******************************************************
 *          Structures
 ******************************************************/

/******************************************************
 *          Function Prototypes
 ******************************************************/
static void mesh_app_init(wiced_bool_t is_provisioned);
static uint32_t mesh_app_proc_rx_cmd(uint16_t opcode, uint8_t *p_data, uint32_t length);
static void mesh_config_client_message_handler(uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data);
static uint8_t mesh_provisioner_process_set_local_device(uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_add_vendor_model(uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_set_dev_key(uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_set_adv_tx_power(uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_connect(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_disconnect(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_start(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_oob_value(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_search_proxy(uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_proxy_connect(uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_proxy_disconnect(uint8_t *p_data, uint32_t length);
#ifdef OPCODES_AGGREGATOR_SUPPORTED
static uint8_t mesh_provisioner_process_aggregator_start(uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_aggregator_finish(uint8_t *p_data, uint32_t length);
#endif
static uint8_t mesh_provisioner_process_raw_model_data(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_scan_capabilities_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_scan_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_scan_start(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_scan_stop(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_extended_scan_start(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_node_reset(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_beacon_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_beacon_set(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_composition_data_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_default_ttl_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_default_ttl_set(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_gatt_proxy_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_gatt_proxy_set(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
#ifdef DIRECTED_FORWARDING_SERVER_SUPPORTED
static uint8_t mesh_provisioner_process_df_directed_control_get(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length);
static uint8_t mesh_provisioner_process_df_directed_control_set(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length);
static uint8_t mesh_provisioner_process_df_path_metric_get(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length);
static uint8_t mesh_provisioner_process_df_path_metric_set(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length);
static uint8_t mesh_provisioner_process_df_discovery_table_capabilities_get(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length);
static uint8_t mesh_provisioner_process_df_discovery_table_capabilities_set(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length);
static uint8_t mesh_provisioner_process_df_forwarding_table_add(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length);
static uint8_t mesh_provisioner_process_df_forwarding_table_delete(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length);
static uint8_t mesh_provisioner_process_df_forwarding_table_dependents_add(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length);
static uint8_t mesh_provisioner_process_df_forwarding_table_dependents_delete(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length);
static uint8_t mesh_provisioner_process_df_forwarding_table_dependents_get(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length);
static uint8_t mesh_provisioner_process_df_forwarding_table_entries_count_get(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length);
static uint8_t mesh_provisioner_process_df_forwarding_table_entries_get(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length);
static uint8_t mesh_provisioner_process_df_wanted_lanes_get(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length);
static uint8_t mesh_provisioner_process_df_wanted_lanes_set(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length);
static uint8_t mesh_provisioner_process_df_two_way_path_get(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length);
static uint8_t mesh_provisioner_process_df_two_way_path_set(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length);
static uint8_t mesh_provisioner_process_df_path_echo_interval_get(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length);
static uint8_t mesh_provisioner_process_df_path_echo_interval_set(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length);
static uint8_t mesh_provisioner_process_df_directed_network_transmit_get(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length);
static uint8_t mesh_provisioner_process_df_directed_network_transmit_set(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length);
static uint8_t mesh_provisioner_process_df_directed_relay_retransmit_get(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length);
static uint8_t mesh_provisioner_process_df_directed_relay_retransmit_set(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length);
static uint8_t mesh_provisioner_process_df_rssi_threshold_get(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length);
static uint8_t mesh_provisioner_process_df_rssi_threshold_set(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length);
static uint8_t mesh_provisioner_process_df_directed_paths_get(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length);
static uint8_t mesh_provisioner_process_df_directed_publish_policy_get(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length);
static uint8_t mesh_provisioner_process_df_directed_publish_policy_set(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length);
static uint8_t mesh_provisioner_process_df_path_discovery_timing_control_get(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length);
static uint8_t mesh_provisioner_process_df_path_discovery_timing_control_set(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length);
static uint8_t mesh_provisioner_process_df_directed_control_network_transmit_get(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length);
static uint8_t mesh_provisioner_process_df_directed_control_network_transmit_set(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length);
static uint8_t mesh_provisioner_process_df_directed_control_relay_retransmit_get(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length);
static uint8_t mesh_provisioner_process_df_directed_control_relay_retransmit_set(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length);
#endif
#ifdef NETWORK_FILTER_SERVER_SUPPORTED
static uint8_t mesh_provisioner_process_network_filter_get(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length);
static uint8_t mesh_provisioner_process_network_filter_set(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length);
#endif
#ifdef LARGE_COMPOSITION_DATA_SUPPORTED
uint8_t mesh_provisioner_process_large_compos_data_get(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length);
uint8_t mesh_provisioner_process_models_metadata_get(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length);
#endif
#ifdef SAR_CONFIGURATION_SUPPORTED
static uint8_t mesh_provisioner_process_sar_transmitter_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_sar_transmitter_set(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_sar_receiver_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_sar_receiver_set(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
#endif
#ifdef PRIVATE_PROXY_SUPPORTED
static uint8_t mesh_provisioner_process_private_beacon_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_private_beacon_set(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_private_gatt_proxy_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_private_gatt_proxy_set(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_private_node_identity_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_private_node_identity_set(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_on_demand_private_proxy_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_on_demand_private_proxy_set(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_solicitation_pdu_rpl_items_clear(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_send_solicitation_pdu(uint8_t *p_data, uint32_t length);
#endif
#ifdef MESH_DFU_SUPPORTED
uint8_t mesh_provisioner_process_fw_upload_start(uint8_t *p_data, uint32_t length);
uint8_t mesh_provisioner_process_fw_upload_data(uint8_t *p_data, uint32_t length);
uint8_t mesh_provisioner_process_fw_upload_finish(uint8_t *p_data, uint32_t length);
uint8_t mesh_provisioner_process_fw_update_metadata_check(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
uint8_t mesh_provisioner_process_fw_distribution_start(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
uint8_t mesh_provisioner_process_fw_distribution_suspend(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
uint8_t mesh_provisioner_process_fw_distribution_resume(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
uint8_t mesh_provisioner_process_fw_distribution_stop(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
uint8_t mesh_provisioner_process_fw_distribution_get_status(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
#endif
static uint8_t mesh_provisioner_process_relay_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_relay_set(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_friend_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_friend_set(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_key_refresh_phase_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_key_refresh_phase_set(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_node_identity_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_node_identity_set(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_model_publication_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_model_publication_set(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_model_subscription_change(wiced_bt_mesh_event_t *p_event, uint8_t opcode, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_model_subscription_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_netkey_change(wiced_bt_mesh_event_t *p_event, uint8_t operation, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_netkey_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_appkey_change(wiced_bt_mesh_event_t *p_event, uint8_t operation, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_appkey_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_model_app_change(wiced_bt_mesh_event_t *p_event, uint8_t operation, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_model_app_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_heartbeat_subscription_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_heartbeat_subscription_set(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_heartbeat_publication_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_heartbeat_publication_set(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_network_transmit_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_health_fault_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_health_fault_clear(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_health_fault_test(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_health_period_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_health_period_set(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_health_attention_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_health_attention_set(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_lpn_poll_timeout_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_network_transmit_set(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_proxy_filter_type_set(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_proxy_filter_change(wiced_bt_mesh_event_t *p_event, wiced_bool_t is_add, uint8_t *p_data, uint32_t length);
static void mesh_provisioner_hci_event_provision_end_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_provision_status_data_t *p_data);
void mesh_provisioner_hci_event_scan_report_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_provision_scan_report_data_t *p_data);
void mesh_provisioner_hci_event_scan_extended_report_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_provision_scan_extended_report_data_t *p_data);
void mesh_provisioner_hci_event_proxy_device_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_proxy_device_network_data_t *p_data);
void mesh_provisioner_hci_event_provision_link_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_provision_link_status_data_t *p_data);
void mesh_provisioner_hci_event_provision_link_report_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_provision_link_report_data_t *p_data);
static void mesh_provisioner_hci_event_device_capabilities_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_provision_device_capabilities_data_t *p_data);
static void mesh_provisioner_hci_event_device_get_oob_data_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_provision_device_oob_request_data_t *p_data);
static void mesh_provisioner_hci_event_proxy_connection_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_connect_status_data_t *p_data);
static void mesh_provisioner_hci_event_node_reset_status_send(wiced_bt_mesh_hci_event_t *p_hci_event);
static void mesh_provisioner_hci_event_node_identity_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_node_identity_status_data_t *p_data);
static void mesh_provisioner_hci_event_composition_data_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_composition_data_status_data_t *p_data);
static void mesh_provisioner_hci_event_friend_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_friend_status_data_t *p_data);
static void mesh_provisioner_hci_event_key_refresh_phase_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_key_refresh_phase_status_data_t *p_data);
static void mesh_provisioner_hci_event_default_ttl_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_default_ttl_status_data_t *p_data);
static void mesh_provisioner_hci_event_relay_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_relay_status_data_t *p_data);
static void mesh_provisioner_hci_event_gatt_proxy_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_gatt_proxy_status_data_t *p_data);
static void mesh_provisioner_hci_event_beacon_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_beacon_status_data_t *p_data);
#ifdef SAR_CONFIGURATION_SUPPORTED
static void mesh_provisioner_hci_event_sar_transmitter_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_sar_xmtr_t *p_data);
static void mesh_provisioner_hci_event_sar_receiver_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_sar_rcvr_t *p_data);
#endif
#ifdef PRIVATE_PROXY_SUPPORTED
static void mesh_provisioner_hci_event_private_beacon_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_private_beacon_status_data_t *p_data);
static void mesh_provisioner_hci_event_private_gatt_proxy_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_private_gatt_proxy_status_data_t *p_data);
static void mesh_provisioner_hci_event_private_node_identity_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_private_node_identity_status_data_t *p_data);
static void mesh_provisioner_hci_event_on_demand_private_proxy_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_on_demand_private_proxy_status_data_t *p_data);
static void mesh_provisioner_hci_event_solicitation_pdu_rpl_items_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_unicast_address_range_t* p_data);
#endif
static void mesh_provisioner_hci_event_model_publication_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_model_publication_status_data_t *p_data);
static void mesh_provisioner_hci_event_model_subscription_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_model_subscription_status_data_t *p_data);
static void mesh_provisioner_hci_event_model_subscription_list_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_model_subscription_list_data_t *p_data);
static void mesh_provisioner_hci_event_netkey_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_netkey_status_data_t *p_data);
static void mesh_provisioner_hci_event_netkey_list_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_netkey_list_data_t *p_data);
static void mesh_provisioner_hci_event_appkey_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_appkey_status_data_t *p_data);
static void mesh_provisioner_hci_event_appkey_list_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_appkey_list_data_t *p_data);
static void mesh_provisioner_hci_event_model_app_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_model_app_bind_status_data_t *p_data);
static void mesh_provisioner_hci_event_model_app_list_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_model_app_list_data_t *p_data);
static void mesh_provisioner_hci_event_hearbeat_subscription_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_heartbeat_subscription_status_data_t *p_data);
static void mesh_provisioner_hci_event_hearbeat_publication_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_heartbeat_publication_status_data_t *p_data);
static void mesh_provisioner_hci_event_network_transmit_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_network_transmit_status_data_t *p_data);
static void mesh_provisioner_hci_event_health_current_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_health_fault_status_data_t *p_data);
static void mesh_provisioner_hci_event_health_fault_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_health_fault_status_data_t *p_data);
static void mesh_provisioner_hci_event_health_period_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_health_period_status_data_t *p_data);
static void mesh_provisioner_hci_event_health_attention_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_health_attention_status_data_t *p_data);
static void mesh_provisioner_hci_event_lpn_poll_timeout_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_lpn_poll_timeout_status_data_t *p_data);
static void mesh_provisioner_hci_event_proxy_filter_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_proxy_filter_status_data_t *p_data);
static void mesh_provisioner_hci_event_scan_capabilities_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_provision_scan_capabilities_status_data_t *p_data);
static void mesh_provisioner_hci_event_scan_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_provision_scan_status_data_t *p_data);

static void mesh_provisioner_hci_send_status(uint8_t status);

#ifdef DIRECTED_FORWARDING_SERVER_SUPPORTED
static void mesh_provisioner_hci_event_df_directed_control_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_df_directed_control_status_data_t* p_data);
static void mesh_provisioner_hci_event_df_path_metric_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_df_path_metric_status_data_t* p_data);
static void mesh_provisioner_hci_event_df_discovery_table_capabilities_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_df_discovery_table_capabilities_status_data_t* p_data);
static void mesh_provisioner_hci_event_df_forwarding_table_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_df_forwarding_table_status_data_t* p_data);
static void mesh_provisioner_hci_event_df_forwarding_table_dependents_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_df_forwarding_table_dependents_status_data_t* p_data);
static void mesh_provisioner_hci_event_df_forwarding_table_dependents_get_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_df_forwarding_table_dependents_get_status_data_t* p_data);
static void mesh_provisioner_hci_event_df_forwarding_table_entries_count_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_df_forwarding_table_entries_count_status_data_t* p_data);
static void mesh_provisioner_hci_event_df_forwarding_table_entries_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_df_forwarding_table_entries_status_data_t* p_data);
static void mesh_provisioner_hci_event_df_wanted_lanes_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_df_wanted_lanes_status_data_t* p_data);
static void mesh_provisioner_hci_event_df_two_way_path_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_df_two_way_path_status_data_t* p_data);
static void mesh_provisioner_hci_event_df_path_echo_interval_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_df_path_echo_interval_status_data_t* p_data);
static void mesh_provisioner_hci_event_df_directed_network_transmit_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_df_directed_transmit_status_data_t* p_data);
static void mesh_provisioner_hci_event_df_directed_relay_retransmit_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_df_directed_transmit_status_data_t* p_data);
static void mesh_provisioner_hci_event_df_rssi_threshold_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_df_rssi_threshold_status_data_t* p_data);
static void mesh_provisioner_hci_event_df_directed_paths_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_df_directed_paths_status_data_t* p_data);
static void mesh_provisioner_hci_event_df_directed_publish_policy_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_df_directed_publish_policy_status_data_t* p_data);
static void mesh_provisioner_hci_event_df_path_discovery_timing_control_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_df_path_discovery_timing_control_status_data_t* p_data);
static void mesh_provisioner_hci_event_df_directed_control_network_transmit_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_df_directed_transmit_status_data_t* p_data);
static void mesh_provisioner_hci_event_df_directed_control_relay_retransmit_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_df_directed_transmit_status_data_t* p_data);
#endif

#if defined(CERTIFICATE_BASED_PROVISIONING_SUPPORTED)
static void mesh_provisioner_hci_event_record_list(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_core_provisioning_list_t *p_data);
static void mesh_provisioner_hci_event_record_response(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_core_provisioning_record_t *p_data);

static uint8_t mesh_provisioner_process_send_invite(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
static uint8_t mesh_provisioner_process_record_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length);
#endif

#ifdef OPCODES_AGGREGATOR_SUPPORTED
static void mesh_provisioner_hci_agg_item_add_status_send(uint8_t status);
#endif

#ifdef NETWORK_FILTER_SERVER_SUPPORTED
static void mesh_provisioner_hci_event_network_filter_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_network_filter_status_data_t* p_data);
#endif

#ifdef LARGE_COMPOSITION_DATA_SUPPORTED
static void mesh_provisioner_hci_event_large_compos_data_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_large_compos_data_status_data_t *p_data);
static void mesh_provisioner_hci_event_models_metadata_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_models_metadata_status_data_t *p_data);
#endif

#ifdef MESH_DFU_SUPPORTED
void mesh_provisioner_hci_event_fw_distribution_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_fw_distribution_status_data_t *p_data);
void mesh_provisioner_hci_event_fw_update_metadata_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_dfu_metadata_status_data_t *p_data);
#endif

/******************************************************
 *          Variables Definitions
 ******************************************************/
uint8_t mesh_mfr_name[WICED_BT_MESH_PROPERTY_LEN_DEVICE_MANUFACTURER_NAME] = { 'C', 'y', 'p', 'r', 'e', 's', 's', 0 };
uint8_t mesh_model_num[WICED_BT_MESH_PROPERTY_LEN_DEVICE_MODEL_NUMBER]     = { '1', '2', '3', '4', 0, 0, 0, 0 };
uint8_t mesh_system_id[8]                                                  = { 0xbb, 0xb8, 0xa1, 0x80, 0x5f, 0x9f, 0x91, 0x71 };

extern wiced_transport_buffer_pool_t* host_trans_pool;
// Application can change TX power. 0 is the minimum
extern uint8_t wiced_bt_mesh_core_adv_tx_power;

/*
* Structure to save vendor-specific model data
*/
typedef struct
{
    uint8_t element_index;
    uint16_t company_id;
    uint16_t model_id;
    uint8_t num_opcodes;
    uint8_t *p_opcodes;
} wiced_bt_mesh_vendor_specific_model_t;

#define MESH_APP_MESH_MAX_VENDOR_MODELS 10
static wiced_bt_mesh_vendor_specific_model_t vendor_model_data[MESH_APP_MESH_MAX_VENDOR_MODELS] = {0};

wiced_bool_t mesh_vendor_client_message_handler(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint16_t data_len);

wiced_bt_mesh_core_config_model_t   mesh_element1_models[] =
{
    WICED_BT_MESH_DEVICE,

#ifdef DIRECTED_FORWARDING_SERVER_SUPPORTED
    WICED_BT_MESH_DIRECTED_FORWARDING_CLIENT,
#endif
#ifdef NETWORK_FILTER_SERVER_SUPPORTED
    WICED_BT_MESH_NETWORK_FILTER_CLIENT,
#endif
#ifdef LARGE_COMPOSITION_DATA_SUPPORTED
    WICED_BT_MESH_MODEL_LARGE_COMPOS_DATA_CLIENT,
#endif
#ifdef WICED_BT_MESH_MODEL_PROPERTY_CLIENT_INCLUDED
    WICED_BT_MESH_MODEL_PROPERTY_CLIENT,
#endif
#ifdef WICED_BT_MESH_MODEL_BATTERY_CLIENT_INCLUDED
    WICED_BT_MESH_MODEL_BATTERY_CLIENT,
#endif
#ifdef WICED_BT_MESH_MODEL_LIGHT_LC_CLIENT_INCLUDED
    WICED_BT_MESH_MODEL_LIGHT_LC_CLIENT,
#endif
#ifdef WICED_BT_MESH_MODEL_LIGHT_XYL_CLIENT_INCLUDED
    WICED_BT_MESH_MODEL_LIGHT_XYL_CLIENT,
#endif
#ifdef WICED_BT_MESH_MODEL_LOCATION_CLIENT_INCLUDED
    WICED_BT_MESH_MODEL_LOCATION_CLIENT,
#endif
#ifdef WICED_BT_MESH_MODEL_POWER_LEVEL_CLIENT_INCLUDED
    WICED_BT_MESH_MODEL_POWER_LEVEL_CLIENT,
#endif
#ifdef WICED_BT_MESH_MODEL_POWER_ONOFF_CLIENT_INCLUDED
    WICED_BT_MESH_MODEL_POWER_ONOFF_CLIENT,
#endif
#ifdef WICED_BT_MESH_MODEL_SCHEDULER_CLIENT_INCLUDED
    WICED_BT_MESH_MODEL_SCHEDULER_CLIENT,
#endif
#ifdef WICED_BT_MESH_MODEL_TIME_CLIENT_INCLUDED
    WICED_BT_MESH_MODEL_TIME_CLIENT,
#endif
    WICED_BT_MESH_MODEL_CONFIG_CLIENT,
    WICED_BT_MESH_MODEL_HEALTH_CLIENT,
    WICED_BT_MESH_MODEL_REMOTE_PROVISION_SERVER,
    WICED_BT_MESH_MODEL_REMOTE_PROVISION_CLIENT,
    WICED_BT_MESH_MODEL_DEFAULT_TRANSITION_TIME_CLIENT,
#ifdef SAR_CONFIGURATION_SUPPORTED
    WICED_BT_MESH_MODEL_SAR_CONFIG_CLIENT,
#ifdef PTS
    WICED_BT_MESH_MODEL_SAR_CONFIG_SERVER,
#endif
#endif
#ifdef PRIVATE_PROXY_SUPPORTED
    WICED_BT_MESH_MODEL_PRIVATE_PROXY_CLIENT,
#endif
#ifdef OPCODES_AGGREGATOR_SUPPORTED
    WICED_BT_MESH_MODEL_OPCODES_AGGREGATOR_CLIENT,
#endif
#ifdef MESH_DFU_SUPPORTED
    WICED_BT_MESH_MODEL_FW_STANDALONE_UPDATER,
#endif
#ifdef WICED_BT_MESH_MODEL_SENSOR_CLIENT_INCLUDED
    WICED_BT_MESH_MODEL_SENSOR_CLIENT,
#endif
#ifdef WICED_BT_MESH_MODEL_SCENE_CLIENT_INCLUDED
    WICED_BT_MESH_MODEL_SCENE_CLIENT,
#endif
#ifdef WICED_BT_MESH_MODEL_ONOFF_CLIENT_INCLUDED
    WICED_BT_MESH_MODEL_ONOFF_CLIENT,
#endif
#ifdef WICED_BT_MESH_MODEL_LEVEL_CLIENT_INCLUDED
    WICED_BT_MESH_MODEL_LEVEL_CLIENT,
#endif
#ifdef WICED_BT_MESH_MODEL_LIGHT_LIGHTNESS_CLIENT_INCLUDED
    WICED_BT_MESH_MODEL_LIGHT_LIGHTNESS_CLIENT,
#endif
#ifdef WICED_BT_MESH_MODEL_LIGHT_CTL_CLIENT_INCLUDED
    WICED_BT_MESH_MODEL_LIGHT_CTL_CLIENT,
#endif
#ifdef WICED_BT_MESH_MODEL_LIGHT_HSL_CLIENT_INCLUDED
    WICED_BT_MESH_MODEL_LIGHT_HSL_CLIENT,
#endif
    // This a client which assume to know all the keys, so we will allow
    // vendor commands from all the companies.
    { MESH_COMPANY_ID_UNUSED, 0, mesh_vendor_client_message_handler, NULL, NULL },
    { MESH_COMPANY_ID_UNUSED, 0, mesh_vendor_client_message_handler, NULL, NULL },
    { MESH_COMPANY_ID_UNUSED, 0, mesh_vendor_client_message_handler, NULL, NULL },
    { MESH_COMPANY_ID_UNUSED, 0, mesh_vendor_client_message_handler, NULL, NULL },
    { MESH_COMPANY_ID_UNUSED, 0, mesh_vendor_client_message_handler, NULL, NULL },
    { MESH_COMPANY_ID_UNUSED, 0, mesh_vendor_client_message_handler, NULL, NULL },
    { MESH_COMPANY_ID_UNUSED, 0, mesh_vendor_client_message_handler, NULL, NULL },
    { MESH_COMPANY_ID_UNUSED, 0, mesh_vendor_client_message_handler, NULL, NULL },
    { MESH_COMPANY_ID_UNUSED, 0, mesh_vendor_client_message_handler, NULL, NULL },
    { MESH_COMPANY_ID_UNUSED, 0, mesh_vendor_client_message_handler, NULL, NULL },
};

#define MESH_APP_NUM_MODELS  (sizeof(mesh_element1_models) / sizeof(wiced_bt_mesh_core_config_model_t))

#define MESH_PROVISIONER_CLIENT_ELEMENT_INDEX   0

wiced_bt_mesh_core_config_element_t mesh_elements[] =
{
    {
        .location = MESH_ELEM_LOC_MAIN,                                 // location description as defined in the GATT Bluetooth Namespace Descriptors section of the Bluetooth SIG Assigned Numbers
        .default_transition_time = MESH_DEFAULT_TRANSITION_TIME_IN_MS,  // Default transition time for models of the element in milliseconds
        .onpowerup_state = WICED_BT_MESH_ON_POWER_UP_STATE_RESTORE,     // Default element behavior on power up
        .default_level = 0,                                             // Default value of the variable controlled on this element (for example power, lightness, temperature, hue...)
        .range_min = 1,                                                 // Minimum value of the variable controlled on this element (for example power, lightness, temperature, hue...)
        .range_max = 0xffff,                                            // Maximum value of the variable controlled on this element (for example power, lightness, temperature, hue...)
        .move_rollover = 0,                                             // If true when level gets to range_max during move operation, it switches to min, otherwise move stops.
        .properties_num = 0,                                            // Number of properties in the array models
        .properties = NULL,                                             // Array of properties in the element.
        .sensors_num = 0,                                               // Number of sensors in the sensor array
        .sensors = NULL,                                                // Array of sensors of that element
        .models_num = MESH_APP_NUM_MODELS,                              // Number of models in the array models
        .models = mesh_element1_models,                                 // Array of models located in that element. Model data is defined by structure wiced_bt_mesh_core_config_model_t
    },
};

wiced_bt_mesh_core_config_t  mesh_config =
{
    .company_id         = MESH_COMPANY_ID_CYPRESS,                  // Company identifier assigned by the Bluetooth SIG
    .product_id         = MESH_PID,                                 // Vendor-assigned product identifier
    .vendor_id          = MESH_VID,                                 // Vendor-assigned product version identifier
    .features                  = 0,                                 // no relay, no friend, no proxy
    .friend_cfg         =                                           // Empty Configuration of the Friend Feature
    {
        .receive_window        = 0,                                 // Receive Window value in milliseconds supported by the Friend node.
        .cache_buf_len  = 0,                                        // Length of the buffer for the cache
        .max_lpn_num    = 0                                         // Max number of Low Power Nodes with established friendship. Must be > 0 if Friend feature is supported.
    },
    .low_power          =                                           // Configuration of the Low Power Feature
    {
        .rssi_factor           = 0,                                 // contribution of the RSSI measured by the Friend node used in Friend Offer Delay calculations.
        .receive_window_factor = 0,                                 // contribution of the supported Receive Window used in Friend Offer Delay calculations.
        .min_cache_size_log    = 0,                                 // minimum number of messages that the Friend node can store in its Friend Cache.
        .receive_delay         = 0,                                 // Receive delay in 1 ms units to be requested by the Low Power node.
        .poll_timeout          = 0                                  // Poll timeout in 100ms units to be requested by the Low Power node.
    },
    .gatt_client_only          = WICED_FALSE,                       // Can connect to mesh over GATT or ADV
    .elements_num  = (uint8_t)(sizeof(mesh_elements) / sizeof(mesh_elements[0])),   // number of elements on this device
    .elements      = mesh_elements                                  // Array of elements for this device
};

/*
 * Mesh application library will call into application functions if provided by the application.
 */
wiced_bt_mesh_app_func_table_t wiced_bt_mesh_app_func_table =
{
    mesh_app_init,          // application initialization
    NULL,                   // Default SDK platform button processing
    NULL,                   // GATT connection status
    NULL,                   // attention processing
    NULL,                   // notify period set
    mesh_app_proc_rx_cmd,   // WICED HCI command
    NULL,                   // LPN sleep
    NULL                    // factory reset
};

// This application is the only one that can connect to the mesh over proxy and
// need to process proxy status messages.  It needs to fill the pointer in the mesh_application.c.
extern void *p_proxy_status_message_handler;
extern void mesh_proxy_client_process_filter_status(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint16_t data_len);


/******************************************************
 *               Function Definitions
 ******************************************************/
void mesh_app_init(wiced_bool_t is_provisioned)
{
#if 0
    // Set Debug trace level for mesh_models_lib and mesh_provisioner_lib
    wiced_bt_mesh_models_set_trace_level(WICED_BT_MESH_CORE_TRACE_INFO);
#endif
#if 0
    // Set Debug trace level for all modules but Info level for CORE_AES_CCM module
    wiced_bt_mesh_core_set_trace_level(WICED_BT_MESH_CORE_TRACE_FID_ALL, WICED_BT_MESH_CORE_TRACE_DEBUG);
    wiced_bt_mesh_core_set_trace_level(WICED_BT_MESH_CORE_TRACE_FID_CORE_AES_CCM, WICED_BT_MESH_CORE_TRACE_INFO);
#endif

#if 0
    // App can set TX power here. 0 means minimum power and 4 is the max. Actual power table is on the controller.
    WICED_BT_TRACE("tx_power:%d to 0\n", wiced_bt_mesh_core_adv_tx_power);
    wiced_bt_mesh_core_adv_tx_power = 0;
#endif

    wiced_bt_cfg_settings.device_name = (uint8_t *)"Provisioner Client";
    wiced_bt_cfg_settings.gatt_cfg.appearance = APPEARANCE_GENERIC_TAG;
    // Adv Data is fixed. Spec allows to put URI, Name, Appearance and Tx Power in the Scan Response Data.
    if (!is_provisioned)
    {
        wiced_bt_ble_advert_elem_t  adv_elem[3];
        uint8_t                     buf[2];
        uint8_t                     num_elem = 0;
        adv_elem[num_elem].advert_type = BTM_BLE_ADVERT_TYPE_NAME_COMPLETE;
        adv_elem[num_elem].len = (uint16_t)strlen((const char*)wiced_bt_cfg_settings.device_name);
        adv_elem[num_elem].p_data = wiced_bt_cfg_settings.device_name;
        num_elem++;

        adv_elem[num_elem].advert_type = BTM_BLE_ADVERT_TYPE_APPEARANCE;
        adv_elem[num_elem].len = 2;
        buf[0] = (uint8_t)wiced_bt_cfg_settings.gatt_cfg.appearance;
        buf[1] = (uint8_t)(wiced_bt_cfg_settings.gatt_cfg.appearance >> 8);
        adv_elem[num_elem].p_data = buf;
        num_elem++;

        wiced_bt_mesh_set_raw_scan_response_data(num_elem, adv_elem);
    }
#ifdef NETWORK_FILTER_SERVER_SUPPORTED
    if (is_provisioned)
        wiced_bt_mesh_network_filter_init();
#endif

    wiced_bt_mesh_remote_provisioning_server_init();

    // for the embedded client we are already provisioned because
    // host should know everything and setup local device configuration through WICED HCI commands
    wiced_bt_mesh_provision_client_init(mesh_config_client_message_handler, is_provisioned);
    wiced_bt_mesh_client_init(mesh_config_client_message_handler, is_provisioned);

    wiced_bt_mesh_config_client_init(mesh_config_client_message_handler, is_provisioned);
    wiced_bt_mesh_health_client_init(mesh_config_client_message_handler, is_provisioned);
    wiced_bt_mesh_proxy_client_init(mesh_config_client_message_handler, is_provisioned);

    wiced_bt_mesh_model_default_transition_time_client_init(0, mesh_default_transition_time_client_message_handler, is_provisioned);

#ifdef WICED_BT_MESH_MODEL_ONOFF_CLIENT_INCLUDED
    wiced_bt_mesh_model_onoff_client_init(0, mesh_onoff_client_message_handler, is_provisioned);
#endif
#ifdef WICED_BT_MESH_MODEL_LEVEL_CLIENT_INCLUDED
    wiced_bt_mesh_model_level_client_init(0, mesh_level_client_message_handler, is_provisioned);
#endif
#ifdef WICED_BT_MESH_MODEL_LIGHT_LIGHTNESS_CLIENT_INCLUDED
    wiced_bt_mesh_model_light_lightness_client_init(0, mesh_light_lightness_client_message_handler, is_provisioned);
#endif
#ifdef WICED_BT_MESH_MODEL_LIGHT_CTL_CLIENT_INCLUDED
    wiced_bt_mesh_model_light_ctl_client_init(0, mesh_light_ctl_client_message_handler, is_provisioned);
#endif
#ifdef WICED_BT_MESH_MODEL_LIGHT_HSL_CLIENT_INCLUDED
    wiced_bt_mesh_model_light_hsl_client_init(0, mesh_light_hsl_client_message_handler, is_provisioned);
#endif
#ifdef WICED_BT_MESH_MODEL_SENSOR_CLIENT_INCLUDED
    wiced_bt_mesh_model_sensor_client_init(0, mesh_sensor_client_message_handler, is_provisioned);
#endif
#ifdef WICED_BT_MESH_MODEL_SCENE_CLIENT_INCLUDED
    wiced_bt_mesh_model_scene_client_init(0, mesh_scene_client_message_handler, is_provisioned);
#endif
#ifdef WICED_BT_MESH_MODEL_PROPERTY_CLIENT_INCLUDED
    wiced_bt_mesh_model_property_client_init(0, mesh_property_client_message_handler, is_provisioned);
#endif
#ifdef WICED_BT_MESH_MODEL_BATTERY_CLIENT_INCLUDED
    wiced_bt_mesh_model_battery_client_init(mesh_battery_client_message_handler, is_provisioned);
#endif
#ifdef WICED_BT_MESH_MODEL_LIGHT_LC_CLIENT_INCLUDED
    wiced_bt_mesh_model_light_lc_client_init(0, mesh_light_lc_client_message_handler, is_provisioned);
#endif
#ifdef WICED_BT_MESH_MODEL_LIGHT_XYL_CLIENT_INCLUDED
    wiced_bt_mesh_model_light_xyl_client_init(0, mesh_light_xyl_client_message_handler, is_provisioned);
#endif
#ifdef WICED_BT_MESH_MODEL_LOCATION_CLIENT_INCLUDED
    wiced_bt_mesh_model_location_client_init(mesh_location_client_message_handler, is_provisioned);
#endif
#ifdef WICED_BT_MESH_MODEL_POWER_LEVEL_CLIENT_INCLUDED
    wiced_bt_mesh_model_power_level_client_init(0, mesh_power_level_client_message_handler, is_provisioned);
#endif
#ifdef WICED_BT_MESH_MODEL_POWER_ONOFF_CLIENT_INCLUDED
    wiced_bt_mesh_model_power_onoff_client_init(0, mesh_power_onoff_client_message_handler, is_provisioned);
#endif
#ifdef WICED_BT_MESH_MODEL_SCHEDULER_CLIENT_INCLUDED
    wiced_bt_mesh_model_scheduler_client_init(mesh_scheduler_client_message_handler, is_provisioned);
#endif
#ifdef WICED_BT_MESH_MODEL_TIME_CLIENT_INCLUDED
    wiced_bt_mesh_model_time_client_init(mesh_time_client_message_handler, is_provisioned);
#endif
#ifdef MESH_DFU_SUPPORTED
    wiced_bt_mesh_model_fw_distribution_server_init();
#endif

    p_proxy_status_message_handler = mesh_proxy_client_process_filter_status;
}

#ifdef PTS
// Add this to enable retransmit cancellation
wiced_bt_mesh_event_t *p_out_event = NULL;
#endif

wiced_bool_t mesh_model_raw_data_message_handler(wiced_bt_mesh_event_t *p_event, const uint8_t *params, uint16_t params_len)
{
    wiced_bt_mesh_hci_event_t *p_hci_event = wiced_bt_mesh_create_hci_event(p_event);
    uint8_t *p = p_hci_event->data;
    if (p_hci_event == NULL)
    {
        WICED_BT_TRACE("model raw data no mem\n");
        return WICED_FALSE;
    }
    WICED_BT_TRACE("model raw data opcode:%04x\n", p_event->opcode);

#ifdef PTS
    // If we are still retransmitting cancel transmit
    wiced_bt_mesh_models_utils_cancel_send(&p_out_event, p_event->src);
#endif

    if (p_event->company_id == MESH_COMPANY_ID_BT_SIG)
    {
        *p++ = (p_event->opcode >> 8) & 0xff;
        *p++ = p_event->opcode & 0xff;
    }
    else
    {
#define MESH_APP_PAYLOAD_OP_LONG              0x80
#define MESH_APP_PAYLOAD_OP_MANUF_SPECIFIC    0x40
        *p++ = (p_event->opcode | MESH_APP_PAYLOAD_OP_LONG | MESH_APP_PAYLOAD_OP_MANUF_SPECIFIC) & 0xff;
        *p++ = (p_event->company_id >> 8) & 0xff;
        *p++ = p_event->company_id & 0xff;
    }
    memcpy(p, params, params_len);
    p += params_len;

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_RAW_MODEL_DATA, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
    wiced_bt_mesh_release_event(p_event);
    return WICED_TRUE;
}

/*
 * Process event received from the Configuration Server.
 */
void mesh_config_client_message_handler(uint16_t event, wiced_bt_mesh_event_t *p_event, void *p_data)
{
    wiced_bt_mesh_hci_event_t *p_hci_event = wiced_bt_mesh_create_hci_event(p_event);
    if (p_hci_event == NULL)
    {
        WICED_BT_TRACE("config clt no mem event:%d\n", event);
        return;
    }
    WICED_BT_TRACE("config clt msg:%d\n", event);

    switch (event)
    {
    case WICED_BT_MESH_TX_COMPLETE:
        WICED_BT_TRACE("tx complete status:%d\n", p_event->status.tx_flag);
        wiced_bt_mesh_send_hci_tx_complete(p_hci_event, p_event);
        break;

#if defined(CERTIFICATE_BASED_PROVISIONING_SUPPORTED)
    case WICED_BT_MESH_DEVICE_PROVISIONING_RECORD_LIST:
        mesh_provisioner_hci_event_record_list(p_hci_event, (wiced_bt_mesh_core_provisioning_list_t *)p_data);
        break;

    case WICED_BT_MESH_DEVICE_PROVISIONING_RECORD_RESP:
        mesh_provisioner_hci_event_record_response(p_hci_event, (wiced_bt_mesh_core_provisioning_record_t *)p_data);
        break;
#endif

    case WICED_BT_MESH_CONFIG_NODE_RESET_STATUS:
        mesh_provisioner_hci_event_node_reset_status_send(p_hci_event);
        break;

    case WICED_BT_MESH_CONFIG_COMPOSITION_DATA_STATUS:
        mesh_provisioner_hci_event_composition_data_status_send(p_hci_event, (wiced_bt_mesh_config_composition_data_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_FRIEND_STATUS:
        mesh_provisioner_hci_event_friend_status_send(p_hci_event, (wiced_bt_mesh_config_friend_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_GATT_PROXY_STATUS:
        mesh_provisioner_hci_event_gatt_proxy_status_send(p_hci_event, (wiced_bt_mesh_config_gatt_proxy_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_RELAY_STATUS:
        mesh_provisioner_hci_event_relay_status_send(p_hci_event, (wiced_bt_mesh_config_relay_status_data_t *)p_data);
        break;

#ifdef DIRECTED_FORWARDING_SERVER_SUPPORTED
    case WICED_BT_MESH_DF_DIRECTED_CONTROL_STATUS:
        mesh_provisioner_hci_event_df_directed_control_status_send(p_hci_event, (wiced_bt_mesh_df_directed_control_status_data_t*)p_data);
        break;

    case WICED_BT_MESH_DF_PATH_METRIC_STATUS:
        mesh_provisioner_hci_event_df_path_metric_status_send(p_hci_event, (wiced_bt_mesh_df_path_metric_status_data_t*)p_data);
        break;

    case WICED_BT_MESH_DF_DISCOVERY_TABLE_CAPABILITIES_STATUS:
        mesh_provisioner_hci_event_df_discovery_table_capabilities_status_send(p_hci_event, (wiced_bt_mesh_df_discovery_table_capabilities_status_data_t*)p_data);
        break;

    case WICED_BT_MESH_DF_FORWARDING_TABLE_STATUS:
        mesh_provisioner_hci_event_df_forwarding_table_status_send(p_hci_event, (wiced_bt_mesh_df_forwarding_table_status_data_t*)p_data);
        break;

    case WICED_BT_MESH_DF_FORWARDING_TABLE_DEPENDENTS_STATUS:
        mesh_provisioner_hci_event_df_forwarding_table_dependents_status_send(p_hci_event, (wiced_bt_mesh_df_forwarding_table_dependents_status_data_t*)p_data);
        break;

    case WICED_BT_MESH_DF_FORWARDING_TABLE_DEPENDENTS_GET_STATUS:
        mesh_provisioner_hci_event_df_forwarding_table_dependents_get_status_send(p_hci_event, (wiced_bt_mesh_df_forwarding_table_dependents_get_status_data_t*)p_data);
        break;

    case WICED_BT_MESH_DF_FORWARDING_TABLE_ENTRIES_COUNT_STATUS:
        mesh_provisioner_hci_event_df_forwarding_table_entries_count_status_send(p_hci_event, (wiced_bt_mesh_df_forwarding_table_entries_count_status_data_t*)p_data);
        break;

    case WICED_BT_MESH_DF_FORWARDING_TABLE_ENTRIES_STATUS:
        mesh_provisioner_hci_event_df_forwarding_table_entries_status_send(p_hci_event, (wiced_bt_mesh_df_forwarding_table_entries_status_data_t*)p_data);
        break;

    case WICED_BT_MESH_DF_WANTED_LANES_STATUS:
        mesh_provisioner_hci_event_df_wanted_lanes_status_send(p_hci_event, (wiced_bt_mesh_df_wanted_lanes_status_data_t*)p_data);
        break;

    case WICED_BT_MESH_DF_TWO_WAY_PATH_STATUS:
        mesh_provisioner_hci_event_df_two_way_path_status_send(p_hci_event, (wiced_bt_mesh_df_two_way_path_status_data_t*)p_data);
        break;

    case WICED_BT_MESH_DF_PATH_ECHO_INTERVAL_STATUS:
        mesh_provisioner_hci_event_df_path_echo_interval_status_send(p_hci_event, (wiced_bt_mesh_df_path_echo_interval_status_data_t*)p_data);
        break;

    case WICED_BT_MESH_DF_DIRECTED_NETWORK_TRANSMIT_STATUS:
        mesh_provisioner_hci_event_df_directed_network_transmit_status_send(p_hci_event, (wiced_bt_mesh_df_directed_transmit_status_data_t*)p_data);
        break;

    case WICED_BT_MESH_DF_DIRECTED_RELAY_RETRANSMIT_STATUS:
        mesh_provisioner_hci_event_df_directed_relay_retransmit_status_send(p_hci_event, (wiced_bt_mesh_df_directed_transmit_status_data_t*)p_data);
        break;

    case WICED_BT_MESH_DF_RSSI_THRESHOLD_STATUS:
        mesh_provisioner_hci_event_df_rssi_threshold_status_send(p_hci_event, (wiced_bt_mesh_df_rssi_threshold_status_data_t*)p_data);
        break;

    case WICED_BT_MESH_DF_DIRECTED_PATHS_STATUS:
        mesh_provisioner_hci_event_df_directed_paths_status_send(p_hci_event, (wiced_bt_mesh_df_directed_paths_status_data_t*)p_data);
        break;

    case WICED_BT_MESH_DF_DIRECTED_PUBLISH_POLICY_STATUS:
        mesh_provisioner_hci_event_df_directed_publish_policy_status_send(p_hci_event, (wiced_bt_mesh_df_directed_publish_policy_status_data_t*)p_data);
        break;

    case WICED_BT_MESH_DF_PATH_DISCOVERY_TIMING_CONTROL_STATUS:
        mesh_provisioner_hci_event_df_path_discovery_timing_control_status_send(p_hci_event, (wiced_bt_mesh_df_path_discovery_timing_control_status_data_t*)p_data);
        break;

    case WICED_BT_MESH_DF_DIRECTED_CONTROL_NETWORK_TRANSMIT_STATUS:
        mesh_provisioner_hci_event_df_directed_control_network_transmit_status_send(p_hci_event, (wiced_bt_mesh_df_directed_transmit_status_data_t*)p_data);
        break;

    case WICED_BT_MESH_DF_DIRECTED_CONTROL_RELAY_RETRANSMIT_STATUS:
        mesh_provisioner_hci_event_df_directed_control_relay_retransmit_status_send(p_hci_event, (wiced_bt_mesh_df_directed_transmit_status_data_t*)p_data);
        break;

#endif
#ifdef NETWORK_FILTER_SERVER_SUPPORTED
    case WICED_BT_MESH_NETWORK_FILTER_STATUS:
        mesh_provisioner_hci_event_network_filter_status_send(p_hci_event, (wiced_bt_mesh_network_filter_status_data_t*)p_data);
        break;
#endif
#ifdef LARGE_COMPOSITION_DATA_SUPPORTED
    case WICED_BT_MESH_CONFIG_LARGE_COMPOS_DATA_STATUS:
        mesh_provisioner_hci_event_large_compos_data_status_send(p_hci_event, (wiced_bt_mesh_config_large_compos_data_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_MODELS_METADATA_STATUS:
        mesh_provisioner_hci_event_models_metadata_status_send(p_hci_event, (wiced_bt_mesh_config_models_metadata_status_data_t *)p_data);
        break;
#endif

    case WICED_BT_MESH_CONFIG_BEACON_STATUS:
        mesh_provisioner_hci_event_beacon_status_send(p_hci_event, (wiced_bt_mesh_config_beacon_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_DEFAULT_TTL_STATUS:
        mesh_provisioner_hci_event_default_ttl_status_send(p_hci_event, (wiced_bt_mesh_config_default_ttl_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_NODE_IDENTITY_STATUS:
        mesh_provisioner_hci_event_node_identity_status_send(p_hci_event, (wiced_bt_mesh_config_node_identity_status_data_t *)p_data);
        break;

#ifdef SAR_CONFIGURATION_SUPPORTED
    case WICED_BT_MESH_CONFIG_SAR_TRANSMITTER_STATUS:
        mesh_provisioner_hci_event_sar_transmitter_status_send(p_hci_event, (wiced_bt_mesh_sar_xmtr_t*)p_data);
        break;

    case WICED_BT_MESH_CONFIG_SAR_RECEIVER_STATUS:
        mesh_provisioner_hci_event_sar_receiver_status_send(p_hci_event, (wiced_bt_mesh_sar_rcvr_t*)p_data);
        break;
#endif

#ifdef PRIVATE_PROXY_SUPPORTED
    case WICED_BT_MESH_CONFIG_PRIVATE_BEACON_STATUS:
        mesh_provisioner_hci_event_private_beacon_status_send(p_hci_event, (wiced_bt_mesh_config_private_beacon_status_data_t*)p_data);
        break;

    case WICED_BT_MESH_CONFIG_PRIVATE_GATT_PROXY_STATUS:
        mesh_provisioner_hci_event_private_gatt_proxy_status_send(p_hci_event, (wiced_bt_mesh_config_private_gatt_proxy_status_data_t*)p_data);
        break;

    case WICED_BT_MESH_CONFIG_PRIVATE_NODE_IDENTITY_STATUS:
        mesh_provisioner_hci_event_private_node_identity_status_send(p_hci_event, (wiced_bt_mesh_config_private_node_identity_status_data_t*)p_data);
        break;

    case WICED_BT_MESH_CONFIG_ON_DEMAND_PRIVATE_PROXY_STATUS:
        mesh_provisioner_hci_event_on_demand_private_proxy_status_send(p_hci_event, (wiced_bt_mesh_config_on_demand_private_proxy_status_data_t*)p_data);
        break;

    case WICED_BT_MESH_SOLICITATION_PDU_RPL_ITEMS_STATUS:
        mesh_provisioner_hci_event_solicitation_pdu_rpl_items_status_send(p_hci_event, (wiced_bt_mesh_unicast_address_range_t*)p_data);
        break;
#endif

    case WICED_BT_MESH_CONFIG_MODEL_PUBLICATION_STATUS:
        mesh_provisioner_hci_event_model_publication_status_send(p_hci_event, (wiced_bt_mesh_config_model_publication_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_MODEL_SUBSCRIPTION_STATUS:
        mesh_provisioner_hci_event_model_subscription_status_send(p_hci_event, (wiced_bt_mesh_config_model_subscription_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_MODEL_SUBSCRIPTION_LIST:
        mesh_provisioner_hci_event_model_subscription_list_send(p_hci_event, (wiced_bt_mesh_config_model_subscription_list_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_NETKEY_STATUS:
        mesh_provisioner_hci_event_netkey_status_send(p_hci_event, (wiced_bt_mesh_config_netkey_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_KEY_REFRESH_PHASE_STATUS:
        mesh_provisioner_hci_event_key_refresh_phase_status_send(p_hci_event, (wiced_bt_mesh_config_key_refresh_phase_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_NETKEY_LIST:
        mesh_provisioner_hci_event_netkey_list_send(p_hci_event, (wiced_bt_mesh_config_netkey_list_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_APPKEY_STATUS:
        mesh_provisioner_hci_event_appkey_status_send(p_hci_event, (wiced_bt_mesh_config_appkey_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_APPKEY_LIST:
        mesh_provisioner_hci_event_appkey_list_send(p_hci_event, (wiced_bt_mesh_config_appkey_list_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_MODEL_APP_BIND_STATUS:
        mesh_provisioner_hci_event_model_app_status_send(p_hci_event, (wiced_bt_mesh_config_model_app_bind_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_MODEL_APP_BIND_LIST:
        mesh_provisioner_hci_event_model_app_list_send(p_hci_event, (wiced_bt_mesh_config_model_app_list_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_HEARBEAT_SUBSCRIPTION_STATUS:
        mesh_provisioner_hci_event_hearbeat_subscription_status_send(p_hci_event, (wiced_bt_mesh_config_heartbeat_subscription_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_HEARBEAT_PUBLICATION_STATUS:
        mesh_provisioner_hci_event_hearbeat_publication_status_send(p_hci_event, (wiced_bt_mesh_config_heartbeat_publication_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_NETWORK_TRANSMIT_STATUS:
        mesh_provisioner_hci_event_network_transmit_status_send(p_hci_event, (wiced_bt_mesh_config_network_transmit_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_CONFIG_LPN_POLL_TIMEOUT_STATUS:
        mesh_provisioner_hci_event_lpn_poll_timeout_status_send(p_hci_event, (wiced_bt_mesh_lpn_poll_timeout_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_HEALTH_CURRENT_STATUS:
        mesh_provisioner_hci_event_health_current_status_send(p_hci_event, (wiced_bt_mesh_health_fault_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_HEALTH_FAULT_STATUS:
        mesh_provisioner_hci_event_health_fault_status_send(p_hci_event, (wiced_bt_mesh_health_fault_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_HEALTH_PERIOD_STATUS:
        mesh_provisioner_hci_event_health_period_status_send(p_hci_event, (wiced_bt_mesh_health_period_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_HEALTH_ATTENTION_STATUS:
        mesh_provisioner_hci_event_health_attention_status_send(p_hci_event, (wiced_bt_mesh_health_attention_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_PROXY_FILTER_STATUS:
        mesh_provisioner_hci_event_proxy_filter_status_send(p_hci_event, (wiced_bt_mesh_proxy_filter_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_PROVISION_SCAN_CAPABILITIES_STATUS:
        mesh_provisioner_hci_event_scan_capabilities_status_send(p_hci_event, (wiced_bt_mesh_provision_scan_capabilities_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_PROVISION_SCAN_STATUS:
        mesh_provisioner_hci_event_scan_status_send(p_hci_event, (wiced_bt_mesh_provision_scan_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_PROVISION_SCAN_REPORT:
        mesh_provisioner_hci_event_scan_report_send(p_hci_event, (wiced_bt_mesh_provision_scan_report_data_t *)p_data);
        break;

    case WICED_BT_MESH_PROVISION_SCAN_EXTENDED_REPORT:
        mesh_provisioner_hci_event_scan_extended_report_send(p_hci_event, (wiced_bt_mesh_provision_scan_extended_report_data_t *)p_data);
        break;

    case WICED_BT_MESH_PROXY_DEVICE:
        mesh_provisioner_hci_event_proxy_device_send(p_hci_event, (wiced_bt_mesh_proxy_device_network_data_t *)p_data);
        break;

    case WICED_BT_MESH_PROVISION_LINK_STATUS:
        mesh_provisioner_hci_event_provision_link_status_send(p_hci_event, (wiced_bt_mesh_provision_link_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_PROVISION_LINK_REPORT:
        mesh_provisioner_hci_event_provision_link_report_send(p_hci_event, (wiced_bt_mesh_provision_link_report_data_t *)p_data);
        break;

    case WICED_BT_MESH_PROVISION_END:
        mesh_provisioner_hci_event_provision_end_send(p_hci_event, (wiced_bt_mesh_provision_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_PROVISION_DEVICE_CAPABILITIES:
        mesh_provisioner_hci_event_device_capabilities_send(p_hci_event, (wiced_bt_mesh_provision_device_capabilities_data_t *)p_data);
        break;

    case WICED_BT_MESH_PROVISION_GET_OOB_DATA:
        mesh_provisioner_hci_event_device_get_oob_data_send(p_hci_event, (wiced_bt_mesh_provision_device_oob_request_data_t *)p_data);
        break;

    case WICED_BT_MESH_PROXY_CONNECTION_STATUS:
        mesh_provisioner_hci_event_proxy_connection_status_send(p_hci_event, (wiced_bt_mesh_connect_status_data_t *)p_data);
        break;

#ifdef MESH_DFU_SUPPORTED
    case WICED_BT_MESH_FW_DISTRIBUTION_STATUS:
        mesh_provisioner_hci_event_fw_distribution_status_send(p_hci_event, (wiced_bt_mesh_fw_distribution_status_data_t *)p_data);
        break;

    case WICED_BT_MESH_FW_UPDATE_METADATA_STATUS:
        mesh_provisioner_hci_event_fw_update_metadata_status_send(p_hci_event, (wiced_bt_mesh_dfu_metadata_status_data_t *)p_data);
        break;
#endif

    default:
        WICED_BT_TRACE("ignored\n");
        break;
    }
    wiced_bt_mesh_release_event(p_event);
}


/*
 * In 2 chip solutions MCU can send commands to change provisioner state.
 */
uint32_t mesh_app_proc_rx_cmd(uint16_t opcode, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_event_t *p_event = NULL;
    uint8_t status = HCI_CONTROL_MESH_STATUS_SUCCESS;

    WICED_BT_TRACE("%s opcode:%x\n", __FUNCTION__, opcode);

    if (
        mesh_default_transition_time_proc_rx_cmd(opcode, p_data, length) ||
#ifdef WICED_BT_MESH_MODEL_PROPERTY_CLIENT_INCLUDED
       mesh_property_client_proc_rx_cmd(opcode, p_data, length) ||
#endif
#ifdef WICED_BT_MESH_MODEL_BATTERY_CLIENT_INCLUDED
       mesh_battery_client_proc_rx_cmd(opcode, p_data, length) ||
#endif
#ifdef WICED_BT_MESH_MODEL_LIGHT_LC_CLIENT_INCLUDED
       mesh_light_lc_client_proc_rx_cmd(opcode, p_data, length) ||
#endif
#ifdef WICED_BT_MESH_MODEL_LIGHT_XYL_CLIENT_INCLUDED
       mesh_light_xyl_client_proc_rx_cmd(opcode, p_data, length) ||
#endif
#ifdef WICED_BT_MESH_MODEL_LOCATION_CLIENT_INCLUDED
       mesh_location_client_proc_rx_cmd(opcode, p_data, length) ||
#endif
#ifdef WICED_BT_MESH_MODEL_POWER_LEVEL_CLIENT_INCLUDED
       mesh_power_level_client_proc_rx_cmd(opcode, p_data, length) ||
#endif
#ifdef WICED_BT_MESH_MODEL_POWER_ONOFF_CLIENT_INCLUDED
       mesh_power_onoff_client_proc_rx_cmd(opcode, p_data, length) ||
#endif
#ifdef WICED_BT_MESH_MODEL_SCHEDULER_CLIENT_INCLUDED
        mesh_scheduler_client_proc_rx_cmd(opcode, p_data, length) ||
#endif
#ifdef WICED_BT_MESH_MODEL_TIME_CLIENT_INCLUDED
        mesh_time_client_proc_rx_cmd(opcode, p_data, length) ||
#endif
#ifdef WICED_BT_MESH_MODEL_ONOFF_CLIENT_INCLUDED
        mesh_onoff_client_proc_rx_cmd(opcode, p_data, length) ||
#endif
#ifdef WICED_BT_MESH_MODEL_LEVEL_CLIENT_INCLUDED
        mesh_level_client_proc_rx_cmd(opcode, p_data, length) ||
#endif
#ifdef WICED_BT_MESH_MODEL_LIGHT_LIGHTNESS_CLIENT_INCLUDED
        mesh_light_lightness_client_proc_rx_cmd(opcode, p_data, length) ||
#endif
#ifdef WICED_BT_MESH_MODEL_LIGHT_HSL_CLIENT_INCLUDED
        mesh_light_hsl_client_proc_rx_cmd(opcode, p_data, length) ||
#endif
#ifdef WICED_BT_MESH_MODEL_LIGHT_CTL_CLIENT_INCLUDED
        mesh_light_ctl_client_proc_rx_cmd(opcode, p_data, length) ||
#endif
#ifdef WICED_BT_MESH_MODEL_SENSOR_CLIENT_INCLUDED
        mesh_sensor_client_proc_rx_cmd(opcode, p_data, length) ||
#endif
#ifdef WICED_BT_MESH_MODEL_SCENE_CLIENT_INCLUDED
        mesh_scene_client_proc_rx_cmd(opcode, p_data, length) ||
#endif
        mesh_vendor_client_proc_rx_cmd(opcode, p_data, length))
        return WICED_TRUE;

    switch (opcode)
    {
    case HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_ADD:
        status = mesh_provisioner_process_add_vendor_model(p_data, length);
        mesh_provisioner_hci_send_status(status);
        return WICED_TRUE;

    case HCI_CONTROL_MESH_COMMAND_SET_LOCAL_DEVICE:
        status = mesh_provisioner_process_set_local_device(p_data, length);
        mesh_provisioner_hci_send_status(status);
        return WICED_TRUE;

    case HCI_CONTROL_MESH_COMMAND_SET_DEVICE_KEY:
        status = mesh_provisioner_process_set_dev_key(p_data, length);
        mesh_provisioner_hci_send_status(status);
        return WICED_TRUE;

    case HCI_CONTROL_MESH_COMMAND_SEARCH_PROXY:
        status = mesh_provisioner_process_search_proxy(p_data, length);
        mesh_provisioner_hci_send_status(status);
        return WICED_TRUE;

    case HCI_CONTROL_MESH_COMMAND_PROXY_CONNECT:
        status = mesh_provisioner_process_proxy_connect(p_data, length);
        mesh_provisioner_hci_send_status(status);
        return WICED_TRUE;

    case HCI_CONTROL_MESH_COMMAND_PROXY_DISCONNECT:
        status = mesh_provisioner_process_proxy_disconnect(p_data, length);
        mesh_provisioner_hci_send_status(status);
        return WICED_TRUE;

    case HCI_CONTROL_MESH_COMMAND_SET_ADV_TX_POWER:
        status = mesh_provisioner_process_set_adv_tx_power(p_data, length);
        mesh_provisioner_hci_send_status(status);
        break;

#ifdef PRIVATE_PROXY_SUPPORTED
    case HCI_CONTROL_MESH_COMMAND_SEND_SOLICITATION_PDU:
        status = mesh_provisioner_send_solicitation_pdu(p_data, length);
        mesh_provisioner_hci_send_status(status);
        return WICED_TRUE;
#endif

    case HCI_CONTROL_MESH_COMMAND_PROVISION_SCAN_CAPABILITIES_GET:
    case HCI_CONTROL_MESH_COMMAND_PROVISION_SCAN_GET:
    case HCI_CONTROL_MESH_COMMAND_PROVISION_SCAN_START:
    case HCI_CONTROL_MESH_COMMAND_PROVISION_SCAN_STOP:
    case HCI_CONTROL_MESH_COMMAND_PROVISION_SCAN_EXTENDED_START:
    case HCI_CONTROL_MESH_COMMAND_PROVISION_CONNECT:
    case HCI_CONTROL_MESH_COMMAND_PROVISION_DISCONNECT:
    case HCI_CONTROL_MESH_COMMAND_PROVISION_START:
    case HCI_CONTROL_MESH_COMMAND_PROVISION_OOB_VALUE:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_NODE_RESET:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_BEACON_GET:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_BEACON_SET:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_COMPOSITION_DATA_GET:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_DEFAULT_TTL_GET:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_DEFAULT_TTL_SET:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_GATT_PROXY_GET:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_GATT_PROXY_SET:
#ifdef DIRECTED_FORWARDING_SERVER_SUPPORTED
    case HCI_CONTROL_MESH_COMMAND_DF_DIRECTED_CONTROL_GET:
    case HCI_CONTROL_MESH_COMMAND_DF_DIRECTED_CONTROL_SET:
    case HCI_CONTROL_MESH_COMMAND_DF_PATH_METRIC_GET:
    case HCI_CONTROL_MESH_COMMAND_DF_PATH_METRIC_SET:
    case HCI_CONTROL_MESH_COMMAND_DF_DISCOVERY_TABLE_CAPABILITIES_GET:
    case HCI_CONTROL_MESH_COMMAND_DF_DISCOVERY_TABLE_CAPABILITIES_SET:
    case HCI_CONTROL_MESH_COMMAND_DF_FORWARDING_TABLE_ADD:
    case HCI_CONTROL_MESH_COMMAND_DF_FORWARDING_TABLE_DELETE:
    case HCI_CONTROL_MESH_COMMAND_DF_FORWARDING_TABLE_DEPENDENTS_ADD:
    case HCI_CONTROL_MESH_COMMAND_DF_FORWARDING_TABLE_DEPENDENTS_DELETE:
    case HCI_CONTROL_MESH_COMMAND_DF_FORWARDING_TABLE_DEPENDENTS_GET:
    case HCI_CONTROL_MESH_COMMAND_DF_FORWARDING_TABLE_ENTRIES_COUNT_GET:
    case HCI_CONTROL_MESH_COMMAND_DF_FORWARDING_TABLE_ENTRIES_GET:
    case HCI_CONTROL_MESH_COMMAND_DF_WANTED_LANES_GET:
    case HCI_CONTROL_MESH_COMMAND_DF_WANTED_LANES_SET:
    case HCI_CONTROL_MESH_COMMAND_DF_TWO_WAY_PATH_GET:
    case HCI_CONTROL_MESH_COMMAND_DF_TWO_WAY_PATH_SET:
    case HCI_CONTROL_MESH_COMMAND_DF_PATH_ECHO_INTERVAL_GET:
    case HCI_CONTROL_MESH_COMMAND_DF_PATH_ECHO_INTERVAL_SET:
    case HCI_CONTROL_MESH_COMMAND_DF_DIRECTED_NETWORK_TRANSMIT_GET:
    case HCI_CONTROL_MESH_COMMAND_DF_DIRECTED_NETWORK_TRANSMIT_SET:
    case HCI_CONTROL_MESH_COMMAND_DF_DIRECTED_RELAY_RETRANSMIT_GET:
    case HCI_CONTROL_MESH_COMMAND_DF_DIRECTED_RELAY_RETRANSMIT_SET:
    case HCI_CONTROL_MESH_COMMAND_DF_RSSI_THRESHOLD_GET:
    case HCI_CONTROL_MESH_COMMAND_DF_RSSI_THRESHOLD_SET:
    case HCI_CONTROL_MESH_COMMAND_DF_DIRECTED_PATHS_GET:
    case HCI_CONTROL_MESH_COMMAND_DF_DIRECTED_PUBLISH_POLICY_GET:
    case HCI_CONTROL_MESH_COMMAND_DF_DIRECTED_PUBLISH_POLICY_SET:
    case HCI_CONTROL_MESH_COMMAND_DF_PATH_DISCOVERY_TIMING_CONTROL_GET:
    case HCI_CONTROL_MESH_COMMAND_DF_PATH_DISCOVERY_TIMING_CONTROL_SET:
    case HCI_CONTROL_MESH_COMMAND_DF_DIRECTED_CONTROL_NETWORK_TRANSMIT_GET:
    case HCI_CONTROL_MESH_COMMAND_DF_DIRECTED_CONTROL_NETWORK_TRANSMIT_SET:
    case HCI_CONTROL_MESH_COMMAND_DF_DIRECTED_CONTROL_RELAY_RETRANSMIT_GET:
    case HCI_CONTROL_MESH_COMMAND_DF_DIRECTED_CONTROL_RELAY_RETRANSMIT_SET:
#endif
#ifdef NETWORK_FILTER_SERVER_SUPPORTED
    case HCI_CONTROL_MESH_COMMAND_NETWORK_FILTER_GET:
    case HCI_CONTROL_MESH_COMMAND_NETWORK_FILTER_SET:
#endif
#ifdef LARGE_COMPOSITION_DATA_SUPPORTED
    case HCI_CONTROL_MESH_COMMAND_CONFIG_LARGE_COMPOS_DATA_GET:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_MODELS_METADATA_GET:
#endif
#ifdef SAR_CONFIGURATION_SUPPORTED
    case HCI_CONTROL_MESH_COMMAND_CONFIG_SAR_TRANSMITTER_GET:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_SAR_TRANSMITTER_SET:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_SAR_RECEIVER_GET:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_SAR_RECEIVER_SET:
#endif
#ifdef PRIVATE_PROXY_SUPPORTED
    case HCI_CONTROL_MESH_COMMAND_CONFIG_PRIVATE_BEACON_GET:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_PRIVATE_BEACON_SET:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_PRIVATE_GATT_PROXY_GET:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_PRIVATE_GATT_PROXY_SET:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_PRIVATE_NODE_IDENTITY_GET:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_PRIVATE_NODE_IDENTITY_SET:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_ON_DEMAND_PRIVATE_PROXY_GET:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_ON_DEMAND_PRIVATE_PROXY_SET:
    case HCI_CONTROL_MESH_COMMAND_SOLICITATION_PDU_RPL_ITEMS_CLEAR:
#endif
    case HCI_CONTROL_MESH_COMMAND_CONFIG_RELAY_GET:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_RELAY_SET:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_FRIEND_GET:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_FRIEND_SET:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_KEY_REFRESH_PHASE_GET:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_KEY_REFRESH_PHASE_SET:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_NODE_IDENTITY_GET:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_NODE_IDENTITY_SET:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_PUBLICATION_GET:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_PUBLICATION_SET:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_SUBSCRIPTION_ADD:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_SUBSCRIPTION_DELETE:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_SUBSCRIPTION_OVERWRITE:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_SUBSCRIPTION_DELETE_ALL:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_SUBSCRIPTION_GET:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_NET_KEY_ADD:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_NET_KEY_DELETE:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_NET_KEY_UPDATE:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_NET_KEY_GET:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_APP_KEY_ADD:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_APP_KEY_DELETE:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_APP_KEY_UPDATE:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_APP_KEY_GET:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_APP_BIND:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_APP_UNBIND:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_APP_GET:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_HEARBEAT_SUBSCRIPTION_GET:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_HEARBEAT_SUBSCRIPTION_SET:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_HEARBEAT_PUBLICATION_GET:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_HEARBEAT_PUBLICATION_SET:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_NETWORK_TRANSMIT_GET:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_NETWORK_TRANSMIT_SET:
    case HCI_CONTROL_MESH_COMMAND_CONFIG_LPN_POLL_TIMEOUT_GET:
    case HCI_CONTROL_MESH_COMMAND_RAW_MODEL_DATA:
#if defined(CERTIFICATE_BASED_PROVISIONING_SUPPORTED)
    case HCI_CONTROL_MESH_COMMAND_PROVISION_SEND_INVITE:
    case HCI_CONTROL_MESH_COMMAND_PROVISION_RETRIEVE_RECORD:
#endif
        p_event = wiced_bt_mesh_create_event_from_wiced_hci(opcode, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_CONFIG_CLNT, &p_data, &length);
        if (p_event == NULL)
        {
            WICED_BT_TRACE("bad hdr\n");
            return WICED_FALSE;
        }
        break;

    case HCI_CONTROL_MESH_COMMAND_PROXY_FILTER_TYPE_SET:
    case HCI_CONTROL_MESH_COMMAND_PROXY_FILTER_ADDRESSES_ADD:
    case HCI_CONTROL_MESH_COMMAND_PROXY_FILTER_ADDRESSES_DELETE:
        p_event = wiced_bt_mesh_create_event_from_wiced_hci(opcode, MESH_COMPANY_ID_UNUSED, WICED_BT_MESH_CORE_MODEL_ID_CONFIG_CLNT, &p_data, &length);
        if (p_event == NULL)
        {
            WICED_BT_TRACE("bad hdr\n");
            return WICED_FALSE;
        }
        break;

    case HCI_CONTROL_MESH_COMMAND_HEALTH_FAULT_GET:
    case HCI_CONTROL_MESH_COMMAND_HEALTH_FAULT_CLEAR:
    case HCI_CONTROL_MESH_COMMAND_HEALTH_FAULT_TEST:
    case HCI_CONTROL_MESH_COMMAND_HEALTH_PERIOD_GET:
    case HCI_CONTROL_MESH_COMMAND_HEALTH_PERIOD_SET:
    case HCI_CONTROL_MESH_COMMAND_HEALTH_ATTENTION_GET:
    case HCI_CONTROL_MESH_COMMAND_HEALTH_ATTENTION_SET:
        p_event = wiced_bt_mesh_create_event_from_wiced_hci(opcode, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_HEALTH_CLNT, &p_data, &length);
        if (p_event == NULL)
        {
            WICED_BT_TRACE("bad hdr\n");
            return WICED_FALSE;
        }
        break;

#ifdef OPCODES_AGGREGATOR_SUPPORTED
    case HCI_CONTROL_MESH_COMMAND_OPCODES_AGGREGATOR_START:
        status = mesh_provisioner_process_aggregator_start(p_data, length);
        mesh_provisioner_hci_send_status(status);
        return WICED_TRUE;

    case HCI_CONTROL_MESH_COMMAND_OPCODES_AGGREGATOR_FINISH:
        status = mesh_provisioner_process_aggregator_finish(p_data, length);
        mesh_provisioner_hci_send_status(status);
        return WICED_TRUE;
#endif

#ifdef MESH_DFU_SUPPORTED
    case HCI_CONTROL_MESH_COMMAND_FW_DISTRIBUTION_UPLOAD_START:
    case HCI_CONTROL_MESH_COMMAND_FW_DISTRIBUTION_UPLOAD_DATA:
    case HCI_CONTROL_MESH_COMMAND_FW_DISTRIBUTION_UPLOAD_FINISH:
        break;

    case HCI_CONTROL_MESH_COMMAND_FW_UPDATE_METADATA_CHECK:
    case HCI_CONTROL_MESH_COMMAND_FW_DISTRIBUTION_START:
    case HCI_CONTROL_MESH_COMMAND_FW_DISTRIBUTION_SUSPEND:
    case HCI_CONTROL_MESH_COMMAND_FW_DISTRIBUTION_RESUME:
    case HCI_CONTROL_MESH_COMMAND_FW_DISTRIBUTION_STOP:
    case HCI_CONTROL_MESH_COMMAND_FW_DISTRIBUTION_GET_STATUS:
        p_event = wiced_bt_mesh_create_event_from_wiced_hci(opcode, MESH_COMPANY_ID_BT_SIG, WICED_BT_MESH_CORE_MODEL_ID_FW_DISTRIBUTION_CLNT, &p_data, &length);
        if (p_event == NULL)
        {
            WICED_BT_TRACE("bad hdr\n");
            return WICED_FALSE;
        }
        break;
#endif

    default:
        WICED_BT_TRACE("bad hdr\n");
        return WICED_FALSE;
    }

    switch (opcode)
    {
    case HCI_CONTROL_MESH_COMMAND_PROVISION_SCAN_CAPABILITIES_GET:
        status = mesh_provisioner_process_scan_capabilities_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_PROVISION_SCAN_GET:
        status = mesh_provisioner_process_scan_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_PROVISION_SCAN_START:
        status = mesh_provisioner_process_scan_start(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_PROVISION_SCAN_STOP:
        status = mesh_provisioner_process_scan_stop(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_PROVISION_SCAN_EXTENDED_START:
        status = mesh_provisioner_process_extended_scan_start(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_PROVISION_CONNECT:
        status = mesh_provisioner_process_connect(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_PROVISION_DISCONNECT:
        status = mesh_provisioner_process_disconnect(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_PROVISION_START:
        status = mesh_provisioner_process_start(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_PROVISION_OOB_VALUE:
        status = mesh_provisioner_process_oob_value(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_NODE_RESET:
        status = mesh_provisioner_process_node_reset(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_BEACON_GET:
        status = mesh_provisioner_process_beacon_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_BEACON_SET:
        status = mesh_provisioner_process_beacon_set(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_COMPOSITION_DATA_GET:
        status = mesh_provisioner_process_composition_data_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_DEFAULT_TTL_GET:
        status = mesh_provisioner_process_default_ttl_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_DEFAULT_TTL_SET:
        status = mesh_provisioner_process_default_ttl_set(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_GATT_PROXY_GET:
        status = mesh_provisioner_process_gatt_proxy_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_GATT_PROXY_SET:
        status = mesh_provisioner_process_gatt_proxy_set(p_event, p_data, length);
        break;

#ifdef DIRECTED_FORWARDING_SERVER_SUPPORTED
    case HCI_CONTROL_MESH_COMMAND_DF_DIRECTED_CONTROL_GET:
        status = mesh_provisioner_process_df_directed_control_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_DF_DIRECTED_CONTROL_SET:
        status = mesh_provisioner_process_df_directed_control_set(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_DF_PATH_METRIC_GET:
        status = mesh_provisioner_process_df_path_metric_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_DF_PATH_METRIC_SET:
        status = mesh_provisioner_process_df_path_metric_set(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_DF_DISCOVERY_TABLE_CAPABILITIES_GET:
        status = mesh_provisioner_process_df_discovery_table_capabilities_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_DF_DISCOVERY_TABLE_CAPABILITIES_SET:
        status = mesh_provisioner_process_df_discovery_table_capabilities_set(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_DF_FORWARDING_TABLE_ADD:
        status = mesh_provisioner_process_df_forwarding_table_add(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_DF_FORWARDING_TABLE_DELETE:
        status = mesh_provisioner_process_df_forwarding_table_delete(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_DF_FORWARDING_TABLE_DEPENDENTS_ADD:
        status = mesh_provisioner_process_df_forwarding_table_dependents_add(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_DF_FORWARDING_TABLE_DEPENDENTS_DELETE:
        status = mesh_provisioner_process_df_forwarding_table_dependents_delete(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_DF_FORWARDING_TABLE_DEPENDENTS_GET:
        status = mesh_provisioner_process_df_forwarding_table_dependents_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_DF_FORWARDING_TABLE_ENTRIES_COUNT_GET:
        status = mesh_provisioner_process_df_forwarding_table_entries_count_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_DF_FORWARDING_TABLE_ENTRIES_GET:
        status = mesh_provisioner_process_df_forwarding_table_entries_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_DF_WANTED_LANES_GET:
        status = mesh_provisioner_process_df_wanted_lanes_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_DF_WANTED_LANES_SET:
        status = mesh_provisioner_process_df_wanted_lanes_set(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_DF_TWO_WAY_PATH_GET:
        status = mesh_provisioner_process_df_two_way_path_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_DF_TWO_WAY_PATH_SET:
        status = mesh_provisioner_process_df_two_way_path_set(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_DF_PATH_ECHO_INTERVAL_GET:
        status = mesh_provisioner_process_df_path_echo_interval_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_DF_PATH_ECHO_INTERVAL_SET:
        status = mesh_provisioner_process_df_path_echo_interval_set(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_DF_DIRECTED_NETWORK_TRANSMIT_GET:
        status = mesh_provisioner_process_df_directed_network_transmit_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_DF_DIRECTED_NETWORK_TRANSMIT_SET:
        status = mesh_provisioner_process_df_directed_network_transmit_set(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_DF_DIRECTED_RELAY_RETRANSMIT_GET:
        status = mesh_provisioner_process_df_directed_relay_retransmit_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_DF_DIRECTED_RELAY_RETRANSMIT_SET:
        status = mesh_provisioner_process_df_directed_relay_retransmit_set(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_DF_RSSI_THRESHOLD_GET:
        status = mesh_provisioner_process_df_rssi_threshold_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_DF_RSSI_THRESHOLD_SET:
        status = mesh_provisioner_process_df_rssi_threshold_set(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_DF_DIRECTED_PATHS_GET:
        status = mesh_provisioner_process_df_directed_paths_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_DF_DIRECTED_PUBLISH_POLICY_GET:
        status = mesh_provisioner_process_df_directed_publish_policy_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_DF_DIRECTED_PUBLISH_POLICY_SET:
        status = mesh_provisioner_process_df_directed_publish_policy_set(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_DF_PATH_DISCOVERY_TIMING_CONTROL_GET:
        status = mesh_provisioner_process_df_path_discovery_timing_control_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_DF_PATH_DISCOVERY_TIMING_CONTROL_SET:
        status = mesh_provisioner_process_df_path_discovery_timing_control_set(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_DF_DIRECTED_CONTROL_NETWORK_TRANSMIT_GET:
        status = mesh_provisioner_process_df_directed_control_network_transmit_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_DF_DIRECTED_CONTROL_NETWORK_TRANSMIT_SET:
        status = mesh_provisioner_process_df_directed_control_network_transmit_set(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_DF_DIRECTED_CONTROL_RELAY_RETRANSMIT_GET:
        status = mesh_provisioner_process_df_directed_control_relay_retransmit_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_DF_DIRECTED_CONTROL_RELAY_RETRANSMIT_SET:
        status = mesh_provisioner_process_df_directed_control_relay_retransmit_set(p_event, p_data, length);
        break;

#endif
#ifdef NETWORK_FILTER_SERVER_SUPPORTED
    case HCI_CONTROL_MESH_COMMAND_NETWORK_FILTER_GET:
        status = mesh_provisioner_process_network_filter_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_NETWORK_FILTER_SET:
        status = mesh_provisioner_process_network_filter_set(p_event, p_data, length);
        break;
#endif
#ifdef LARGE_COMPOSITION_DATA_SUPPORTED
    case HCI_CONTROL_MESH_COMMAND_CONFIG_LARGE_COMPOS_DATA_GET:
        status = mesh_provisioner_process_large_compos_data_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_MODELS_METADATA_GET:
        status = mesh_provisioner_process_models_metadata_get(p_event, p_data, length);
        break;
#endif
#ifdef SAR_CONFIGURATION_SUPPORTED
    case HCI_CONTROL_MESH_COMMAND_CONFIG_SAR_TRANSMITTER_GET:
        status = mesh_provisioner_process_sar_transmitter_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_SAR_TRANSMITTER_SET:
        status = mesh_provisioner_process_sar_transmitter_set(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_SAR_RECEIVER_GET:
        status = mesh_provisioner_process_sar_receiver_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_SAR_RECEIVER_SET:
        status = mesh_provisioner_process_sar_receiver_set(p_event, p_data, length);
        break;
#endif
#ifdef PRIVATE_PROXY_SUPPORTED
    case HCI_CONTROL_MESH_COMMAND_CONFIG_PRIVATE_BEACON_GET:
        status = mesh_provisioner_process_private_beacon_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_PRIVATE_BEACON_SET:
        status = mesh_provisioner_process_private_beacon_set(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_PRIVATE_GATT_PROXY_GET:
        status = mesh_provisioner_process_private_gatt_proxy_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_PRIVATE_GATT_PROXY_SET:
        status = mesh_provisioner_process_private_gatt_proxy_set(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_PRIVATE_NODE_IDENTITY_GET:
        status = mesh_provisioner_process_private_node_identity_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_PRIVATE_NODE_IDENTITY_SET:
        status = mesh_provisioner_process_private_node_identity_set(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_ON_DEMAND_PRIVATE_PROXY_GET:
        status = mesh_provisioner_process_on_demand_private_proxy_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_ON_DEMAND_PRIVATE_PROXY_SET:
        status = mesh_provisioner_process_on_demand_private_proxy_set(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_SOLICITATION_PDU_RPL_ITEMS_CLEAR:
        status = mesh_provisioner_process_solicitation_pdu_rpl_items_clear(p_event, p_data, length);
        break;
#endif

    case HCI_CONTROL_MESH_COMMAND_CONFIG_RELAY_GET:
        status = mesh_provisioner_process_relay_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_RELAY_SET:
        status = mesh_provisioner_process_relay_set(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_FRIEND_GET:
        status = mesh_provisioner_process_friend_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_FRIEND_SET:
        status = mesh_provisioner_process_friend_set(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_KEY_REFRESH_PHASE_GET:
        status = mesh_provisioner_process_key_refresh_phase_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_KEY_REFRESH_PHASE_SET:
        status = mesh_provisioner_process_key_refresh_phase_set(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_NODE_IDENTITY_GET:
        status = mesh_provisioner_process_node_identity_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_NODE_IDENTITY_SET:
        status = mesh_provisioner_process_node_identity_set(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_PUBLICATION_GET:
        status = mesh_provisioner_process_model_publication_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_PUBLICATION_SET:
        status = mesh_provisioner_process_model_publication_set(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_SUBSCRIPTION_ADD:
        status = mesh_provisioner_process_model_subscription_change(p_event, OPERATION_ADD, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_SUBSCRIPTION_DELETE:
        status = mesh_provisioner_process_model_subscription_change(p_event, OPERATION_DELETE, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_SUBSCRIPTION_OVERWRITE:
        status = mesh_provisioner_process_model_subscription_change(p_event, OPERATION_OVERWRITE, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_SUBSCRIPTION_DELETE_ALL:
        status = mesh_provisioner_process_model_subscription_change(p_event, OPERATION_DELETE_ALL, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_SUBSCRIPTION_GET:
        status = mesh_provisioner_process_model_subscription_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_NET_KEY_ADD:
        status = mesh_provisioner_process_netkey_change(p_event, OPERATION_ADD, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_NET_KEY_DELETE:
        status = mesh_provisioner_process_netkey_change(p_event, OPERATION_DELETE, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_NET_KEY_UPDATE:
        status = mesh_provisioner_process_netkey_change(p_event, OPERATION_OVERWRITE, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_NET_KEY_GET:
        status = mesh_provisioner_process_netkey_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_APP_KEY_ADD:
        status = mesh_provisioner_process_appkey_change(p_event, OPERATION_ADD, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_APP_KEY_DELETE:
        status = mesh_provisioner_process_appkey_change(p_event, OPERATION_DELETE, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_APP_KEY_UPDATE:
        status = mesh_provisioner_process_appkey_change(p_event, OPERATION_OVERWRITE, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_APP_KEY_GET:
        status = mesh_provisioner_process_appkey_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_APP_BIND:
        status = mesh_provisioner_process_model_app_change(p_event, OPERATION_ADD, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_APP_UNBIND:
        status = mesh_provisioner_process_model_app_change(p_event, OPERATION_DELETE, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_MODEL_APP_GET:
        status = mesh_provisioner_process_model_app_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_HEARBEAT_SUBSCRIPTION_GET:
        status = mesh_provisioner_process_heartbeat_subscription_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_HEARBEAT_SUBSCRIPTION_SET:
        status = mesh_provisioner_process_heartbeat_subscription_set(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_HEARBEAT_PUBLICATION_GET:
        status = mesh_provisioner_process_heartbeat_publication_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_HEARBEAT_PUBLICATION_SET:
        status = mesh_provisioner_process_heartbeat_publication_set(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_NETWORK_TRANSMIT_GET:
        status = mesh_provisioner_process_network_transmit_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_NETWORK_TRANSMIT_SET:
        status = mesh_provisioner_process_network_transmit_set(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_CONFIG_LPN_POLL_TIMEOUT_GET:
        status = mesh_provisioner_process_lpn_poll_timeout_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_PROXY_FILTER_TYPE_SET:
        status = mesh_provisioner_process_proxy_filter_type_set(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_PROXY_FILTER_ADDRESSES_ADD:
    case HCI_CONTROL_MESH_COMMAND_PROXY_FILTER_ADDRESSES_DELETE:
        status = mesh_provisioner_process_proxy_filter_change(p_event, opcode == HCI_CONTROL_MESH_COMMAND_PROXY_FILTER_ADDRESSES_ADD, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_HEALTH_FAULT_GET:
        status = mesh_provisioner_process_health_fault_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_HEALTH_FAULT_CLEAR:
        status = mesh_provisioner_process_health_fault_clear(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_HEALTH_FAULT_TEST:
        status = mesh_provisioner_process_health_fault_test(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_HEALTH_PERIOD_GET:
        status = mesh_provisioner_process_health_period_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_HEALTH_PERIOD_SET:
        status = mesh_provisioner_process_health_period_set(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_HEALTH_ATTENTION_GET:
        status = mesh_provisioner_process_health_attention_get(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_HEALTH_ATTENTION_SET:
        status = mesh_provisioner_process_health_attention_set(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_RAW_MODEL_DATA:
        status = mesh_provisioner_process_raw_model_data(p_event, p_data, length);
        break;

#if defined(CERTIFICATE_BASED_PROVISIONING_SUPPORTED)
    case HCI_CONTROL_MESH_COMMAND_PROVISION_SEND_INVITE:
        status = mesh_provisioner_process_send_invite(p_event,  p_data, length);
        break;
    case HCI_CONTROL_MESH_COMMAND_PROVISION_RETRIEVE_RECORD:
        status = mesh_provisioner_process_record_get(p_event, p_data, length);
        break;
#endif

#ifdef MESH_DFU_SUPPORTED
    case HCI_CONTROL_MESH_COMMAND_FW_DISTRIBUTION_UPLOAD_START:
        status = mesh_provisioner_process_fw_upload_start(p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_FW_DISTRIBUTION_UPLOAD_DATA:
        status = mesh_provisioner_process_fw_upload_data(p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_FW_DISTRIBUTION_UPLOAD_FINISH:
        status = mesh_provisioner_process_fw_upload_finish(p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_FW_UPDATE_METADATA_CHECK:
        status = mesh_provisioner_process_fw_update_metadata_check(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_FW_DISTRIBUTION_START:
        status = mesh_provisioner_process_fw_distribution_start(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_FW_DISTRIBUTION_SUSPEND:
        status = mesh_provisioner_process_fw_distribution_suspend(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_FW_DISTRIBUTION_RESUME:
        status = mesh_provisioner_process_fw_distribution_resume(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_FW_DISTRIBUTION_STOP:
        status = mesh_provisioner_process_fw_distribution_stop(p_event, p_data, length);
        break;

    case HCI_CONTROL_MESH_COMMAND_FW_DISTRIBUTION_GET_STATUS:
        status = mesh_provisioner_process_fw_distribution_get_status(p_event, p_data, length);
        break;
#endif

    default:
        wiced_bt_mesh_release_event(p_event);
        break;
    }
    mesh_provisioner_hci_send_status(status);
    return WICED_TRUE;
}

/*
 * Process command from MCU to disconnect GATT Proxy
 */
uint8_t mesh_provisioner_process_raw_model_data(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
#ifdef PTS
    // Send with out_event_queue so we can cancel retransmit when response is received
    wiced_result_t result;
    uint16_t opcode = (p_data[0] << 8) + p_data[1];
    if (wiced_bt_mesh_models_utils_send(p_event, &p_out_event, WICED_TRUE, opcode, p_data + 2, length - 2, NULL) != WICED_BT_SUCCESS)
        return HCI_CONTROL_MESH_STATUS_ERROR;
#else
    if ((p_data[0] & (MESH_APP_PAYLOAD_OP_LONG | MESH_APP_PAYLOAD_OP_MANUF_SPECIFIC)) == (MESH_APP_PAYLOAD_OP_LONG | MESH_APP_PAYLOAD_OP_MANUF_SPECIFIC))
    {
        p_event->opcode = p_data[0] & ~(MESH_APP_PAYLOAD_OP_LONG | MESH_APP_PAYLOAD_OP_MANUF_SPECIFIC);
        p_event->company_id = (p_data[1] << 8) + p_data[2];
        p_data += 3;
        length -= 3;
    }
    else
    {
        p_event->company_id = MESH_COMPANY_ID_BT_SIG;
        p_event->opcode = (p_data[0] << 8) + p_data[1];
        p_data += 2;
        length -= 2;
    }
    if (wiced_bt_mesh_core_send(p_event, p_data, length, NULL) != WICED_BT_SUCCESS)
        return HCI_CONTROL_MESH_STATUS_ERROR;
#endif
    return HCI_CONTROL_MESH_STATUS_SUCCESS;
}

/*
 * Process command from MCU to setup local device
 */
uint8_t mesh_provisioner_process_set_local_device(uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_local_device_set_data_t set;
    uint8_t model_level_access;

    STREAM_TO_UINT16(set.addr, p_data);
    STREAM_TO_ARRAY(set.dev_key, p_data, 16);
    STREAM_TO_ARRAY(set.network_key, p_data, 16);
    STREAM_TO_UINT16(set.net_key_idx, p_data);
    STREAM_TO_UINT32(set.iv_idx, p_data);
    STREAM_TO_UINT8(set.key_refresh, p_data);
    STREAM_TO_UINT8(set.iv_update, p_data);
    STREAM_TO_UINT8(model_level_access, p_data);

    WICED_BT_TRACE("addr:%x net_key_idx:%x iv_idx:%x key_refresh:%d iv_upd:%d model_access:%d\n", set.addr, set.net_key_idx, set.iv_idx, set.key_refresh, set.iv_update, model_level_access);
    mesh_gatt_client_local_device_set(&set);
    mesh_app_init(WICED_TRUE);

    // Application can register to receive all raw model messages.
    // if not, this application wants to process all vendor specific messages
    if (model_level_access)
    {
        extern wiced_bt_mesh_core_received_msg_handler_t p_app_model_message_handler;
        p_app_model_message_handler = mesh_model_raw_data_message_handler;
    }
    return HCI_CONTROL_MESH_STATUS_SUCCESS;
}

/*
* Process command from MCU to set ADV Tx Power.
* 0: MULTI_ADV_TX_POWER_MIN is the minimum
* 4: MULTI_ADV_TX_POWER_MAX is the maximum
*/
uint8_t mesh_provisioner_process_set_adv_tx_power(uint8_t *p_data, uint32_t length)
{
    uint8_t adv_tx_power = 0;

    if (length == 1)
    {
        STREAM_TO_UINT8(adv_tx_power, p_data);

        if (adv_tx_power <= MULTI_ADV_TX_POWER_MAX)
        {
            wiced_bt_mesh_core_adv_tx_power = adv_tx_power;
            return HCI_CONTROL_MESH_STATUS_SUCCESS;
        }
    }
    return HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
* Process command from MCU to set device key.  MCU can set device key once and then perform multiple configuration commands.
*/
uint8_t mesh_provisioner_process_set_dev_key(uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_set_dev_key_data_t set;
    STREAM_TO_UINT16(set.dst, p_data);
    STREAM_TO_ARRAY(set.dev_key, p_data, 16);
    STREAM_TO_UINT16(set.net_key_idx, p_data);
    wiced_bt_mesh_provision_set_dev_key(&set);
    return HCI_CONTROL_MESH_STATUS_SUCCESS;
}

uint8_t mesh_provisioner_process_add_vendor_model(uint8_t* p_data, uint32_t length)
{
    uint8_t element_index;
    uint16_t company_id;
    uint16_t model_id;
    uint8_t i = 0;
    uint8_t num_opcodes = 0;

    // Find the empty block in the mesh_element1_models structure and save it
    STREAM_TO_UINT16(company_id, p_data);
    STREAM_TO_UINT16(model_id, p_data);
    STREAM_TO_UINT8(num_opcodes, p_data);

    WICED_BT_TRACE("mesh_provisioner_process_add_vendor_model: company_id:%04x model_id:%x num_opcodes:%x \n", company_id, model_id, num_opcodes);

    // Find empty block to add vendor model
    wiced_bool_t b_found = WICED_FALSE;
    for (i = 0; i < MESH_APP_NUM_MODELS; i++)
    {
        if ((mesh_element1_models[i].company_id == company_id) && (mesh_element1_models[i].model_id == model_id))
        {
            WICED_BT_TRACE("mesh_provisioner_process_add_vendor_model: model_id: %d already exists\n", model_id);
            return HCI_CONTROL_MESH_STATUS_ERROR;
        }

        if (mesh_element1_models[i].company_id != MESH_COMPANY_ID_UNUSED)
            continue;

        b_found = WICED_TRUE;
        mesh_element1_models[i].company_id = company_id;
        mesh_element1_models[i].model_id = model_id;
        break;
    }

    if (!b_found)
    {
        WICED_BT_TRACE("mesh_provisioner_process_add_vendor_model: no empty block found in mesh_element1_models\n");
        return HCI_CONTROL_MESH_STATUS_ERROR;
    }

    element_index = i;

    // Find empty block for vendor model data
    b_found = WICED_FALSE;
    for (i = 0; i < MESH_APP_MESH_MAX_VENDOR_MODELS; i++)
    {
        if (vendor_model_data[i].company_id)
            continue;
        b_found = WICED_TRUE;
        break;
    }

    if (!b_found)
    {
        WICED_BT_TRACE("mesh_provisioner_process_add_vendor_model: no empty block found in vendor_model_data\n");
        return HCI_CONTROL_MESH_STATUS_ERROR;
    }

    //know where the element data is stored
    vendor_model_data[i].element_index = element_index;
    vendor_model_data[i].company_id = company_id;
    vendor_model_data[i].model_id = model_id;
    vendor_model_data[i].num_opcodes = num_opcodes;
    vendor_model_data[i].p_opcodes = NULL;

    if (num_opcodes)
    {
        // each vendor specific opcode is 3-bytes long
        uint8_t* p_opcodes = wiced_bt_get_buffer(num_opcodes * 3);
        if (p_opcodes)
        {
            vendor_model_data[i].p_opcodes = p_opcodes;
            STREAM_TO_ARRAY(vendor_model_data[i].p_opcodes, p_data, num_opcodes * 3);
        }
        else
        {
            WICED_BT_TRACE("mesh_provisioner_process_add_vendor_model: could not allocate buffer for p_opcodes\n");
            return HCI_CONTROL_MESH_STATUS_ERROR;
        }
    }

    return HCI_CONTROL_MESH_STATUS_SUCCESS;
}

extern void mesh_vendor_client_process_data(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint16_t data_len);

/*
 * This function is called when core receives a valid message for the define Vendor
 * Model (MESH_VENDOR_COMPANY_ID/MESH_VENDOR_MODEL_ID) combination.  The function shall return TRUE if it
 * was able to process the message, and FALSE if the message is unknown.  In the latter case the core
 * will call other registered models.
 */
wiced_bool_t mesh_vendor_client_message_handler(wiced_bt_mesh_event_t *p_event,  uint8_t *p_data, uint16_t data_len)
{
    uint8_t i = 0;
    uint8_t j = 0;
    uint8_t *p_op = NULL;
    uint8_t num_opcodes = 0;

    WICED_BT_TRACE("mesh_vendor_client_message_handler: company_id:%04x opcode:%x model_id:%x\n", p_event->company_id, p_event->opcode, p_event->model_id);

    // model_id 0xffff means request to check if that opcode belongs to that model
    if (p_event->model_id == 0xffff)
    {
        for (i = 0; i < MESH_APP_MESH_MAX_VENDOR_MODELS; i++)
        {
            if (vendor_model_data[i].company_id == p_event->company_id)
            {
                p_op = vendor_model_data[i].p_opcodes;
                num_opcodes = vendor_model_data[i].num_opcodes;
                if (num_opcodes && (p_op != NULL))
                {
                    for (j = 0; j < num_opcodes; j++)
                    {
                        // first byte is the opcode
                        if (p_op[0] == p_event->opcode)
                        {
                            p_event->model_id = vendor_model_data[i].model_id;
                            p_event->status.rpl_delay = MESH_APP_RPL_DELAY;
                            return WICED_TRUE;
                        }
                        else
                            p_op += 3;
                    }
                }
                else
                {
                    p_event->model_id = vendor_model_data[i].model_id;
                    p_event->status.rpl_delay = MESH_APP_RPL_DELAY;
                    return WICED_TRUE;
                }
            }
        }
        return WICED_FALSE;
    }

    mesh_vendor_client_process_data(p_event, p_data, data_len);
    return WICED_TRUE;
}

/*
 * Send Scan Info Get command to Remote Provisioning Server
 */
uint8_t mesh_provisioner_process_scan_capabilities_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    return wiced_bt_mesh_provision_scan_capabilities_get(p_event) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Send Scan Get command to Remote Provisioning Server
 */
uint8_t mesh_provisioner_process_scan_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    return wiced_bt_mesh_provision_scan_get(p_event) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Send Start Scan command to Remote Provisioning Server
 */
uint8_t mesh_provisioner_process_scan_start(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_provision_scan_start_data_t data;
    memset(&data, 0, sizeof(data));
    STREAM_TO_UINT8(data.scanned_items_limit, p_data);
    STREAM_TO_UINT8(data.timeout, p_data);
    if (length >= 2 + MESH_DEVICE_UUID_LEN)
    {
        data.scan_single_uuid = WICED_TRUE;
        memcpy(data.uuid, p_data, MESH_DEVICE_UUID_LEN);
    }
    return wiced_bt_mesh_provision_scan_start(p_event, &data) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Send Scan Extended Start command to Remote Provisioning Server
 */
uint8_t mesh_provisioner_process_extended_scan_start(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_provision_scan_extended_start_t data;
    int i;

    memset(&data, 0, sizeof(data));
    STREAM_TO_UINT8(data.num_ad_filters, p_data);
    length -= 1;

    for (i = 0; (i < data.num_ad_filters) && (i < WICED_BT_MESH_AD_FILTER_TYPES_MAX) && (length > 0); i++)
    {
        STREAM_TO_UINT8(data.ad_filter_types[i], p_data);
        length--;
    }
    if ((length != 17) && (length != 0))
    {
        WICED_BT_TRACE("ext scan start len:%d\n", length);
        return WICED_FALSE;
    }
    if (length == 17)
    {
        data.uuid_present = WICED_TRUE;
        memcpy(data.uuid, p_data, 16);
        p_data += 16;
        STREAM_TO_UINT8(data.timeout, p_data);
    }
    return wiced_bt_mesh_provision_scan_extended_start(p_event, &data) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Send Stop Scan command to Remote Provisioning Server
 */
uint8_t mesh_provisioner_process_scan_stop(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    return wiced_bt_mesh_provision_scan_stop(p_event) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to connect provisioning link
 */
uint8_t mesh_provisioner_process_connect(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_provision_connect_data_t connect;
    uint8_t use_pb_gatt;

    STREAM_TO_ARRAY(connect.uuid, p_data, 16);
    STREAM_TO_UINT8(connect.identify_duration, p_data);
    STREAM_TO_UINT8(connect.procedure, p_data);
    STREAM_TO_UINT8(use_pb_gatt, p_data);

    return wiced_bt_mesh_provision_connect(p_event, &connect, use_pb_gatt)  ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to disconnect provisioning link
 */
uint8_t mesh_provisioner_process_disconnect(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    return wiced_bt_mesh_provision_disconnect(p_event) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to start provisioning on the established provisioning link
 */
uint8_t mesh_provisioner_process_start(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_provision_start_data_t start;

    STREAM_TO_UINT16(start.addr, p_data);
    STREAM_TO_UINT16(start.net_key_idx, p_data);
    STREAM_TO_UINT8(start.algorithm, p_data);
    STREAM_TO_UINT8(start.public_key_type, p_data);
    STREAM_TO_UINT8(start.auth_method, p_data);
    STREAM_TO_UINT8(start.auth_action, p_data);
    STREAM_TO_UINT8(start.auth_size, p_data);

    return wiced_bt_mesh_provision_start(p_event, &start) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to with OOB value to be used in the link calculation
 */
uint8_t mesh_provisioner_process_oob_value(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    return wiced_bt_mesh_provision_client_set_oob(p_event, p_data, length) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Start Search for devices that support GATT Proxy functionality
 */
uint8_t mesh_provisioner_process_search_proxy(uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_client_search_proxy(p_data[0]);
    return HCI_CONTROL_MESH_STATUS_SUCCESS;
}

/*
 * Process command from MCU to connect to a GATT Proxy
 */
uint8_t mesh_provisioner_process_proxy_connect(uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_proxy_connect_data_t connect;

    if (length == 3)
    {
        connect.connect_type = CONNECT_TYPE_NODE_ID;
        STREAM_TO_UINT16(connect.node_id, p_data);
        STREAM_TO_UINT8(connect.scan_duration, p_data);
    }
    else if (length == BD_ADDR_LEN + 2)
    {
        connect.connect_type = CONNECT_TYPE_BDADDR;
        STREAM_TO_BDADDR(connect.bd_addr, p_data);
        STREAM_TO_UINT8(connect.bd_addr_type, p_data);
        STREAM_TO_UINT8(connect.scan_duration, p_data);
    }
    else if (length == 1)
    {
        connect.connect_type = CONNECT_TYPE_NET_ID;
        STREAM_TO_UINT8(connect.scan_duration, p_data);
    }
    else
    {
        return HCI_CONTROL_MESH_STATUS_ERROR;
    }
    return wiced_bt_mesh_client_proxy_connect(&connect)  ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to disconnect GATT Proxy
 */
uint8_t mesh_provisioner_process_proxy_disconnect(uint8_t *p_data, uint32_t length)
{
    return wiced_bt_mesh_client_proxy_disconnect() ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

#ifdef OPCODES_AGGREGATOR_SUPPORTED
uint8_t mesh_provisioner_process_aggregator_start(uint8_t *p_data, uint32_t length)
{
    uint16_t dst_addr, app_key_idx, element_addr;
    uint8_t dst_elem_idx;
    wiced_bool_t is_command, result;

    STREAM_TO_UINT16(dst_addr, p_data);
    STREAM_TO_UINT8(dst_elem_idx, p_data);
    STREAM_TO_UINT16(app_key_idx, p_data);
    STREAM_TO_UINT8(is_command, p_data);
    STREAM_TO_UINT16(element_addr, p_data);
    result = wiced_bt_mesh_opcodes_aggregator_start(dst_addr, dst_elem_idx, app_key_idx, is_command, element_addr, mesh_provisioner_hci_agg_item_add_status_send);

    return result ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_aggregator_finish(uint8_t *p_data, uint32_t length)
{
    uint8_t status;
    wiced_bool_t result;

    STREAM_TO_UINT8(status, p_data);
    result = wiced_bt_mesh_opcodes_aggregator_finish_and_send(status);

    return result ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

void mesh_provisioner_hci_agg_item_add_status_send(uint8_t status)
{
    uint8_t *p_buffer = wiced_transport_allocate_buffer(host_trans_pool);
    uint8_t *p = p_buffer;

    UINT8_TO_STREAM(p, status);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_OPCODES_AGGREGATOR_ADD_STATUS, p_buffer, (uint16_t)(p - p_buffer));
}
#endif

/*
 * Process command from MCU to Reset Node
 */
uint8_t mesh_provisioner_process_node_reset(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    return wiced_bt_mesh_config_node_reset(p_event) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to Get Mesh Secure Beacon Status
 */
uint8_t mesh_provisioner_process_beacon_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    return wiced_bt_mesh_config_beacon_get(p_event) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to Set Mesh Secure Beacon Status
 */
uint8_t mesh_provisioner_process_beacon_set(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_config_beacon_set_data_t data;

    STREAM_TO_UINT8(data.state, p_data);

    return wiced_bt_mesh_config_beacon_set(p_event, &data) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to Get Mesh Node Composition Data
 */
uint8_t mesh_provisioner_process_composition_data_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_config_composition_data_get_data_t data;

    STREAM_TO_UINT8(data.page_number, p_data);

    return wiced_bt_mesh_config_composition_data_get(p_event, &data) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to Get Value of the Node's Default TTL
 */
uint8_t mesh_provisioner_process_default_ttl_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    return wiced_bt_mesh_config_default_ttl_get(p_event) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to Set Value of the Node's Default TTL
 */
uint8_t mesh_provisioner_process_default_ttl_set(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_config_default_ttl_set_data_t data;

    STREAM_TO_UINT8(data.ttl, p_data);

    return wiced_bt_mesh_config_default_ttl_set(p_event, &data) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to Get GATT Proxy State
 */
uint8_t mesh_provisioner_process_gatt_proxy_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    return wiced_bt_mesh_config_gatt_proxy_get(p_event) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to Set GATT Proxy Status
 */
uint8_t mesh_provisioner_process_gatt_proxy_set(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_config_gatt_proxy_set_data_t data;

    STREAM_TO_UINT8(data.state, p_data);

    return wiced_bt_mesh_config_gatt_proxy_set(p_event, &data) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

#ifdef DIRECTED_FORWARDING_SERVER_SUPPORTED
/*
 * Process command from MCU to Set Directed Forwarding Control Status
 */
uint8_t mesh_provisioner_process_df_directed_control_set(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length)
{
    wiced_bt_mesh_df_state_control_t data;

    if (length != 7)
        return HCI_CONTROL_MESH_STATUS_ERROR;

    STREAM_TO_UINT16(data.netkey_idx, p_data);
    STREAM_TO_UINT8(data.forwarding, p_data);
    STREAM_TO_UINT8(data.relay, p_data);
    STREAM_TO_UINT8(data.proxy, p_data);
    STREAM_TO_UINT8(data.proxy_use_directed_default, p_data);
    STREAM_TO_UINT8(data.friend, p_data);

    return wiced_bt_mesh_df_send_directed_control_set(p_event, &data) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to Get Directed Forwarding Control Status and parameters
 */
uint8_t mesh_provisioner_process_df_directed_control_get(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length)
{
    uint16_t netkey_idx;
    if (length != 2)
        return HCI_CONTROL_MESH_STATUS_ERROR;

    STREAM_TO_UINT16(netkey_idx, p_data);
    return wiced_bt_mesh_df_send_directed_control_get(p_event, netkey_idx) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_df_path_metric_get(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length)
{
    uint16_t netkey_idx;
    if (length != 2)
        return HCI_CONTROL_MESH_STATUS_ERROR;

    STREAM_TO_UINT16(netkey_idx, p_data);
    return wiced_bt_mesh_df_send_path_metric_get(p_event, netkey_idx) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_df_path_metric_set(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length)
{
    uint16_t netkey_idx;
    uint8_t metric_type, lifetime;

    if (length != 4)
        return HCI_CONTROL_MESH_STATUS_ERROR;

    STREAM_TO_UINT16(netkey_idx, p_data);
    STREAM_TO_UINT8(metric_type, p_data);
    STREAM_TO_UINT8(lifetime, p_data);

    return wiced_bt_mesh_df_send_path_metric_set(p_event, netkey_idx, metric_type, lifetime) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_df_discovery_table_capabilities_get(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length)
{
    uint16_t netkey_idx;
    if (length != 2)
        return HCI_CONTROL_MESH_STATUS_ERROR;

    STREAM_TO_UINT16(netkey_idx, p_data);
    return wiced_bt_mesh_df_send_discovery_table_capabilities_get(p_event, netkey_idx) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_df_discovery_table_capabilities_set(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length)
{
    uint16_t netkey_idx;
    uint8_t max_concurrent_init;

    if (length != 3)
        return HCI_CONTROL_MESH_STATUS_ERROR;

    STREAM_TO_UINT16(netkey_idx, p_data);
    STREAM_TO_UINT8(max_concurrent_init, p_data);

    return wiced_bt_mesh_df_send_discovery_table_capabilities_set(p_event, netkey_idx, max_concurrent_init) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_df_forwarding_table_add(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length)
{
    uint16_t netkey_idx, po, pt, po_bearer, pt_bearer;
    uint8_t back_path_valid, po_elem_cnt, pt_elem_cnt;

    if (length != 13)
        return HCI_CONTROL_MESH_STATUS_ERROR;

    STREAM_TO_UINT16(netkey_idx, p_data);
    STREAM_TO_UINT8(back_path_valid, p_data);
    STREAM_TO_UINT16(po, p_data);
    STREAM_TO_UINT8(po_elem_cnt, p_data);
    STREAM_TO_UINT16(pt, p_data);
    STREAM_TO_UINT8(pt_elem_cnt, p_data);
    STREAM_TO_UINT16(po_bearer, p_data);
    STREAM_TO_UINT16(pt_bearer, p_data);

    return wiced_bt_mesh_df_send_forwarding_tbl_add(p_event, netkey_idx, back_path_valid != 0 ? WICED_TRUE : WICED_FALSE,
        po, po_elem_cnt, pt, pt_elem_cnt, po_bearer, pt_bearer) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_df_forwarding_table_delete(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length)
{
    uint16_t netkey_idx, po, pt;

    if (length != 6)
        return HCI_CONTROL_MESH_STATUS_ERROR;

    STREAM_TO_UINT16(netkey_idx, p_data);
    STREAM_TO_UINT16(po, p_data);
    STREAM_TO_UINT16(pt, p_data);
    return wiced_bt_mesh_df_send_forwarding_tbl_del(p_event, netkey_idx, po, pt) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_df_forwarding_table_dependents_add(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length)
{
    uint16_t netkey_idx, po, pt;
    uint8_t po_cnt, pt_cnt, i;
    uint16_t dependents[32];
    uint8_t elem_cnts[32];

    if (length < 8)
        return HCI_CONTROL_MESH_STATUS_ERROR;
    length -= 8;
    STREAM_TO_UINT16(netkey_idx, p_data);
    STREAM_TO_UINT16(po, p_data);
    STREAM_TO_UINT16(pt, p_data);
    STREAM_TO_UINT8(po_cnt, p_data);
    STREAM_TO_UINT8(pt_cnt, p_data);
    if(length != (po_cnt + pt_cnt) * 3)
        return HCI_CONTROL_MESH_STATUS_ERROR;
    for (i = 0; i < (po_cnt + pt_cnt); i++)
    {
        STREAM_TO_UINT16(dependents[i], p_data);
        STREAM_TO_UINT8(elem_cnts[i], p_data);
    }
    return wiced_bt_mesh_df_send_forwarding_tbl_dependents_add(p_event, netkey_idx, po, pt, po_cnt, pt_cnt, dependents, elem_cnts) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_df_forwarding_table_dependents_delete(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length)
{
    uint16_t netkey_idx, po, pt;
    uint8_t po_cnt, pt_cnt, i;
    uint16_t dependents[32];

    if (length < 8)
        return HCI_CONTROL_MESH_STATUS_ERROR;
    length -= 8;
    STREAM_TO_UINT16(netkey_idx, p_data);
    STREAM_TO_UINT16(po, p_data);
    STREAM_TO_UINT16(pt, p_data);
    STREAM_TO_UINT8(po_cnt, p_data);
    STREAM_TO_UINT8(pt_cnt, p_data);
    if (length != (po_cnt + pt_cnt) * 2)
        return HCI_CONTROL_MESH_STATUS_ERROR;
    for (i = 0; i < (po_cnt + pt_cnt); i++)
    {
        STREAM_TO_UINT16(dependents[i], p_data);
    }
    return wiced_bt_mesh_df_send_forwarding_tbl_dependents_del(p_event, netkey_idx, po, pt, po_cnt, pt_cnt, dependents) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_df_forwarding_table_dependents_get(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length)
{
    uint16_t netkey_idx, start_idx, po, pt, update_id;
    uint8_t get_po, get_pt, fixed;

    if (length < 8)
        return HCI_CONTROL_MESH_STATUS_ERROR;
    length -= 8;
    STREAM_TO_UINT16(netkey_idx, p_data);
    STREAM_TO_UINT8(get_po, p_data);
    STREAM_TO_UINT8(get_pt, p_data);
    STREAM_TO_UINT8(fixed, p_data);
    STREAM_TO_UINT16(start_idx, p_data);
    STREAM_TO_UINT16(po, p_data);
    STREAM_TO_UINT16(pt, p_data);
    STREAM_TO_UINT16(update_id, p_data);
    return wiced_bt_mesh_df_send_forwarding_tbl_dependents_get(p_event, netkey_idx, get_po ? WICED_TRUE : WICED_FALSE, get_pt ? WICED_TRUE : WICED_FALSE, fixed ? WICED_TRUE : WICED_FALSE,
        start_idx, po, pt, update_id) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_df_forwarding_table_entries_count_get(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length)
{
    uint16_t netkey_idx;
    if (length != 2)
        return HCI_CONTROL_MESH_STATUS_ERROR;
    STREAM_TO_UINT16(netkey_idx, p_data);
    return wiced_bt_mesh_df_send_forwarding_table_entries_count_get(p_event, netkey_idx) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_df_forwarding_table_entries_get(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length)
{
    uint16_t netkey_idx;
    uint8_t fixed, non_fixed;
    uint16_t start_index, po, dst, update_id;
    if (length != 12)
        return HCI_CONTROL_MESH_STATUS_ERROR;
    STREAM_TO_UINT16(netkey_idx, p_data);
    STREAM_TO_UINT8(fixed, p_data);
    STREAM_TO_UINT8(non_fixed, p_data);
    STREAM_TO_UINT16(start_index, p_data);
    STREAM_TO_UINT16(po, p_data);
    STREAM_TO_UINT16(dst, p_data);
    STREAM_TO_UINT16(update_id, p_data);
    return wiced_bt_mesh_df_send_forwarding_table_entries_get(p_event, netkey_idx,
        fixed != 0 ? WICED_TRUE : WICED_FALSE, non_fixed != 0 ? WICED_TRUE : WICED_FALSE,
        start_index, po, dst, update_id) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_df_wanted_lanes_get(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length)
{
    uint16_t netkey_idx;
    if (length != 2)
        return HCI_CONTROL_MESH_STATUS_ERROR;
    STREAM_TO_UINT16(netkey_idx, p_data);
    return wiced_bt_mesh_df_send_wanted_lanes_get(p_event, netkey_idx) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_df_wanted_lanes_set(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length)
{
    uint16_t netkey_idx;
    uint8_t wanted_lanes;
    if (length != 3)
        return HCI_CONTROL_MESH_STATUS_ERROR;
    STREAM_TO_UINT16(netkey_idx, p_data);
    STREAM_TO_UINT8(wanted_lanes, p_data);
    return wiced_bt_mesh_df_send_wanted_lanes_set(p_event, netkey_idx, wanted_lanes) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_df_two_way_path_get(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length)
{
    uint16_t netkey_idx;
    if (length != 2)
        return HCI_CONTROL_MESH_STATUS_ERROR;
    STREAM_TO_UINT16(netkey_idx, p_data);
    return wiced_bt_mesh_df_send_two_way_path_get(p_event, netkey_idx) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_df_two_way_path_set(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length)
{
    uint16_t netkey_idx;
    uint8_t  two_way_path;
    if (length != 3)
        return HCI_CONTROL_MESH_STATUS_ERROR;
    STREAM_TO_UINT16(netkey_idx, p_data);
    STREAM_TO_UINT8(two_way_path, p_data);
    return wiced_bt_mesh_df_send_two_way_path_set(p_event, netkey_idx, two_way_path != 0) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_df_path_echo_interval_get(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length)
{
    uint16_t netkey_idx;
    if (length != 2)
        return HCI_CONTROL_MESH_STATUS_ERROR;
    STREAM_TO_UINT16(netkey_idx, p_data);
    return wiced_bt_mesh_df_send_path_echo_interval_get(p_event, netkey_idx) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_df_path_echo_interval_set(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length)
{
    uint16_t netkey_idx;
    uint8_t unicast;
    uint8_t multicast;
    if (length != 4)
        return HCI_CONTROL_MESH_STATUS_ERROR;
    STREAM_TO_UINT16(netkey_idx, p_data);
    STREAM_TO_UINT8(unicast, p_data);
    STREAM_TO_UINT8(multicast, p_data);

    return wiced_bt_mesh_df_send_path_echo_interval_set(p_event, netkey_idx, unicast, multicast) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_df_directed_network_transmit_get(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length)
{
    if (length != 0)
        return HCI_CONTROL_MESH_STATUS_ERROR;
    return wiced_bt_mesh_df_send_directed_network_transmit_get(p_event) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_df_directed_network_transmit_set(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length)
{
    uint8_t count, interval;
    if (length != 2)
        return HCI_CONTROL_MESH_STATUS_ERROR;
    STREAM_TO_UINT8(count, p_data);
    STREAM_TO_UINT8(interval, p_data);
    return wiced_bt_mesh_df_send_network_transmit_set(p_event, count, interval) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_df_directed_relay_retransmit_get(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length)
{
    if (length != 0)
        return HCI_CONTROL_MESH_STATUS_ERROR;
    return wiced_bt_mesh_df_send_directed_relay_retransmit_get(p_event) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_df_directed_relay_retransmit_set(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length)
{
    uint8_t count, interval;
    if (length != 2)
        return HCI_CONTROL_MESH_STATUS_ERROR;
    STREAM_TO_UINT8(count, p_data);
    STREAM_TO_UINT8(interval, p_data);
    return wiced_bt_mesh_df_send_relay_retransmit_set(p_event, count, interval) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_df_rssi_threshold_get(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length)
{
    if (length != 0)
        return HCI_CONTROL_MESH_STATUS_ERROR;
    return wiced_bt_mesh_df_send_rssi_threshold_get(p_event) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_df_rssi_threshold_set(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length)
{
    uint8_t rssi_margin;
    if (length != 1)
        return HCI_CONTROL_MESH_STATUS_ERROR;
    STREAM_TO_UINT8(rssi_margin, p_data);
    return wiced_bt_mesh_df_send_rssi_threshold_set(p_event, rssi_margin) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_df_directed_paths_get(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length)
{
    if (length != 0)
        return HCI_CONTROL_MESH_STATUS_ERROR;
    return wiced_bt_mesh_df_send_directed_paths_get(p_event) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_df_directed_publish_policy_get(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length)
{
    uint16_t elem_addr;
    uint16_t company_id;
    uint16_t model_id;
    if (length != 6)
        return HCI_CONTROL_MESH_STATUS_ERROR;
    STREAM_TO_UINT16(elem_addr, p_data);
    STREAM_TO_UINT16(company_id, p_data);
    STREAM_TO_UINT16(model_id, p_data);
    return wiced_bt_mesh_df_send_directed_publish_policy_get(p_event, elem_addr, company_id, model_id) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_df_directed_publish_policy_set(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length)
{
    uint8_t directed;
    uint16_t elem_addr;
    uint16_t company_id;
    uint16_t model_id;
    if (length != 7)
        return HCI_CONTROL_MESH_STATUS_ERROR;
    STREAM_TO_UINT8(directed, p_data);
    STREAM_TO_UINT16(elem_addr, p_data);
    STREAM_TO_UINT16(company_id, p_data);
    STREAM_TO_UINT16(model_id, p_data);
    return wiced_bt_mesh_df_send_directed_publish_policy_set(p_event, directed ? WICED_TRUE : WICED_FALSE, elem_addr, company_id, model_id) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_df_path_discovery_timing_control_get(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length)
{
    if (length != 0)
        return HCI_CONTROL_MESH_STATUS_ERROR;
    return wiced_bt_mesh_df_send_path_discovery_timing_control_get(p_event) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_df_path_discovery_timing_control_set(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length)
{
    wiced_bt_mesh_df_path_discovery_timing_control_status_data_t data;
    if (length != 6)
        return HCI_CONTROL_MESH_STATUS_ERROR;
    STREAM_TO_UINT16(data.path_monitoring_interval, p_data);
    STREAM_TO_UINT16(data.path_discovery_retry_interval, p_data);
    STREAM_TO_UINT8(data.path_discovery_interval_high, p_data);
    STREAM_TO_UINT8(data.lane_discovery_guard_interval_high, p_data);
    return wiced_bt_mesh_df_send_path_discovery_timing_control_set(p_event, &data) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_df_directed_control_network_transmit_get(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length)
{
    if (length != 0)
        return HCI_CONTROL_MESH_STATUS_ERROR;
    return wiced_bt_mesh_df_send_directed_control_network_transmit_get(p_event) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_df_directed_control_network_transmit_set(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length)
{
    uint8_t count, interval;
    if (length != 2)
        return HCI_CONTROL_MESH_STATUS_ERROR;
    STREAM_TO_UINT8(count, p_data);
    STREAM_TO_UINT8(interval, p_data);
    return wiced_bt_mesh_df_send_control_network_transmit_set(p_event, count, interval) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_df_directed_control_relay_retransmit_get(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length)
{
    if (length != 0)
        return HCI_CONTROL_MESH_STATUS_ERROR;
    return wiced_bt_mesh_df_send_directed_control_relay_retransmit_get(p_event) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_df_directed_control_relay_retransmit_set(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length)
{
    uint8_t count, interval;
    if (length != 2)
        return HCI_CONTROL_MESH_STATUS_ERROR;
    STREAM_TO_UINT8(count, p_data);
    STREAM_TO_UINT8(interval, p_data);
    return wiced_bt_mesh_df_send_control_relay_retransmit_set(p_event, count, interval) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

#endif
#ifdef NETWORK_FILTER_SERVER_SUPPORTED
/*
 * Process command from MCU to Get Network Filter Status and parameters
 */
static uint8_t mesh_provisioner_process_network_filter_get(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length)
{
    uint16_t netkey_idx;
    if (length != 2)
        return HCI_CONTROL_MESH_STATUS_ERROR;

    STREAM_TO_UINT16(netkey_idx, p_data);
    return wiced_bt_mesh_network_filter_get(p_event, netkey_idx) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to Set Network Filter Status
 */
static uint8_t mesh_provisioner_process_network_filter_set(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint32_t length)
{
    uint16_t netkey_idx;
    uint8_t filter_mode;
    uint8_t cnt, i;
    uint16_t addr[16];

    if (length < 3 || length >(3 + 32) || (length % 2) == 0)
        return HCI_CONTROL_MESH_STATUS_ERROR;
    cnt = (length - 3) / 2;

    STREAM_TO_UINT16(netkey_idx, p_data);
    STREAM_TO_UINT8(filter_mode, p_data);
    for (i = 0; i < cnt; i++)
    {
        STREAM_TO_UINT16(addr[i], p_data);
    }

    return wiced_bt_mesh_network_filter_set(p_event, netkey_idx, filter_mode, cnt, addr) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}
#endif

#ifdef LARGE_COMPOSITION_DATA_SUPPORTED
/*
* Process command from MCU to Get Mesh Node Large Composition Data
*/
uint8_t mesh_provisioner_process_large_compos_data_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_config_large_compos_data_get_data_t data;

    STREAM_TO_UINT8(data.page, p_data);
    STREAM_TO_UINT16(data.offset, p_data);

    return wiced_bt_mesh_config_large_compos_data_get(p_event, &data) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
* Process command from MCU to Get Mesh Node Models Metadata
*/
uint8_t mesh_provisioner_process_models_metadata_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_config_models_metadata_get_data_t data;

    STREAM_TO_UINT8(data.page, p_data);
    STREAM_TO_UINT16(data.offset, p_data);

    return wiced_bt_mesh_config_models_metadata_get(p_event, &data) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}
#endif

/*
 * Process command from MCU to Get Relay Status and parameters
 */
uint8_t mesh_provisioner_process_relay_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    return wiced_bt_mesh_config_relay_get(p_event) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to Set Relay Status and parameters
 */
uint8_t mesh_provisioner_process_relay_set(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_config_relay_set_data_t data;

    STREAM_TO_UINT8(data.state, p_data);
    STREAM_TO_UINT8(data.retransmit_count, p_data);
    STREAM_TO_UINT16(data.retransmit_interval, p_data);

    return wiced_bt_mesh_config_relay_set(p_event, &data) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to Get Friend State
 */
uint8_t mesh_provisioner_process_friend_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    return wiced_bt_mesh_config_friend_get(p_event) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to Set Friend State
 */
uint8_t mesh_provisioner_process_friend_set(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_config_friend_set_data_t data;

    STREAM_TO_UINT8(data.state, p_data);

    return wiced_bt_mesh_config_friend_set(p_event, &data) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to Get Key Refresh Phase
 */
uint8_t mesh_provisioner_process_key_refresh_phase_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_config_key_refresh_phase_get_data_t data;

    STREAM_TO_UINT16(data.net_key_idx, p_data);

    return wiced_bt_mesh_config_key_refresh_phase_get(p_event, &data) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to Set Get Key Refresh Phase
 */
uint8_t mesh_provisioner_process_key_refresh_phase_set(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_config_key_refresh_phase_set_data_t data;

    STREAM_TO_UINT16(data.net_key_idx, p_data);
    STREAM_TO_UINT8(data.transition, p_data);

    return wiced_bt_mesh_config_key_refresh_phase_set(p_event, &data) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to Get Node Identity State
 */
uint8_t mesh_provisioner_process_node_identity_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_config_node_identity_get_data_t data;

    STREAM_TO_UINT16(data.net_key_idx, p_data);

    return wiced_bt_mesh_config_node_identity_get(p_event, &data) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to Set Node Identity State
 */
uint8_t mesh_provisioner_process_node_identity_set(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_config_node_identity_set_data_t data;

    STREAM_TO_UINT16(data.net_key_idx, p_data);
    STREAM_TO_UINT8(data.identity, p_data);

    return wiced_bt_mesh_config_node_identity_set(p_event, &data) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

#ifdef SAR_CONFIGURATION_SUPPORTED
/*
* Process command from MCU to Get SAR Transmitter state
*/
uint8_t mesh_provisioner_process_sar_transmitter_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    return wiced_bt_mesh_config_sar_transmitter_get(p_event) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
* Process command from MCU to Set SAR Transmitter state
*/
uint8_t mesh_provisioner_process_sar_transmitter_set(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_sar_xmtr_t xmtr;

    if (length != MESH_SAR_XMTR_PARAMS_LEN)
        return HCI_CONTROL_MESH_STATUS_ERROR;

    memcpy(&xmtr, p_data, MESH_SAR_XMTR_PARAMS_LEN);

    return wiced_bt_mesh_config_sar_transmitter_set(p_event, &xmtr) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
* Process command from MCU to Get SAR Receiver state
*/
uint8_t mesh_provisioner_process_sar_receiver_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    return wiced_bt_mesh_config_sar_receiver_get(p_event) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
* Process command from MCU to Set SAR Receiver state
*/
uint8_t mesh_provisioner_process_sar_receiver_set(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_sar_rcvr_t rcvr;

    if (length != MESH_SAR_RCVR_PARAMS_LEN)
        return HCI_CONTROL_MESH_STATUS_ERROR;

    memcpy(&rcvr, p_data, MESH_SAR_RCVR_PARAMS_LEN);

    return wiced_bt_mesh_config_sar_receiver_set(p_event, &rcvr) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}
#endif

#ifdef PRIVATE_PROXY_SUPPORTED
/*
 * Process command from MCU to Get Mesh Private Beacon Status
 */
uint8_t mesh_provisioner_process_private_beacon_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    return wiced_bt_mesh_config_private_beacon_get(p_event) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to Set Mesh Private Beacon Status
 */
uint8_t mesh_provisioner_process_private_beacon_set(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_config_private_beacon_set_data_t data;

    STREAM_TO_UINT8(data.state, p_data);
    if (length == 2)
    {
        data.set_interval = WICED_TRUE;
        STREAM_TO_UINT8(data.random_update_interval, p_data);
    }
    else
        data.set_interval = WICED_FALSE;

    return wiced_bt_mesh_config_private_beacon_set(p_event, &data) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to Get Private GATT Proxy State
 */
uint8_t mesh_provisioner_process_private_gatt_proxy_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    return wiced_bt_mesh_config_private_gatt_proxy_get(p_event) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to Set Private GATT Proxy Status
 */
uint8_t mesh_provisioner_process_private_gatt_proxy_set(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_config_private_gatt_proxy_set_data_t data;

    STREAM_TO_UINT8(data.state, p_data);

    return wiced_bt_mesh_config_private_gatt_proxy_set(p_event, &data) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to Get Private Node Identity State
 */
uint8_t mesh_provisioner_process_private_node_identity_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_config_private_node_identity_get_data_t data;

    STREAM_TO_UINT16(data.net_key_idx, p_data);

    return wiced_bt_mesh_config_private_node_identity_get(p_event, &data) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to Set Private Node Identity State
 */
uint8_t mesh_provisioner_process_private_node_identity_set(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_config_private_node_identity_set_data_t data;

    STREAM_TO_UINT16(data.net_key_idx, p_data);
    STREAM_TO_UINT8(data.identity, p_data);

    return wiced_bt_mesh_config_private_node_identity_set(p_event, &data) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to Get On-Demand Private Proxy State
 */
uint8_t mesh_provisioner_process_on_demand_private_proxy_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    return wiced_bt_mesh_config_on_demand_private_proxy_get(p_event) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to Set On-Demand Private Proxy Status
 */
uint8_t mesh_provisioner_process_on_demand_private_proxy_set(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_config_on_demand_private_proxy_set_data_t data;

    STREAM_TO_UINT8(data.state, p_data);

    return wiced_bt_mesh_config_on_demand_private_proxy_set(p_event, &data) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to Clear Solicitation PDU RPL list
 */
uint8_t mesh_provisioner_process_solicitation_pdu_rpl_items_clear(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    uint16_t address;
    uint8_t range;
    wiced_bt_mesh_unicast_address_range_t data;

    STREAM_TO_UINT16(address, p_data);
    STREAM_TO_UINT8(range, p_data);

    if (range > 1)
        data.length_present = 1;
    else
        data.length_present = 0;
    data.range_start = address;
    data.range_length = range;

    return wiced_bt_mesh_config_solicitation_pdu_rpl_items_clear(p_event, &data) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
* Process command from MCU to send solicitation PDU
*/
uint8_t mesh_provisioner_send_solicitation_pdu(uint8_t *p_data, uint32_t length)
{
    uint16_t net_key_idx, src, dst;
    uint32_t seq;

    STREAM_TO_UINT16(net_key_idx, p_data);
    STREAM_TO_UINT24(seq, p_data);
    STREAM_TO_UINT16(src, p_data);
    STREAM_TO_UINT16(dst, p_data);

    return wiced_bt_mesh_core_send_solicitation_pdu(net_key_idx, seq, src, dst) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}
#endif

/*
 * Process command from MCU to Model Publication
 */
uint8_t mesh_provisioner_process_model_publication_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_config_model_publication_get_data_t data;

    STREAM_TO_UINT16(data.element_addr, p_data);
    STREAM_TO_UINT16(data.company_id, p_data);
    STREAM_TO_UINT16(data.model_id, p_data);

    return wiced_bt_mesh_config_model_publication_get(p_event, &data) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to Set Model Publication
 */
uint8_t mesh_provisioner_process_model_publication_set(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_config_model_publication_set_data_t data;

    STREAM_TO_UINT16(data.element_addr, p_data);
    STREAM_TO_UINT16(data.company_id, p_data);
    STREAM_TO_UINT16(data.model_id, p_data);
    STREAM_TO_ARRAY(data.publish_addr, p_data, 16);
    STREAM_TO_UINT16(data.app_key_idx, p_data);
    STREAM_TO_UINT8(data.credential_flag, p_data);
    STREAM_TO_UINT8(data.publish_ttl, p_data);
    STREAM_TO_UINT32(data.publish_period, p_data);
    STREAM_TO_UINT8(data.publish_retransmit_count, p_data);
    STREAM_TO_UINT16(data.publish_retransmit_interval, p_data);

    return wiced_bt_mesh_config_model_publication_set(p_event, &data) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to Add, Delete, Overwrite. or Delete All addresses from a Model Subscription
 */
uint8_t mesh_provisioner_process_model_subscription_change(wiced_bt_mesh_event_t *p_event, uint8_t operation, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_config_model_subscription_change_data_t data;

    data.operation = operation;
    STREAM_TO_UINT16(data.element_addr, p_data);
    STREAM_TO_UINT16(data.company_id, p_data);
    STREAM_TO_UINT16(data.model_id, p_data);
    if (operation != OPERATION_DELETE_ALL)
    {
        STREAM_TO_ARRAY(data.addr, p_data, 16);
    }
    else
    {
        memset(data.addr, 0, 16*sizeof(uint8_t));
    }
    return wiced_bt_mesh_config_model_subscription_change(p_event, &data) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to Get Model Subscription list
 */
uint8_t mesh_provisioner_process_model_subscription_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_config_model_subscription_get_data_t data;

    STREAM_TO_UINT16(data.element_addr, p_data);
    STREAM_TO_UINT16(data.company_id, p_data);
    STREAM_TO_UINT16(data.model_id, p_data);
    return wiced_bt_mesh_config_model_subscription_get(p_event, &data) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to Add, Delete, Update Network Key
 */
uint8_t mesh_provisioner_process_netkey_change(wiced_bt_mesh_event_t *p_event, uint8_t operation, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_config_netkey_change_data_t data;

    data.operation = operation;
    STREAM_TO_UINT16(data.net_key_idx, p_data);
    if (operation != OPERATION_DELETE)
    {
        STREAM_TO_ARRAY(data.net_key, p_data, 16);
    }
    else
    {
        memset(data.net_key, 0, 16*sizeof(uint8_t));
    }
    return wiced_bt_mesh_config_netkey_change(p_event, &data) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to Get Network Keys list
 */
uint8_t mesh_provisioner_process_netkey_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    return wiced_bt_mesh_config_netkey_get(p_event) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to Add, Delete, Update an Application Key
 */
uint8_t mesh_provisioner_process_appkey_change(wiced_bt_mesh_event_t *p_event, uint8_t operation, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_config_appkey_change_data_t data = { 0 };

    data.operation = operation;
    STREAM_TO_UINT16(data.net_key_idx, p_data);
    STREAM_TO_UINT16(data.app_key_idx, p_data);
    if (operation != OPERATION_DELETE)
    {
        STREAM_TO_ARRAY(data.app_key, p_data, 16);
    }
    else
    {
        memset(data.app_key, 0, 16*sizeof(uint8_t));
    }
    return wiced_bt_mesh_config_appkey_change(p_event, &data) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to Get Application Keys list
 */
uint8_t mesh_provisioner_process_appkey_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_config_appkey_get_data_t data;

    STREAM_TO_UINT16(data.net_key_idx, p_data);
    return wiced_bt_mesh_config_appkey_get(p_event, &data) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to Bond or Unbind a Model to an Application Key
 */
uint8_t mesh_provisioner_process_model_app_change(wiced_bt_mesh_event_t *p_event, uint8_t operation, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_config_model_app_bind_data_t data;

    data.operation = operation;
    STREAM_TO_UINT16(data.element_addr, p_data);
    STREAM_TO_UINT16(data.company_id, p_data);
    STREAM_TO_UINT16(data.model_id, p_data);
    STREAM_TO_UINT16(data.app_key_idx, p_data);
    return wiced_bt_mesh_config_model_app_bind(p_event, &data) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to Get Application Keys bound to a Model
 */
uint8_t mesh_provisioner_process_model_app_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_config_model_app_get_data_t data;

    STREAM_TO_UINT16(data.element_addr, p_data);
    STREAM_TO_UINT16(data.company_id, p_data);
    STREAM_TO_UINT16(data.model_id, p_data);
    return wiced_bt_mesh_config_model_app_get(p_event, &data) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to get hearbeat subscription on a device
 */
uint8_t mesh_provisioner_process_heartbeat_subscription_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    return wiced_bt_mesh_config_heartbeat_subscription_get(p_event) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to set hearbeat subscription on a device
 */
uint8_t mesh_provisioner_process_heartbeat_subscription_set(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_config_heartbeat_subscription_set_data_t set;

    STREAM_TO_UINT16(set.subscription_src, p_data);
    STREAM_TO_UINT16(set.subscription_dst, p_data);
    STREAM_TO_UINT32(set.period, p_data);

    return wiced_bt_mesh_config_heartbeat_subscription_set(p_event, &set) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to get hearbeat publication on a device
 */
uint8_t mesh_provisioner_process_heartbeat_publication_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    return wiced_bt_mesh_config_heartbeat_publication_get(p_event) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}


/*
 * Process command from MCU to set hearbeat publication on a device
 */
uint8_t mesh_provisioner_process_heartbeat_publication_set(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_config_heartbeat_publication_set_data_t set;

    STREAM_TO_UINT16(set.publication_dst, p_data);
    STREAM_TO_UINT32(set.count, p_data);
    STREAM_TO_UINT32(set.period, p_data);
    STREAM_TO_UINT8(set.ttl, p_data);
    STREAM_TO_UINT8(set.feature_relay, p_data);
    STREAM_TO_UINT8(set.feature_proxy, p_data);
    STREAM_TO_UINT8(set.feature_friend, p_data);
    STREAM_TO_UINT8(set.feature_low_power, p_data);
    STREAM_TO_UINT16(set.net_key_idx, p_data);

    return wiced_bt_mesh_config_heartbeat_publication_set(p_event, &set) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to get network transmit parameters of a device
 */
uint8_t mesh_provisioner_process_network_transmit_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    return wiced_bt_mesh_config_network_transmit_params_get(p_event) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
* Process command from MCU to set network transmit parameters of a device
*/
uint8_t mesh_provisioner_process_network_transmit_set(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_config_network_transmit_set_data_t set;

    STREAM_TO_UINT8(set.count, p_data);
    STREAM_TO_UINT16(set.interval, p_data);

    return wiced_bt_mesh_config_network_transmit_params_set(p_event, &set) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to get the current Registered Fault state identified by Company ID of an element
 */
uint8_t mesh_provisioner_process_health_fault_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_health_fault_get_data_t get;

    STREAM_TO_UINT16(get.company_id, p_data);

    return wiced_bt_mesh_health_fault_get(p_event, &get) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to clear the current Registered Fault state identified by Company ID of an element
 */
uint8_t mesh_provisioner_process_health_fault_clear(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_health_fault_clear_data_t clear;

    STREAM_TO_UINT16(clear.company_id, p_data);

    return wiced_bt_mesh_health_fault_clear(p_event, &clear) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to invoke a self-test procedure of an element
 */
uint8_t mesh_provisioner_process_health_fault_test(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_health_fault_test_data_t test;

    STREAM_TO_UINT8(test.id, p_data);
    STREAM_TO_UINT16(test.company_id, p_data);

    return wiced_bt_mesh_health_fault_test(p_event, &test) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to get the current Health Period state of an element
 */
uint8_t mesh_provisioner_process_health_period_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    return wiced_bt_mesh_health_period_get(p_event) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

/*
 * Process command from MCU to set the current Health Period state of an element
 */
uint8_t mesh_provisioner_process_health_period_set(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_health_period_set_data_t period;

    STREAM_TO_UINT8(period.divisor, p_data);

    return wiced_bt_mesh_health_period_set(p_event, &period) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_health_attention_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    return wiced_bt_mesh_health_attention_get(p_event) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_health_attention_set(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_health_attention_set_data_t attention;

    STREAM_TO_UINT8(attention.timer, p_data);

    return wiced_bt_mesh_health_attention_set(p_event, &attention) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_lpn_poll_timeout_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_lpn_poll_timeout_get_data_t get;

    STREAM_TO_UINT16(get.lpn_addr, p_data);

    return wiced_bt_mesh_lpn_poll_timeout_get(p_event, &get) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_proxy_filter_type_set(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_proxy_filter_set_type_data_t set;

    STREAM_TO_UINT8(set.type, p_data);

    return wiced_bt_mesh_proxy_set_filter_type(p_event, &set) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_proxy_filter_change(wiced_bt_mesh_event_t *p_event, wiced_bool_t is_add, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_proxy_filter_change_addr_data_t *p_addr;
    uint16_t addr_num = length / 2;
    uint16_t i;
    uint8_t  res;

    if ((p_addr = (wiced_bt_mesh_proxy_filter_change_addr_data_t *)wiced_bt_get_buffer(sizeof(wiced_bt_mesh_proxy_filter_change_addr_data_t) + (addr_num - 1) * 2)) == NULL)
        return HCI_CONTROL_MESH_STATUS_ERROR;

    p_addr->addr_num = addr_num;
    for (i = 0; i < addr_num; i++)
        STREAM_TO_UINT16(p_addr->addr[i], p_data);

    res = wiced_bt_mesh_proxy_filter_change_addr(p_event, is_add, p_addr) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
    wiced_bt_free_buffer(p_addr);
    return res;
}

#if defined(CERTIFICATE_BASED_PROVISIONING_SUPPORTED)
uint8_t mesh_provisioner_process_send_invite(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    return wiced_bt_mesh_provision_send_invite(p_event) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_record_get(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_provision_device_record_fragment_data_t data;

    BE_STREAM_TO_UINT16(data.record_id, p_data);
    BE_STREAM_TO_UINT16(data.fragment_offset, p_data);
    BE_STREAM_TO_UINT16(data.total_length, p_data);

    return wiced_bt_mesh_provision_retrieve_record(p_event, &data) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}
#endif

#ifdef MESH_DFU_SUPPORTED
uint8_t fw_distribution_server_process_upload_start(wiced_bt_mesh_event_t* p_event, uint8_t* p_data, uint16_t data_len);
void fw_distribution_server_blob_transfer_callback(uint16_t event, void* p_data);
uint8_t fw_distribution_server_get_upload_phase(void);
uint8_t mesh_fw_distribution_get_distribution_state(void);

void mesh_provisioner_hci_send_fw_distr_status(uint8_t status)
{
    uint8_t* p_buffer = wiced_transport_allocate_buffer(host_trans_pool);
    uint8_t* p = p_buffer;

    UINT8_TO_STREAM(p, status);
    UINT8_TO_STREAM(p, fw_distribution_server_get_upload_phase());
    UINT8_TO_STREAM(p, mesh_fw_distribution_get_distribution_state());

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_FW_DISTRIBUTION_UPLOAD_STATUS, p_buffer, (uint16_t)(p - p_buffer));
}

uint8_t mesh_provisioner_process_fw_upload_start(uint8_t *p_data, uint32_t length)
{
    uint8_t status;

    if (wiced_firmware_upgrade_init_nv_locations())
        status = fw_distribution_server_process_upload_start(NULL, p_data, length);
    else
        status = WICED_BT_MESH_FW_DISTR_STATUS_NOT_SUPPORTED;

    mesh_provisioner_hci_send_fw_distr_status(status);

    if (status == WICED_BT_MESH_FW_DISTR_STATUS_SUCCESS)
        fw_distribution_server_blob_transfer_callback(WICED_BT_MESH_BLOB_TRANSFER_START, NULL);

    return status;
}

uint8_t mesh_provisioner_process_fw_upload_data(uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_blob_transfer_block_data_t data;

    STREAM_TO_UINT32(data.offset, p_data);
    data.data_len = length - 4;
    data.p_data = p_data;
    fw_distribution_server_blob_transfer_callback(WICED_BT_MESH_BLOB_TRANSFER_DATA, &data);

    mesh_provisioner_hci_send_fw_distr_status(WICED_BT_MESH_FW_DISTR_STATUS_SUCCESS);
    return HCI_CONTROL_MESH_STATUS_SUCCESS;
}

uint8_t mesh_provisioner_process_fw_upload_finish(uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_blob_transfer_finish_t finish;

    STREAM_TO_UINT8(finish.blob_transfer_result, p_data);
    fw_distribution_server_blob_transfer_callback(WICED_BT_MESH_BLOB_TRANSFER_FINISH, &finish);

    mesh_provisioner_hci_send_fw_distr_status(WICED_BT_MESH_FW_DISTR_STATUS_SUCCESS);
    return HCI_CONTROL_MESH_STATUS_SUCCESS;
}

uint8_t mesh_provisioner_process_fw_update_metadata_check(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_dfu_metadata_check_data_t data;

    STREAM_TO_UINT8(data.index, p_data);
    data.metadata.len = length - 1;
    memcpy(data.metadata.data, p_data, data.metadata.len);

    return wiced_bt_mesh_dfu_metadata_check(p_event, &data, mesh_config_client_message_handler) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_fw_distribution_start(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    wiced_bt_mesh_fw_distribution_start_data_t start;

    STREAM_TO_UINT8(start.firmware_id.fw_id_len, p_data);
    memcpy(start.firmware_id.fw_id, p_data, start.firmware_id.fw_id_len);
    p_data += start.firmware_id.fw_id_len;
    STREAM_TO_UINT8(start.metadata.len, p_data);
    memcpy(start.metadata.data, p_data, start.metadata.len);
    p_data += start.metadata.len;
    STREAM_TO_UINT32(start.firmware_size, p_data);
    STREAM_TO_UINT16(start.proxy_addr, p_data);
    STREAM_TO_UINT16(start.group_addr, p_data);
    STREAM_TO_UINT16(start.group_size, p_data);
    for (int i = 0; i < start.group_size; i++)
    {
        STREAM_TO_UINT16(start.update_nodes[i].addr, p_data);
        STREAM_TO_UINT8(start.update_nodes[i].low_power, p_data);
    }

    return wiced_bt_mesh_fw_provider_start(p_event, &start, mesh_config_client_message_handler) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_fw_distribution_suspend(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    return wiced_bt_mesh_fw_provider_suspend(p_event) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_fw_distribution_resume(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    return wiced_bt_mesh_fw_provider_resume(p_event) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_fw_distribution_stop(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    return wiced_bt_mesh_fw_provider_stop(p_event) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}

uint8_t mesh_provisioner_process_fw_distribution_get_status(wiced_bt_mesh_event_t *p_event, uint8_t *p_data, uint32_t length)
{
    return wiced_bt_mesh_fw_provider_get_status(p_event, mesh_config_client_message_handler) ? HCI_CONTROL_MESH_STATUS_SUCCESS : HCI_CONTROL_MESH_STATUS_ERROR;
}
#endif

void mesh_provisioner_hci_send_status(uint8_t status)
{
    uint8_t *p_buffer = wiced_transport_allocate_buffer(host_trans_pool);
    uint8_t *p = p_buffer;

    UINT8_TO_STREAM(p, status);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_COMMAND_STATUS, p_buffer, (uint16_t)(p - p_buffer));
}

/*
 * Send Remote Provisioner scan info event over transport
 */
void mesh_provisioner_hci_event_scan_capabilities_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_provision_scan_capabilities_status_data_t *p_data)
{
    uint8_t *p = p_hci_event->data;

    UINT8_TO_STREAM(p, p_data->max_scanned_items);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_PROVISION_SCAN_CAPABILITIES_STATUS, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}

/*
 * Send Remote Provisioner scan status event over transport
 */
void mesh_provisioner_hci_event_scan_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_provision_scan_status_data_t *p_data)
{
    uint8_t *p = p_hci_event->data;

    UINT8_TO_STREAM(p, p_data->status);
    UINT8_TO_STREAM(p, p_data->state);
    UINT8_TO_STREAM(p, p_data->scanned_items_limit);
    UINT8_TO_STREAM(p, p_data->timeout);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_PROVISION_SCAN_STATUS, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}

/*
 * Send provisioner scan report event over transport
 */
void mesh_provisioner_hci_event_scan_report_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_provision_scan_report_data_t *p_data)
{
    uint8_t *p = p_hci_event->data;

    UINT8_TO_STREAM(p, p_data->rssi);
    ARRAY_TO_STREAM(p, p_data->uuid, 16);
    UINT16_TO_STREAM(p, p_data->oob);
    UINT32_TO_STREAM(p, p_data->uri_hash);
#ifdef PROVISION_SCAN_REPORT_INCLUDE_BDADDR
    BDADDR_TO_STREAM(p, p_data->bdaddr);
#endif
    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_PROVISION_SCAN_REPORT, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}

/*
 * Send provisioner scan extended report event over transport
 */
void mesh_provisioner_hci_event_scan_extended_report_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_provision_scan_extended_report_data_t *p_data)
{
    uint8_t* p = p_hci_event->data;
    uint16_t data_len;
    int i;

    memcpy(p, (uint8_t *)p_data, 1 + MESH_DEVICE_UUID_LEN + 2); // status, uuid, oob
    p += 1 + MESH_DEVICE_UUID_LEN + 2;

    for (i = 0; (i < sizeof(p_data->adv_data)) && (p_data->adv_data[i] != 0); i++)
    {
        *p++ = p_data->adv_data[i];
    }
    WICED_BT_TRACE_ARRAY(p_hci_event->data, (uint16_t)(p - (uint8_t *)p_hci_event->data), "extended report ");
    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_PROVISION_SCAN_EXTENDED_REPORT, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}

/*
 * Send Proxy Device Network Data event over transport
 */
void mesh_provisioner_hci_event_proxy_device_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_proxy_device_network_data_t *p_data)
{
    uint8_t *p = p_hci_event->data;

    BDADDR_TO_STREAM(p, p_data->bd_addr);
    UINT8_TO_STREAM(p, p_data->bd_addr_type);
    UINT8_TO_STREAM(p, p_data->rssi);
    UINT16_TO_STREAM(p, p_data->net_key_idx);
    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_PROXY_DEVICE_NETWORK_DATA, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));

}

/*
 * Send link status event over transport
 */
void mesh_provisioner_hci_event_provision_link_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_provision_link_status_data_t *p_data)
{
    uint8_t *p = p_hci_event->data;

    WICED_BT_TRACE("provision connection not sent status from %x addr:%x connected:%d\n", p_hci_event->src, p_data->status);
    UINT8_TO_STREAM(p, p_data->status);

    // mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_PROVISION_LINK_STATUS, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}

void mesh_provisioner_hci_event_provision_link_report_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_provision_link_report_data_t *p_data)
{
    uint8_t *p = p_hci_event->data;
//    uint8_t connected = ((p_data->rpr_state == WICED_BT_MESH_REMOTE_PROVISION_STATE_LINK_ACTIVE) || (p_data->rpr_state == WICED_BT_MESH_REMOTE_PROVISION_STATE_OUTBOUND_PDU_TRANSFER));

    WICED_BT_TRACE("provision link report from %x status:%d state:%d reason:%d over_gatt:%d\n", p_hci_event->src, p_data->link_status, p_data->rpr_state, p_data->reason, p_data->over_gatt);
    UINT8_TO_STREAM(p, p_data->link_status);
    UINT8_TO_STREAM(p, p_data->rpr_state);
    UINT8_TO_STREAM(p, p_data->reason);
    UINT8_TO_STREAM(p, p_data->over_gatt);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_PROVISION_LINK_REPORT, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}

/*
 * Send provisioner provisioning end event over transport
 */
void mesh_provisioner_hci_event_provision_end_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_provision_status_data_t *p_data)
{
    uint8_t *p = p_hci_event->data;

    WICED_BT_TRACE("provision end addr:%x result:%d\n", p_data->addr, p_data->result);

    UINT16_TO_STREAM(p, p_data->provisioner_addr);
    UINT16_TO_STREAM(p, p_data->addr);
    UINT16_TO_STREAM(p, p_data->net_key_idx);
    UINT8_TO_STREAM(p, p_data->result);
    ARRAY_TO_STREAM(p, p_data->dev_key, WICED_BT_MESH_KEY_LEN);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_PROVISION_END, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}

/*
 * Send to the MCU device capabilities received during provisioning
 */
void mesh_provisioner_hci_event_device_capabilities_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_provision_device_capabilities_data_t *p_data)
{
    uint8_t *p = p_hci_event->data;

    WICED_BT_TRACE("mesh prov caps from:%x num_elements:%d key_type:%d\n", p_data->provisioner_addr, p_data->elements_num, p_data->pub_key_type);

    UINT16_TO_STREAM(p, p_data->provisioner_addr);
    UINT8_TO_STREAM(p, p_data->elements_num);
    UINT16_TO_STREAM(p, p_data->algorithms);
    UINT8_TO_STREAM(p, p_data->pub_key_type);
    UINT8_TO_STREAM(p, p_data->static_oob_type);
    UINT8_TO_STREAM(p, p_data->output_oob_size);
    UINT16_TO_STREAM(p, p_data->output_oob_action);
    UINT8_TO_STREAM(p, p_data->input_oob_size);
    UINT16_TO_STREAM(p, p_data->input_oob_action);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_PROVISION_DEVICE_CAPABILITIES, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}

/*
 * Send to the MCU Out of Band data request
 */
void mesh_provisioner_hci_event_device_get_oob_data_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_provision_device_oob_request_data_t *p_oob_data)
{
    uint8_t *p = p_hci_event->data;

    WICED_BT_TRACE("mesh prov oob req from:%x type:%d size:%d action:%d\n", p_oob_data->provisioner_addr, p_oob_data->type, p_oob_data->size, p_oob_data->action);

    UINT16_TO_STREAM(p, p_oob_data->provisioner_addr);
    UINT8_TO_STREAM(p, p_oob_data->type);
    UINT8_TO_STREAM(p, p_oob_data->size);
    UINT8_TO_STREAM(p, p_oob_data->action);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_PROVISION_OOB_DATA, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}

void mesh_provisioner_hci_event_proxy_connection_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_connect_status_data_t *p_data)
{
    uint8_t *p = p_hci_event->data;

    WICED_BT_TRACE("mesh proxy connection status:%d\n", p_data->connected);

    UINT16_TO_STREAM(p, p_data->provisioner_addr);
    UINT16_TO_STREAM(p, p_data->addr);
    UINT8_TO_STREAM(p, p_data->connected);
    UINT8_TO_STREAM(p, p_data->over_gatt);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_PROXY_CONNECTION_STATUS, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}

#if defined(CERTIFICATE_BASED_PROVISIONING_SUPPORTED)

void mesh_provisioner_hci_event_record_list(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_core_provisioning_list_t *p_data)
{
    uint8_t *p = p_hci_event->data;
//    WICED_BT_TRACE_ARRAY((uint8_t *)p_data->list, p_data->list_size*sizeof(uint16_t), " mesh prov record list");

    UINT16_TO_STREAM(p, p_data->extensions);
    for (uint8_t i = 0; i < p_data->list_size; ++i)
    {
        UINT16_TO_STREAM(p, p_data->list[i]);
    }
    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_PROVISION_RECORD_LIST, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}

void mesh_provisioner_hci_event_record_response(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_core_provisioning_record_t *p_data)
{
    uint8_t *p = p_hci_event->data;
//    WICED_BT_TRACE_ARRAY(p_data->data, p_data->size, " mesh prov record fragment");

    UINT8_TO_STREAM(p, p_data->status);
    UINT16_TO_STREAM(p, p_data->u.response.record_id);
    UINT16_TO_STREAM(p, p_data->u.response.fragment_offset);
    UINT16_TO_STREAM(p, p_data->u.response.total_length);
    for (uint8_t i = 0; i < p_data->size; ++i)
    {
        UINT8_TO_STREAM(p, p_data->data[i]);
    }

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_PROVISION_RECORD_RESPONSE, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}
#endif

void mesh_provisioner_hci_event_proxy_filter_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_proxy_filter_status_data_t *p_data)
{
    uint8_t *p = p_hci_event->data;

    UINT8_TO_STREAM(p, p_data->type);
    UINT16_TO_STREAM(p, p_data->list_size);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_PROXY_FILTER_STATUS, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}

void mesh_provisioner_hci_event_node_reset_status_send(wiced_bt_mesh_hci_event_t *p_hci_event)
{
    uint8_t *p = p_hci_event->data;

    WICED_BT_TRACE("node reset status\n", p_hci_event->src);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_NODE_RESET_STATUS, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}

void mesh_provisioner_hci_event_node_identity_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_node_identity_status_data_t *p_data)
{
    uint8_t *p = p_hci_event->data;

    WICED_BT_TRACE("node identity status src:%x\n", p_hci_event->src);

    UINT8_TO_STREAM(p, p_data->status);
    UINT16_TO_STREAM(p, p_data->net_key_idx);
    UINT8_TO_STREAM(p, p_data->identity);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_NODE_IDENTITY_STATUS, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}


void mesh_provisioner_hci_event_composition_data_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_composition_data_status_data_t *p_data)
{
    uint8_t *p = p_hci_event->data;

    WICED_BT_TRACE("comp status src:%x page_num:%d len:%d\n", p_hci_event->src, p_data->page_number, p_data->data_len);

    UINT8_TO_STREAM(p, p_data->page_number);
    ARRAY_TO_STREAM(p, p_data->data, p_data->data_len);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_COMPOSITION_DATA_STATUS, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
    wiced_bt_free_buffer(p_data);
}

void mesh_provisioner_hci_event_friend_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_friend_status_data_t *p_data)
{
    uint8_t *p = p_hci_event->data;

    WICED_BT_TRACE("friend status src:%x\n", p_hci_event->src);

    UINT8_TO_STREAM(p, p_data->state);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_FRIEND_STATUS, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}

void mesh_provisioner_hci_event_key_refresh_phase_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_key_refresh_phase_status_data_t *p_data)
{
    uint8_t *p = p_hci_event->data;

    WICED_BT_TRACE("kr status src:%x\n", p_hci_event->src);

    UINT8_TO_STREAM(p, p_data->status);
    UINT16_TO_STREAM(p, p_data->net_key_idx);
    UINT8_TO_STREAM(p, p_data->phase);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_KEY_REFRESH_PHASE_STATUS, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}

void mesh_provisioner_hci_event_default_ttl_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_default_ttl_status_data_t *p_data)
{
    uint8_t *p = p_hci_event->data;

    WICED_BT_TRACE("default_ttl status src:%x\n", p_hci_event->src);

    UINT8_TO_STREAM(p, p_data->ttl);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_DEFAULT_TTL_STATUS, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}

void mesh_provisioner_hci_event_relay_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_relay_status_data_t *p_data)
{
    uint8_t *p = p_hci_event->data;

    WICED_BT_TRACE("relay status src:%x\n", p_hci_event->src);

    UINT8_TO_STREAM(p, p_data->state);
    UINT8_TO_STREAM(p, p_data->retransmit_count);
    UINT16_TO_STREAM(p, p_data->retransmit_interval);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_RELAY_STATUS, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}

#ifdef DIRECTED_FORWARDING_SERVER_SUPPORTED
void mesh_provisioner_hci_event_df_directed_control_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_df_directed_control_status_data_t* p_data)
{
    uint8_t* p = p_hci_event->data;

    WICED_BT_TRACE("df control status src:%x\n", p_hci_event->src);

    UINT8_TO_STREAM(p, p_data->status);
    UINT16_TO_STREAM(p, p_data->control.netkey_idx);
    UINT8_TO_STREAM(p, p_data->control.forwarding);
    UINT8_TO_STREAM(p, p_data->control.relay);
    UINT8_TO_STREAM(p, p_data->control.proxy);
    UINT8_TO_STREAM(p, p_data->control.proxy_use_directed_default);
    UINT8_TO_STREAM(p, p_data->control.friend);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_DF_DIRECTED_CONTROL_STATUS, (uint8_t*)p_hci_event, (uint16_t)(p - (uint8_t*)p_hci_event));
}
static void mesh_provisioner_hci_event_df_path_metric_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_df_path_metric_status_data_t* p_data)
{
    uint8_t* p = p_hci_event->data;
    WICED_BT_TRACE("df path_metric_status src:%x\n", p_hci_event->src);

    UINT8_TO_STREAM(p, p_data->status);
    UINT16_TO_STREAM(p, p_data->netkey_idx);
    UINT8_TO_STREAM(p, p_data->type);
    UINT8_TO_STREAM(p, p_data->lifetime);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_DF_PATH_METRIC_STATUS, (uint8_t*)p_hci_event, (uint16_t)(p - (uint8_t*)p_hci_event));
}
static void mesh_provisioner_hci_event_df_discovery_table_capabilities_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_df_discovery_table_capabilities_status_data_t* p_data)
{
    uint8_t* p = p_hci_event->data;
    WICED_BT_TRACE("df discovery_table_capabilities_status src:%x\n", p_hci_event->src);

    UINT8_TO_STREAM(p, p_data->status);
    UINT16_TO_STREAM(p, p_data->netkey_idx);
    UINT8_TO_STREAM(p, p_data->max_concurrent_init);
    UINT8_TO_STREAM(p, p_data->max_discovery_table_entries_count);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_DF_DISCOVERY_TABLE_CAPABILITIES_STATUS, (uint8_t*)p_hci_event, (uint16_t)(p - (uint8_t*)p_hci_event));
}
static void mesh_provisioner_hci_event_df_forwarding_table_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_df_forwarding_table_status_data_t* p_data)
{
    uint8_t* p = p_hci_event->data;
    WICED_BT_TRACE("df forwarding_table_status src:%x\n", p_hci_event->src);

    UINT8_TO_STREAM(p, p_data->status);
    UINT16_TO_STREAM(p, p_data->netkey_idx);
    UINT16_TO_STREAM(p, p_data->po);
    UINT16_TO_STREAM(p, p_data->dst);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_DF_FORWARDING_TABLE_STATUS, (uint8_t*)p_hci_event, (uint16_t)(p - (uint8_t*)p_hci_event));
}
static void mesh_provisioner_hci_event_df_forwarding_table_dependents_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_df_forwarding_table_dependents_status_data_t* p_data)
{
    uint8_t* p = p_hci_event->data;
    WICED_BT_TRACE("df forwarding_table_dependents_status src:%x\n", p_hci_event->src);

    UINT8_TO_STREAM(p, p_data->status);
    UINT16_TO_STREAM(p, p_data->netkey_idx);
    UINT16_TO_STREAM(p, p_data->po);
    UINT16_TO_STREAM(p, p_data->dst);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_DF_FORWARDING_TABLE_DEPENDENTS_STATUS, (uint8_t*)p_hci_event, (uint16_t)(p - (uint8_t*)p_hci_event));
}
static void mesh_provisioner_hci_event_df_forwarding_table_dependents_get_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_df_forwarding_table_dependents_get_status_data_t* p_data)
{
    uint8_t ui8;
    uint8_t* p = p_hci_event->data;
    WICED_BT_TRACE("df forwarding_table_dependents_get_status src:%x\n", p_hci_event->src);

    UINT8_TO_STREAM(p, p_data->status);
    UINT16_TO_STREAM(p, p_data->netkey_idx);
    ui8 = p_data->get_po ? 1 : 0;
    UINT8_TO_STREAM(p, ui8);
    ui8 = p_data->get_pt ? 1 : 0;
    UINT8_TO_STREAM(p, ui8);
    ui8 = p_data->fixed ? 1 : 0;
    UINT8_TO_STREAM(p, ui8);
    UINT16_TO_STREAM(p, p_data->start_idx);
    UINT16_TO_STREAM(p, p_data->po);
    UINT16_TO_STREAM(p, p_data->dst);
    UINT16_TO_STREAM(p, p_data->update_id);
    UINT8_TO_STREAM(p, p_data->po_cnt);
    UINT8_TO_STREAM(p, p_data->pt_cnt);
    for (ui8 = 0; ui8 < (p_data->po_cnt + p_data->pt_cnt); ui8++)
    {
        UINT16_TO_STREAM(p, p_data->dependents[ui8].addr);
        UINT8_TO_STREAM(p, p_data->dependents[ui8].sec_elem_cnt);
    }

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_DF_FORWARDING_TABLE_DEPENDENTS_GET_STATUS, (uint8_t*)p_hci_event, (uint16_t)(p - (uint8_t*)p_hci_event));
}
static void mesh_provisioner_hci_event_df_forwarding_table_entries_count_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_df_forwarding_table_entries_count_status_data_t* p_data)
{
    uint8_t* p = p_hci_event->data;
    WICED_BT_TRACE("df forwarding_table_entries_count_status src:%x\n", p_hci_event->src);
    UINT8_TO_STREAM(p, p_data->status);
    UINT16_TO_STREAM(p, p_data->netkey_idx);
    UINT16_TO_STREAM(p, p_data->update_id);
    UINT16_TO_STREAM(p, p_data->fixed);
    UINT16_TO_STREAM(p, p_data->non_fixed);
    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_DF_FORWARDING_TABLE_ENTRIES_COUNT_STATUS, (uint8_t*)p_hci_event, (uint16_t)(p - (uint8_t*)p_hci_event));
}
static void mesh_provisioner_hci_event_df_forwarding_table_entries_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_df_forwarding_table_entries_status_data_t* p_data)
{
    uint8_t i, ui8;
    wiced_bt_mesh_df_forwarding_table_entry_t* entry;
    uint8_t* p = p_hci_event->data;
    WICED_BT_TRACE("df forwarding_table_entries_status src:%x\n", p_hci_event->src);
    UINT8_TO_STREAM(p, p_data->status);
    UINT16_TO_STREAM(p, p_data->netkey_idx);
    ui8 = p_data->fixed ? 1 : 0;
    UINT8_TO_STREAM(p, ui8);
    ui8 = p_data->non_fixed ? 1 : 0;
    UINT8_TO_STREAM(p, ui8);
    UINT16_TO_STREAM(p, p_data->start_idx);
    UINT16_TO_STREAM(p, p_data->po);
    UINT16_TO_STREAM(p, p_data->dst);
    UINT16_TO_STREAM(p, p_data->update_id);
    for (i = 0; i < p_data->entries_cnt; i++)
    {
        entry = &p_data->entries[i];
        ui8 = entry->fixed ? 1 : 0;
        UINT8_TO_STREAM(p, ui8);
        ui8 = entry->back_validated ? 1 : 0;
        UINT8_TO_STREAM(p, ui8);
        UINT16_TO_STREAM(p, entry->po.addr);
        UINT8_TO_STREAM(p, entry->po.sec_elem_cnt);
        UINT16_TO_STREAM(p, entry->po_dependents_cnt);
        UINT16_TO_STREAM(p, entry->po_bearer);
        UINT16_TO_STREAM(p, entry->pt.addr);
        UINT8_TO_STREAM(p, entry->pt.sec_elem_cnt);
        UINT16_TO_STREAM(p, entry->pt_dependents_cnt);
        UINT16_TO_STREAM(p, entry->pt_bearer);
        if (!entry->fixed)
        {
            UINT8_TO_STREAM(p, entry->non_fixed.lane_cnt);
            UINT16_TO_STREAM(p, entry->non_fixed.path_remaining_time);
            UINT8_TO_STREAM(p, entry->non_fixed.po_fn);
        }
    }
    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_DF_FORWARDING_TABLE_ENTRIES_STATUS, (uint8_t*)p_hci_event, (uint16_t)(p - (uint8_t*)p_hci_event));
}
static void mesh_provisioner_hci_event_df_wanted_lanes_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_df_wanted_lanes_status_data_t* p_data)
{
    uint8_t* p = p_hci_event->data;
    WICED_BT_TRACE("df wanted_lanes_status src:%x\n", p_hci_event->src);
    UINT8_TO_STREAM(p, p_data->status);
    UINT16_TO_STREAM(p, p_data->netkey_idx);
    UINT8_TO_STREAM(p, p_data->wanted_lines);
    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_DF_WANTED_LANES_STATUS, (uint8_t*)p_hci_event, (uint16_t)(p - (uint8_t*)p_hci_event));
}
static void mesh_provisioner_hci_event_df_two_way_path_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_df_two_way_path_status_data_t* p_data)
{
    uint8_t ui8;
    uint8_t* p = p_hci_event->data;
    WICED_BT_TRACE("df two_way_path_status src:%x\n", p_hci_event->src);
    UINT8_TO_STREAM(p, p_data->status);
    UINT16_TO_STREAM(p, p_data->netkey_idx);
    ui8 = p_data->two_way_path ? 1 : 0;
    UINT8_TO_STREAM(p, ui8);
    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_DF_TWO_WAY_PATH_STATUS, (uint8_t*)p_hci_event, (uint16_t)(p - (uint8_t*)p_hci_event));
}
static void mesh_provisioner_hci_event_df_path_echo_interval_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_df_path_echo_interval_status_data_t* p_data)
{
    uint8_t* p = p_hci_event->data;
    WICED_BT_TRACE("df path_echo_interval_status src:%x\n", p_hci_event->src);
    UINT8_TO_STREAM(p, p_data->status);
    UINT16_TO_STREAM(p, p_data->netkey_idx);
    UINT8_TO_STREAM(p, p_data->unicast);
    UINT8_TO_STREAM(p, p_data->multicast);
    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_DF_PATH_ECHO_INTERVAL_STATUS, (uint8_t*)p_hci_event, (uint16_t)(p - (uint8_t*)p_hci_event));
}
static void mesh_provisioner_hci_event_df_directed_network_transmit_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_df_directed_transmit_status_data_t* p_data)
{
    uint8_t* p = p_hci_event->data;
    WICED_BT_TRACE("df directed_network_transmit_status src:%x\n", p_hci_event->src);
    UINT8_TO_STREAM(p, p_data->count);
    UINT8_TO_STREAM(p, p_data->interval);
    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_DF_DIRECTED_NETWORK_TRANSMIT_STATUS, (uint8_t*)p_hci_event, (uint16_t)(p - (uint8_t*)p_hci_event));
}
static void mesh_provisioner_hci_event_df_directed_relay_retransmit_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_df_directed_transmit_status_data_t* p_data)
{
    uint8_t* p = p_hci_event->data;
    WICED_BT_TRACE("df directed_relay_retransmit_status src:%x\n", p_hci_event->src);
    UINT8_TO_STREAM(p, p_data->count);
    UINT8_TO_STREAM(p, p_data->interval);
    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_DF_DIRECTED_RELAY_RETRANSMIT_STATUS, (uint8_t*)p_hci_event, (uint16_t)(p - (uint8_t*)p_hci_event));
}
static void mesh_provisioner_hci_event_df_rssi_threshold_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_df_rssi_threshold_status_data_t* p_data)
{
    uint8_t* p = p_hci_event->data;
    WICED_BT_TRACE("df rssi_threshold_status src:%x\n", p_hci_event->src);
    UINT8_TO_STREAM(p, p_data->default_threshold);
    UINT8_TO_STREAM(p, p_data->margin);
    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_DF_RSSI_THRESHOLD_STATUS, (uint8_t*)p_hci_event, (uint16_t)(p - (uint8_t*)p_hci_event));
}
static void mesh_provisioner_hci_event_df_directed_paths_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_df_directed_paths_status_data_t* p_data)
{
    uint8_t* p = p_hci_event->data;
    WICED_BT_TRACE("df directed_paths_status src:%x\n", p_hci_event->src);
    UINT16_TO_STREAM(p, p_data->node_paths);
    UINT16_TO_STREAM(p, p_data->relay_paths);
    UINT16_TO_STREAM(p, p_data->proxy_paths);
    UINT16_TO_STREAM(p, p_data->friend_paths);
    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_DF_DIRECTED_PATHS_STATUS, (uint8_t*)p_hci_event, (uint16_t)(p - (uint8_t*)p_hci_event));
}
static void mesh_provisioner_hci_event_df_directed_publish_policy_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_df_directed_publish_policy_status_data_t* p_data)
{
    uint8_t ui8;
    uint8_t* p = p_hci_event->data;
    WICED_BT_TRACE("df directed_publish_policy_status src:%x\n", p_hci_event->src);
    UINT8_TO_STREAM(p, p_data->status);
    ui8 = p_data->directed ? 1 : 0;
    UINT8_TO_STREAM(p, ui8);
    UINT16_TO_STREAM(p, p_data->elem_addr);
    UINT16_TO_STREAM(p, p_data->company_id);
    UINT16_TO_STREAM(p, p_data->model_id);
    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_DF_DIRECTED_PUBLISH_POLICY_STATUS, (uint8_t*)p_hci_event, (uint16_t)(p - (uint8_t*)p_hci_event));
}
static void mesh_provisioner_hci_event_df_path_discovery_timing_control_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_df_path_discovery_timing_control_status_data_t* p_data)
{
    uint8_t ui8;
    uint8_t* p = p_hci_event->data;
    WICED_BT_TRACE("df path_discovery_timing_control_status src:%x\n", p_hci_event->src);
    UINT16_TO_STREAM(p, p_data->path_monitoring_interval);
    UINT16_TO_STREAM(p, p_data->path_discovery_retry_interval);
    ui8 = p_data->path_discovery_interval_high ? 1 : 0;
    UINT8_TO_STREAM(p, ui8);
    ui8 = p_data->lane_discovery_guard_interval_high ? 1 : 0;
    UINT8_TO_STREAM(p, ui8);
    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_DF_PATH_DISCOVERY_TIMING_CONTROL_STATUS, (uint8_t*)p_hci_event, (uint16_t)(p - (uint8_t*)p_hci_event));
}
static void mesh_provisioner_hci_event_df_directed_control_network_transmit_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_df_directed_transmit_status_data_t* p_data)
{
    uint8_t* p = p_hci_event->data;
    WICED_BT_TRACE("df directed_control_network_transmit_status src:%x\n", p_hci_event->src);
    UINT8_TO_STREAM(p, p_data->count);
    UINT8_TO_STREAM(p, p_data->interval);
    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_DF_DIRECTED_CONTROL_NETWORK_TRANSMIT_STATUS, (uint8_t*)p_hci_event, (uint16_t)(p - (uint8_t*)p_hci_event));
}
static void mesh_provisioner_hci_event_df_directed_control_relay_retransmit_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_df_directed_transmit_status_data_t* p_data)
{
    uint8_t* p = p_hci_event->data;
    WICED_BT_TRACE("df directed_control_relay_retransmit_status src:%x\n", p_hci_event->src);
    UINT8_TO_STREAM(p, p_data->count);
    UINT8_TO_STREAM(p, p_data->interval);
    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_DF_DIRECTED_CONTROL_RELAY_RETRANSMIT_STATUS, (uint8_t*)p_hci_event, (uint16_t)(p - (uint8_t*)p_hci_event));
}
#endif
#ifdef NETWORK_FILTER_SERVER_SUPPORTED
void mesh_provisioner_hci_event_network_filter_status_send(wiced_bt_mesh_hci_event_t* p_hci_event, wiced_bt_mesh_network_filter_status_data_t* p_data)
{
    uint8_t* p = p_hci_event->data;
    uint8_t i;

    WICED_BT_TRACE("network filter status src:%x\n", p_hci_event->src);

    UINT8_TO_STREAM(p, p_data->status);
    UINT16_TO_STREAM(p, p_data->netkey_idx);
    UINT8_TO_STREAM(p, p_data->filter_mode);
    UINT8_TO_STREAM(p, p_data->addr_cnt);
    for (i = 0; i < p_data->addr_cnt; i++)
    {
        UINT16_TO_STREAM(p, p_data->addr[i]);
    }

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_NETWORK_FILTER_STATUS, (uint8_t*)p_hci_event, (uint16_t)(p - (uint8_t*)p_hci_event));
}

#endif

#ifdef LARGE_COMPOSITION_DATA_SUPPORTED
void mesh_provisioner_hci_event_large_compos_data_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_large_compos_data_status_data_t *p_data)
{
    uint8_t *p = p_hci_event->data;

    WICED_BT_TRACE("large comp status src:%x page:%d offset:%d len:%d\n", p_hci_event->src, p_data->page, p_data->offset, p_data->data_len);

    UINT8_TO_STREAM(p, p_data->page);
    UINT16_TO_STREAM(p, p_data->offset);
    UINT16_TO_STREAM(p, p_data->total_size);
    ARRAY_TO_STREAM(p, p_data->p_data, p_data->data_len);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_LARGE_COMPOS_DATA_STATUS, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}

void mesh_provisioner_hci_event_models_metadata_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_models_metadata_status_data_t *p_data)
{
    uint8_t *p = p_hci_event->data;

    WICED_BT_TRACE("metadata status src:%x page:%d offset:%d len:%d\n", p_hci_event->src, p_data->page, p_data->offset, p_data->data_len);

    UINT8_TO_STREAM(p, p_data->page);
    UINT16_TO_STREAM(p, p_data->offset);
    UINT16_TO_STREAM(p, p_data->total_size);
    ARRAY_TO_STREAM(p, p_data->p_data, p_data->data_len);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_MODELS_METADATA_STATUS, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}
#endif

void mesh_provisioner_hci_event_gatt_proxy_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_gatt_proxy_status_data_t *p_data)
{
    uint8_t *p = p_hci_event->data;

    WICED_BT_TRACE("gatt_proxy status src:%x\n", p_hci_event->src);

    UINT8_TO_STREAM(p, p_data->state);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_GATT_PROXY_STATUS, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}

void mesh_provisioner_hci_event_beacon_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_beacon_status_data_t *p_data)
{
    uint8_t *p = p_hci_event->data;

    WICED_BT_TRACE("beacon status src:%x\n", p_hci_event->src);

    UINT8_TO_STREAM(p, p_data->state);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_BEACON_STATUS, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}

#ifdef SAR_CONFIGURATION_SUPPORTED
void mesh_provisioner_hci_event_sar_transmitter_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_sar_xmtr_t *p_data)
{
    uint8_t *p = p_hci_event->data;

    WICED_BT_TRACE("SAR transmitter status src:%x\n", p_hci_event->src);

    memcpy(p, p_data, MESH_SAR_XMTR_PARAMS_LEN);
    p += MESH_SAR_XMTR_PARAMS_LEN;

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_SAR_TRANSMITTER_STATUS, (uint8_t*)p_hci_event, (uint16_t)(p - (uint8_t*)p_hci_event));
}

void mesh_provisioner_hci_event_sar_receiver_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_sar_rcvr_t *p_data)
{
    uint8_t *p = p_hci_event->data;

    WICED_BT_TRACE("SAR receiver status src:%x\n", p_hci_event->src);

    memcpy(p, p_data, MESH_SAR_RCVR_PARAMS_LEN);
    p += MESH_SAR_RCVR_PARAMS_LEN;

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_SAR_RECEIVER_STATUS, (uint8_t*)p_hci_event, (uint16_t)(p - (uint8_t*)p_hci_event));
}
#endif

#ifdef PRIVATE_PROXY_SUPPORTED
void mesh_provisioner_hci_event_private_beacon_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_private_beacon_status_data_t *p_data)
{
    uint8_t* p = p_hci_event->data;

    WICED_BT_TRACE("private beacon status src:%x\n", p_hci_event->src);

    UINT8_TO_STREAM(p, p_data->state);
    UINT8_TO_STREAM(p, p_data->random_update_interval);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_PRIVATE_BEACON_STATUS, (uint8_t*)p_hci_event, (uint16_t)(p - (uint8_t*)p_hci_event));
}

void mesh_provisioner_hci_event_private_gatt_proxy_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_private_gatt_proxy_status_data_t *p_data)
{
    uint8_t* p = p_hci_event->data;

    WICED_BT_TRACE("private_gatt_proxy status src:%x\n", p_hci_event->src);

    UINT8_TO_STREAM(p, p_data->state);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_PRIVATE_GATT_PROXY_STATUS, (uint8_t*)p_hci_event, (uint16_t)(p - (uint8_t*)p_hci_event));
}

void mesh_provisioner_hci_event_private_node_identity_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_private_node_identity_status_data_t *p_data)
{
    uint8_t* p = p_hci_event->data;

    WICED_BT_TRACE("private node identity status src:%x\n", p_hci_event->src);

    UINT8_TO_STREAM(p, p_data->status);
    UINT16_TO_STREAM(p, p_data->net_key_idx);
    UINT8_TO_STREAM(p, p_data->identity);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_PRIVATE_NODE_IDENTITY_STATUS, (uint8_t*)p_hci_event, (uint16_t)(p - (uint8_t*)p_hci_event));
}

void mesh_provisioner_hci_event_on_demand_private_proxy_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_on_demand_private_proxy_status_data_t *p_data)
{
    uint8_t* p = p_hci_event->data;

    WICED_BT_TRACE("on-demand private proxy status src:%x\n", p_hci_event->src);

    UINT8_TO_STREAM(p, p_data->state);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_ON_DEMAND_PRIVATE_PROXY_STATUS, (uint8_t*)p_hci_event, (uint16_t)(p - (uint8_t*)p_hci_event));
}

void mesh_provisioner_hci_event_solicitation_pdu_rpl_items_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_unicast_address_range_t *p_data)
{
    uint8_t* p = p_hci_event->data;
    uint8_t range_length = 1;

    if (p_data->length_present)
        range_length = p_data->range_length;

    WICED_BT_TRACE("solicitation PDU RPL items status src:%x\n", p_hci_event->src);

    UINT16_TO_STREAM(p, p_data->range_start);
    UINT8_TO_STREAM(p, range_length);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_SOLICITATION_PDU_RPL_ITEMS_STATUS, (uint8_t*)p_hci_event, (uint16_t)(p - (uint8_t*)p_hci_event));
}
#endif

void mesh_provisioner_hci_event_model_publication_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_model_publication_status_data_t *p_data)
{
    uint8_t *p = p_hci_event->data;

    WICED_BT_TRACE("model pub status src:%x\n", p_hci_event->src);

    UINT8_TO_STREAM(p, p_data->status);
    UINT16_TO_STREAM(p, p_data->element_addr);
    UINT16_TO_STREAM(p, p_data->company_id);
    UINT16_TO_STREAM(p, p_data->model_id);
    UINT16_TO_STREAM(p, p_data->publish_addr);
    UINT16_TO_STREAM(p, p_data->app_key_idx);
    UINT8_TO_STREAM(p, p_data->credential_flag);
    UINT8_TO_STREAM(p, p_data->publish_ttl);
    UINT32_TO_STREAM(p, p_data->publish_period);
    UINT8_TO_STREAM(p, p_data->publish_retransmit_count);
    UINT16_TO_STREAM(p, p_data->publish_retransmit_interval);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_MODEL_PUBLICATION_STATUS, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}

void mesh_provisioner_hci_event_model_subscription_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_model_subscription_status_data_t *p_data)
{
    uint8_t *p = p_hci_event->data;

    WICED_BT_TRACE("model sub status src:%x\n", p_hci_event->src);

    UINT8_TO_STREAM(p, p_data->status);
    UINT16_TO_STREAM(p, p_data->element_addr);
    UINT16_TO_STREAM(p, p_data->company_id);
    UINT16_TO_STREAM(p, p_data->model_id);
    UINT16_TO_STREAM(p, p_data->addr);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_MODEL_SUBSCRIPTION_STATUS, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}

void mesh_provisioner_hci_event_model_subscription_list_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_model_subscription_list_data_t *p_data)
{
    uint8_t *p = p_hci_event->data;
    int i;

    WICED_BT_TRACE("model sub list src:%x\n", p_hci_event->src);

    UINT8_TO_STREAM(p, p_data->status);
    UINT16_TO_STREAM(p, p_data->element_addr);
    UINT16_TO_STREAM(p, p_data->company_id);
    UINT16_TO_STREAM(p, p_data->model_id);
    for (i = 0; i < p_data->num_addr; i++)
    {
        UINT16_TO_STREAM(p, p_data->addr[i]);
    }
    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_MODEL_SUBSCRIPTION_LIST, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}

void mesh_provisioner_hci_event_netkey_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_netkey_status_data_t *p_data)
{
    uint8_t *p = p_hci_event->data;

    WICED_BT_TRACE("netkey status src:%x\n", p_hci_event->src);

    UINT8_TO_STREAM(p, p_data->status);
    UINT16_TO_STREAM(p, p_data->net_key_idx);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_NETKEY_STATUS, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}

void mesh_provisioner_hci_event_netkey_list_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_netkey_list_data_t *p_data)
{
    uint8_t *p = p_hci_event->data;
    int i;

    WICED_BT_TRACE("netkey list src:%x\n", p_hci_event->src);

    for (i = 0; i < p_data->num_keys; i++)
    {
        UINT16_TO_STREAM(p, p_data->net_key_idx[i]);
    }
    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_NETKEY_LIST, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}

void mesh_provisioner_hci_event_appkey_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_appkey_status_data_t *p_data)
{
    uint8_t *p = p_hci_event->data;

    WICED_BT_TRACE("appkey status src:%x\n", p_hci_event->src);

    UINT8_TO_STREAM(p, p_data->status);
    UINT16_TO_STREAM(p, p_data->net_key_idx);
    UINT16_TO_STREAM(p, p_data->app_key_idx);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_APPKEY_STATUS, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}

void mesh_provisioner_hci_event_appkey_list_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_appkey_list_data_t *p_data)
{
    uint8_t *p = p_hci_event->data;
    int i;

    WICED_BT_TRACE("appkey list src:%x\n", p_hci_event->src);

    UINT8_TO_STREAM(p, p_data->status);
    UINT16_TO_STREAM(p, p_data->net_key_idx);
    for (i = 0; i < p_data->num_keys; i++)
    {
        UINT16_TO_STREAM(p, p_data->app_key_idx[i]);
    }
    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_APPKEY_LIST, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}

void mesh_provisioner_hci_event_model_app_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_model_app_bind_status_data_t *p_data)
{
    uint8_t *p = p_hci_event->data;

    WICED_BT_TRACE("model app status src:%x\n", p_hci_event->src);

    UINT8_TO_STREAM(p, p_data->status);
    UINT16_TO_STREAM(p, p_data->element_addr);
    UINT16_TO_STREAM(p, p_data->company_id);
    UINT16_TO_STREAM(p, p_data->model_id);
    UINT16_TO_STREAM(p, p_data->app_key_idx);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_MODEL_APP_BIND_STATUS, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}

void mesh_provisioner_hci_event_model_app_list_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_model_app_list_data_t *p_data)
{
    uint8_t *p = p_hci_event->data;
    int i;

    WICED_BT_TRACE("model_app list src:%x\n", p_hci_event->src);

    UINT8_TO_STREAM(p, p_data->status);
    UINT16_TO_STREAM(p, p_data->element_addr);
    UINT16_TO_STREAM(p, p_data->company_id);
    UINT16_TO_STREAM(p, p_data->model_id);
    for (i = 0; i < p_data->num_keys; i++)
    {
        UINT16_TO_STREAM(p, p_data->app_key_idx[i]);
    }
    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_MODEL_APP_LIST, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}

void mesh_provisioner_hci_event_hearbeat_subscription_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_heartbeat_subscription_status_data_t *p_data)
{
    uint8_t *p = p_hci_event->data;

    WICED_BT_TRACE("subs status src:%x status:%d subs src/dst:%x/%x period:%d count:%d hops min/max:%d/%d\n",
        p_hci_event->src, p_data->status, p_data->subscription_src, p_data->subscription_dst, p_data->period, p_data->count, p_data->min_hops, p_data->max_hops);

    UINT8_TO_STREAM(p, p_data->status);
    UINT16_TO_STREAM(p, p_data->subscription_src);
    UINT16_TO_STREAM(p, p_data->subscription_dst);
    UINT32_TO_STREAM(p, p_data->period);
    UINT16_TO_STREAM(p, p_data->count);
    UINT8_TO_STREAM(p, p_data->min_hops);
    UINT8_TO_STREAM(p, p_data->max_hops);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_HEARTBEAT_SUBSCRIPTION_STATUS, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}

void mesh_provisioner_hci_event_hearbeat_publication_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_heartbeat_publication_status_data_t *p_data)
{
    uint8_t *p = p_hci_event->data;

    WICED_BT_TRACE("pubs status src:%x status:%d pubs dst:%x period:%d count:%d hops ttl:%d net_key_idx:%d\n",
        p_hci_event->src, p_data->status, p_data->publication_dst, p_data->period, p_data->count, p_data->ttl, p_data->net_key_idx);

    UINT8_TO_STREAM(p, p_data->status);
    UINT16_TO_STREAM(p, p_data->publication_dst);
    UINT32_TO_STREAM(p, p_data->period);
    UINT16_TO_STREAM(p, p_data->count);
    UINT8_TO_STREAM(p, p_data->ttl);
    UINT8_TO_STREAM(p, p_data->feature_relay);
    UINT8_TO_STREAM(p, p_data->feature_proxy);
    UINT8_TO_STREAM(p, p_data->feature_friend);
    UINT8_TO_STREAM(p, p_data->feature_low_power);
    UINT16_TO_STREAM(p, p_data->net_key_idx);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_HEARTBEAT_PUBLICATION_STATUS, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}

void mesh_provisioner_hci_event_network_transmit_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_config_network_transmit_status_data_t *p_data)
{
    uint8_t *p = p_hci_event->data;

    WICED_BT_TRACE("net xmit params src:%x count:%d interval:%d\n", p_hci_event->src, p_data->count, p_data->interval);

    UINT8_TO_STREAM(p, p_data->count);
    UINT32_TO_STREAM(p, p_data->interval);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_NETWORK_TRANSMIT_PARAMS_STATUS, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}

void mesh_provisioner_hci_event_health_current_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_health_fault_status_data_t *p_data)
{
    uint8_t *p = p_hci_event->data;

    UINT16_TO_STREAM(p, p_data->app_key_idx);
    UINT8_TO_STREAM(p, p_data->test_id);
    UINT16_TO_STREAM(p, p_data->company_id);
    ARRAY_TO_STREAM(p, p_data->fault_array, p_data->count);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_HEALTH_CURRENT_STATUS, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}

void mesh_provisioner_hci_event_health_fault_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_health_fault_status_data_t *p_data)
{
    uint8_t *p = p_hci_event->data;

    UINT16_TO_STREAM(p, p_data->app_key_idx);
    UINT8_TO_STREAM(p, p_data->test_id);
    UINT16_TO_STREAM(p, p_data->company_id);
    ARRAY_TO_STREAM(p, p_data->fault_array, p_data->count);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_HEALTH_FAULT_STATUS, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}

void mesh_provisioner_hci_event_health_period_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_health_period_status_data_t *p_data)
{
    uint8_t *p = p_hci_event->data;

    UINT16_TO_STREAM(p, p_data->app_key_idx);
    UINT8_TO_STREAM(p, p_data->divisor);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_HEALTH_PERIOD_STATUS, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}

void mesh_provisioner_hci_event_health_attention_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_health_attention_status_data_t *p_data)
{
    uint8_t *p = p_hci_event->data;

    UINT8_TO_STREAM(p, p_data->timer);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_HEALTH_ATTENTION_STATUS, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}

void mesh_provisioner_hci_event_lpn_poll_timeout_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_lpn_poll_timeout_status_data_t *p_data)
{
    uint8_t *p = p_hci_event->data;

    UINT16_TO_STREAM(p, p_data->lpn_addr);
    UINT32_TO_STREAM(p, p_data->poll_timeout);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_LPN_POLL_TIMEOUT_STATUS, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}

#ifdef MESH_DFU_SUPPORTED
void mesh_provisioner_hci_event_fw_distribution_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_fw_distribution_status_data_t *p_data)
{
    uint8_t *p = p_hci_event->data;

    UINT8_TO_STREAM(p, p_data->state);
    UINT16_TO_STREAM(p, p_data->list_size);
    UINT16_TO_STREAM(p, p_data->node_index);
    UINT16_TO_STREAM(p, p_data->num_nodes);
    for (int i = 0; i < p_data->num_nodes; i++)
    {
        UINT16_TO_STREAM(p, p_data->node[i].unicast_address);
        UINT8_TO_STREAM(p, p_data->node[i].phase);
        UINT8_TO_STREAM(p, p_data->node[i].progress);
    }

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_FW_DISTRIBUTION_STATUS, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}

void mesh_provisioner_hci_event_fw_update_metadata_status_send(wiced_bt_mesh_hci_event_t *p_hci_event, wiced_bt_mesh_dfu_metadata_status_data_t *p_data)
{
    uint8_t *p = p_hci_event->data;

    UINT8_TO_STREAM(p, p_data->status);
    UINT8_TO_STREAM(p, p_data->add_info);
    UINT8_TO_STREAM(p, p_data->index);

    mesh_transport_send_data(HCI_CONTROL_MESH_EVENT_FW_UPDATE_METADATA_STATUS, (uint8_t *)p_hci_event, (uint16_t)(p - (uint8_t *)p_hci_event));
}
#endif

// ToDo. Currently provisioner client will only scan passive not returning the name of the unprovisioned device
// which is in the scan response.  To change that, we need to use wiced_bt_ble_scan instead of wiced_bt_ble_observe.
void wiced_bt_ble_set_scan_mode(uint8_t is_active)
{
}

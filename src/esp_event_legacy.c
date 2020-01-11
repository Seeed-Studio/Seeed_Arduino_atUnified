// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "esp_event_legacy.h"

/**
  * @brief  Send a event to event task
  *
  * @attention 1. Other task/modules, such as the TCPIP module, can call this API to send an event to event task
  *
  * @param  system_event_t * event : event
  *
  * @return ESP_OK : succeed
  * @return others : fail
  */
esp_err_t esp_event_send(system_event_t *event) {}

/**
  * @brief  Default event handler for system events
  *
  * This function performs default handling of system events.
  * When using esp_event_loop APIs, it is called automatically before invoking the user-provided
  * callback function.
  *
  * Applications which implement a custom event loop must call this function
  * as part of event processing.
  *
  * @param  event pointer to event to be handled
  * @return ESP_OK if an event was handled successfully
  */
esp_err_t esp_event_process_default(system_event_t *event) {}

/**
  * @brief  Install default event handlers for Ethernet interface
  *
  */
void esp_event_set_default_eth_handlers() {}

/**
  * @brief  Install default event handlers for Wi-Fi interfaces (station and AP)
  *
  */
void esp_event_set_default_wifi_handlers() {}

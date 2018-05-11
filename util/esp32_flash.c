/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
Routines for handling the (slightly more complicated) esp32 flash.
Broken out because esp-idf is expected to get better routines for this.
*/

#include <libesphttpd/esp.h>
#ifdef ESP32
#include "esp_log.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "rom/cache.h"
#include "rom/ets_sys.h"
#include "rom/spi_flash.h"
#include "rom/crc.h"
#include "rom/rtc.h"
#include "esp_partition.h"
#include "esp_ota_ops.h"

static const char *TAG = "esp32_flash.c";

const esp_partition_t *update_partition = NULL;
int esp32flashGetUpdateMem(uint32_t *loc, uint32_t *size) {
	const esp_partition_t *configured = esp_ota_get_boot_partition();
    const esp_partition_t *running = esp_ota_get_running_partition();
    
    if (configured != running) {
        ESP_LOGW(TAG, "Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x", configured->address, running->address);
        ESP_LOGW(TAG, "(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)");
    }
    ESP_LOGI(TAG, "Running partition type %d subtype %d (offset 0x%08x)", running->type, running->subtype, running->address);
	update_partition = esp_ota_get_next_update_partition(NULL);
    if( update_partition ){
    	ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%x", update_partition->subtype, update_partition->address );
	    *loc  = update_partition->address;
	    *size = update_partition->size;
	    return 1;
    }
    return 0;
}

int esp32flashSetOtaAsCurrentImage() {
	if( update_partition==NULL ){
		ESP_LOGE(TAG, "no partition to set the boot flag. update it first");
		return 0;
	}
	int err = esp_ota_set_boot_partition(update_partition);
	if (err != ESP_OK) {
	    ESP_LOGE(TAG, "esp_ota_set_boot_partition failed! err=0x%x", err);
	    return 0;
	}
	ESP_LOGI(TAG, "esp_ota_set_boot_partition succeeded (%d) !!!!!", err);
	return 1;
}

int esp32flashRebootIntoOta() {
	software_reset();
	return 1;
}


#endif

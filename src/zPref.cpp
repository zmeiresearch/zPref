/*==============================================================================
   zPref - NVS-backed preferences library for ESP32

   Copyright 2020-2026 Ivan Vasilev, Zmei Research Ltd.

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
  ============================================================================*/

//==============================================================================
//  Includes
//==============================================================================

#include "zPref.h"
#include <logger.h>
#include "nvs_flash.h"
#include "nvs.h"

//==============================================================================
//  Defines
//==============================================================================
#define CMP_NAME                "zPref"

//==============================================================================
//  Local types
//==============================================================================

//==============================================================================
//  Local function definitions
//==============================================================================

//==============================================================================
//  Local data
//==============================================================================

//==============================================================================
//  Local functions
//==============================================================================

//==============================================================================
//  Exported data
//==============================================================================

//==============================================================================
//  NVS helper method implementations
//==============================================================================

// NVS get implementations
bool zPrefBase::nvs_getBool(const char* key, bool default_value) {
    int8_t value;
    esp_err_t err = nvs_get_i8(nvs_handle(), key, &value);
    return (err == ESP_OK) ? (bool)value : default_value;
}

int8_t zPrefBase::nvs_getChar(const char* key, int8_t default_value) {
    int8_t value;
    esp_err_t err = nvs_get_i8(nvs_handle(), key, &value);
    return (err == ESP_OK) ? value : default_value;
}

uint8_t zPrefBase::nvs_getUChar(const char* key, uint8_t default_value) {
    uint8_t value;
    esp_err_t err = nvs_get_u8(nvs_handle(), key, &value);
    return (err == ESP_OK) ? value : default_value;
}

int16_t zPrefBase::nvs_getShort(const char* key, int16_t default_value) {
    int16_t value;
    esp_err_t err = nvs_get_i16(nvs_handle(), key, &value);
    return (err == ESP_OK) ? value : default_value;
}

uint16_t zPrefBase::nvs_getUShort(const char* key, uint16_t default_value) {
    uint16_t value;
    esp_err_t err = nvs_get_u16(nvs_handle(), key, &value);
    return (err == ESP_OK) ? value : default_value;
}

int32_t zPrefBase::nvs_getInt(const char* key, int32_t default_value) {
    int32_t value;
    esp_err_t err = nvs_get_i32(nvs_handle(), key, &value);
    return (err == ESP_OK) ? value : default_value;
}

uint32_t zPrefBase::nvs_getUInt(const char* key, uint32_t default_value) {
    uint32_t value;
    esp_err_t err = nvs_get_u32(nvs_handle(), key, &value);
    return (err == ESP_OK) ? value : default_value;
}

int32_t zPrefBase::nvs_getLong(const char* key, int32_t default_value) {
    int32_t value;
    esp_err_t err = nvs_get_i32(nvs_handle(), key, &value);
    return (err == ESP_OK) ? value : default_value;
}

uint32_t zPrefBase::nvs_getULong(const char* key, uint32_t default_value) {
    uint32_t value;
    esp_err_t err = nvs_get_u32(nvs_handle(), key, &value);
    return (err == ESP_OK) ? value : default_value;
}

int64_t zPrefBase::nvs_getLong64(const char* key, int64_t default_value) {
    int64_t value;
    esp_err_t err = nvs_get_i64(nvs_handle(), key, &value);
    return (err == ESP_OK) ? value : default_value;
}

uint64_t zPrefBase::nvs_getULong64(const char* key, uint64_t default_value) {
    uint64_t value;
    esp_err_t err = nvs_get_u64(nvs_handle(), key, &value);
    return (err == ESP_OK) ? value : default_value;
}

String zPrefBase::nvs_getString(const char* key, String default_value) {
    size_t required_size = 0;
    esp_err_t err = nvs_get_str(nvs_handle(), key, NULL, &required_size);
    if (err != ESP_OK) {
        return default_value;
    }

    char* value = (char*)malloc(required_size);
    if (value == NULL) {
        return default_value;
    }

    err = nvs_get_str(nvs_handle(), key, value, &required_size);
    if (err != ESP_OK) {
        free(value);
        return default_value;
    }

    String result = String(value);
    free(value);
    return result;
}

// NVS set implementations
size_t zPrefBase::nvs_putBool(const char* key, bool value) {
    int8_t val = (int8_t)value;
    esp_err_t err = nvs_set_i8(nvs_handle(), key, val);
    return (err == ESP_OK) ? 1 : 0;
}

size_t zPrefBase::nvs_putChar(const char* key, int8_t value) {
    esp_err_t err = nvs_set_i8(nvs_handle(), key, value);
    return (err == ESP_OK) ? 1 : 0;
}

size_t zPrefBase::nvs_putUChar(const char* key, uint8_t value) {
    esp_err_t err = nvs_set_u8(nvs_handle(), key, value);
    return (err == ESP_OK) ? 1 : 0;
}

size_t zPrefBase::nvs_putShort(const char* key, int16_t value) {
    esp_err_t err = nvs_set_i16(nvs_handle(), key, value);
    return (err == ESP_OK) ? 1 : 0;
}

size_t zPrefBase::nvs_putUShort(const char* key, uint16_t value) {
    esp_err_t err = nvs_set_u16(nvs_handle(), key, value);
    return (err == ESP_OK) ? 1 : 0;
}

size_t zPrefBase::nvs_putInt(const char* key, int32_t value) {
    esp_err_t err = nvs_set_i32(nvs_handle(), key, value);
    return (err == ESP_OK) ? 1 : 0;
}

size_t zPrefBase::nvs_putUInt(const char* key, uint32_t value) {
    esp_err_t err = nvs_set_u32(nvs_handle(), key, value);
    return (err == ESP_OK) ? 1 : 0;
}

size_t zPrefBase::nvs_putLong(const char* key, int32_t value) {
    esp_err_t err = nvs_set_i32(nvs_handle(), key, value);
    return (err == ESP_OK) ? 1 : 0;
}

size_t zPrefBase::nvs_putULong(const char* key, uint32_t value) {
    esp_err_t err = nvs_set_u32(nvs_handle(), key, value);
    return (err == ESP_OK) ? 1 : 0;
}

size_t zPrefBase::nvs_putLong64(const char* key, int64_t value) {
    esp_err_t err = nvs_set_i64(nvs_handle(), key, value);
    return (err == ESP_OK) ? 1 : 0;
}

size_t zPrefBase::nvs_putULong64(const char* key, uint64_t value) {
    esp_err_t err = nvs_set_u64(nvs_handle(), key, value);
    return (err == ESP_OK) ? 1 : 0;
}

size_t zPrefBase::nvs_putString(const char* key, String value) {
    esp_err_t err = nvs_set_str(nvs_handle(), key, value.c_str());
    return (err == ESP_OK) ? value.length() : 0;
}

//==============================================================================
//  Exported functions
//==============================================================================
void zPrefBase::commit()
{
    LOG(eLogDebug, "Committing NVS changes");
    // Commit any pending changes to NVS
    esp_err_t err = nvs_commit(nvs_handle());
    if (err != ESP_OK) {
        LOG(eLogWarn, "Error committing NVS changes: %s", esp_err_to_name(err));
    }
}

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
//  Multi-include guard
//==============================================================================
#pragma once

//==============================================================================
//  Includes
//==============================================================================
#include "zPrefBase.h"
#include "nvs_flash.h"
#include "nvs.h"

//==============================================================================
//  Defines
//==============================================================================
#define CONFIG_VERSION_KEY      "CfgVersion"    // Maximum 15 characters per NVS spec

//==============================================================================
//  Exported types
//==============================================================================

/**
 * @brief Base class for NVS-backed preferences
 *
 * Users should inherit from this class and define their own configuration
 * variables using the DECLARE_CONFIG_VARIABLE macro.
 *
 * Example:
 * @code
 * class MyConfig : public zPref<MyConfig> {
 *     public:
 *         DECLARE_CONFIG_VARIABLE(String, DeviceName);
 *         DECLARE_CONFIG_VARIABLE(Bool, EnableWifi);
 *
 *         MyConfig() : zPref<MyConfig>("MyApp", 1) {}
 * };
 * @endcode
 */
template<typename TDerived>
class zPref : public zPrefBase
{
    private:
        nvs_handle_t _nvs_handle;
        eStatus status = eNOTINITIALIZED;
        const char* _partition_name;
        const char* _namespace;
        uint32_t _currentVersion;

    public:
        nvs_handle_t& nvs_handle() { return _nvs_handle; };    // For zPrefBase

    public:
        /**
         * @brief Constructor
         * @param nvs_namespace NVS namespace (max 15 characters)
         * @param currentVersion Current configuration version (for migration)
         */
        zPref(const char* nvs_namespace = "zPref", uint32_t currentVersion = 1) :
            _nvs_handle(0),
            _partition_name(NVS_DEFAULT_PART_NAME),
            _namespace(nvs_namespace),
            _currentVersion(currentVersion) {};

        /**
         * @brief Initialize the NVS partition and open the namespace
         * @param partition_name NVS partition name (defaults to NVS_DEFAULT_PART_NAME)
         * @return eStatus - eOK on success, eFAILED on error
         */
        eStatus Init(const char* partition_name = NVS_DEFAULT_PART_NAME) {
            eStatus retVal = eOK;
            status = eINPROGRESS;
            _partition_name = partition_name;
            LOG(eLogInfo, "Initializing NVS partition: %s, namespace: %s", _partition_name, _namespace);

            // Initialize NVS flash partition
            esp_err_t err = nvs_flash_init_partition(_partition_name);
            if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
                LOG(eLogWarn, "NVS partition needs erasing, erasing partition %s", _partition_name);
                ESP_ERROR_CHECK(nvs_flash_erase_partition(_partition_name));
                err = nvs_flash_init_partition(_partition_name);
            }

            if (err != ESP_OK) {
                LOG(eLogWarn, "Error initializing NVS flash partition %s: %s", _partition_name, esp_err_to_name(err));
                retVal = eFAILED;
            } else {
                // Open NVS handle with the specified partition
                err = nvs_open_from_partition(_partition_name, _namespace, NVS_READWRITE, &_nvs_handle);
                if (err != ESP_OK) {
                    LOG(eLogWarn, "Error opening NVS namespace %s in partition %s: %s",
                        _namespace, _partition_name, esp_err_to_name(err));
                    retVal = eFAILED;
                } else {
                    // Read stored configuration version
                    uint32_t storedVersion = nvs_getULong(CONFIG_VERSION_KEY, 0);

                    // Check if migration is needed
                    if (storedVersion != _currentVersion) {
                        LOG(eLogInfo, "Configuration version mismatch: stored=%d, current=%d",
                            storedVersion, _currentVersion);

                        // Call user's initialization/migration hook
                        retVal = static_cast<TDerived*>(this)->OnInit(storedVersion, _currentVersion);

                        // If migration was successful, update the stored version
                        if (retVal == eOK) {
                            nvs_putULong(CONFIG_VERSION_KEY, _currentVersion);
                            commit();
                            LOG(eLogInfo, "Configuration version updated to %d", _currentVersion);
                        }
                    } else {
                        // Versions match, just call OnInit with matching versions
                        retVal = static_cast<TDerived*>(this)->OnInit(storedVersion, _currentVersion);
                    }
                }
            }

            status = (retVal == eOK) ? eOK : eFAILED;
            return retVal;
        }

        /**
         * @brief Reset all configuration variables to defaults
         * @return eStatus - eOK on success
         *
         * Users should override this method to reset their specific variables
         */
        virtual eStatus Reset() {
            LOG(eLogInfo, "Reset called - override this method to reset your variables");
            return eOK;
        }

        /**
         * @brief Hook called after NVS is initialized
         * @param storedVersion The configuration version stored in NVS (0 if new/erased)
         * @param currentVersion The current configuration version from constructor
         * @return eStatus - eOK to continue, eFAILED to abort initialization
         *
         * Override this method to perform version migration or other initialization tasks.
         * The library automatically stores the current version after successful OnInit().
         *
         * Example:
         * @code
         * eStatus OnInit(uint32_t storedVersion, uint32_t currentVersion) override {
         *     if (storedVersion < currentVersion) {
         *         LOG(eLogInfo, "Migrating from v%d to v%d", storedVersion, currentVersion);
         *         // Perform migration based on versions
         *         if (storedVersion == 0) {
         *             // First boot or after erase - set all defaults
         *             Reset();
         *         } else if (storedVersion == 1 && currentVersion == 2) {
         *             // Migrate from v1 to v2
         *             NewVariable.SetDefault();
         *         }
         *     }
         *     return eOK;
         * }
         * @endcode
         */
        virtual eStatus OnInit(uint32_t storedVersion, uint32_t currentVersion) {
            (void)storedVersion;
            (void)currentVersion;
            return eOK;
        }

        /**
         * @brief Close the NVS handle
         */
        void End() {
            LOG(eLogInfo, "Closing NVS handle");
            if (_nvs_handle != 0) {
                nvs_close(_nvs_handle);
                _nvs_handle = 0;
            }
        }

        /**
         * @brief Get the current status
         * @return eStatus
         */
        eStatus Status() { return status; };
};

//==============================================================================
//  Exported data
//==============================================================================

//==============================================================================
//  Exported functions
//==============================================================================

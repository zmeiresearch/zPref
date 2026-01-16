/*==============================================================================
   zPref Example - Basic Usage

   This example demonstrates how to create a custom configuration class
   using the zPref library.
  ============================================================================*/

#include <Arduino.h>
#include <logger.h>
#include <zPref.h>

//==============================================================================
// Define your default values
//==============================================================================
#define CONFIG_DEFAULT_DeviceName       "MyESP32Device"
#define CONFIG_DEFAULT_WifiSSID         "MyWifiNetwork"
#define CONFIG_DEFAULT_WifiPassword     "MyPassword123"
#define CONFIG_DEFAULT_EnableDebug      true
#define CONFIG_DEFAULT_ServerPort       8080
#define CONFIG_DEFAULT_UpdateInterval   60000LL  // milliseconds

//==============================================================================
// Define your configuration class
//==============================================================================
class MyConfig : public zPref
{
    private:
        static const uint32_t CONFIG_VERSION = 1;

    public:
        // Declare your configuration variables using the macro
        DECLARE_CONFIG_VARIABLE(String,  DeviceName);
        DECLARE_CONFIG_VARIABLE(String,  WifiSSID);
        DECLARE_CONFIG_VARIABLE(String,  WifiPassword);
        DECLARE_CONFIG_VARIABLE(Bool,    EnableDebug);
        DECLARE_CONFIG_VARIABLE(UShort,  ServerPort);
        DECLARE_CONFIG_VARIABLE(Long64,  UpdateInterval);

    public:
        MyConfig() : zPref("MyApp", CONFIG_VERSION) {}

        // Override Reset to set all variables to defaults
        eStatus Reset() override {
            LOG(eLogInfo, "Resetting configuration to defaults");
            DeviceName.SetDefault();
            WifiSSID.SetDefault();
            WifiPassword.SetDefault();
            EnableDebug.SetDefault();
            ServerPort.SetDefault();
            UpdateInterval.SetDefault();
            return eOK;
        }

        // Override OnInit for version migration
        eStatus OnInit(uint32_t storedVersion, uint32_t currentVersion) override {
            LOG(eLogInfo, "Configuration initialized: stored v%d, current v%d",
                storedVersion, currentVersion);

            // Handle version migration
            if (storedVersion < currentVersion) {
                LOG(eLogInfo, "Migrating configuration from v%d to v%d",
                    storedVersion, currentVersion);

                // Version 0 means first boot or after erase
                if (storedVersion == 0) {
                    LOG(eLogInfo, "First boot - initializing with defaults");
                    Reset();
                }

                // Example: Migration from version 1 to 2
                // if (storedVersion == 1 && currentVersion >= 2) {
                //     LOG(eLogInfo, "Migrating from v1 to v2");
                //     // Add any new variables with defaults
                //     // NewVariable.SetDefault();
                // }
            }

            return eOK;
        }
};

//==============================================================================
// Create a global instance
//==============================================================================
MyConfig Config;

//==============================================================================
// Setup
//==============================================================================
void setup() {
    Serial.begin(115200);
    delay(1000);

    // Initialize the logger
    LogInit(NULL);
    LOG(eLogInfo, "Starting zPref example");

    // Initialize the configuration
    eStatus status = Config.Init();
    if (status != eOK) {
        LOG(eLogError, "Failed to initialize configuration!");
        return;
    }

    // Read configuration values
    LOG(eLogInfo, "Current configuration:");
    LOG(eLogInfo, "  DeviceName: %s", Config.DeviceName.Get().c_str());
    LOG(eLogInfo, "  WifiSSID: %s", Config.WifiSSID.Get().c_str());
    LOG(eLogInfo, "  EnableDebug: %d", Config.EnableDebug.Get());
    LOG(eLogInfo, "  ServerPort: %d", Config.ServerPort.Get());
    LOG(eLogInfo, "  UpdateInterval: %lld", Config.UpdateInterval.Get());

    // Modify a configuration value
    LOG(eLogInfo, "Changing DeviceName to 'NewDeviceName'");
    Config.DeviceName = String("NewDeviceName");

    // Use the () operator to get values
    String deviceName = Config.DeviceName();
    LOG(eLogInfo, "New DeviceName: %s", deviceName.c_str());

    // Use Set() method to set values with string
    Config.Set("ServerPort", "9090");
    LOG(eLogInfo, "New ServerPort: %d", Config.ServerPort.Get());

    // Get value as string
    String portStr = Config.GetString("ServerPort");
    LOG(eLogInfo, "ServerPort as string: %s", portStr.c_str());
}

void loop() {
    // Logger task must be called regularly
    LogTask();
    delay(10);
}

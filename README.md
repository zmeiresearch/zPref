# zPref

Type-safe NVS-backed preferences library for ESP32 with automatic persistence and version migration support.

## Features

- **Type-Safe**: Strongly typed configuration variables with compile-time checking
- **Automatic Persistence**: Values automatically saved to ESP32 NVS (Non-Volatile Storage)
- **Default Values**: Define default values at compile time
- **Multiple Data Types**: Support for String, Bool, integers (8/16/32/64-bit), and more
- **Easy Access**: Get/Set values with simple operators or methods
- **Version Migration**: Built-in hooks for configuration version upgrades
- **String Interface**: Set/Get any value as a string for CLI or API integration
- **Template-Based**: Create custom configuration classes by inheritance

## Dependencies

- **zGlobals**: Status codes and utility macros - `https://github.com/zmeiresearch/zGlobals.git`
- **zLogger**: Logging library - `https://github.com/zmeiresearch/zLogger.git`
- **ESP32 Arduino Framework**: Arduino.h
- **ESP-IDF NVS**: Non-volatile storage

## Installation

### PlatformIO

Add to your `platformio.ini`:

```ini
lib_deps =
    https://github.com/zmeiresearch/zPref.git
```

For a specific version or branch:

```ini
lib_deps =
    https://github.com/zmeiresearch/zPref.git#v1.0.0
```

## Quick Start

### 1. Define Default Values

```cpp
// Define your default values as macros
#define CONFIG_DEFAULT_DeviceName       "MyESP32"
#define CONFIG_DEFAULT_WifiSSID         "MyNetwork"
#define CONFIG_DEFAULT_WifiPassword     "MyPassword"
#define CONFIG_DEFAULT_ServerPort       8080
```

### 2. Create Your Configuration Class

```cpp
#include <zPref.h>

class MyConfig : public zPref
{
    private:
        static const uint32_t CONFIG_VERSION = 1;

    public:
        // Declare configuration variables
        DECLARE_CONFIG_VARIABLE(String, DeviceName);
        DECLARE_CONFIG_VARIABLE(String, WifiSSID);
        DECLARE_CONFIG_VARIABLE(String, WifiPassword);
        DECLARE_CONFIG_VARIABLE(UShort, ServerPort);

    public:
        MyConfig() : zPref("MyApp", CONFIG_VERSION) {}

        // Override Reset to restore defaults
        eStatus Reset() override {
            DeviceName.SetDefault();
            WifiSSID.SetDefault();
            WifiPassword.SetDefault();
            ServerPort.SetDefault();
            return eOK;
        }

        // Override OnInit for version migration
        eStatus OnInit(uint32_t storedVersion, uint32_t currentVersion) override {
            if (storedVersion == 0) {
                // First boot - initialize with defaults
                Reset();
            }
            return eOK;
        }
};

// Create global instance
MyConfig Config;
```

### 3. Use in Your Code

```cpp
void setup() {
    // Initialize
    LogInit(NULL);
    Config.Init();

    // Read values
    String name = Config.DeviceName.Get();
    uint16_t port = Config.ServerPort();  // Operator () also works

    // Write values
    Config.DeviceName = String("NewName");
    Config.ServerPort.Set(9090);

    // Values are automatically saved to NVS!
}
```

## Supported Data Types

The library supports the following types (matching ESP32 Preferences):

| Type Alias | C++ Type    | Size (bytes) | Example Default                      |
|-----------|-------------|--------------|--------------------------------------|
| Bool      | bool        | 1            | `#define CONFIG_DEFAULT_MyVar true`  |
| UChar     | uint8_t     | 1            | `#define CONFIG_DEFAULT_MyVar 42`    |
| UShort    | uint16_t    | 2            | `#define CONFIG_DEFAULT_MyVar 1234`  |
| Long64    | int64_t     | 8            | `#define CONFIG_DEFAULT_MyVar 5000LL`|
| String    | String      | variable     | `#define CONFIG_DEFAULT_MyVar "text"`|

You can also use standard types directly (int8_t, uint32_t, etc.) if you define the corresponding getter/setter macros.

## Usage Examples

### Reading Configuration Values

```cpp
// Method 1: Get()
String deviceName = Config.DeviceName.Get();

// Method 2: operator()
String deviceName = Config.DeviceName();

// Method 3: String interface
String deviceNameStr = Config.GetString("DeviceName");
```

### Writing Configuration Values

```cpp
// Method 1: Set()
Config.DeviceName.Set("NewName");

// Method 2: operator=
Config.DeviceName = String("NewName");

// Method 3: String interface
Config.Set("DeviceName", "NewName");
```

### Resetting to Defaults

```cpp
// Reset single variable
Config.DeviceName.SetDefault();

// Reset all (if you implemented Reset())
Config.Reset();
```

### Version Migration

The library automatically handles version storage and migration. Simply:
1. Specify the current version in your constructor
2. Override `OnInit()` to receive stored and current versions
3. Implement migration logic based on the version parameters

The library will:
- Automatically read the stored version from NVS
- Call your `OnInit()` with both versions as parameters
- Automatically update the stored version after successful migration

```cpp
class MyConfig : public zPref
{
    private:
        static const uint32_t CONFIG_VERSION = 2;

    public:
        DECLARE_CONFIG_VARIABLE(String, DeviceName);
        DECLARE_CONFIG_VARIABLE(String, NewVariable);

        MyConfig() : zPref("MyApp", CONFIG_VERSION) {}

        eStatus OnInit(uint32_t storedVersion, uint32_t currentVersion) override {
            if (storedVersion < currentVersion) {
                LOG(eLogInfo, "Migrating config from v%d to v%d",
                    storedVersion, currentVersion);

                // Version 0 = first boot or after NVS erase
                if (storedVersion == 0) {
                    LOG(eLogInfo, "First boot - initializing all defaults");
                    Reset();
                }
                // Migrate from v1 to v2
                else if (storedVersion == 1) {
                    LOG(eLogInfo, "Adding new variable from v2");
                    NewVariable.SetDefault();
                }
                // Can chain multiple versions
                // else if (storedVersion == 2 && currentVersion >= 3) { ... }
            }

            return eOK;
        }
};
```

**Note:** The library uses the NVS key "CfgVersion" to store the version. You don't need to create a version variable yourself.

### Custom Namespace

You can specify a custom NVS namespace (max 15 characters):

```cpp
// In constructor (with custom namespace and version)
MyConfig() : zPref("MyNamespace", 1) {}
```

### Custom Partition

Use a non-default NVS partition:

```cpp
void setup() {
    Config.Init("custom_partition");
}
```

## API Reference

### Configuration Class

Create your configuration class by inheriting from `zPref`:

```cpp
class MyConfig : public zPref {
    public:
        // Your configuration variables
        DECLARE_CONFIG_VARIABLE(String, VariableName);

        // Constructor with namespace and version
        MyConfig() : zPref("namespace", 1) {}

        // Optional overrides
        eStatus Reset() override;
        eStatus OnInit(uint32_t storedVersion, uint32_t currentVersion) override;
};
```

### Methods

#### `eStatus Init(const char* partition_name = NVS_DEFAULT_PART_NAME)`
Initialize the NVS partition and load configuration. Automatically reads the stored version from NVS and calls `OnInit()` with version parameters.

#### `eStatus Reset()`
Reset all configuration variables to defaults. Override this in your class.

#### `eStatus OnInit(uint32_t storedVersion, uint32_t currentVersion)`
Called after NVS initialization with version information. Override for version migration.

**Parameters:**
- `storedVersion`: Version stored in NVS (0 if first boot or after erase)
- `currentVersion`: Current version specified in constructor

**Returns:** `eOK` to continue initialization, `eFAILED` to abort

**Note:** The library automatically updates the stored version to `currentVersion` after successful `OnInit()`.

#### `void End()`
Close the NVS handle.

#### `eStatus Status()`
Get the current initialization status.

### Variable Access

#### `T Get()`
Get the current value.

#### `T operator()()`
Get the current value (alternative syntax).

#### `size_t Set(T value)`
Set a new value and persist to NVS.

#### `size_t operator=(T value)`
Set a new value (alternative syntax).

#### `size_t SetDefault()`
Reset this variable to its default value.

#### `String GetString()`
Get value as a string.

#### `size_t FromString(const char* value)`
Set value from a string.

### String Interface

#### `String GetString(String key)`
Get any configuration variable as a string by name.

#### `size_t Set(const char* key, const char* value)`
Set any configuration variable from a string by name.

## Advanced Usage

### Adding New Data Types

To support additional data types, you need to:

1. Define the getter/setter implementation macros
2. Ensure the type converter supports the type

Example for a custom float type:

```cpp
// In your config class header
#define GETTER_IMPL_Float [this](String _k, float _d){ return this->nvs_getFloat(_k.c_str(), _d); }
#define SETTER_IMPL_Float [this](String _k, float _v){ size_t ret = this->nvs_putFloat(_k.c_str(), _v); this->commit(); return ret; }

// You'd also need to implement nvs_getFloat and nvs_putFloat in zPrefBase
```

### Thread Safety

The library uses NVS, which handles concurrent access internally. However, if you need to perform atomic read-modify-write operations, implement your own locking mechanism.

### Error Handling

All `Set()` operations return `size_t` indicating bytes written (0 on failure):

```cpp
if (Config.DeviceName.Set("NewName") == 0) {
    LOG(eLogError, "Failed to save DeviceName");
}
```

## Notes

- **NVS Key Limit**: NVS keys are limited to 15 characters
- **Version Storage**: The library automatically stores the configuration version in NVS using the key "CfgVersion"
- **Initialization**: Always call `LogInit()` before `Config.Init()`
- **Lazy Loading**: Variables are loaded from NVS on first access for efficiency
- **Persistence**: Every `Set()` operation commits to NVS immediately
- **Memory**: String values allocate memory dynamically
- **Version Migration**: Specify the version in constructor, implement `OnInit()` for migration logic

## Example Projects

See the `examples/` directory for complete examples:
- **BasicUsage**: Simple configuration with common data types
- More examples coming soon!

## License

MIT License - see LICENSE file for details

## Author

Ivan Vasilev, Zmei Research Ltd.

## Contributing

Contributions welcome! Please submit pull requests or open issues on GitHub.

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
#include <vector>
#include <functional>
#include <Arduino.h>
#include "nvs_flash.h"
#include "nvs.h"
#include <globals.h>
#include <memory>
#include "type_converter.hpp"
#include <logger.h>

//==============================================================================
//  Defines
//==============================================================================

//==============================================================================
//  Exported types
//==============================================================================
class zPrefVariableBase {
    public:
        String _key;

    protected:
        zPrefVariableBase(String key): _key(key) {};
        virtual ~zPrefVariableBase() {};

    public:
        virtual size_t FromString(const char * const val) = 0;
        virtual bool GetString(char * const buf, size_t len) = 0;
        virtual String GetString() = 0;
};

class zPrefBase {
    public:
        virtual nvs_handle_t& nvs_handle() = 0;

    protected:
        void commit();

        // NVS helper methods for different data types
        bool nvs_getBool(const char* key, bool default_value);
        int8_t nvs_getChar(const char* key, int8_t default_value);
        uint8_t nvs_getUChar(const char* key, uint8_t default_value);
        int16_t nvs_getShort(const char* key, int16_t default_value);
        uint16_t nvs_getUShort(const char* key, uint16_t default_value);
        int32_t nvs_getInt(const char* key, int32_t default_value);
        uint32_t nvs_getUInt(const char* key, uint32_t default_value);
        int32_t nvs_getLong(const char* key, int32_t default_value);
        uint32_t nvs_getULong(const char* key, uint32_t default_value);
        int64_t nvs_getLong64(const char* key, int64_t default_value);
        uint64_t nvs_getULong64(const char* key, uint64_t default_value);
        String nvs_getString(const char* key, String default_value);

        size_t nvs_putBool(const char* key, bool value);
        size_t nvs_putChar(const char* key, int8_t value);
        size_t nvs_putUChar(const char* key, uint8_t value);
        size_t nvs_putShort(const char* key, int16_t value);
        size_t nvs_putUShort(const char* key, uint16_t value);
        size_t nvs_putInt(const char* key, int32_t value);
        size_t nvs_putUInt(const char* key, uint32_t value);
        size_t nvs_putLong(const char* key, int32_t value);
        size_t nvs_putULong(const char* key, uint32_t value);
        size_t nvs_putLong64(const char* key, int64_t value);
        size_t nvs_putULong64(const char* key, uint64_t value);
        size_t nvs_putString(const char* key, String value);

    public:
        void AddVariable(shared_ptr<zPrefVariableBase> var) {
            _variables.push_back(var);
        };
        String GetString(String key) {
            for (auto var : _variables) {
                if (key.equals(var->_key)) {
                    return var->GetString();
                }
            }
            return "";
        };
        bool GetString(const char * const key, char * const buf, size_t len) {
            for (auto var : _variables) {
                if (String(key).equals(var->_key)) {
                    return var->GetString(buf, len);
                }
            }
            return false;
        };
        size_t Set(const char * const key, const char * const val) {
            for (auto var : _variables) {
                if (String(key).equals(var->_key)) {
                    return var->FromString(val);
                }
            }
            return 0;
        };
        size_t Set(String key, String val) {
            return Set(key.c_str(), val.c_str());
        };


    protected:
        std::vector<shared_ptr<zPrefVariableBase>> _variables;
};

template<typename T>
class zPrefVariable : public zPrefVariableBase
{
    private:
        T                   _current;
        bool                initialized = false;
        const T             _default;
        std::function<T(String, T)>        _getter;
        std::function<size_t(String, T)>   _setter;
        void initialize() {
            _current = this->_getter(_key, _default);
            initialized = true;
        }

    public:
        zPrefVariable(
            const char * key,
            T defaultVal,
            zPrefBase& config,
            std::function<T(String, T)> g,
            std::function<size_t(String, T)> s):
                zPrefVariableBase(key),
                _default(defaultVal),
                _getter(g),
                _setter(s) {
                    config.AddVariable(shared_ptr<zPrefVariableBase>(this));
            };
        const T& operator()() { return Get(); };
        size_t operator=(T val) { return Set(val); };
        const T& Get() {
            if (!initialized) initialize();
            return _current;
        };
        size_t Set(T val) {
            size_t ret = this->_setter(_key, val);
            _current = val; // Unconditionally update the current value even if setting it in NVS fails
            return ret;
        };
        size_t SetDefault() {
            return this->_setter(_key, _default);
        };
        size_t FromString(const char * const val) {
            return this->Set(getValue_as<T>(val));
        };
        String GetString() {
            return String(this->Get());
        };
        bool GetString(char * const buf, size_t len) {
            String val = String(this->Get());
            if (val.length() > len) {
                return false;
            }
            val.toCharArray(buf, len);
            return true;
        };
};

// Type aliases for common types
//typedef bool Bool;
//typedef uint16_t UShort;
//typedef uint8_t UChar;
//typedef int64_t Long64;

// Helper macros for creating getter/setter implementations
#define GETTER_IMPL_Bool    [this](String _k, Bool _d){ return this->nvs_getBool(_k.c_str(), _d); }
#define SETTER_IMPL_Bool    [this](String _k, Bool _v){ size_t ret = this->nvs_putBool(_k.c_str(), _v); this->commit(); return ret; }
#define GETTER_IMPL_UChar   [this](String _k, UChar _d){ return this->nvs_getUChar(_k.c_str(), _d); }
#define SETTER_IMPL_UChar   [this](String _k, UChar _v){ size_t ret = this->nvs_putUChar(_k.c_str(), _v); this->commit(); return ret; }
#define GETTER_IMPL_UShort  [this](String _k, UShort _d){ return this->nvs_getUShort(_k.c_str(), _d); }
#define SETTER_IMPL_UShort  [this](String _k, UShort _v){ size_t ret = this->nvs_putUShort(_k.c_str(), _v); this->commit(); return ret; }
#define GETTER_IMPL_Long64  [this](String _k, Long64 _d){ return this->nvs_getLong64(_k.c_str(), _d); }
#define SETTER_IMPL_Long64  [this](String _k, Long64 _v){ size_t ret = this->nvs_putLong64(_k.c_str(), _v); this->commit(); return ret; }
#define GETTER_IMPL_String  [this](String _k, String _d){ return this->nvs_getString(_k.c_str(), _d); }
#define SETTER_IMPL_String  [this](String _k, String _v){ size_t ret = this->nvs_putString(_k.c_str(), _v); this->commit(); return ret; }

// Macro to declare a configuration variable
// Usage: DECLARE_CONFIG_VARIABLE(String, MyVarName)
// Requires: CONFIG_DEFAULT_MyVarName to be defined
#define DECLARE_CONFIG_VARIABLE(type, name)  zPrefVariable <type> name = zPrefVariable <type>(#name, CONFIG_DEFAULT_##name, *this, GETTER_IMPL_##type, SETTER_IMPL_##type)

//==============================================================================
//  Exported data
//==============================================================================

//==============================================================================
//  Exported functions
//==============================================================================

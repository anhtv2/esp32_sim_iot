#pragma once
#include <Preferences.h>

namespace DataStorage
{
  Preferences preferences;
  String data_storage_name = "my-iot-device-app";

  bool hasKey(const char* key) {
    preferences.begin(data_storage_name, false);
    bool hasValue = preferences.containsKey(key);
    preferences.end();
    return hasValue;
  }

  int getValue(const char* key) {
    preferences.begin("my-app", false);
    int storedValue = preferences.getInt(key);
    preferences.end();
    return storedValue;
  }

  void setValue(const char* key, int value) {
    preferences.begin(data_storage_name, false);
    preferences.putInt(key, value);
    preferences.end();
  }
}
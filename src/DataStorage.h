#pragma once
#include <Preferences.h>

namespace DataStorage
{
    Preferences preferences;
    const char *my_namespace = "my_app";

    bool has_value(const char *key)
    {
        preferences.begin(my_namespace, false);
        int stored_int_value = preferences.getInt(key, INT_MIN);
        String stored_string_value = preferences.getString(key, "");

        // Check if either the integer or string value is different from the default
        bool has = (stored_int_value != INT_MIN) || (stored_string_value != "");

        preferences.end();
        return has;
    }

    int get_int_value(const char *key, int default_value)
    {
        preferences.begin(my_namespace, false);
        int stored_value = preferences.getInt(key, default_value);
        preferences.end();
        return stored_value;
    }

    void set_int_value(const char *key, int value)
    {
        preferences.begin(my_namespace, false); // Open in read-write mode
        preferences.putInt(key, value);
        preferences.end();
    }

    String get_string_value(const char *key, const String &default_value)
    {
        preferences.begin(my_namespace, false);
        String stored_value = preferences.getString(key, default_value.c_str());
        preferences.end();
        return stored_value;
    }

    void set_string_value(const char *key, const String &value)
    {
        preferences.begin(my_namespace, false); // Open in read-write mode
        preferences.putString(key, value);
        preferences.end();
    }
}
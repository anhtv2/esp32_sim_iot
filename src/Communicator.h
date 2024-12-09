#pragma once
#include <Arduino.h>

#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
namespace Communicator
{
    String house_id = "325719";
    int (*exec)(const char *);

    const char *mqtt_server = "broker.hivemq.com";
    const int mqtt_port = 1883;
    const String general_topic = "iot/kstn2024_nhom3/";
    String prefix_topic = general_topic + house_id + "/";

    const char *mqtt_user = "your_username";
    const char *mqtt_password = "your_password";
    const char *client_id = "esp32_client";
    PubSubClient client;

    void set_house_id(const String&id);

    inline bool publish_data(String topic, String json_string);
    void setup_communicator(PubSubClient &_client, String id, int (*func)(const char *))
    {
        exec = func;
        set_house_id(id);
        client = _client;
        client.setServer(mqtt_server, mqtt_port);

        auto callback = [](char *topic, byte *payload, unsigned int length)
        {
            Serial.println("length of message: " + String(length));
            Serial.println("Message arrived in topic: " + String(topic));
            // Process the message payload
            Serial.print("Message: ");
            char message[length];
            message[length] = '\0';
            for (int i = 0; i < length; i++)
            {
                message[i] = (char)payload[i];
            }
            Serial.println(message);
            if (exec(message))
                publish_data("command/result", "success");
        };

        client.setCallback(callback);
    }
    inline bool subscribe_command()
    {
        Serial.println("Sub" + prefix_topic + "command");
        return client.subscribe((prefix_topic + "command").c_str());
    }
    inline bool unsubscribe_command()
    {
        return client.unsubscribe((prefix_topic + "command").c_str());
    }
    inline void reconnect()
    {
        // Loop until we're reconnected
        while (!client.connected())
        {
            Serial.print("Attempting MQTT connection...");
            // Create a random client ID
            String clientId = "esp32-client-";
            clientId += String(random(0xffff), HEX);
            // Attempt to connect
            if (client.connect(clientId.c_str()))
            {
                Serial.println("connected to mqtt broker");
                Serial.println("subscribe to command topic again");
                subscribe_command();
                // Once connected, publish an announcement...
                // client.publish("outTopic", "hello world");
            }
            else
            {
                Serial.print("failed, rc=");
                Serial.print(client.state());
                Serial.println(" try again in 5 seconds");
                // Wait 5 seconds before retrying
                delay(5000);
            }
        }
    }

    inline bool publish_data(String topic, String json_string)
    {
        reconnect();
        topic = prefix_topic + topic;
        Serial.println("publishing data to topic : " + topic);
        Serial.println("Data content : " + json_string);
        // debug
        client.publish(topic.c_str(), json_string.c_str());
        Serial.println("finish sending data to broker");

        return true;
    }

    inline void init_topic()
    {
        reconnect();
        Serial.println("init topic for device");
        client.publish((general_topic + "init").c_str(), house_id.c_str());
        Serial.println("finish sending request to broker");
    }

    inline void remain()
    {
        client.loop();
    }
    void set_house_id(const String &id)
    {
        house_id = id;
        unsubscribe_command();
        prefix_topic = general_topic + house_id + "/";
        subscribe_command();
    }
};
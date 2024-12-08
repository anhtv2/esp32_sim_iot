#include <Arduino.h>

#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "LED.h"
#include "Timer.h"
#include "Sensor.h"
#include "LCD.h"
#include "Door.h"
#define len(arr) (sizeof(arr) / sizeof(arr[0]))
// put function declarations here:
const String device_id = "325719";
bool unaccepted = true;
LED led[2];
// LCD lcd;

namespace CommandProcessor
{
#define ALREADY_DONE 1
#define WRONG -1
#define SUCCESS 0
  int solved_id = -1;
  inline int exec(const char *json_string)
  {

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, json_string);

    // Access a specific field
    const int id = doc["id"];
    const char *obj_type = doc["device"];

    Serial.println(String(id));
    Serial.println(String(obj_type));

    if (id <= solved_id)
    {
      Serial.println("duplicate command");
      return ALREADY_DONE;
    }

    if (!(strcmp(obj_type, "led")))
    {
      const int obj_id = doc["ledIndex"];
      const char *cmd = doc["command"];
      // check obj id
      if (obj_id >= len(led))
      {
        return WRONG;
      }
      // do command
      if (!(strcmp(cmd, "invert")))
      {
        led[obj_id].invert_state();
      }
      else if (!strcmp(cmd, "off"))
      {
        led[obj_id].turn_off();
      }
      else if (!strcmp(cmd, "on"))
      {
        led[obj_id].turn_on();
      }
      solved_id = id;
      return SUCCESS;
    }

    if (!(strcmp(obj_type, "door")))
    {
      const char *cmd = doc["command"];
      Serial.print("command on door");
      Serial.println(cmd);
      if (!strcmp(cmd, "on"))
      {
        door::change_angle(90);
      }
      else if (!strcmp(cmd, "off"))
      {
        door::change_angle(0);
      }
      else if (!strcmp(cmd, "change_angle"))
      {
        const int angle = doc["angle"];
        door::change_angle(angle);
      }
      solved_id = id;
      return SUCCESS;
    }
    if (!(strcmp(obj_type, "lcd")))
    {
      const char *cmd = doc["command"];
      Serial.print("command in lcd ");
      Serial.println(cmd);
      if (!strcmp(cmd, "print"))
      {
        const char *message = doc["message"];
          int row=0;
          row = doc["row"];
          int col =0;
          col = doc["col"];
          Serial.print("row: ");
          Serial.println(row);
          Serial.print("col: ");
          Serial.println(col);
        Serial.println(String(message));
        lcd::print(String(message), row, col);
      }
      else if (!strcmp(cmd, "clear"))
      {
        lcd::clear(); 
      }
      solved_id = id;
      return SUCCESS;
    }
    return WRONG;
  }
};
namespace Communicator
{
  const char *mqtt_server = "broker.mqtt-dashboard.com";
  const int mqtt_port = 1883;
  const String general_topic = "iot/dhhn/";
  const String prefix_topic = general_topic + device_id + "/";

  const char *mqtt_user = "your_username";
  const char *mqtt_password = "your_password";
  const char *client_id = "esp32_client";
  PubSubClient client;
  
 inline bool publish_data(String topic, String json_string);
  void setup_communicator(PubSubClient &_client)
  {
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
      if(CommandProcessor::exec(message))
        publish_data("command/result","success");
      
    };

    client.setCallback(callback);
  }
  inline bool subscribe_command()
  {
    return client.subscribe((prefix_topic + "command").c_str());
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
    client.publish((general_topic + "init").c_str(), device_id.c_str());
    Serial.println("finish sending request to broker");
  }

  inline void remain()
  {
    client.loop();
  }
};

WiFiClient wifi_client;
PubSubClient client(wifi_client);
#define comm Communicator

int motion_detected = 0;

inline void setup_wifi()
{
  led[0].set_pin(32);
  led[1].set_pin(33);
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();

  WiFi.mode(WIFI_STA);
  WiFi.begin("Wokwi-GUEST", "", 6);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
inline void send_data()
{

  // Create a JSON document
  const size_t capacity = JSON_OBJECT_SIZE(3);
  DynamicJsonDocument doc(capacity);

  // Add data to the JSON document
  doc["temperature"] = sensor::get_temperature();
  doc["humidity"] = sensor::get_humidity();
  // doc["motion"] = sensor::check_motion();
  String json_string;
  serializeJson(doc, json_string);

  // sensor_data, command, command/result, init , init/result
  comm::publish_data("sensor_data", json_string.c_str());
}

void setup()
{
  led[1].turn_off();
  door::setup(18);
  door::change_angle(0);
  lcd::setup();
  sensor::setup_dht(19);
  sensor::setup_pir(23, []()
                    {
                      if(sensor::check_motion()){
                        Serial.println("motion deteced");
                        led[1].turn_on();
                        motion_detected = 1;
                      }
                      else{
                        led[1].turn_off();
                        motion_detected = 0;
                      } });

  using namespace CommandProcessor;
  Serial.begin(115200);
  setup_wifi();
  comm::setup_communicator(client);

  comm::reconnect();
  // comm.subscribe_command();

  Serial.println("finish connecting to mqtt server");

  comm::publish_data("abc", "init connection to server");
  Timer::periodic_callback(10000, []()
                           { send_data(); });
}
unsigned long prev_millis = 0;
void loop()
{
  // if (unaccepted)
  // {
  //   unsigned long cur_millis = millis();

  //   // Check if it's time to run the function
  //   if (cur_millis - prev_millis >= 5000)
  //   {
  //     // Reset the timer
  //     prev_millis = cur_millis;
  //     comm::init_topic();
  //   }
  // }

  if (motion_detected == 1)
  {

    // lcd::print("motion detected");
    motion_detected = 2;
    const size_t capacity = JSON_OBJECT_SIZE(3);
    DynamicJsonDocument doc(capacity);

  // Add data to the JSON document
    doc["motion"] = 1;
    String json_string;
    serializeJson(doc, json_string);
    comm::publish_data("sensor_data",json_string );
  }
  else if (motion_detected == 0)
  {

    // lcd::clear();
  }
  comm::reconnect();
  comm::remain();
  // lcd.print(String(sensor.get_temperature()));
  delay(10);
}

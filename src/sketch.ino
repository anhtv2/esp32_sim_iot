#include <Arduino.h>

#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
using namespace std;

#include "LED.h"
#include "Timer.h"
#include "Sensor.h"
#include "LCD.h"
#include "Door.h"
#include "DataStorage.h"
#include "Communicator.h"
#include "SerialCommand.h"

#define len(arr) (sizeof(arr) / sizeof(arr[0]))
// put function declarations here:
String house_id = "325719";
bool unaccepted = true;
LED led[10];
LED motion_detection_led;
int led_counter = 1;
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
        int row = 0;
        row = doc["row"];
        int col = 0;
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

WiFiClient wifi_client;
PubSubClient client(wifi_client);
#define comm Communicator

int motion_detected = 0;

inline void setup_wifi()
{

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

const char *house_id_key = "house_id";
const char *led_counter_key = "led_counter";
const char *led_index_key = "led_";

void setup_data()
{
  // DataStorage::set_string_value("house_id", "325719");
  if (DataStorage::has_value(house_id_key))
  {
    house_id = DataStorage::get_string_value(house_id_key, "");
    
  }
  if (DataStorage::has_value(led_counter_key))
  {
    led_counter = DataStorage::get_int_value(led_counter_key, 1);

    for (int i = 1; i < led_counter; i++)
    {
      led[i].set_pin(DataStorage::get_int_value((led_index_key + String(i)).c_str(), 0));
    }
  }
}
void setup()
{
  Serial.begin(115200);
  setup_data();

  // setup for Communicator

  led[0].set_pin(32);
  DataStorage::set_int_value((led_index_key + String(0)).c_str(), 32);

  motion_detection_led.set_pin(33);
  motion_detection_led.turn_off();

  door::setup(18);
  door::change_angle(0);
  lcd::setup();
  sensor::setup_dht(19);
  sensor::setup_pir(23, []()
                    {
                      if(sensor::check_motion()){
                        Serial.println("motion detected");
                        motion_detection_led.turn_on();
                        motion_detected = 1;
                      }
                      else{
                        motion_detection_led.turn_off();
                        motion_detected = 0;
                      } });

  using namespace CommandProcessor;
  setup_wifi();
  comm::setup_communicator(client, house_id, &CommandProcessor::exec);

  comm::reconnect();
  // comm.subscribe_command();

  Serial.println("finish connecting to mqtt server");

  comm::publish_data("abc", "init connection to server");
  Timer::periodic_callback(10000, []()
                           { send_data(); });
}
unsigned long prev_millis = 0;

void exec_serial_command(const vector<String> &words)
{
  if (words[0] == "set")
  {
    if (words.size() < 3)
      return;
    if (words[1] == "house_id")
    {
      house_id = words[2];
      Serial.println("house id is changed to " + house_id);
      DataStorage::set_string_value(house_id_key, house_id);
      Communicator::set_house_id(house_id);
      return;
    }
  }
  if (words[0] == "add")
  {
    if (words.size() < 3)
      return;
    if (words[1] == "led")
    {
      if (led_counter >= len(led))
      {
        return;
      }
      int pin = words[2].toInt();
      led[led_counter].set_pin(pin);
      DataStorage::set_int_value((led_index_key + String(led_counter)).c_str(), pin);
      DataStorage::set_int_value(led_counter_key, ++led_counter);

      return;
    }
  }
  if (words[0] == "check")
  {
    if (words.size() < 2)
      return;
    if (words[1] == "house_id")
    {
      if (!DataStorage::has_value(house_id_key))
      {
        Serial.println("house id hasn't been config yet");
      }
      Serial.println("id of house is :" + DataStorage::get_string_value("house_id", ""));
      return;
    }
    if (words[1] == "led")
    {
      for (int i = 0; i < DataStorage::get_int_value(led_counter_key, 1); i++)
      {
        Serial.println("led " + String(i) + " is on pin " + String(DataStorage::get_int_value((led_index_key + String(i)).c_str(), 0)));
      }
    }
  }
}
void loop()
{
  if (Serial.available())
  {
    vector<String> serial_command = SerialCommand::read_and_split();
    exec_serial_command(serial_command);
    for (auto a : serial_command)
    {
      Serial.println(a);
    }
  }

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
    comm::publish_data("sensor_data", json_string);
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

/* By Abdullah As-Sadeed*/

/* https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json */

/*
  Board: "ESP32 Dev Module"
  Upload Speed: "921600"
  CPU Frequency: "240MHz (WiFi/BT)"
  Flash Frequency: "40MHz"
  Flash Mode: "DIO"
  Flash Size: "4MB (32Mb)"
  Partition Scheme: "Huge APP (3MB No OTA/1MB SPIFFS)"
  Core Debug Level: "Warn"
  PSRAM: "Disabled"
  Arduino Runs On: "Core 1"
  Events Run On: "Core 1"
  Erase All Flash Before Sketch Upload: "Disabled"
  JTAG Adapter: "Disabled"
  Zigbee Mode: "Disabled"
  Programmer: "Esptool"
*/

/*
  /home/bitscoper/.arduino15/packages/esp32/hardware/esp32/3.0.1/cores/esp32/HardwareSerial.h

  #ifndef RX1
  #if CONFIG_IDF_TARGET_ESP32
  #define RX1 26
  #endif
  #endif

  #ifndef TX1
  #if CONFIG_IDF_TARGET_ESP32
  #define TX1 27
  #endif
  #endif

*/

#include "FS.h"
#include "SPIFFS.h"

#include "Arduino.h"
#include "esp_arduino_version.h"

#include "LiquidCrystal_I2C.h"

#include "ESP32Servo.h"
#include "rdm6300.h" /* https://github.com/arduino12/rdm6300 */

#include "Wire.h"

#include "MPU9250_WE.h"

#include "BH1750.h"

#include "Seeed_BME280.h"

#include "TinyGPSPlus.h"

#include "Lewis.h"

#include "esp_wifi.h"
#include "WiFi.h"

#include "AsyncTCP.h"          /* https://github.com/me-no-dev/AsyncTCP */
#include "ESPAsyncWebServer.h" /* https://github.com/me-no-dev/ESPAsyncWebServer */

#include "ArduinoJson.h"

/* Local */
#include "Confidentials.h"

#define FORMAT_SPIFFS_IF_FAILED true

#define SG90_Pin 27
#define RDM6300_Pin 35
#define Work_LED_Pin 2

#define SIM800L__NEO7M Serial1

#define RCWL0516_Pin 13

#define Flame_Sensor_Pin 15

#define Slave_Board Serial2

#define Relay_1_Pin 32
#define Relay_2_Pin 33

#define Buzzer_LED_Pin 4

LiquidCrystal_I2C LCD(0x3f, 16, 2);

Servo SG90;

Rdm6300 RDM6300;

MPU9250_WE MPU9250 = MPU9250_WE(0x68);

BH1750 bh1750(0x23);

BME280 bme280;

TinyGPSPlus GPS;

Lewis Morse_Code;

AsyncWebServer Server(80);
AsyncEventSource Events("/Events");

boolean Any_Failed_Authentication = false, MPU9250_Status, MPU9250_Magnetometer_Status, BH1750_Status, BME280_Status, BlackBody_Motion, Flame, Called_Yet = false;

unsigned int Satellites;

unsigned long SSE_Delay = 500, Last_SSE_Time = 0, SSEs = 0, Call_Delay = 60000, Last_Call_Time = 0;

float Acceleration_X, Acceleration_Y, Acceleration_Z, Resultant_Acceleration, Gyro_X, Gyro_Y, Gyro_Z, Magneto_X, Magneto_Y, Magneto_Z, Light, Temperature_MPU9250, Temperature_BME280, Humidity, Pressure, Altitude_BME280, Latitude, Longitude, Speed, Course, Altitude_GPS, HDOP, MQ_2, MQ_3, MQ_4, MQ_5, MQ_6, MQ_7, MQ_8, MQ_9, MQ_135, Slave_UpTime;

const char *Compile_Date_Time = __DATE__ " " __TIME__;

String Scan_I2C(void)
{
  byte error, address;
  int I2C_Devices_Count = 0;
  String I2C_Scan_Result = "";

  for (address = 0x01; address < 0x7f; address++)
  {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0)
    {
      I2C_Scan_Result += "0x";
      I2C_Scan_Result += String(address, HEX);
      I2C_Scan_Result += "  ";

      I2C_Devices_Count++;
    }
    else if (error != 2)
    {
      I2C_Scan_Result += "Error ";
      I2C_Scan_Result += error;
      I2C_Scan_Result += " at address 0x";
      I2C_Scan_Result += String(address, HEX);
      I2C_Scan_Result += "\n";
    }
  }

  if (I2C_Devices_Count == 0)
  {
    I2C_Scan_Result += "No device";
  }

  return I2C_Scan_Result;
}

void SetUp_LCD(void)
{
  LCD.init();
  LCD.backlight();

  LCD.clear();
}

void SetUp_SG90(void)
{
  SG90.setPeriodHertz(50);
  SG90.attach(SG90_Pin, 500, 2400);
}

void Authenticate(void)
{
  unsigned int Card;

  LCD.setCursor(2, 0);
  LCD.print("Authenticate");

  SG90.write(90);

  do
  {
    Card = RDM6300.get_tag_id();

    if (RDM6300.get_tag_id() && Card != Authenticated_Card)
    {
      LCD.setCursor(2, 0);
      LCD.print(" Wrong Card ");
      SG90.write(120);

      Any_Failed_Authentication = true;
    }
    else
    {
      LCD.setCursor(2, 0);
      LCD.print("Authenticate");
      SG90.write(90);
    }

    delay(10);
  } while (Card != Authenticated_Card);

  LCD.clear();
  LCD.setCursor(4, 0);
  LCD.print("Booting");
  LCD.setCursor(2, 1);
  LCD.print("Full System");
  SG90.write(0);
}

void SetUp_MPU9250(void)
{
  if (MPU9250.init())
  {
    MPU9250_Status = true;
  }
  else
  {
    MPU9250_Status = false;
  }

  if (MPU9250.initMagnetometer())
  {
    MPU9250_Magnetometer_Status = true;
  }
  else
  {
    MPU9250_Magnetometer_Status = false;
  }

  if (MPU9250_Status && MPU9250_Magnetometer_Status)
  {
    delay(1000);

    MPU9250.autoOffsets();

    MPU9250.enableAccDLPF(true);
    MPU9250.setAccDLPF(MPU9250_DLPF_6); /* 7 is not good */

    MPU9250.enableGyrDLPF();
    MPU9250.setGyrDLPF(MPU9250_DLPF_6); /* 7 is not good */

    MPU9250.setAccRange(MPU6500_ACC_RANGE_16G); /* Highest */

    MPU9250.setGyrRange(MPU9250_GYRO_RANGE_2000); /* Highest */

    MPU9250.setMagOpMode(AK8963_CONT_MODE_100HZ); /* Highest */

    MPU9250.setSampleRateDivider(0); /* Lowest */

    delay(200);
  };
}

void SetUp_BH1750(void)
{
  if (bme280.init())
  {
    BME280_Status = true;
  }
  else
  {
    BME280_Status = false;
  }
}

void SetUp_BME280(void)
{
  if (bh1750.begin(BH1750::CONTINUOUS_HIGH_RES_MODE_2)) /* Highest */
  {
    BH1750_Status = true;
  }
  else
  {
    BH1750_Status = false;
  }
}

void SetUp_Relays(void)
{
  pinMode(Relay_1_Pin, OUTPUT);
  pinMode(Relay_2_Pin, OUTPUT);

  digitalWrite(Relay_1_Pin, HIGH);
  digitalWrite(Relay_2_Pin, HIGH);
}

void SetUp_WiFi(void)
{
  const char *HostName = "Bitscoper_IoT";

  WiFi.mode(WIFI_STA);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(HostName);

  WiFi.begin(WiFi_SSID, WiFi_PassWord);

  while (WiFi.status() != WL_CONNECTED)
  {
  }
}

void SetUp_Work_LED(void)
{
  pinMode(Work_LED_Pin, OUTPUT);
  digitalWrite(Work_LED_Pin, LOW);
}

void Add_Common_Headers(AsyncWebServerResponse *response)
{
  response->addHeader("X-Content-Type-Options", "nosniff");
  response->addHeader("X-Frame-Options", "SAMEORIGIN");
  response->addHeader("Referrer-Policy", "strict-origin");
  response->addHeader("Server", "Bitscoper IoT");

  response->addHeader("Access-Control-Allow-Origin", "*");
}

void Handle_Error_404(AsyncWebServerRequest *request)
{
  String Response_Text = "HTTP 404\n\n";

  Response_Text += "Version: ";
  int version = request->version();
  if (version == 0)
  {
    Response_Text += "HTTP/1.0\n\n";
  }
  else if (version == 1)
  {
    Response_Text += "HTTP/1.1\n\n";
  }
  Response_Text += "Host: ";
  Response_Text += request->host();
  Response_Text += "\n\nURL: ";
  Response_Text += request->url();
  Response_Text += "\n\nMethod: ";
  int method = request->method();
  if (method == HTTP_GET)
  {
    Response_Text += "GET";
  }
  else if (method == HTTP_POST)
  {
    Response_Text += "POST";
  }
  else if (method == HTTP_DELETE)
  {
    Response_Text += "DELETE";
  }
  else if (method == HTTP_PUT)
  {
    Response_Text += "PUT";
  }
  else if (method == HTTP_PATCH)
  {
    Response_Text += "PATCH";
  }
  else if (method == HTTP_HEAD)
  {
    Response_Text += "HEAD";
  }
  else if (method == HTTP_OPTIONS)
  {
    Response_Text += "OPTIONS";
  }
  Response_Text += "\n\nTotal arguments: ";
  int args = request->args();
  Response_Text += args;
  Response_Text += "\n\n";
  if (args > 0)
  {
    Response_Text += "Arguments:\n";
    for (int i = 0; i < args; i++)
    {
      Response_Text += "\t  ";
      Response_Text += request->argName(i).c_str();
      Response_Text += ": ";
      Response_Text += request->arg(i).c_str();
      Response_Text += "\n";
    }
    Response_Text += "\n";
  }
  Response_Text += "Total request headers: ";
  Response_Text += request->headers();

  AsyncWebServerResponse *response = request->beginResponse(404, "text/plain; charset=utf-8", Response_Text);
  Add_Common_Headers(response);
  response->addHeader("Content-Disposition", "inline; filename=\"Bitscoper IoT | Error 404\"");
  request->send(response);
}

void Handle_I2C_Scan(AsyncWebServerRequest *request)
{
  AsyncWebServerResponse *response = request->beginResponse(200, "text/html; charset=utf-8", Scan_I2C());
  Add_Common_Headers(response);
  request->send(response);
}

void Handle_Relays(AsyncWebServerRequest *request)
{
  String Relay_Number, Relay_State, Response_Text;

  int args = request->args();
  if (args > 0)
  {
    for (int i = 0; i < args; i++)
    {
      if (request->argName(i) == "Number")
      {
        Relay_Number = request->arg(i);
      }
      else if (request->argName(i) == "State")
      {
        Relay_State = request->arg(i);
      }
    }
  }

  if (Relay_Number == "1")
  {
    if (Relay_State == "1")
    {
      digitalWrite(Relay_1_Pin, LOW);

      Response_Text = "Relay 1 On";
    }
    else if (Relay_State == "0")
    {
      digitalWrite(Relay_1_Pin, HIGH);

      Response_Text = "Relay 1 Off";
    }
  }
  else if (Relay_Number == "2")
  {
    if (Relay_State == "1")
    {
      digitalWrite(Relay_2_Pin, LOW);

      Response_Text = "Relay 2 On";
    }
    else if (Relay_State == "0")
    {
      digitalWrite(Relay_2_Pin, HIGH);

      Response_Text = "Relay 2 Off";
    }
  }

  AsyncWebServerResponse *response = request->beginResponse(200, "text/plain; charset=utf-8", Response_Text);
  Add_Common_Headers(response);
  request->send(response);
}

void Handle_Morse_Code(AsyncWebServerRequest *request)
{
  String Message;

  int args = request->args();
  if (args > 0)
  {
    for (int i = 0; i < args; i++)
    {
      if (request->argName(i) == "String")
      {
        Message = request->arg(i);
      }
    }
  }

  AsyncWebServerResponse *response = request->beginResponse(200, "text/plain; charset=utf-8", "");
  Add_Common_Headers(response);
  request->send(response);

  Morse_Code.print(Message);
}

void Handle_WiFi_Scan(AsyncWebServerRequest *request)
{
  String WiFi_Scan_Result = "";

  int n = WiFi.scanComplete();
  if (n == -2)
  {
    WiFi.scanNetworks(true);
  }
  else if (n)
  {
    WiFi_Scan_Result += "<table>\n<tbody>\n<tr>\n<th colspan=\"5\">Nearby Wi-Fi Access Points</th>\n</tr>\n<tr>\n<th>RSSI</th>\n<th>SSID</th>\n<th>BSSID</th>\n<th>Channel</th>\n<th>Encryption</th>\n</tr>\n";
    for (int i = 0; i < n; ++i)
    {
      WiFi_Scan_Result += "<tr>\n<td>";
      WiFi_Scan_Result += WiFi.RSSI(i);
      WiFi_Scan_Result += "</td>\n<td>";
      WiFi_Scan_Result += WiFi.SSID(i);
      WiFi_Scan_Result += "</td>\n<td>";
      WiFi_Scan_Result += WiFi.BSSIDstr(i);
      WiFi_Scan_Result += "</td>\n<td>";
      WiFi_Scan_Result += WiFi.channel(i);
      WiFi_Scan_Result += "</td>\n<td>";
      int encryption = WiFi.encryptionType(i);
      if (encryption == WIFI_AUTH_OPEN)
      {
        WiFi_Scan_Result += "Open";
      }
      else if (encryption == WIFI_AUTH_WEP)
      {
        WiFi_Scan_Result += "WEP";
      }
      else if (encryption == WIFI_AUTH_WPA_PSK)
      {
        WiFi_Scan_Result += "WPA PSK";
      }
      else if (encryption == WIFI_AUTH_WPA2_PSK)
      {
        WiFi_Scan_Result += "WPA2 PSK";
      }
      else if (encryption == WIFI_AUTH_WPA_WPA2_PSK)
      {
        WiFi_Scan_Result += "WPA WPA2 PSK";
      }
      else if (encryption == WIFI_AUTH_WPA2_ENTERPRISE)
      {
        WiFi_Scan_Result += "WPA2 Enterprise";
      }
      WiFi_Scan_Result += "</td>\n</tr>\n";
    }
    WiFi_Scan_Result += "</tbody>\n</table>\n";

    WiFi.scanDelete();
    if (WiFi.scanComplete() == -2)
    {
      WiFi.scanNetworks(true);
    }
  }
  AsyncWebServerResponse *response = request->beginResponse(200, "text/html; charset=utf-8", WiFi_Scan_Result);
  Add_Common_Headers(response);
  request->send(response);
}

void Handle_ReBoot(AsyncWebServerRequest *request)
{
  boolean Authorized_ReBoot = false;
  String Response_Text = "";

  int args = request->args();
  if (args > 0)
  {
    for (int i = 0; i < args; i++)
    {
      if (request->argName(i) == "Confirm" && request->arg(i) == "true")
      {
        Authorized_ReBoot = true;
        Response_Text = "Rebooting";
      }
    }
  }

  AsyncWebServerResponse *response = request->beginResponse(200, "text/plain; charset=utf-8", Response_Text);
  Add_Common_Headers(response);
  request->send(response);

  if (Authorized_ReBoot)
  {
    esp_restart();
  }
}

void Get_Slave_Readings(void)
{
  while (Slave_Board.available())
  {
    JsonDocument Slave_Readings;

    DeserializationError JSON_Error = deserializeJson(Slave_Readings, Slave_Board);

    if (JSON_Error == DeserializationError::Ok)
    {
      MQ_2 = Slave_Readings["MQ_2"].as<float>();
      MQ_3 = Slave_Readings["MQ_3"].as<float>();
      MQ_4 = Slave_Readings["MQ_4"].as<float>();
      MQ_5 = Slave_Readings["MQ_5"].as<float>();
      MQ_6 = Slave_Readings["MQ_6"].as<float>();
      MQ_7 = Slave_Readings["MQ_7"].as<float>();
      MQ_8 = Slave_Readings["MQ_8"].as<float>();
      MQ_9 = Slave_Readings["MQ_9"].as<float>();
      MQ_135 = Slave_Readings["MQ_135"].as<float>();

      Slave_UpTime = Slave_Readings["UpTime"].as<float>();
    }
  }
}

void Call_About_Flame(void)
{
  if (Flame)
  {
    if (!Called_Yet || ((millis() - Last_Call_Time) > Call_Delay))
    {
      SIM800L__NEO7M.print("ATD+ ");
      SIM800L__NEO7M.print(Phone_Number);
      SIM800L__NEO7M.print(";\r\n");

      Called_Yet = true;
      Last_Call_Time = millis();
    }
  }
}

void Take_Auto_Actions(void)
{
  Call_About_Flame();
}

void Get_Readings(void)
{
  if (MPU9250_Status && MPU9250_Magnetometer_Status)
  {

    xyzFloat acceleration = MPU9250.getGValues();
    xyzFloat gyro = MPU9250.getGyrValues();
    xyzFloat magneto = MPU9250.getMagValues();
    Resultant_Acceleration = MPU9250.getResultantG(acceleration);

    Acceleration_X = acceleration.x;
    Acceleration_Y = acceleration.y;
    Acceleration_Z = acceleration.z;

    Gyro_X = gyro.x;
    Gyro_Y = gyro.y;
    Gyro_Z = gyro.z;

    Magneto_X = magneto.x;
    Magneto_Y = magneto.y;
    Magneto_Z = magneto.z;

    Temperature_MPU9250 = MPU9250.getTemperature();
  }
  else
  {
    Acceleration_X = 0;
    Acceleration_Y = 0;
    Acceleration_Z = 0;
    Resultant_Acceleration = 0;
    Gyro_X = 0;
    Gyro_Y = 0;
    Gyro_Z = 0;
    Magneto_X = 0;
    Magneto_Y = 0;
    Magneto_Z = 0;
    Temperature_MPU9250 = 0;
  }

  if (BH1750_Status && bh1750.measurementReady())
  {
    Light = bh1750.readLightLevel();
  }
  else
  {
    Light = 0;
  }

  if (BME280_Status)
  {
    Temperature_BME280 = bme280.getTemperature();
    Humidity = bme280.getHumidity();
    Pressure = bme280.getPressure();
    Altitude_BME280 = bme280.calcAltitude(Pressure);
  }
  else
  {
    Temperature_BME280 = 0;
    Humidity = 0;
    Pressure = 0;
    Altitude_BME280 = 0;
  }

  while (SIM800L__NEO7M.available())
  {
    GPS.encode(SIM800L__NEO7M.read());
  }

  Latitude = GPS.location.lat();
  Longitude = GPS.location.lng();
  Speed = GPS.speed.kmph();
  Course = GPS.course.deg();
  Altitude_GPS = GPS.altitude.meters();
  Satellites = GPS.satellites.value();
  HDOP = GPS.hdop.value();

  BlackBody_Motion = digitalRead(RCWL0516_Pin);

  Flame = !digitalRead(Flame_Sensor_Pin);

  do
  {
    Get_Slave_Readings();
  } while (MQ_2 == 0 && MQ_3 == 0 && MQ_4 == 0 && MQ_5 == 0 && MQ_6 == 0 && MQ_7 == 0 && MQ_8 == 0 && MQ_9 == 0 && MQ_135 == 0 && Slave_UpTime == 0);

  Take_Auto_Actions();
}

String Process_Page(const String &var)
{
  int chip_id = 0;

  for (int i = 0; i < 17; i = i + 8)
  {
    chip_id |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }

  Get_Readings();

  if (var == "CHIP_MODEL")
  {
    return String(ESP.getChipModel());
  }
  else if (var == "CHIP_REVISION")
  {
    return String(ESP.getChipRevision());
  }
  else if (var == "CHIP_CORES")
  {
    return String(ESP.getChipCores());
  }
  else if (var == "CHIP_ID")
  {
    return String(chip_id);
  }
  else if (var == "MAC_ADDRESS")
  {
    return String(WiFi.macAddress());
  }
  else if (var == "HOST_NAME")
  {
    return String(WiFi.getHostname());
  }
  else if (var == "LOCAL_IP_ADDRESS")
  {
    return String(WiFi.localIP().toString().c_str());
  }
  else if (var == "COMPILE_DATE_TIME")
  {
    return String(Compile_Date_Time);
  }
  else if (var == "ANY_FAILED_AUTHENTICATION")
  {
    if (Any_Failed_Authentication)
    {
      return "Yes";
    }
    else
    {
      return "No";
    }
  }
  else if (var == "UPTIME")
  {
    return String(millis() / 1000);
  }
  else if (var == "SLAVE_UPTIME")
  {
    return String(Slave_UpTime);
  }
  else if (var == "Acceleration_X")
  {
    return String(Acceleration_X);
  }
  else if (var == "Acceleration_Y")
  {
    return String(Acceleration_Y);
  }
  else if (var == "Acceleration_Z")
  {
    return String(Acceleration_Z);
  }
  else if (var == "Resultant_Acceleration")
  {
    return String(Resultant_Acceleration);
  }
  else if (var == "Gyro_X")
  {
    return String(Gyro_X);
  }
  else if (var == "Gyro_Y")
  {
    return String(Gyro_Y);
  }
  else if (var == "Gyro_Z")
  {
    return String(Gyro_Z);
  }
  else if (var == "Magneto_X")
  {
    return String(Magneto_X);
  }
  else if (var == "Magneto_Y")
  {
    return String(Magneto_Y);
  }
  else if (var == "Magneto_Z")
  {
    return String(Magneto_Z);
  }
  else if (var == "LIGHT")
  {
    return String(Light);
  }
  else if (var == "Temperature_MPU9250")
  {
    return String(Temperature_MPU9250);
  }
  else if (var == "Temperature_BME280")
  {
    return String(Temperature_BME280);
  }
  else if (var == "HUMIDITY")
  {
    return String(Humidity);
  }
  else if (var == "PRESSURE")
  {
    return String(Pressure);
  }
  else if (var == "ALTITUDE")
  {
    return String(Altitude_BME280);
  }
  else if (var == "LATITUDE")
  {
    return String(Latitude);
  }
  else if (var == "LONGITUDE")
  {
    return String(Longitude);
  }
  else if (var == "SPEED")
  {
    return String(Speed);
  }
  else if (var == "COURSE")
  {
    return String(Course);
  }
  else if (var == "ALTITUDE_GPS")
  {
    return String(Altitude_GPS);
  }
  else if (var == "SATELLITES")
  {
    return String(Satellites);
  }
  else if (var == "HDOP")
  {
    return String(HDOP);
  }
  else if (var == "MQ_2")
  {
    return String(MQ_2);
  }
  else if (var == "MQ_3")
  {
    return String(MQ_3);
  }
  else if (var == "MQ_4")
  {
    return String(MQ_4);
  }
  else if (var == "MQ_5")
  {
    return String(MQ_5);
  }
  else if (var == "MQ_6")
  {
    return String(MQ_6);
  }
  else if (var == "MQ_7")
  {
    return String(MQ_7);
  }
  else if (var == "MQ_8")
  {
    return String(MQ_8);
  }
  else if (var == "MQ_9")
  {
    return String(MQ_9);
  }
  else if (var == "MQ_135")
  {
    return String(MQ_135);
  }

  return String();
}

void Handle_Root(AsyncWebServerRequest *request)
{
  File xhtml_body_file = SPIFFS.open("/Client.xhtml", "r");

  String xhtml_body_content = xhtml_body_file.readString();
  xhtml_body_file.close();

  const char *xhtml_body PROGMEM = xhtml_body_content.c_str();

  AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html; charset=utf-8", xhtml_body, Process_Page);
  Add_Common_Headers(response);
  request->send(response);
}

void Serve_FavIcon(AsyncWebServerRequest *request)
{
  File favicon_file = SPIFFS.open("/favicon.ico", "r");

  AsyncWebServerResponse *response = request->beginResponse(favicon_file, "image/vnd.microsoft.icon");
  response->addHeader("Content-Type", "image/vnd.microsoft.icon");
  Add_Common_Headers(response);
  request->send(response);
}

void Serve_CSS(AsyncWebServerRequest *request)
{
  File CSS_file = SPIFFS.open("/Client.css", "r");
  String CSS = CSS_file.readString();
  CSS_file.close();

  AsyncWebServerResponse *response = request->beginResponse(200, "text/css", CSS);
  Add_Common_Headers(response);
  request->send(response);
}

void Serve_JavaScript(AsyncWebServerRequest *request)
{
  File JavaScript_file = SPIFFS.open("/Client.js", "r");
  String JavaScript = JavaScript_file.readString();
  JavaScript_file.close();

  AsyncWebServerResponse *response = request->beginResponse(200, "text/javascript", JavaScript);
  Add_Common_Headers(response);
  request->send(response);
}

void Assign_URLs(void)
{
  Server.on("/", HTTP_GET, Handle_Root);

  Server.on("/favicon.ico", HTTP_GET, Serve_FavIcon);
  Server.on("/CSS", HTTP_GET, Serve_CSS);
  Server.on("/JavaScript", HTTP_GET, Serve_JavaScript);

  Server.on("/Scan_I2C", HTTP_GET, Handle_I2C_Scan);

  Server.on("/Relays", HTTP_GET, Handle_Relays);

  Server.on("/Morse_Code", HTTP_GET, Handle_Morse_Code);

  Server.on("/Scan_WiFi", HTTP_GET, Handle_WiFi_Scan);

  Server.on("/ReBoot", HTTP_GET, Handle_ReBoot);
}

void SetUp_Server(void)
{
  Assign_URLs();

  Server.addHandler(&Events);

  Server.onNotFound(Handle_Error_404);

  Server.begin();
}

void Display_SSE_Count(void)
{
  LCD.setCursor(0, 1);
  LCD.print("SSEs:");
  LCD.print(SSEs);
}

void setup(void)
{
  Serial.begin(115200);

  SPIFFS.begin();

  Wire.begin();
  // Serial.println(Scan_I2C());

  SetUp_LCD();

  SetUp_SG90();
  RDM6300.begin(RDM6300_Pin);
  Authenticate();

  SetUp_MPU9250();

  SetUp_BH1750();

  SetUp_BME280();

  SIM800L__NEO7M.begin(9600);

  pinMode(RCWL0516_Pin, INPUT);

  pinMode(Flame_Sensor_Pin, INPUT);

  Slave_Board.begin(115200);

  SetUp_Relays();

  Morse_Code.begin(Buzzer_LED_Pin);

  SetUp_WiFi();

  SetUp_Server();

  SetUp_Work_LED();

  LCD.clear();
  LCD.setCursor(0, 0);
  LCD.print("IP:");
  LCD.print(WiFi.localIP());
  Display_SSE_Count();
}

void loop(void)
{
  if ((millis() - Last_SSE_Time) > SSE_Delay)
  {
    digitalWrite(Work_LED_Pin, HIGH);

    JsonDocument Readings;

    Get_Readings();

    Readings["Acceleration_X"] = String(Acceleration_X);
    Readings["Acceleration_Y"] = String(Acceleration_Y);
    Readings["Acceleration_Z"] = String(Acceleration_Z);
    Readings["Resultant_Acceleration"] = String(Resultant_Acceleration);

    Readings["Gyro_X"] = String(Gyro_X);
    Readings["Gyro_Y"] = String(Gyro_Y);
    Readings["Gyro_Z"] = String(Gyro_Z);

    Readings["Magneto_X"] = String(Magneto_X);
    Readings["Magneto_Y"] = String(Magneto_Y);
    Readings["Magneto_Z"] = String(Magneto_Z);

    Readings["Light"] = String(Light);

    Readings["Temperature_MPU9250"] = String(Temperature_MPU9250);

    Readings["Temperature_BME280"] = String(Temperature_BME280);

    Readings["Humidity"] = String(Humidity);
    Readings["Pressure"] = String(Pressure);
    Readings["Altitude_BME280"] = String(Altitude_BME280);

    Readings["Latitude"] = String(Latitude);
    Readings["Longitude"] = String(Longitude);
    Readings["Speed"] = String(Speed);
    Readings["Course"] = String(Course);
    Readings["Altitude_GPS"] = String(Altitude_GPS);
    Readings["Satellites"] = String(Satellites);
    Readings["HDOP"] = String(HDOP);

    Readings["BlackBody_Motion"] = String(BlackBody_Motion);

    Readings["Flame"] = String(Flame);

    Readings["MQ_2"] = String(MQ_2);
    Readings["MQ_3"] = String(MQ_3);
    Readings["MQ_4"] = String(MQ_4);
    Readings["MQ_5"] = String(MQ_5);
    Readings["MQ_6"] = String(MQ_6);
    Readings["MQ_7"] = String(MQ_7);
    Readings["MQ_8"] = String(MQ_8);
    Readings["MQ_9"] = String(MQ_9);
    Readings["MQ_135"] = String(MQ_135);

    Readings["Slave_UpTime"] = String(Slave_UpTime);

    String JSON_String;
    serializeJson(Readings, JSON_String);

    Last_SSE_Time = millis();
    Events.send(JSON_String.c_str(), "json", Last_SSE_Time);

    SSEs++;
    Display_SSE_Count();

    digitalWrite(Work_LED_Pin, LOW);
  }
}

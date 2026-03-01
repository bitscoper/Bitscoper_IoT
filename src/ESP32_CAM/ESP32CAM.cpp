/* By Abdullah As-Sadeed*/

/* Arduino IDE Board Manager URL: https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json

Arduino IDE LittleFS Uploader Plugin: https://github.com/earlephilhower/arduino-littlefs-upload/ */

/*
  Board: "AI Thinker ESP32_CAM-CAM"
  CPU Frequency: "240MHz (WiFi/BT)"
  Core Debug Level: "Warn"
  Flash Frequency: "80MHz"
  Flash Mode: "DIO"
  Partition Scheme: "Huge APP (3MB No OTA/1MB LittleFS)"
  Programmer: "Esptool"
*/

#include <Arduino.h>
#include <esp_arduino_version.h>

#include <FS.h>
#include <LittleFS.h>

#include <esp_wifi.h>
#include <WiFi.h>

#include <AsyncTCP.h> /* https://github.com/me-no-dev/AsyncTCP/ */                   /* Manual Download */
#include <ESPAsyncWebServer.h> /* https://github.com/me-no-dev/ESPAsyncWebServer/ */ /* Manual Download */

#include <ArduinoJson.h> /* https://github.com/bblanchon/ArduinoJson/ */

#include <Confidentials.h> /* Local */

#define LED_BUILTIN 33

#define ARDUINO_MEGA_2560 Serial
#define ARDUINO_MEGA_2560_BAUD_RATE 115200

#define FORMAT_LITTLEFS_IF_FAILED true

#define WEBSERVER_PORT 80

#define FLASH_LED 4

AsyncWebServer Server(WEBSERVER_PORT);
AsyncEventSource Events("/Server_Sent_Events");

void SetUp_LED_BUILTIN()
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); /* Off */
}

void SetUp_WiFi(void)
{
  digitalWrite(LED_BUILTIN, LOW); /* On */

  WiFi.mode(WIFI_STA);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname("Bitscoper_IoT");

  WiFi.begin(WiFi_SSID, WiFi_PassWord);

  while (WiFi.status() != WL_CONNECTED)
  {
  }

  digitalWrite(LED_BUILTIN, HIGH); /* Off */
}

void Add_Common_Headers(AsyncWebServerResponse *Response)
{
  Response->addHeader("Access-Control-Allow-Origin", "*");
  Response->addHeader("Referrer-Policy", "strict-origin");
  Response->addHeader("Server", "Bitscoper IoT");
  Response->addHeader("X-Content-Type-Options", "nosniff");
  Response->addHeader("X-Frame-Options", "SAMEORIGIN");
}

String Process_Page(const String &Variable)
{
  int Chip_ID = 0;

  for (int Iteration = 0; Iteration < 17; Iteration = Iteration + 8)
  {
    Chip_ID |= ((ESP.getEfuseMac() >> (40 - Iteration)) & 0xff) << Iteration;
  }

  if (Variable == "ESP32_CAM_CHIP_MODEL")
  {
    return String(ESP.getChipModel());
  }
  else if (Variable == "ESP32_CAM_CHIP_REVISION")
  {
    return String(ESP.getChipRevision());
  }
  else if (Variable == "ESP32_CAM_CHIP_CORES")
  {
    return String(ESP.getChipCores());
  }
  else if (Variable == "ESP32_CAM_CHIP_ID")
  {
    return String(Chip_ID);
  }
  else if (Variable == "ESP32_CAM_MAC_ADDRESS")
  {
    return String(WiFi.macAddress());
  }
  else if (Variable == "ESP32_CAM_HOST_NAME")
  {
    return String(WiFi.getHostname());
  }
  else if (Variable == "ESP32_CAM_LOCAL_IP_ADDRESS")
  {
    return String(WiFi.localIP().toString().c_str());
  }
  else if (Variable == "ESP32_CAM_UPTIME")
  {
    float UpTime = millis() / 1000;

    return String(UpTime);
  }

  return String();
}

void Handle_Root(AsyncWebServerRequest *Request)
{
  digitalWrite(LED_BUILTIN, LOW); /* On */

  File XHTML_Body_File = LittleFS.open("/Client.xhtml", "r");

  String XHTML_Body_Content = XHTML_Body_File.readString();
  XHTML_Body_File.close();

  const char *XHTML_Body PROGMEM = XHTML_Body_Content.c_str();

  AsyncWebServerResponse *Response = Request->beginResponse_P(200, "text/html; charset=utf-8", XHTML_Body, Process_Page);
  Add_Common_Headers(Response);
  Request->send(Response);

  digitalWrite(LED_BUILTIN, HIGH); /* Off */
}

void Serve_FavIcon(AsyncWebServerRequest *Request)
{
  digitalWrite(LED_BUILTIN, LOW); /* On */

  File FavIcon_File = LittleFS.open("/favicon.ico", "r");

  AsyncWebServerResponse *Response = Request->beginResponse(FavIcon_File, "Bitscoper_IoT");
  Response->addHeader("Content-Type", "image/vnd.microsoft.icon");
  Add_Common_Headers(Response);
  Request->send(Response);

  digitalWrite(LED_BUILTIN, HIGH); /* Off */
}

void Serve_CSS(AsyncWebServerRequest *Request)
{
  digitalWrite(LED_BUILTIN, LOW); /* On */

  File CSS_File = LittleFS.open("/Client.css", "r");
  String CSS = CSS_File.readString();
  CSS_File.close();

  AsyncWebServerResponse *Response = Request->beginResponse(200, "text/css", CSS);
  Add_Common_Headers(Response);
  Request->send(Response);

  digitalWrite(LED_BUILTIN, HIGH); /* Off */
}

void Serve_JavaScript(AsyncWebServerRequest *Request)
{
  digitalWrite(LED_BUILTIN, LOW); /* On */

  File JavaScript_File = LittleFS.open("/Client.js", "r");
  String JavaScript = JavaScript_File.readString();
  JavaScript_File.close();

  AsyncWebServerResponse *Response = Request->beginResponse(200, "text/javascript", JavaScript);
  Add_Common_Headers(Response);
  Request->send(Response);

  digitalWrite(LED_BUILTIN, HIGH); /* Off */
}

String Scan_WiFi(void)
{
  digitalWrite(LED_BUILTIN, LOW); /* On */

  String Result = "";

  int n = WiFi.scanComplete();
  if (n == -2)
  {
    WiFi.scanNetworks(true);
  }
  else if (n)
  {
    Result += "<table>\n<tbody>\n<tr>\n<th colspan=\"5\">Wi-Fi Access Points</th>\n</tr>\n<tr>\n<th>RSSI</th>\n<th>SSID</th>\n<th>BSSID</th>\n<th>Channel</th>\n<th>Encryption</th>\n</tr>\n";
    for (int Iteration = 0; Iteration < n; ++Iteration)
    {
      Result += "<tr>\n<td>";
      Result += WiFi.RSSI(Iteration);
      Result += "</td>\n<td>";
      Result += WiFi.SSID(Iteration);
      Result += "</td>\n<td>";
      Result += WiFi.BSSIDstr(Iteration);
      Result += "</td>\n<td>";
      Result += WiFi.channel(Iteration);
      Result += "</td>\n<td>";
      int Encryption = WiFi.encryptionType(Iteration);
      if (Encryption == WIFI_AUTH_OPEN)
      {
        Result += "Open";
      }
      else if (Encryption == WIFI_AUTH_WEP)
      {
        Result += "WEP";
      }
      else if (Encryption == WIFI_AUTH_WPA_PSK)
      {
        Result += "WPA PSK";
      }
      else if (Encryption == WIFI_AUTH_WPA2_PSK)
      {
        Result += "WPA2 PSK";
      }
      else if (Encryption == WIFI_AUTH_WPA_WPA2_PSK)
      {
        Result += "WPA WPA2 PSK";
      }
      else if (Encryption == WIFI_AUTH_WPA2_ENTERPRISE)
      {
        Result += "WPA2 Enterprise";
      }
      Result += "</td>\n</tr>\n";
    }
    Result += "</tbody>\n</table>\n";

    WiFi.scanDelete();
    if (WiFi.scanComplete() == -2)
    {
      WiFi.scanNetworks(true);
    }
  }

  return Result;

  digitalWrite(LED_BUILTIN, HIGH); /* Off */
}

void Handle_Request_JSON(AsyncWebServerRequest *Request)
{
  digitalWrite(LED_BUILTIN, LOW); /* On */

  int Arguments = Request->args();

  if (Arguments > 0)
  {
    for (int Iteration = 0; Iteration < Arguments; Iteration++)
    {
      if (Request->argName(Iteration) == "JSON")
      {
        JsonDocument Request_JSON;

        DeserializationError Request_JSON_Error = deserializeJson(Request_JSON, Request->arg(Iteration));

        if (Request_JSON_Error == DeserializationError::Ok)
        {
          String Response_String = "";

          if (Request_JSON.containsKey("Flash_LED"))
          {
            bool Flash_LED_State = Request_JSON["Flash_LED"].as<bool>();

            if (Flash_LED_State)
            {
              digitalWrite(FLASH_LED, HIGH);
            }
            else if (!Flash_LED_State)
            {
              digitalWrite(FLASH_LED, LOW);
            }
          }

          if (Request_JSON.containsKey("Scan_WiFi"))
          {
            Response_String = Scan_WiFi();
          }

          ARDUINO_MEGA_2560.println();
          serializeJson(Request_JSON, ARDUINO_MEGA_2560);
          ARDUINO_MEGA_2560.flush();

          AsyncWebServerResponse *Response = Request->beginResponse(200, "text/html; charset=utf-8", Response_String);
          Add_Common_Headers(Response);
          Request->send(Response);

          if (Request_JSON.containsKey("ReBoot_ESP32_CAM"))
          {
            if (Request_JSON["ReBoot_ESP32_CAM"].as<bool>())
            {
              ESP.restart();
            };
          }
        }
      }
    }
  }

  digitalWrite(LED_BUILTIN, HIGH); /* Off */
}

void Assign_URLs(void)
{
  Server.on("/", HTTP_GET, Handle_Root);

  Server.on("/favicon.ico", HTTP_GET, Serve_FavIcon);
  Server.on("/Client.css", HTTP_GET, Serve_CSS);
  Server.on("/Client.js", HTTP_GET, Serve_JavaScript);

  Server.on("/Requests", HTTP_GET, Handle_Request_JSON);
}

void Handle_Error_404(AsyncWebServerRequest *Request)
{
  digitalWrite(LED_BUILTIN, LOW); /* On */

  String Response_String = "HTTP 404\n\n";

  Response_String += "Version: ";
  int version = Request->version();
  if (version == 0)
  {
    Response_String += "HTTP/1.0\n\n";
  }
  else if (version == 1)
  {
    Response_String += "HTTP/1.1\n\n";
  }

  Response_String += "Host: ";
  Response_String += Request->host();

  Response_String += "\n\nURL: ";
  Response_String += Request->url();

  Response_String += "\n\nMethod: ";
  int method = Request->method();
  if (method == HTTP_GET)
  {
    Response_String += "GET";
  }
  else if (method == HTTP_POST)
  {
    Response_String += "POST";
  }
  else if (method == HTTP_DELETE)
  {
    Response_String += "DELETE";
  }
  else if (method == HTTP_PUT)
  {
    Response_String += "PUT";
  }
  else if (method == HTTP_PATCH)
  {
    Response_String += "PATCH";
  }
  else if (method == HTTP_HEAD)
  {
    Response_String += "HEAD";
  }
  else if (method == HTTP_OPTIONS)
  {
    Response_String += "OPTIONS";
  }

  Response_String += "\n\nTotal arguments: ";
  int Arguments = Request->args();
  Response_String += Arguments;
  Response_String += "\n\n";

  if (Arguments > 0)
  {
    Response_String += "Arguments:\n";

    for (int Iteration = 0; Iteration < Arguments; Iteration++)
    {
      Response_String += "\t  ";
      Response_String += Request->argName(Iteration).c_str();
      Response_String += ": ";
      Response_String += Request->arg(Iteration).c_str();
      Response_String += "\n";
    }

    Response_String += "\n";
  }

  Response_String += "Total Request headers: ";
  Response_String += Request->headers();

  AsyncWebServerResponse *Response = Request->beginResponse(404, "text/plain; charset=utf-8", Response_String);
  Add_Common_Headers(Response);
  Response->addHeader("Content-Disposition", "inline; filename=\"Bitscoper_IoT | Error 404\"");
  Request->send(Response);

  digitalWrite(LED_BUILTIN, HIGH); /* Off */
}

void SetUp_Server(void)
{
  Assign_URLs();

  Server.addHandler(&Events);

  Server.onNotFound(Handle_Error_404);

  Server.begin();
}

void SetUp_Flash_LED()
{
  pinMode(FLASH_LED, OUTPUT);
  digitalWrite(FLASH_LED, LOW);
}

void setup(void)
{
  SetUp_LED_BUILTIN();

  ARDUINO_MEGA_2560.begin(ARDUINO_MEGA_2560_BAUD_RATE);
  // ARDUINO_MEGA_2560.setDebugOutput(true);

  if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED))
  {
    return;
  };

  SetUp_WiFi();

  SetUp_Server();

  SetUp_Flash_LED();
}

void Send_IP_Address_To_Arduino_Mega_2560(void)
{
  JsonDocument Response_JSON;

  Response_JSON["IP_Address"] = String(WiFi.localIP().toString().c_str());

  ARDUINO_MEGA_2560.println();
  serializeJson(Response_JSON, ARDUINO_MEGA_2560);
  ARDUINO_MEGA_2560.flush();
}

void loop(void)
{
  while (ARDUINO_MEGA_2560.available())
  {
    digitalWrite(LED_BUILTIN, LOW); /* On */

    JsonDocument ARDUINO_MEGA_2560_JSON;

    DeserializationError ARDUINO_MEGA_2560_JSON_Error = deserializeJson(ARDUINO_MEGA_2560_JSON, ARDUINO_MEGA_2560);

    if (ARDUINO_MEGA_2560_JSON_Error == DeserializationError::Ok)
    {
      Send_IP_Address_To_Arduino_Mega_2560();

      String Arduino_Mega_2560_JSON_String;
      serializeJson(ARDUINO_MEGA_2560_JSON, Arduino_Mega_2560_JSON_String);
      Events.send(Arduino_Mega_2560_JSON_String.c_str(), "SSE", millis());
    }

    digitalWrite(LED_BUILTIN, HIGH); /* Off */
  }
}

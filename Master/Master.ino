/* by Abdullah As-Sadeed */

/*
Board: "Arduino Mega or Mega 2560"
Processor: "ATmega2560 (Mega 2560)"
Programmer: "AVR ISP"
*/

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

#include <MPU9250_WE.h>

#include <BH1750.h>

#include <Seeed_BME280.h>

#include <rdm6300.h>
#include <MFRC522.h>

#include <TinyGPSPlus.h>

#include <RTClib.h>

#include <Stepper.h>
#include <Servo.h>

#include <Adafruit_SSD1306.h>

#include <ArduinoJson.h>

#define MPU9250_ADDRESS 0x69
#define BH1750_ADDRESS 0x23

#define MQ2 A0
#define MQ3 A1
#define MQ4 A2
#define MQ5 A3
#define MQ6 A4
#define MQ7 A5
#define MQ8 A6
#define MQ9 A7
#define MQ135 A8

#define RCWL0516 24

#define FLAME_SENSOR 22

#define RDM6300_UART Serial3

#define RC522_SS 53
#define RC522_RESET 5

#define NEO7M Serial2
#define NEO7M_BAUD_RATE 9600

#define SG90_PIN 7
#define SG90_STARTUP_POSITION 180

#define SSD1306_ADDRESS 0x3C
#define SSD1306_RESET -1
#define SSD1306_WIDTH 128
#define SSD1306_HEIGHT 64

#define RGB_LED_RED 10
#define RGB_LED_GREEN 9
#define RGB_LED_BLUE 8

#define BUZZER 11

#define RELAY_1 23
#define RELAY_2 25

#define ESP32 Serial
#define ESP32_BAUD_RATE 115200

MPU9250_WE MPU9250 = MPU9250_WE(MPU9250_ADDRESS);

BH1750 bh1750(BH1750_ADDRESS);

BME280 bme280;

Rdm6300 RDM6300;

MFRC522 RC522(RC522_SS, RC522_RESET);
MFRC522::MIFARE_Key RC522_Key;

TinyGPSPlus GPS;

RTC_DS3231 DS3231;

Servo SG90;

Adafruit_SSD1306 SSD1306(SSD1306_WIDTH, SSD1306_HEIGHT, &Wire, SSD1306_RESET);

struct I2C_Status_Type
{
  boolean MPU9250, MPU9250_Magnetometer, BH1750, BME280, SSD1306;
};
I2C_Status_Type I2C_Status;

struct Acceleration_Type
{
  float X, Y, Z, Resultant;
};
struct Gyro_Type
{
  float X, Y, Z;
};
struct Magneto_Type
{
  float X, Y, Z;
};
struct MPU9250_Readings_Type
{
  struct Acceleration_Type Acceleration;
  struct Gyro_Type Gyro;
  struct Magneto_Type Magneto;
  float Temperature;
};
MPU9250_Readings_Type MPU9250_Readings;

float Light;

struct BME280_Readings_Type
{
  float Temperature, Humidity, Pressure, Altitude;
};
BME280_Readings_Type BME280_Readings;

struct MQs_Readings_Type
{
  float MQ2, MQ3, MQ4, MQ5, MQ6, MQ7, MQ8, MQ9, MQ135;
};
MQs_Readings_Type MQs_Readings;

boolean BlackBody_Motion, Flame;

struct NEO7M_Readings_Type
{
  unsigned int Satellites;
  float Latitude, Longitude, Speed, Course, Altitude, HDOP;
};
NEO7M_Readings_Type NEO7M_Readings;

unsigned long RDM6300_Reading;

struct RC522_Reading_Type
{
  String PICC_Type;
  boolean MIFARE_Classic_Validity;
  unsigned long UID;
};
RC522_Reading_Type RC522_Reading;

struct DS3231_OutPut_Type
{
  int UNIX_Time, Year, Month, Day, Week_Day, Hour, Minute, Second;
  float Temperature;
};
DS3231_OutPut_Type DS3231_OutPut;

const char *Compilation_Date_Time = __DATE__ " " __TIME__;

void (*Reset_Arduino_Mega_2560)(void) = 0;

void SetUp_Buzzer(void)
{
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);
}

String Scan_I2C(void)
{
  byte error, address;
  unsigned int count = 0;
  String result = "";

  for (address = 0x01; address < 0x7f; address++)
  {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      result += "0x";
      result += String(address, HEX);
      result += " ";

      count++;
    }
    else if (error != 2)
    {
      result += "Error ";
      result += error;
      result += " at address 0x";
      result += String(address, HEX);
      result += "\n";
    }
  }

  if (count == 0)
  {
    result += "No device";
  }

  return result;
}

void SetUp_MPU9250(void)
{
  if (MPU9250.init())
  {
    I2C_Status.MPU9250 = true;
  }
  else
  {
    I2C_Status.MPU9250 = false;
    digitalWrite(BUZZER, HIGH);
  }

  if (MPU9250.initMagnetometer())
  {
    I2C_Status.MPU9250_Magnetometer = true;
  }
  else
  {
    I2C_Status.MPU9250_Magnetometer = false;
    digitalWrite(BUZZER, HIGH);
  }

  if (I2C_Status.MPU9250 && I2C_Status.MPU9250_Magnetometer)
  {
    delay(1000);

    MPU9250.autoOffsets();

    MPU9250.enableAccDLPF(true);
    MPU9250.setAccDLPF(MPU9250_DLPF_6); /* Lowest Noise */

    MPU9250.enableGyrDLPF();
    MPU9250.setGyrDLPF(MPU9250_DLPF_6); /* Lowest Noise */

    MPU9250.setAccRange(MPU6500_ACC_RANGE_16G); /* Highest */

    MPU9250.setGyrRange(MPU9250_GYRO_RANGE_2000); /* Highest */

    MPU9250.setMagOpMode(AK8963_CONT_MODE_100HZ); /* Highest */

    MPU9250.setSampleRateDivider(0); /* Lowest */

    delay(200);
  }
}

void SetUp_BH1750(void)
{
  if (bh1750.begin(BH1750::CONTINUOUS_HIGH_RES_MODE_2)) /* Highest */
  {
    I2C_Status.BH1750 = true;
  }
  else
  {
    I2C_Status.BH1750 = false;
    digitalWrite(BUZZER, HIGH);
  }
}

void SetUp_BME280(void)
{
  if (bme280.init())
  {
    I2C_Status.BME280 = true;
  }
  else
  {
    I2C_Status.BME280 = false;
    digitalWrite(BUZZER, HIGH);
  }
}

void SetUp_RDM6300(void)
{
  RDM6300_UART.begin(RDM6300_BAUDRATE);
  RDM6300.begin(&RDM6300_UART);
}

void SetUp_RC522(void)
{
  RC522.PCD_Init();

  for (byte i = 0; i < 6; i++)
  {
    RC522_Key.keyByte[i] = 0xFF;
  }
}

void Attach_SG90(void)
{
  SG90.attach(SG90_PIN);
}

void SetUp_SG90(void)
{
  Attach_SG90();
  SG90.write(SG90_STARTUP_POSITION);
}

void SetUp_DS3231(void)
{
  DS3231.begin();

  if (DS3231.lostPower())
  {
    DS3231.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

void SetUp_BuiltIn_LED(void)
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
}

void SetUp_RGB_LED(void)
{
  pinMode(RGB_LED_RED, OUTPUT);
  pinMode(RGB_LED_GREEN, OUTPUT);
  pinMode(RGB_LED_BLUE, OUTPUT);

  digitalWrite(RGB_LED_RED, LOW);
  digitalWrite(RGB_LED_GREEN, LOW);
  digitalWrite(RGB_LED_BLUE, LOW);
}

void SetUp_SSD1306(void)
{
  if (!SSD1306.begin(SSD1306_SWITCHCAPVCC, SSD1306_ADDRESS))
  {
    I2C_Status.SSD1306 = false;
    digitalWrite(BUZZER, HIGH);
  }

  SSD1306.clearDisplay();

  SSD1306.setTextSize(1);
  SSD1306.setTextColor(SSD1306_WHITE);
  SSD1306.setCursor(25, 0);
  SSD1306.println(F("Bitscoper IoT"));
  SSD1306.display();
}

void SetUp_Relays()
{
  pinMode(RELAY_1, OUTPUT);
  digitalWrite(RELAY_1, HIGH);

  pinMode(RELAY_2, OUTPUT);
  digitalWrite(RELAY_2, HIGH);
}

void setup(void)
{
  SetUp_Buzzer();

  ESP32.begin(ESP32_BAUD_RATE);

  Wire.begin();
  ESP32.println();
  ESP32.println(Scan_I2C());

  SPI.begin();

  SetUp_MPU9250();

  SetUp_BH1750();

  SetUp_BME280();

  pinMode(RCWL0516, INPUT);

  pinMode(FLAME_SENSOR, INPUT);

  SetUp_RDM6300();

  SetUp_RC522();

  NEO7M.begin(NEO7M_BAUD_RATE);

  SetUp_DS3231();

  SetUp_SG90();

  SetUp_BuiltIn_LED();

  SetUp_RGB_LED();

  SetUp_SSD1306();

  SetUp_Relays();
}

void Read_MPU9250(void)
{
  if (I2C_Status.MPU9250 && I2C_Status.MPU9250_Magnetometer)
  {

    xyzFloat acceleration = MPU9250.getGValues();
    xyzFloat gyro = MPU9250.getGyrValues();
    xyzFloat magneto = MPU9250.getMagValues();
    MPU9250_Readings.Acceleration.Resultant =
        MPU9250.getResultantG(acceleration);

    MPU9250_Readings.Acceleration.X = acceleration.x;
    MPU9250_Readings.Acceleration.Y = acceleration.y;
    MPU9250_Readings.Acceleration.Z = acceleration.z;

    MPU9250_Readings.Gyro.X = gyro.x;
    MPU9250_Readings.Gyro.Y = gyro.y;
    MPU9250_Readings.Gyro.Z = gyro.z;

    MPU9250_Readings.Magneto.X = magneto.x;
    MPU9250_Readings.Magneto.Y = magneto.y;
    MPU9250_Readings.Magneto.Z = magneto.z;

    MPU9250_Readings.Temperature = MPU9250.getTemperature();
  }
  else
  {
    MPU9250_Readings.Acceleration.X = 0;
    MPU9250_Readings.Acceleration.Y = 0;
    MPU9250_Readings.Acceleration.Z = 0;
    MPU9250_Readings.Acceleration.Resultant = 0;

    MPU9250_Readings.Gyro.X = 0;
    MPU9250_Readings.Gyro.Y = 0;
    MPU9250_Readings.Gyro.Z = 0;

    MPU9250_Readings.Magneto.X = 0;
    MPU9250_Readings.Magneto.Y = 0;
    MPU9250_Readings.Magneto.Z = 0;

    MPU9250_Readings.Temperature = 0;
  }
}

void Read_BH1750(void)
{
  if (I2C_Status.BH1750 && bh1750.measurementReady())
  {
    Light = bh1750.readLightLevel();
  }
  else
  {
    Light = 0;
  }
}

void Read_BME280(void)
{
  if (I2C_Status.BME280)
  {
    BME280_Readings.Temperature = bme280.getTemperature();
    BME280_Readings.Humidity = bme280.getHumidity();
    BME280_Readings.Pressure = bme280.getPressure();
    BME280_Readings.Altitude = bme280.calcAltitude(BME280_Readings.Pressure);
  }
  else
  {
    BME280_Readings.Temperature = 0;
    BME280_Readings.Humidity = 0;
    BME280_Readings.Pressure = 0;
    BME280_Readings.Altitude = 0;
  }
}

void Read_MQ_Sensors(void)
{
  MQs_Readings.MQ2 = analogRead(MQ2);
  MQs_Readings.MQ3 = analogRead(MQ3);
  MQs_Readings.MQ4 = analogRead(MQ4);
  MQs_Readings.MQ5 = analogRead(MQ5);
  MQs_Readings.MQ6 = analogRead(MQ6);
  MQs_Readings.MQ7 = analogRead(MQ7);
  MQs_Readings.MQ8 = analogRead(MQ8);
  MQs_Readings.MQ9 = analogRead(MQ9);
  MQs_Readings.MQ135 = analogRead(MQ135);
}

void Read_NEO7M(void)
{
  while (NEO7M.available())
  {
    GPS.encode(NEO7M.read());
  }

  NEO7M_Readings.Satellites = GPS.satellites.value();
  NEO7M_Readings.Latitude = GPS.location.lat();
  NEO7M_Readings.Longitude = GPS.location.lng();
  NEO7M_Readings.Speed = GPS.speed.kmph();
  NEO7M_Readings.Course = GPS.course.deg();
  NEO7M_Readings.Altitude = GPS.altitude.meters();
  NEO7M_Readings.HDOP = GPS.hdop.value();
}

void Read_RC522(void)
{
  RC522.PICC_IsNewCardPresent(); /* Needed to read. */

  if (RC522.PICC_ReadCardSerial())
  {
    MFRC522::PICC_Type PICC_Type = RC522.PICC_GetType(RC522.uid.sak);
    RC522_Reading.PICC_Type = RC522.PICC_GetTypeName(PICC_Type);

    if (PICC_Type != MFRC522::PICC_TYPE_MIFARE_MINI && PICC_Type != MFRC522::PICC_TYPE_MIFARE_1K && PICC_Type != MFRC522::PICC_TYPE_MIFARE_4K)
    {
      RC522_Reading.MIFARE_Classic_Validity = false;

      RC522_Reading.UID = 0;
    }
    else
    {
      RC522_Reading.MIFARE_Classic_Validity = true;

      RC522_Reading.UID = (static_cast<unsigned long>(RC522.uid.uidByte[0]) << 24) | (static_cast<unsigned long>(RC522.uid.uidByte[1]) << 16) | (static_cast<unsigned long>(RC522.uid.uidByte[2]) << 8) | static_cast<unsigned long>(RC522.uid.uidByte[3]);

      RC522.PICC_HaltA();
      RC522.PCD_StopCrypto1();
    }
  }
  else
  {
    RC522_Reading.UID = 0;
  }
}

void Read_DS3231(void)
{
  DateTime now = DS3231.now();

  DS3231_OutPut.UNIX_Time = now.unixtime();

  DS3231_OutPut.Year = now.year();
  DS3231_OutPut.Month = now.month();
  DS3231_OutPut.Day = now.day();
  DS3231_OutPut.Week_Day = now.dayOfTheWeek();

  DS3231_OutPut.Hour = now.hour();
  DS3231_OutPut.Minute = now.minute();
  DS3231_OutPut.Second = now.second();

  DS3231_OutPut.Temperature = DS3231.getTemperature();
}

void Send_JSON(void)
{
  JsonDocument Readings_JSON;

  JsonDocument Acceleration_JSON;
  Acceleration_JSON["X"] = MPU9250_Readings.Acceleration.X;
  Acceleration_JSON["Y"] = MPU9250_Readings.Acceleration.Y;
  Acceleration_JSON["Z"] = MPU9250_Readings.Acceleration.Z;
  Acceleration_JSON["Resultant"] =
      MPU9250_Readings.Acceleration.Resultant;

  JsonDocument Gyro_JSON;
  Gyro_JSON["X"] = MPU9250_Readings.Gyro.X;
  Gyro_JSON["Y"] = MPU9250_Readings.Gyro.Y;
  Gyro_JSON["Z"] = MPU9250_Readings.Gyro.Z;

  JsonDocument Magneto_JSON;
  Magneto_JSON["X"] = MPU9250_Readings.Magneto.X;
  Magneto_JSON["Y"] = MPU9250_Readings.Magneto.Y;
  Magneto_JSON["Z"] = MPU9250_Readings.Magneto.Z;

  JsonDocument MPU9250_JSON;
  MPU9250_JSON["Acceleration"] = Acceleration_JSON;
  MPU9250_JSON["Gyro"] = Gyro_JSON;
  MPU9250_JSON["Magneto"] = Magneto_JSON;
  MPU9250_JSON["Temperature"] = MPU9250_Readings.Temperature;

  Readings_JSON["MPU9250"] = MPU9250_JSON;

  Readings_JSON["BH1750"] = Light;

  JsonDocument BME280_JSON;
  BME280_JSON["Temperature"] = BME280_Readings.Temperature;
  BME280_JSON["Humidity"] = BME280_Readings.Humidity;
  BME280_JSON["Pressure"] = BME280_Readings.Pressure;
  BME280_JSON["Altitude"] = BME280_Readings.Altitude;
  Readings_JSON["BME280"] = BME280_JSON;

  Readings_JSON["MQ2"] = MQs_Readings.MQ2;
  Readings_JSON["MQ3"] = MQs_Readings.MQ3;
  Readings_JSON["MQ4"] = MQs_Readings.MQ4;
  Readings_JSON["MQ5"] = MQs_Readings.MQ5;
  Readings_JSON["MQ6"] = MQs_Readings.MQ6;
  Readings_JSON["MQ7"] = MQs_Readings.MQ7;
  Readings_JSON["MQ8"] = MQs_Readings.MQ8;
  Readings_JSON["MQ9"] = MQs_Readings.MQ9;
  Readings_JSON["MQ135"] = MQs_Readings.MQ135;

  Readings_JSON["RCWL0516"] = BlackBody_Motion;

  Readings_JSON["Flame"] = Flame;

  Readings_JSON["RDM6300"] = RDM6300_Reading;

  JsonDocument RC522_JSON;
  RC522_JSON["PICC_Type"] = RC522_Reading.PICC_Type;
  RC522_JSON["MIFARE_Classic_Validity"] = RC522_Reading.MIFARE_Classic_Validity;
  RC522_JSON["UID"] = RC522_Reading.UID;
  Readings_JSON["RC522"] = RC522_JSON;

  JsonDocument NEO7M_JSON;
  NEO7M_JSON["Satellites"] = NEO7M_Readings.Satellites;
  NEO7M_JSON["Latitude"] = NEO7M_Readings.Latitude;
  NEO7M_JSON["Longitude"] = NEO7M_Readings.Longitude;
  NEO7M_JSON["Speed"] = NEO7M_Readings.Speed;
  NEO7M_JSON["Course"] = NEO7M_Readings.Course;
  NEO7M_JSON["Altitude"] = NEO7M_Readings.Altitude;
  NEO7M_JSON["HDOP"] = NEO7M_Readings.HDOP;
  Readings_JSON["NEO7M"] = NEO7M_JSON;

  JsonDocument DS3231_JSON;
  DS3231_JSON["UNIX_Time"] = DS3231_OutPut.UNIX_Time;
  DS3231_JSON["Year"] = DS3231_OutPut.Year;
  DS3231_JSON["Month"] = DS3231_OutPut.Month;
  DS3231_JSON["Day"] = DS3231_OutPut.Day;
  DS3231_JSON["Week_Day"] = DS3231_OutPut.Week_Day;
  DS3231_JSON["Hour"] = DS3231_OutPut.Hour;
  DS3231_JSON["Minute"] = DS3231_OutPut.Minute;
  DS3231_JSON["Second"] = DS3231_OutPut.Second;
  DS3231_JSON["Temperature"] = DS3231_OutPut.Temperature;
  Readings_JSON["DS3231"] = DS3231_JSON;

  Readings_JSON["UpTime"] = millis();

  ESP32.println();
  serializeJson(Readings_JSON, ESP32);
  ESP32.flush();
}

void Receive_JSON(void)
{
  while (ESP32.available())
  {
    JsonDocument ESP32_JSON;

    DeserializationError ESP32_JSON_Error = deserializeJson(ESP32_JSON, ESP32);

    if (ESP32_JSON_Error == DeserializationError::Ok)
    {
      digitalWrite(LED_BUILTIN, HIGH);

      if (ESP32_JSON.containsKey("Reset_Arduino_Mega_2560"))
      {
        bool If_Reset_Arduino_Mega_2560 = ESP32_JSON["Reset_Arduino_Mega_2560"].as<bool>();
        if (If_Reset_Arduino_Mega_2560)
        {
          Reset_Arduino_Mega_2560();
        }
      }

      if (ESP32_JSON.containsKey("SG90_Position"))
      {
        unsigned int SG90_Position = ESP32_JSON["SG90_Position"].as<unsigned int>();

        SG90.write(SG90_Position);
      }

      if (ESP32_JSON.containsKey("Detach_SG90"))
      {
        bool If_Detach_SG90 = ESP32_JSON["Detach_SG90"].as<bool>();

        if (If_Detach_SG90)
        {
          SG90.detach();
        }
      }

      if (ESP32_JSON.containsKey("Attach_SG90"))
      {
        bool If_Attach_SG90 = ESP32_JSON["Attach_SG90"].as<bool>();

        if (If_Attach_SG90)
        {
          Attach_SG90();
        }
      }

      if (ESP32_JSON.containsKey("Relay_1"))
      {
        bool Relay_1_State = ESP32_JSON["Relay_1"].as<bool>();
        if (Relay_1_State)
        {
          digitalWrite(RELAY_1, LOW);
        }
        else if (!Relay_1_State)
        {
          digitalWrite(RELAY_1, HIGH);
        }
      }

      if (ESP32_JSON.containsKey("Relay_2"))
      {
        bool Relay_2_State = ESP32_JSON["Relay_2"].as<bool>();
        if (Relay_2_State)
        {
          digitalWrite(RELAY_2, LOW);
        }
        else if (!Relay_2_State)
        {
          digitalWrite(RELAY_2, HIGH);
        }
      }

      digitalWrite(LED_BUILTIN, LOW);
    }
  }
}

void loop(void)
{
  digitalWrite(LED_BUILTIN, HIGH);

  Read_MPU9250();

  Read_BH1750();

  Read_BME280();

  Read_MQ_Sensors();

  BlackBody_Motion = digitalRead(RCWL0516);

  Flame = !digitalRead(FLAME_SENSOR);

  Read_NEO7M();

  RDM6300_Reading = RDM6300.get_tag_id();

  Read_RC522();

  Read_DS3231();

  digitalWrite(LED_BUILTIN, LOW);

  Send_JSON();

  while (!ESP32.available())
  {
    delay(10);
  }

  Receive_JSON();
}

// TODO:
// Set Buzzer on I2C Error of DS3231
// Set Buzzer on Serial Initialization Failures
// NEO7M Physical Test
// Set Usage of RGB LED
// Buy IR Flame Sensor
// RCWL0516 Interference
// SIM900A
// MAX30102
// DSM501A
// DS3231 Alarm
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

#define DS3231_INTERRUPT 2

#define SG90_PIN 7
#define SG90_STARTUP_POSITION 180

#define ULN2003_1 28
#define ULN2003_2 30
#define ULN2003_3 32
#define ULN2003_4 34
#define ULN2003_STEPS_PER_REVOLUTION 2038
#define ULN2003_SPEED 2

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

Stepper ULN2003(ULN2003_STEPS_PER_REVOLUTION, ULN2003_1, ULN2003_2, ULN2003_3, ULN2003_4);

Adafruit_SSD1306 SSD1306(SSD1306_WIDTH, SSD1306_HEIGHT, &Wire, SSD1306_RESET);

struct I2C_Status_Type
{
  boolean MPU9250, MPU9250_Magnetometer, BH1750, BME280, DS3231, SSD1306;
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

struct UART_Status_Type
{
  boolean RDM6300, NEO7M, ESP32;
};
UART_Status_Type UART_Status;

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
  long UNIX_Time, Alarm_1_Time, Alarm_2_Time;
  float Temperature;
  String Alarm_1_Mode, Alarm_2_Mode;
};
DS3231_OutPut_Type DS3231_OutPut;

const char *Compilation_Date_Time = __DATE__ " " __TIME__;

void (*Reset_Arduino_Mega_2560)(void) = 0;

void SetUp_Buzzer(void)
{
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);
}

void SetUp_ESP32(void)
{
  ESP32.begin(ESP32_BAUD_RATE);
  if (ESP32)
  {
    UART_Status.ESP32 = true;
  }
  else
  {
    UART_Status.ESP32 = false;
    digitalWrite(BUZZER, HIGH);
  }
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
  if (RDM6300_UART)
  {
    UART_Status.RDM6300 = true;

    RDM6300.begin(&RDM6300_UART);
  }
  else
  {
    UART_Status.RDM6300 = false;
    digitalWrite(BUZZER, HIGH);
  }
}

void SetUp_RC522(void)
{
  RC522.PCD_Init();

  for (byte i = 0; i < 6; i++)
  {
    RC522_Key.keyByte[i] = 0xFF;
  }
}

void SetUp_NEO7M(void)
{
  NEO7M.begin(NEO7M_BAUD_RATE);
  if (NEO7M)
  {
    UART_Status.NEO7M = true;
  }
  else
  {
    UART_Status.NEO7M = false;
    digitalWrite(BUZZER, HIGH);
  }
}

void On_Alarm(void)
{
  if (DS3231.alarmFired(1))
  {
    ESP32.println("Alarm 1");
  }
  if (DS3231.alarmFired(2))
  {
    ESP32.println("Alarm 2");
  }
}

void SetUp_DS3231(void)
{
  if (DS3231.begin())
  {
    I2C_Status.DS3231 = true;

    DS3231.disable32K();
    DS3231.writeSqwPinMode(DS3231_OFF);

    if (DS3231.lostPower())
    {
      DS3231.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    pinMode(DS3231_INTERRUPT, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(DS3231_INTERRUPT), On_Alarm, FALLING);
  }
  else
  {
    I2C_Status.DS3231 = false;
    digitalWrite(BUZZER, HIGH);
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

void SetUp_ULN2003()
{
  ULN2003.setSpeed(ULN2003_SPEED);
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

  SetUp_ESP32();

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

  SetUp_NEO7M();

  SetUp_DS3231();

  SetUp_SG90();

  SetUp_ULN2003();

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

  DS3231_OutPut.Temperature = DS3231.getTemperature();

  DS3231_OutPut.Alarm_1_Time = DS3231.getAlarm1().unixtime();
  Ds3231Alarm1Mode Alarm_1_Mode = DS3231.getAlarm1Mode();
  if (Alarm_1_Mode == DS3231_A1_PerSecond)
  {
    DS3231_OutPut.Alarm_1_Mode = "Per Second";
  }
  else if (Alarm_1_Mode == DS3231_A1_Second)
  {
    DS3231_OutPut.Alarm_1_Mode = "Second";
  }
  else if (Alarm_1_Mode == DS3231_A1_Minute)
  {
    DS3231_OutPut.Alarm_1_Mode = "Minute";
  }
  else if (Alarm_1_Mode == DS3231_A1_Hour)
  {
    DS3231_OutPut.Alarm_1_Mode = "Hour";
  }
  else if (Alarm_1_Mode == DS3231_A1_Date)
  {
    DS3231_OutPut.Alarm_1_Mode = "Date";
  }
  else if (Alarm_1_Mode == DS3231_A1_Day)
  {
    DS3231_OutPut.Alarm_1_Mode = "Day";
  }

  DS3231_OutPut.Alarm_2_Time = DS3231.getAlarm2().unixtime();
  Ds3231Alarm2Mode Alarm_2_Mode = DS3231.getAlarm2Mode();
  if (Alarm_2_Mode == DS3231_A2_Minute)
  {
    DS3231_OutPut.Alarm_2_Mode = "Minute";
  }
  else if (Alarm_2_Mode == DS3231_A2_Hour)
  {
    DS3231_OutPut.Alarm_2_Mode = "Hour";
  }
  else if (Alarm_2_Mode == DS3231_A2_Date)
  {
    DS3231_OutPut.Alarm_2_Mode = "Date";
  }
  else if (Alarm_2_Mode == DS3231_A2_Day)
  {
    DS3231_OutPut.Alarm_2_Mode = "Day";
  }
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
  DS3231_JSON["Temperature"] = DS3231_OutPut.Temperature;
  DS3231_JSON["Alarm_1_Time"] = DS3231_OutPut.Alarm_1_Time;
  DS3231_JSON["Alarm_1_Mode"] = DS3231_OutPut.Alarm_1_Mode;
  DS3231_JSON["Alarm_2_Time"] = DS3231_OutPut.Alarm_2_Time;
  DS3231_JSON["Alarm_2_Mode"] = DS3231_OutPut.Alarm_2_Mode;
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

      if (ESP32_JSON.containsKey("Set_DS3231_Time"))
      {
        long UNIX_Time = ESP32_JSON["Set_DS3231_Time"].as<long>();

        DateTime Time = DateTime(UNIX_Time);

        DS3231.adjust(Time);
      }

      if (ESP32_JSON.containsKey("Set_DS3231_Alarm"))
      {
        unsigned int Number = ESP32_JSON["Set_DS3231_Alarm"]["Number"].as<unsigned int>();
        long UNIX_Time = ESP32_JSON["Set_DS3231_Alarm"]["UNIX_Time"].as<long>();
        String Match = ESP32_JSON["Set_DS3231_Alarm"]["Match"];

        DateTime Time = DateTime(UNIX_Time);

        if (Number == 1)
        {
          DS3231.clearAlarm(1);

          if (Match == "Per Second")
          {
            DS3231.setAlarm1(Time, DS3231_A1_PerSecond);
          }
          else if (Match == "Second")
          {
            DS3231.setAlarm1(Time, DS3231_A1_Second);
          }
          else if (Match == "Minute")
          {
            DS3231.setAlarm1(Time, DS3231_A1_Minute);
          }
          else if (Match == "Hour")
          {
            DS3231.setAlarm1(Time, DS3231_A1_Hour);
          }
          else if (Match == "Date")
          {
            DS3231.setAlarm1(Time, DS3231_A1_Date);
          }
          else if (Match == "Day")
          {
            DS3231.setAlarm1(Time, DS3231_A1_Day);
          }
        }
        else if (Number == 2)
        {
          DS3231.clearAlarm(2);

          if (Match == "Minute")
          {
            DS3231.setAlarm2(Time, DS3231_A2_Minute);
          }
          else if (Match == "Hour")
          {
            DS3231.setAlarm2(Time, DS3231_A2_Hour);
          }
          else if (Match == "Date")
          {
            DS3231.setAlarm2(Time, DS3231_A2_Date);
          }
          else if (Match == "Day")
          {
            DS3231.setAlarm2(Time, DS3231_A2_Day);
          }
        }
      }

      if (ESP32_JSON.containsKey("Clear_DS3231_Alarm"))
      {
        unsigned int Number = ESP32_JSON["Clear_DS3231_Alarm"].as<unsigned int>();

        DS3231.clearAlarm(Number);
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

      if (ESP32_JSON.containsKey("ULN2003_Steps"))
      {
        signed int ULN2003_Steps = ESP32_JSON["ULN2003_Steps"].as<signed int>();

        ULN2003.step(ULN2003_Steps);
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
// NEO7M Physical Test
// Set Usage of RGB LED
// Buy IR Flame Sensor
// RCWL0516 Interference
// SIM900A
// MAX30102
// DSM501A
// Test DS3231 Setting Time and Alarm
/* By Abdullah As-Sadeed */

/*
Board: "Arduino Mega or Mega 2560"
Processor: "ATmega2560 (Mega 2560)"
Programmer: "AVR ISP"
*/

#include <Arduino.h>

// #include <EEPROM.h>

#include <Wire.h>
#include <SPI.h>

#include <MPU9250_WE.h> /* https://github.com/wollewald/MPU9250_WE/ */

#include <BH1750.h> /* https://github.com/claws/BH1750/ */

#include <Seeed_BME280.h> /* https://github.com/Seeed-Studio/Grove_BME280/ */

#include <rdm6300.h> /* https://github.com/arduino12/rdm6300/ */ /* Manual Download */
#include <MFRC522.h>                                             /* https://github.com/miguelbalboa/rfid/ */

#include <TinyGPSPlus.h> /* https://github.com/mikalhart/TinyGPSPlus/ */

#include <RTClib.h> /* https://github.com/adafruit/RTClib/ */

#include <Stepper.h>
#include <Servo.h>

#include <Adafruit_SSD1306.h> /* https://github.com/adafruit/Adafruit_SSD1306/ */

#include <ArduinoJson.h> /* https://github.com/bblanchon/ArduinoJson/ */

#define MPU9250_ADDRESS 0x69
#define BH1750_ADDRESS 0x23

#define DSM501A_PM25 42
#define DSM501A_PM1 43
#define DSM501A_SAMPLE_TIME 5000

#define MQ2 A0
#define MQ3 A1
#define MQ4 A2
#define MQ5 A3
#define MQ6 A4
#define MQ7 A5
#define MQ8 A6
#define MQ9 A7
#define MQ135 A8

#define IR_SENSOR A9

#define HC_SR501_PIN 37

#define RCWL0516_PIN 36

#define RDM6300_UART Serial2

#define RC522_SS 53
#define RC522_RESET 5

#define NEO7M_UART Serial1
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

#define SIM900A Serial3
#define SIM900A_BAUD_RATE 9600

#define ESP32_CAM Serial
#define ESP32_CAM_BAUD_RATE 115200

JsonDocument I2C_Devices_JSON;

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

String ESP32_CAM_IP_Address = "";

struct I2C_Status_Type
{
  bool MPU9250, MPU9250_Magnetometer, BH1750, BME280, DS3231, SSD1306;
};
I2C_Status_Type I2C_Status;

struct MPU9250_Acceleration_Type
{
  float X, Y, Z, Resultant;
};
struct MPU9250_Gyro_Type
{
  float X, Y, Z;
};
struct MPU9250_Magneto_Type
{
  float X, Y, Z;
};
struct MPU9250_Readings_Type
{
  struct MPU9250_Acceleration_Type Acceleration;
  struct MPU9250_Gyro_Type Gyro;
  struct MPU9250_Magneto_Type Magneto;
  float Temperature;
};
MPU9250_Readings_Type MPU9250_Readings;

float Light;

struct BME280_Readings_Type
{
  float Temperature, Humidity, Pressure, Altitude;
};
BME280_Readings_Type BME280_Readings;

struct DSM501A_Readings_Type
{
  float PM25_mgm3, PM25_pcs283ml, PM1_mgm3, PM1_pcs283ml;
};
DSM501A_Readings_Type DSM501A_Readings;

struct MQs_Readings_Type
{
  float MQ2, MQ3, MQ4, MQ5, MQ6, MQ7, MQ8, MQ9, MQ135;
};
MQs_Readings_Type MQs_Readings;

float IR_Radiation;

bool HC_SR501_Motion, RCWL0516_Motion;

struct NEO7M_Readings_Type
{
  unsigned int Satellites;
  float Latitude, Longitude, Speed, Course, Altitude, HDOP;
};
NEO7M_Readings_Type NEO7M_Readings;

unsigned long RDM6300_Reading, RC522_UID;

struct DS3231_Alarm_Type
{
  long Time;
  String Mode;
  bool Is_Fired;
};
struct DS3231_OutPut_Type
{
  long Time;
  float Temperature;
  struct DS3231_Alarm_Type Alarm_1, Alarm_2;
};
DS3231_OutPut_Type DS3231_OutPut;

struct Active_Status_Type
{
  bool I2C_Devices, MPU9250, BH1750, BME280, DSM501A, MQs, IR, HC_SR501, RCWL0516, RDM6300, RC522, NEO7M, DS3231;

  Active_Status_Type() : I2C_Devices(false), MPU9250(false), BH1750(false), BME280(false), DSM501A(false), MQs(false), IR(false), HC_SR501(false), RCWL0516(false), RDM6300(false), RC522(false), NEO7M(false), DS3231(false) {}
};
Active_Status_Type Active_Status = Active_Status_Type();

void (*ReBoot_Arduino_Mega_2560)(void) = nullptr;

void Scan_I2C(void)
{
  if (Active_Status.I2C_Devices)
  {
    byte Error, Address;
    unsigned int Count = 0;

    JsonArray Devices = I2C_Devices_JSON.createNestedArray("I2C_Devices");

    for (Address = 0x01; Address < 0x7f; Address++)
    {
      Wire.beginTransmission(Address);
      Error = Wire.endTransmission();

      if (Error == 0)
      {
        Devices.add(String("0x") + String(Address, HEX));
        Count++;
      }
      else if (Error != 2)
      {
        JsonObject Error_Object = I2C_Devices_JSON.createNestedObject("Error_At_" + String(Address, HEX));
        Error_Object["Code"] = Error;
        Error_Object["Address"] = String("0x") + String(Address, HEX);
      }
    }
  }
}

void SetUp_MPU9250(void)
{
  I2C_Status.MPU9250 = MPU9250.init();
  I2C_Status.MPU9250_Magnetometer = MPU9250.initMagnetometer();

  if (I2C_Status.MPU9250 && I2C_Status.MPU9250_Magnetometer)
  {
    MPU9250.autoOffsets();

    MPU9250.enableAccDLPF(true);
    MPU9250.setAccDLPF(MPU9250_DLPF_6); /* Lowest Noise */

    MPU9250.enableGyrDLPF();
    MPU9250.setGyrDLPF(MPU9250_DLPF_6); /* Lowest Noise */

    MPU9250.setAccRange(MPU6500_ACC_RANGE_16G); /* Highest */

    MPU9250.setGyrRange(MPU9250_GYRO_RANGE_2000); /* Highest */

    MPU9250.setMagOpMode(AK8963_CONT_MODE_100HZ); /* Highest */

    MPU9250.setSampleRateDivider(0); /* Lowest */
  }
}

void SetUp_DSM501A(void)
{
  pinMode(DSM501A_PM25, INPUT);
  pinMode(DSM501A_PM1, INPUT);
}

void SetUp_RDM6300(void)
{
  RDM6300_UART.begin(RDM6300_BAUDRATE);
  RDM6300.begin(&RDM6300_UART);
}

void SetUp_RC522(void)
{
  RC522.PCD_Init();

  for (byte Iteration = 0; Iteration < 6; Iteration++)
  {
    RC522_Key.keyByte[Iteration] = 0xFF;
  }
}

void On_Alarm(void)
{
  if (DS3231.alarmFired(1))
  {
    ESP32_CAM.println("Alarm 1");
  }
  if (DS3231.alarmFired(2))
  {
    ESP32_CAM.println("Alarm 2");
  }
}

void SetUp_DS3231(void)
{
  I2C_Status.DS3231 = DS3231.begin();

  if (I2C_Status.DS3231)
  {
    DS3231.disable32K();
    DS3231.writeSqwPinMode(DS3231_OFF);

    if (DS3231.lostPower())
    {
      DS3231.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    pinMode(DS3231_INTERRUPT, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(DS3231_INTERRUPT), On_Alarm, FALLING);
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

void SetUp_SSD1306(void)
{
  I2C_Status.SSD1306 = SSD1306.begin(SSD1306_SWITCHCAPVCC, SSD1306_ADDRESS);

  if (I2C_Status.SSD1306)
  {
    SSD1306.clearDisplay();

    SSD1306.setTextSize(1);
    SSD1306.setTextColor(SSD1306_WHITE);
    SSD1306.setCursor(25, 0);
    SSD1306.println(F("Bitscoper IoT"));
    SSD1306.display();
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

void SetUp_Buzzer(void)
{
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);
}

void SetUp_Relays()
{
  pinMode(RELAY_1, OUTPUT);
  digitalWrite(RELAY_1, HIGH); /* Off */

  pinMode(RELAY_2, OUTPUT);
  digitalWrite(RELAY_2, HIGH); /* Off */
}

void SetUp_SIM900A(void)
{
  SIM900A.begin(SIM900A_BAUD_RATE);

  SIM900A.print("ATE0\r\n"); /* Disable Command Echo */

  delay(1000);

  SIM900A.print("AT+CMEE=2\r\n"); /* Verbose Error */
}

void setup(void)
{
  ESP32_CAM.begin(ESP32_CAM_BAUD_RATE);

  Wire.begin();

  SPI.begin();

  SetUp_MPU9250();

  I2C_Status.BH1750 = bh1750.begin(BH1750::CONTINUOUS_HIGH_RES_MODE_2); /* Highest */

  I2C_Status.BME280 = bme280.init();

  SetUp_DSM501A();

  pinMode(RCWL0516_PIN, INPUT);

  SetUp_RDM6300();

  SetUp_RC522();

  NEO7M_UART.begin(NEO7M_BAUD_RATE);

  SetUp_DS3231();

  SetUp_SG90();

  ULN2003.setSpeed(ULN2003_SPEED);

  SetUp_SSD1306();

  SetUp_BuiltIn_LED();

  SetUp_RGB_LED();

  SetUp_Buzzer();

  SetUp_Relays();

  SetUp_SIM900A();
}

void Read_MPU9250(void)
{
  if (I2C_Status.MPU9250 && I2C_Status.MPU9250_Magnetometer && Active_Status.MPU9250)
  {

    xyzFloat Acceleration = MPU9250.getGValues();
    xyzFloat Gyro = MPU9250.getGyrValues();
    xyzFloat Magneto = MPU9250.getMagValues();

    MPU9250_Readings.Acceleration.X = Acceleration.x;
    MPU9250_Readings.Acceleration.Y = Acceleration.y;
    MPU9250_Readings.Acceleration.Z = Acceleration.z;
    MPU9250_Readings.Acceleration.Resultant =
        MPU9250.getResultantG(Acceleration);

    MPU9250_Readings.Gyro.X = Gyro.x;
    MPU9250_Readings.Gyro.Y = Gyro.y;
    MPU9250_Readings.Gyro.Z = Gyro.z;

    MPU9250_Readings.Magneto.X = Magneto.x;
    MPU9250_Readings.Magneto.Y = Magneto.y;
    MPU9250_Readings.Magneto.Z = Magneto.z;

    MPU9250_Readings.Temperature = MPU9250.getTemperature();
  }
}

void Read_BH1750(void)
{
  if (I2C_Status.BH1750 && Active_Status.BH1750)
  {
    if (bh1750.measurementReady())
    {
      Light = bh1750.readLightLevel();
    }
  }
}

void Read_BME280(void)
{
  if (I2C_Status.BME280 && Active_Status.BME280)
  {
    BME280_Readings.Temperature = bme280.getTemperature();
    BME280_Readings.Humidity = bme280.getHumidity();
    BME280_Readings.Pressure = bme280.getPressure();
    BME280_Readings.Altitude = bme280.calcAltitude(BME280_Readings.Pressure);
  }
}

void Read_DSM501A(void)
{
  if (Active_Status.DSM501A)
  {
    float PM25_Low = 0.0, PM1_Low = 0.0;

    unsigned long Start_Time = millis();
    while (millis() - Start_Time < DSM501A_SAMPLE_TIME)
    {
      PM25_Low += pulseIn(DSM501A_PM25, LOW) / 1000.0;
      PM1_Low += pulseIn(DSM501A_PM1, LOW) / 1000.0;
    }

    float PM25_Low_Ratio = PM25_Low / DSM501A_SAMPLE_TIME * 100;
    DSM501A_Readings.PM25_mgm3 = max(0, 0.00258425 * pow(PM25_Low_Ratio, 2) + 0.0858521 * PM25_Low_Ratio - 0.01345549);
    DSM501A_Readings.PM25_pcs283ml = min(625 * PM25_Low_Ratio, 12500);

    float PM1_Low_Ratio = PM1_Low / DSM501A_SAMPLE_TIME * 100;
    DSM501A_Readings.PM1_mgm3 = max(0, 0.00258425 * pow(PM1_Low_Ratio, 2) + 0.0858521 * PM1_Low_Ratio - 0.01345549);
    DSM501A_Readings.PM1_pcs283ml = min(625 * PM1_Low_Ratio, 12500);
  }
}

void Read_MQ_Sensors(void)
{
  if (Active_Status.MQs)
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
}

void Read_NEO7M(void)
{
  if (Active_Status.NEO7M)
  {
    while (NEO7M_UART.available())
    {
      GPS.encode(NEO7M_UART.read());
    }

    NEO7M_Readings.Satellites = GPS.satellites.value();
    NEO7M_Readings.Latitude = GPS.location.lat();
    NEO7M_Readings.Longitude = GPS.location.lng();
    NEO7M_Readings.Speed = GPS.speed.kmph();
    NEO7M_Readings.Course = GPS.course.deg();
    NEO7M_Readings.Altitude = GPS.altitude.meters();
    NEO7M_Readings.HDOP = GPS.hdop.value();
  }
}

void Read_RC522(void)
{
  if (Active_Status.RC522)
  {
    RC522.PICC_IsNewCardPresent(); /* Needed to read */

    if (RC522.PICC_ReadCardSerial())
    {
      MFRC522::PICC_Type PICC_Type = RC522.PICC_GetType(RC522.uid.sak);

      if (PICC_Type == MFRC522::PICC_TYPE_MIFARE_MINI && PICC_Type == MFRC522::PICC_TYPE_MIFARE_1K && PICC_Type == MFRC522::PICC_TYPE_MIFARE_4K)
      {
        RC522_UID = (static_cast<unsigned long>(RC522.uid.uidByte[0]) << 24) | (static_cast<unsigned long>(RC522.uid.uidByte[1]) << 16) | (static_cast<unsigned long>(RC522.uid.uidByte[2]) << 8) | static_cast<unsigned long>(RC522.uid.uidByte[3]);

        RC522.PICC_HaltA();
        RC522.PCD_StopCrypto1();
      }
    }
  }
}

void Read_DS3231(void)
{
  if (Active_Status.DS3231)
  {
    if (I2C_Status.DS3231)
    {
      DS3231_OutPut.Time = DS3231.now().unixtime();

      DS3231_OutPut.Temperature = DS3231.getTemperature();

      DS3231_OutPut.Alarm_1.Time = DS3231.getAlarm1().unixtime();
      Ds3231Alarm1Mode Alarm_1_Mode = DS3231.getAlarm1Mode();
      if (Alarm_1_Mode == DS3231_A1_PerSecond)
      {
        DS3231_OutPut.Alarm_1.Mode = "Per Second";
      }
      else if (Alarm_1_Mode == DS3231_A1_Second)
      {
        DS3231_OutPut.Alarm_1.Mode = "Second";
      }
      else if (Alarm_1_Mode == DS3231_A1_Minute)
      {
        DS3231_OutPut.Alarm_1.Mode = "Minute";
      }
      else if (Alarm_1_Mode == DS3231_A1_Hour)
      {
        DS3231_OutPut.Alarm_1.Mode = "Hour";
      }
      else if (Alarm_1_Mode == DS3231_A1_Date)
      {
        DS3231_OutPut.Alarm_1.Mode = "Date";
      }
      else if (Alarm_1_Mode == DS3231_A1_Day)
      {
        DS3231_OutPut.Alarm_1.Mode = "Day";
      }
      DS3231_OutPut.Alarm_1.Is_Fired = DS3231.alarmFired(1);

      DS3231_OutPut.Alarm_2.Time = DS3231.getAlarm2().unixtime();
      Ds3231Alarm2Mode Alarm_2_Mode = DS3231.getAlarm2Mode();
      if (Alarm_2_Mode == DS3231_A2_Minute)
      {
        DS3231_OutPut.Alarm_2.Mode = "Minute";
      }
      else if (Alarm_2_Mode == DS3231_A2_Hour)
      {
        DS3231_OutPut.Alarm_2.Mode = "Hour";
      }
      else if (Alarm_2_Mode == DS3231_A2_Date)
      {
        DS3231_OutPut.Alarm_2.Mode = "Date";
      }
      else if (Alarm_2_Mode == DS3231_A2_Day)
      {
        DS3231_OutPut.Alarm_2.Mode = "Day";
      }
      DS3231_OutPut.Alarm_2.Is_Fired = DS3231.alarmFired(2);
    }
  }
}

void Get_Readings(void)
{
  digitalWrite(LED_BUILTIN, HIGH);

  Scan_I2C();

  Read_MPU9250();

  Read_BH1750();

  Read_BME280();

  Read_DSM501A();

  Read_MQ_Sensors();

  if (Active_Status.IR)
  {
    IR_Radiation = analogRead(IR_SENSOR);
  }

  if (Active_Status.HC_SR501)
  {
    HC_SR501_Motion = digitalRead(HC_SR501_PIN);
  }

  if (Active_Status.RCWL0516)
  {
    RCWL0516_Motion = digitalRead(RCWL0516_PIN);
  }

  Read_NEO7M();

  if (Active_Status.RDM6300)
  {
    RDM6300_Reading = RDM6300.get_tag_id();
  }

  Read_RC522();

  Read_DS3231();

  digitalWrite(LED_BUILTIN, LOW);
}

void Send_JSON(void)
{
  JsonDocument Readings_JSON;

  Readings_JSON["Arduino_Mega_2560_UpTime"] = millis();

  if (Active_Status.I2C_Devices)
  {
    for (JsonPair Key_Value : I2C_Devices_JSON.as<JsonObject>())
    {
      Readings_JSON[Key_Value.key()] = Key_Value.value();
    }
  }

  if (I2C_Status.MPU9250 && I2C_Status.MPU9250_Magnetometer && Active_Status.MPU9250)
  {
    JsonDocument MPU9250_Acceleration_JSON;
    MPU9250_Acceleration_JSON["X"] = MPU9250_Readings.Acceleration.X;
    MPU9250_Acceleration_JSON["Y"] = MPU9250_Readings.Acceleration.Y;
    MPU9250_Acceleration_JSON["Z"] = MPU9250_Readings.Acceleration.Z;
    MPU9250_Acceleration_JSON["Resultant"] =
        MPU9250_Readings.Acceleration.Resultant;

    JsonDocument MPU9250_Gyro_JSON;
    MPU9250_Gyro_JSON["X"] = MPU9250_Readings.Gyro.X;
    MPU9250_Gyro_JSON["Y"] = MPU9250_Readings.Gyro.Y;
    MPU9250_Gyro_JSON["Z"] = MPU9250_Readings.Gyro.Z;

    JsonDocument MPU9250_Magneto_JSON;
    MPU9250_Magneto_JSON["X"] = MPU9250_Readings.Magneto.X;
    MPU9250_Magneto_JSON["Y"] = MPU9250_Readings.Magneto.Y;
    MPU9250_Magneto_JSON["Z"] = MPU9250_Readings.Magneto.Z;

    JsonDocument MPU9250_JSON;
    MPU9250_JSON["Acceleration"] = MPU9250_Acceleration_JSON;
    MPU9250_JSON["Gyro"] = MPU9250_Gyro_JSON;
    MPU9250_JSON["Magneto"] = MPU9250_Magneto_JSON;
    MPU9250_JSON["Temperature"] = MPU9250_Readings.Temperature;

    Readings_JSON["MPU9250"] = MPU9250_JSON;
  }

  if (I2C_Status.BH1750 && Active_Status.BH1750)
  {
    Readings_JSON["BH1750"] = Light;
  }

  if (I2C_Status.BME280 && Active_Status.BME280)
  {
    JsonDocument BME280_JSON;
    BME280_JSON["Temperature"] = BME280_Readings.Temperature;
    BME280_JSON["Humidity"] = BME280_Readings.Humidity;
    BME280_JSON["Pressure"] = BME280_Readings.Pressure;
    BME280_JSON["Altitude"] = BME280_Readings.Altitude;
    Readings_JSON["BME280"] = BME280_JSON;
  }

  if (Active_Status.DSM501A)
  {
    JsonDocument DSM501A_JSON;
    DSM501A_JSON["PM25_mgm3"] = DSM501A_Readings.PM25_mgm3;
    DSM501A_JSON["PM25_pcs283ml"] = DSM501A_Readings.PM25_pcs283ml;
    DSM501A_JSON["PM1_mgm3"] = DSM501A_Readings.PM1_mgm3;
    DSM501A_JSON["PM1_pcs283ml"] = DSM501A_Readings.PM1_pcs283ml;
    Readings_JSON["DSM501A"] = DSM501A_JSON;
  }

  if (Active_Status.MQs)
  {
    Readings_JSON["MQ2"] = MQs_Readings.MQ2;
    Readings_JSON["MQ3"] = MQs_Readings.MQ3;
    Readings_JSON["MQ4"] = MQs_Readings.MQ4;
    Readings_JSON["MQ5"] = MQs_Readings.MQ5;
    Readings_JSON["MQ6"] = MQs_Readings.MQ6;
    Readings_JSON["MQ7"] = MQs_Readings.MQ7;
    Readings_JSON["MQ8"] = MQs_Readings.MQ8;
    Readings_JSON["MQ9"] = MQs_Readings.MQ9;
    Readings_JSON["MQ135"] = MQs_Readings.MQ135;
  }

  if (Active_Status.IR)
  {
    Readings_JSON["IR_Radiation"] = IR_Radiation;
  }

  if (Active_Status.HC_SR501)
  {
    Readings_JSON["HC_SR501"] = HC_SR501_Motion;
  }

  if (Active_Status.RCWL0516)
  {
    Readings_JSON["RCWL0516"] = RCWL0516_Motion;
  }

  if (Active_Status.RDM6300)
  {
    Readings_JSON["RDM6300"] = RDM6300_Reading;
  }

  if (Active_Status.RC522)
  {
    Readings_JSON["RC522_UID"] = RC522_UID;
  }

  if (Active_Status.NEO7M)
  {
    JsonDocument NEO7M_JSON;
    NEO7M_JSON["Satellites"] = NEO7M_Readings.Satellites;
    NEO7M_JSON["Latitude"] = NEO7M_Readings.Latitude;
    NEO7M_JSON["Longitude"] = NEO7M_Readings.Longitude;
    NEO7M_JSON["Speed"] = NEO7M_Readings.Speed;
    NEO7M_JSON["Course"] = NEO7M_Readings.Course;
    NEO7M_JSON["Altitude"] = NEO7M_Readings.Altitude;
    NEO7M_JSON["HDOP"] = NEO7M_Readings.HDOP;
    Readings_JSON["NEO7M"] = NEO7M_JSON;
  }

  if (I2C_Status.DS3231 && Active_Status.DS3231)
  {
    JsonDocument DS3231_Alarm_1_JSON;
    DS3231_Alarm_1_JSON["Time"] = DS3231_OutPut.Alarm_1.Time;
    DS3231_Alarm_1_JSON["Mode"] = DS3231_OutPut.Alarm_1.Mode;
    DS3231_Alarm_1_JSON["Is_Fired"] = DS3231_OutPut.Alarm_1.Is_Fired;

    JsonDocument DS3231_Alarm_2_JSON;
    DS3231_Alarm_2_JSON["Time"] = DS3231_OutPut.Alarm_2.Time;
    DS3231_Alarm_2_JSON["Mode"] = DS3231_OutPut.Alarm_2.Mode;
    DS3231_Alarm_2_JSON["Is_Fired"] = DS3231_OutPut.Alarm_2.Is_Fired;

    JsonDocument DS3231_JSON;
    DS3231_JSON["Time"] = DS3231_OutPut.Time;
    DS3231_JSON["Alarm_1"] = DS3231_Alarm_1_JSON;
    DS3231_JSON["Alarm_2"] = DS3231_Alarm_2_JSON;
    DS3231_JSON["Temperature"] = DS3231_OutPut.Temperature;
    Readings_JSON["DS3231"] = DS3231_JSON;
  }

  ESP32_CAM.println();
  serializeJson(Readings_JSON, ESP32_CAM);
  ESP32_CAM.flush();
}

void Receive_JSON(void)
{
  while (ESP32_CAM.available())
  {
    JsonDocument ESP32_CAM_JSON;

    DeserializationError ESP32_CAM_JSON_Error = deserializeJson(ESP32_CAM_JSON, ESP32_CAM);

    if (ESP32_CAM_JSON_Error == DeserializationError::Ok)
    {
      digitalWrite(LED_BUILTIN, HIGH);

      if (ESP32_CAM_JSON.containsKey("Active_Status"))
      {
        Active_Status.I2C_Devices = ESP32_CAM_JSON["Active_Status"]["I2C_Devices"].as<bool>();
        Active_Status.MPU9250 = ESP32_CAM_JSON["Active_Status"]["MPU9250"].as<bool>();
        Active_Status.BH1750 = ESP32_CAM_JSON["Active_Status"]["BH1750"].as<bool>();
        Active_Status.BME280 = ESP32_CAM_JSON["Active_Status"]["BME280"].as<bool>();
        Active_Status.DSM501A = ESP32_CAM_JSON["Active_Status"]["DSM501A"].as<bool>();
        Active_Status.MQs = ESP32_CAM_JSON["Active_Status"]["MQs"].as<bool>();
        Active_Status.IR = ESP32_CAM_JSON["Active_Status"]["IR"].as<bool>();
        Active_Status.HC_SR501 = ESP32_CAM_JSON["Active_Status"]["HC_SR501"].as<bool>();
        Active_Status.RCWL0516 = ESP32_CAM_JSON["Active_Status"]["RCWL0516"].as<bool>();
        Active_Status.RDM6300 = ESP32_CAM_JSON["Active_Status"]["RDM6300"].as<bool>();
        Active_Status.RC522 = ESP32_CAM_JSON["Active_Status"]["RC522"].as<bool>();
        Active_Status.NEO7M = ESP32_CAM_JSON["Active_Status"]["NEO7M"].as<bool>();
        Active_Status.DS3231 = ESP32_CAM_JSON["Active_Status"]["DS3231"].as<bool>();
      }

      if (ESP32_CAM_JSON.containsKey("ReBoot_Arduino_Mega_2560"))
      {
        if (ESP32_CAM_JSON["ReBoot_Arduino_Mega_2560"].as<bool>())
        {
          ReBoot_Arduino_Mega_2560();
        };
      }

      if (ESP32_CAM_JSON.containsKey("Set_DS3231_Time"))
      {
        DateTime Time = DateTime(ESP32_CAM_JSON["Set_DS3231_Time"].as<long>());

        DS3231.adjust(Time);
      }

      if (ESP32_CAM_JSON.containsKey("Set_DS3231_Alarm"))
      {
        unsigned int Number = ESP32_CAM_JSON["Set_DS3231_Alarm"]["Number"].as<unsigned int>();
        DateTime Time = DateTime(ESP32_CAM_JSON["Set_DS3231_Alarm"]["Time"].as<long>());
        String Mode = ESP32_CAM_JSON["Set_DS3231_Alarm"]["Mode"];

        if (Number == 1)
        {
          DS3231.clearAlarm(1);

          if (Mode == "Per Second")
          {
            DS3231.setAlarm1(Time, DS3231_A1_PerSecond);
          }
          else if (Mode == "Second")
          {
            DS3231.setAlarm1(Time, DS3231_A1_Second);
          }
          else if (Mode == "Minute")
          {
            DS3231.setAlarm1(Time, DS3231_A1_Minute);
          }
          else if (Mode == "Hour")
          {
            DS3231.setAlarm1(Time, DS3231_A1_Hour);
          }
          else if (Mode == "Date")
          {
            DS3231.setAlarm1(Time, DS3231_A1_Date);
          }
          else if (Mode == "Day")
          {
            DS3231.setAlarm1(Time, DS3231_A1_Day);
          }
        }
        else if (Number == 2)
        {
          DS3231.clearAlarm(2);

          if (Mode == "Minute")
          {
            DS3231.setAlarm2(Time, DS3231_A2_Minute);
          }
          else if (Mode == "Hour")
          {
            DS3231.setAlarm2(Time, DS3231_A2_Hour);
          }
          else if (Mode == "Date")
          {
            DS3231.setAlarm2(Time, DS3231_A2_Date);
          }
          else if (Mode == "Day")
          {
            DS3231.setAlarm2(Time, DS3231_A2_Day);
          }
        }
      }

      if (ESP32_CAM_JSON.containsKey("Clear_DS3231_Alarm"))
      {
        DS3231.clearAlarm(ESP32_CAM_JSON["Clear_DS3231_Alarm"].as<unsigned int>());
      }

      if (ESP32_CAM_JSON.containsKey("SG90_State"))
      {
        bool SG90_State = ESP32_CAM_JSON["SG90_State"].as<bool>();

        if (SG90_State)
        {
          Attach_SG90();
        }
        else if (!SG90_State)
        {
          SG90.detach();
        }
      }

      if (ESP32_CAM_JSON.containsKey("SG90_Position"))
      {
        unsigned int SG90_Position = ESP32_CAM_JSON["SG90_Position"].as<unsigned int>();

        SG90.write(SG90_Position);
      }

      if (ESP32_CAM_JSON.containsKey("ULN2003_Steps"))
      {
        signed int ULN2003_Steps = ESP32_CAM_JSON["ULN2003_Steps"].as<signed int>();

        ULN2003.step(ULN2003_Steps);
      }

      if (ESP32_CAM_JSON.containsKey("Buzzer"))
      {
        bool Buzzer_State = ESP32_CAM_JSON["Buzzer"].as<bool>();

        if (Buzzer_State)
        {
          digitalWrite(BUZZER, HIGH);
        }
        else if (!Buzzer_State)
        {
          digitalWrite(BUZZER, LOW);
        }
      }

      if (ESP32_CAM_JSON.containsKey("Relay"))
      {
        unsigned int Relay_Number = ESP32_CAM_JSON["Relay"]["Number"].as<unsigned int>(), Relay;
        bool Relay_State = ESP32_CAM_JSON["Relay"]["State"].as<bool>();

        if (Relay_Number == 1)
        {
          Relay = RELAY_1;
        }
        else if (Relay_Number == 2)
        {
          Relay = RELAY_2;
        }

        if (Relay_State)
        {
          digitalWrite(Relay, LOW); /* On */
        }
        else if (!Relay_State)
        {
          digitalWrite(Relay, HIGH); /* Off */
        }
      }

      if (ESP32_CAM_JSON.containsKey("SIM900A_AT"))
      {
        String SIM900A_AT = ESP32_CAM_JSON["SIM900A_AT"].as<String>();

        SIM900A.print(SIM900A_AT);
        SIM900A.print("\r\n");

        while (!SIM900A.available())
        {
        };

        while (SIM900A.available())
        {
          char Character = SIM900A.read();

          ESP32_CAM.print(Character);
        }
      }

      if (ESP32_CAM_JSON.containsKey("IP_Address"))
      {
        ESP32_CAM_IP_Address = ESP32_CAM_JSON["IP_Address"].as<String>();

        if (I2C_Status.SSD1306)
        {
          SSD1306.setTextSize(1);
          SSD1306.setTextColor(SSD1306_WHITE);
          SSD1306.setCursor(30, 10);
          SSD1306.println(ESP32_CAM_IP_Address);
          SSD1306.display();
        }
      }

      digitalWrite(LED_BUILTIN, LOW);
    }
  }
}

void loop(void)
{
  Get_Readings();

  Send_JSON();

  Receive_JSON();
}

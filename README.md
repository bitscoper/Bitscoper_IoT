<div align="center">

# Bitscoper IoT

Bitscoper Internet of Things

[![Build PlatformIO Project](https://github.com/bitscoper/Bitscoper_IoT/actions/workflows/Build%20PlatformIO%20Project.yaml/badge.svg)](https://github.com/bitscoper/Bitscoper_IoT/actions/workflows/Build%20PlatformIO%20Project.yaml)

![Fritzing Sketch](Fritzing/Sketch.png)

</div>

## Commands

### Arduino Mega 2560

#### Build

`platformio run --environment Arduino_Mega_2560`

#### Upload

`platformio run --target upload --environment Arduino_Mega_2560`

#### Open Serial Monitor

`platformio device monitor --environment Arduino_Mega_2560`

### ESP32_CAM

#### Build

`platformio run --environment ESP32_CAM`

#### Upload

`platformio run --target upload --environment ESP32_CAM`

#### Build File System Image

`platformio run --target buildfs --environment ESP32_CAM`

#### Upload File System Image

`platformio run --target uploadfs --environment ESP32_CAM`

#### Open Serial Monitor

`platformio device monitor --environment ESP32_CAM`

### General

#### Build All

`platformio run`

#### List Available Devices

`platformio device list`

#### Clean All

`platformio run --target clean`

#### Full Clean All

`platformio run --target fullclean`

## [To Do](ToDo.md)

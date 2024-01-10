# Errors

This document contains an explanation of the error codes that can be encountered while using DTTS as well as solutions to these errors.

## Quick reference
| Error Code    | Error Message |
|---------------|---------------|
| [1](#error-1-oled-init-failed)         | OLED init failed. |
| [2](#error-2-database-open-failed)         | Database open failed. |
| [3](#error-3-database-creation-failed)         | Database creation failed. |

## All error codes explained in detail
### Error 1: OLED init failed.
This error describes an error with the connected OLED screen. Logically, this error should only show up on the debug serial.
Possible error causes are:
* broken OLED screen
* broken OLED connection
* misconfigured OLED pins

Possible fixes:
* checking the OLED pins in the `definitions.h` file
* checking the connection from the OLED to the ESP32
* replacing the OLED screen

### Error 2: Database open failed.
This error describes an error while opening the database file. This error can be caused by:
* missing database file
* misconfigured database file path
* invalid database file prefix
* broken or missing SD card and/or connection
* invalid file system on the SD card

Possible fixes:
* invoking the `createDatabase()` function to create a new database file before opening the database
* checking the database file path in the `definitions.h` file
* checking if there is a `/sd/` before the actual database file path in the `definitions.h` file
* Checking the connection from the SD card to the ESP32
* formatting the SD card to FAT32 without any partitioning

### Error 3: Database creation failed.
This error describes an error while creating the database file. This error can be caused by:
* missing SD card
* write protection on the SD card
* invalid database file prefix
* broken or missing SD card and/or connection
* wrong file system on the SD card

Possible fixes:
* checking if there is a `/sd/` before the actual database file path in the `definitions.h` file
* Checking the connection from the SD card to the ESP32
* removing the write protection from the File system on the SD card
* formatting the SD card to FAT32 without any partitioning

### Error 4: SD card failed.
This error describes an error while initializing the SD card. This error can be caused by:
* missing SD card
* broken or missing SD card and/or connection
* wrong file system on the SD card

Possible fixes:
* Checking the connection from the SD card to the ESP32
* formatting the SD card to FAT32 without any partitioning
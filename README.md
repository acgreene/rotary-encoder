### Installation

#### ESP32 SDK Installation

- Read Espressif's ESP32 [Get Started](https://docs.espressif.com/projects/esp-idf/en/v4.3.1/esp32c3/get-started/index.html#) 
documentation and setup the SDK for the ESP32 hardware. This consists of the toolchain, build tools, and API to compile 
and build for ESP32.

#### Steps

1. Add the following cmake toolchain environment variables
```shell
ESPBAUD=115200
ESPPORT=/dev/ttyUSB0 # esp32 port
IDF_PATH=/home/alecgreene/documents/esp/esp-idf # path to esp-idf
```

2. Run the export.sh script by ESP-IDF, which adds the ESP-IDF tools 
to the PATH environment variable and makes them usable from the command line.
```shell
. $HOME/documents/esp/esp-idf/export.sh # adjust for your path
```

Note: I set up an alias towards this script, since I run it a lot. I pasted 
the following command to my .bashrc
```shell
alias get_esp_idf='. $HOME/documents/esp/esp-idf/export.sh'
```

### Build and Flash to ESP32
Ensure the installation steps above have been performed, then execute
the following commands:
1. Build the project by running: 
```shell
idf.py build
```

2. Flash the project. This also automatically builds and flashes the
project, so running the build step isn't necessary.
```shell
idf.py -p [PORT] -b [BAUD] flash
```

### Monitor 
Monitor the terminal output 
```shell
idf.py -p PORT monitor
```

Combine building, flashing and monitoring into one step by running:
```shell
idf.py -p PORT flash monitor
```

### ESP32 Troubleshooting

- Check serial devices connected
```shell
ls -l /dev/ttyUSB* 
```

- Check if nay program is using the ESP32 serial port
```shell
sudo lsof /dev/ttyUSB0
```

- If anything is using it, kill those processes
```shell
sudo fuser -k /dev/ttyUSB0
```
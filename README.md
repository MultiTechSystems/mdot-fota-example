# MDot FOTA Example

This is an example FOTA enabled application for MDot using latest LoRaWAN application package specifications.

__LoRaWAN Application Packages__
* Fragmented Data Block Transfer - v2
* Remote Multicast Setup - v2
* Clock Synchronization - v2
* Firmware Management - v1
* Multi-Package Access - v1 

## Setup

Install [Mbed CLI](https://os.mbed.com/docs/mbed-os/v6.14/build-tools/install-and-set-up.html)

Clone the repository and deploy mbed libraries.

```
git clone 
cd mdot-fota-example
mbed deploy
```

Create `version.h` with `APPLICATION_VERSION` string and `APPLICATION_VERSION_CODE` array.

```C
#ifndef __VERSION_H__ 
#define __VERSION_H__ 
#define APPLICATION_VERSION "4.1.99.1" 
const int8_t APPLICATION_VERSION_CODE[4] = {4,1,99,1}; 
#endif
```

## Configure

Edit `config.h` to set network parameters.

## Compile

```
mbed compile -m MTS_MDOT_F411RE -t GCC_ARM
```

## Sign
```
mkdir bin

copy BUILD/MTS_MDOT_F411RE/GCC_ARM/mdot_fota_example.bin bin/mdot_fota_example_4.1.99.1.bin
copy BUILD/MTS_MDOT_F411RE/GCC_ARM/mdot_fota_example_application.bin bin/mdot_fota_example_application_4.1.99.1.bin

multitool device pl -m -v 4.1.99.1 -i MDOT -s example_key.prv -c -o bin\mdot_fota_example_application_4.1.99.1_signed.bin bin\mdot_fota_example_application_4.1.99.1.bin

multitool device co -m -v 4.1.99.1 -i MDOT -s example_key.prv -c -o bin\mdot_fota_example_application_4.1.99.1_signed.lz4 bin\mdot_fota_example_application_4.1.99.1.bin
```

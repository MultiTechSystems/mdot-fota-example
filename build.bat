@echo off

set version=4.1.99.1

set proj_name=mdot-fota-example

set target=MTS_MDOT_F411RE
set toolchain=GCC_ARM
set proj_dir=%CD%
set build_dir=%proj_dir%\BUILD\%target%\%toolchain%
set output_dir=bin
set prv_key=%proj_dir%/example_key.prv

if not exist %output_dir% mkdir %output_dir%

set initial_dir=%CD%

call :Compile

multitool device pl -m -v %version% -i MDOT -s %prv_key% -c -o %output_dir%\%proj_name%_application_%version%_signed.bin %output_dir%\%proj_name%_application_%version%.bin
multitool device co -m -v %version% -i MDOT -s %prv_key% -c -o %output_dir%\%proj_name%_application_%version%_signed.lz4 %output_dir%\%proj_name%_application_%version%.bin

cd %initial_dir%
EXIT /B %ERRORLEVEL%

:Compile
set ver_array=%version:.=,%
echo #ifndef __VERSION_H__ > %proj_dir%\version.h
echo #define __VERSION_H__ >> %proj_dir%\version.h
echo #define APPLICATION_VERSION "%version%" >> %proj_dir%\version.h
echo const int8_t APPLICATION_VERSION_CODE[4] = {%ver_array%}; >> %proj_dir%\version.h
echo #endif >> %proj_dir%\version.h

cd %proj_dir%
mbed compile -m %target% -t %toolchain%

copy %build_dir%\%proj_name%.bin %output_dir%\%proj_name%_%version%.bin
copy %build_dir%\%proj_name%_application.bin %output_dir%\%proj_name%_application_%version%.bin
EXIT /B 0
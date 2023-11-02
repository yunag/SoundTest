@echo off
:: Script for building the project
:: Using MinGW compiler

set BATCH_DIR=%~dp0
set SOURCE_DIR=%BATCH_DIR%/..
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=Debug -G "MinGW Makefiles" ^
-B %SOURCE_DIR%/build/ ^
-S %SOURCE_DIR% && ^
cmake --build %SOURCE_DIR%/build -j

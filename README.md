# Pocketbook Texteditor
Texteditor for Pocketbook using a bluetooth keyboard.

//TODO add screenshots

## Features

## Requirements


## Installation
Download and unzip the file from releases and place the texteditor.app into the "applications" folder of your pocketbook. 

## How to build

First you need to install the basic build tools for linux.

Then you have to download the Pocketbook SDK (https://github.com/pocketbook/SDK_6.3.0/tree/5.19).

In the CMakeLists.txt of this project you have to set the root of the TOOLCHAIN_PATH to the location where you saved the SDK. 
This could be for example:

`SET (TOOLCHAIN_PATH "../../SDK/SDK_6.3.0/SDK-B288")`

Then you have to setup cmake by:

`cmake .`

To build the application run:

`make` 

## Disclamer
Use as your own risk! 
Even though the possibility is low, the application could harm your device or even break it.

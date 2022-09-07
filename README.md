# Pocketbook Texteditor
Texteditor for Pocketbook that connects to an external bluetooth keyboard.

<img src="/screenshots/usageExample.bmp" width="20%" height="20%">
<img src="/screenshots/textEditorNoDevices.bmp" width="20%" height="20%">&nbsp;&nbsp;<img src="/screenshots/textEditorDeviceSelection.bmp" width="20%" height="20%">&nbsp;&nbsp;<img src="/screenshots/textEditorFileSelection.bmp" width="20%" height="20%">&nbsp;&nbsp;<img src="/screenshots/textEditorInput.bmp" width="20%" height="20%">

## Tested on

### Devices
* Pocketbook Touch HD3 (PB632)

### Keyboard
* Logitech K380

## Requirements
* Bluetooth interface at PB
* Console access to PB (via SSH or application PBTerm (https://userpage.physik.fu-berlin.de/~jtt/PB/))
* Root Access is necessary to register new inputs and access key events. (It is necessary to execute "jailbreak.app".) (https://www.mobileread.com/forums/showthread.php?t=325185)

## Installation
Download and unzip the file from releases and place the texteditor.app into the "applications" folder of your pocketbook.

## Usage

Currently the first pairing is not integrated into the application. Via the settings menu of the PB the registration of an keyboard is blocked. Therefore that has to be done once manually.
Enable Bluetooth on the Pocketbook in the Settings menu.
Open the PBTerm application and start bluetoothctl.
There you have to pair your bluetooth keyboard. (https://wiki.archlinux.org/title/bluetooth_keyboard)

Once you have the keyboard paired you can open the texteditor.app and should be able to see your keyboard. Once the keyboard is selected, the input mode is opened. (To exit input mode press "ESC")

### Keymap setup
There are several keymaps available inside kbLayouts. Please select the one acording to your needs, rename it to
"map.keys" and place it inside system/config/textEditor.

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

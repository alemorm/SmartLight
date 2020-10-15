# Smart Light Project

## Smart living room lamp that turns on when the room gets dark

---

### <div align="center">**Project Overview**

![Project Overview][overview]

The ESP32 Microcontroller measures the light in the living room as well as the corresponding time and sends an HTTP POST request to an IFTTT webhook service that then turns a WeMo Smart Plug on/off. The process is overriden from 11:00pm to 8:00am.

## ESP32 Setup

---

There are two ESP32 circuit diagrams that reflect the testing and the final deployment circuits. The difference between the two being the presence of an LED that represents the HTTP request being sent.

### <div align="center">**Test Circuit**

![LDR Test Circuit][testcircuit]

### <div align="center">**Final Circuit**

![LDR Final Circuit][circuit]

### <div align="center">**Actual Final Circuit**

![LDR Real Circuit][realcircuit]

## Code Development

---

- **Host Device** = Windows 10 desktop running [Windows Subsystem for Linux (v1)][wsl] with [Ubuntu 18.04.4 LTS][ubuntu]
- **Embedded Code IDE** = [Arduino IDE (v1.8.13)][arduino]
- **Host Code IDE** = [VSCode (v1.50.0)][vscode]

## Materials

---

- **[ESP32]** (Affordable microcontroller with bluetooth and WiFi networking capabilities)
- **[WeMo Smart Plug][wemoplug]** (Remote-controlled current relay that intefaces with IFTTT)
- **Photoresistor** (Simple and cheap light dependent resistor)
- **Jumper Wires**
- **Breadboard**
- **Power supply** (Standard 5V DC power supply with micro-USB connection for ESP32)
- **Resistors** (Specifically 10kΩ and 100Ω, present in many kits)
- **LED**

## Getting Started

---

To replicate this project, follow these steps:

- `git clone https://github.com/alemorm/SmartLight.git ~/`

- Write the name of your network within the double quotes next to `wifiName` in the file [`auth.keys_template`] and rename it to `auth.keys`

- Write the password of your network within the double quotes next to `wifiKey` in the newly renamed `auth.keys` file

- Create an [IFTTT] account
  
  - Connect a [Webhook] service to your account
    - Click on *Documentation* after you added the webhook and copy the key value within the double quotes next to the `iftttKey` in the `auth.keys` file

  - Connect a [WeMo] smart plug to your account
  
  - Create the following applet:
    - **If This** = Search and select Webhooks
      - Under event name write `lamp_on`
    - **Then That** = Search and select WeMo Smart Plug
      - Select the `Turn on` action

  - Create the second following applet:
    - **If This** = Search and select Webhooks
      - Under event name write `lamp_off`
    - **Then That** = Search and select WeMo Smart Plug
      - Select the `Turn off` action

- Make the utility script [`keyHandler.sh`] executable by running:
  - `chmod +x keyHandler.sh`
  - Run the script by:
    - `keyHandler.sh -b`
    - You should notice the WiFi and IFTTT Key information appear in the `TimeLightClient.ino` source file
  - **IMPORTANT NOTE**: Make sure to run the `keyHandler.sh -b` before 
  
- Download and install the [Arduino IDE][arduino] (can be any PC/Mac/Linux OS version):

- Open the Arduino IDE and follow [this tutorial][esp32tutorial] to add the ESP32
  - This specific project used the NodeMCU-32S variant of the ESP32 but make sure to check which ESP32 variant you are using

- Follow the [circuit schematics][testcircuit] to set up the light-sensing circuit.

- Once the Arduino IDE has been configured to use the ESP32S, open the [`TimeLightClient.ino`] source file on the Arduino IDE and upload the code to the board (making sure the debug_var is set to 1 to see the serial output).
  - Once the code is done compiling and uploading, open the Serial Monitor under **Tools>Serial Monitor** and monitor the light and date/time values.

<!-- Links -->
[overview]: img/project_overview.png
[circuit]: img/circuit_schematic.png
[testcircuit]: img/test_circuit_schematic.png
[realcircuit]: img/real_circuit.jpg
[vstasks]: .vscode/tasks.json
[wsl]: https://docs.microsoft.com/en-us/windows/wsl/install-win10
[ubuntu]: https://www.microsoft.com/en-us/p/ubuntu-1804-lts/9n9tngvndl3q
[arduino]: https://www.arduino.cc/en/Main/software
[vscode]: https://code.visualstudio.com/
[ifttt]: https://ifttt.com/home
[webhook]: https://ifttt.com/maker_webhooks
[wemo]: https://ifttt.com/wemo_switch
[`auth.keys_template`]: auth.keys_template
[`keyHandler.sh`]: keyHandler.sh
[esp32tutorial]: https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/
[`TimeLightClient.ino`]: scripts/TimeLightClient/TimeLightClient.ino
[ESP32]: https://www.espressif.com/en/products/socs/esp32
[wemoplug]: https://www.belkin.com/us/p/P-F7C063/
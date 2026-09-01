# 🚨 **Note:** This is not the final version of TSS 2026 but has most functional capabilities of the final version. More information can be found under [documents/updates/](./documents/updates/). 🚨


# TSS 2026

NASA Spacesuit User Interface Technologies for Students ([SUITS](https://www.nasa.gov/learning-resources/spacesuit-user-interface-technologies-for-students/)) is a design challenge in which college students from across the country help design user interface solutions for future spaceflight needs. The following is a web interface for the SUITS telemetry stream server designed and developed for the challenge.

<img src="frontend/images/suits-introduction.png"/>

## Introduction

TSS (telemetry stream server) is the centralized server for sending and receiving data for the challenge. The following document will detail how you can run your own instance of the server and begin developing your software and hardware.

### Navigation

- <a href="#getting-started">Getting Started</a>
- <a href="#peripheral-devices">Peripheral Devices</a>
- <a href="#development">Development</a>
- <a href="#testing">Testing</a>
- <a href="#jsc-test-week">JSC Test Week</a>
- <a href="#questions">Questions</a>

### Helpful Links

- [Mission Description](https://www.nasa.gov/wp-content/uploads/2025/09/fy26-suits-mission-description.pdf?emrc=345f62?emrc=345f62)
- <a href="/documents/mission_description/mission-description.pdf">Acronym List</a>
- <a href="/documents/maps/">Rock Yard maps</a>
- <a href="/documents/3d-scans/">Rock Yard 3D Scans</a>
- <a href="/documents/telemetry_ranges/eva-telemetry-ranges.pdf">EVA Telemetry Ranges</a>

## Getting Started

1. Clone the repository:
```
git clone https://github.com/SUITS-Techteam/TSS2026.git
```

2. Navigate into the root of the repository on your terminal of choice

3. Allow the build script to be ran as an executable:
```
chmod +x ./build.bat
```

4. To build the server, run:
```
./build.bat
``` 
**NOTE:** TSS only runs on UNIX based operating systems. If running on Windows, you will have to setup [WSL](https://learn.microsoft.com/en-us/windows/wsl/about) and **MUST** [install GCC](https://code.visualstudio.com/docs/cpp/config-wsl#:~:text=From%20the%20WSL%20command%20prompt%2C%20first%20run). If you don't, several errors will be displayed when trying to build the server.

5. To start the server, run:
```
./server.exe
```

You should see the following lines in your terminal appear:

```
Launching Server at IP: 172.20.182.43:14141
Configuring Local Address...
Creating HTTP Socket...
Binding HTTP Socket...
Listening to HTTP Socket...
Creating UDP Socket...
Binding UDP Socket...
Listening to UDP Socket...
Backend and simulation engine initialized successfully
```

6. Type the IP address printed in the first output for `Launching Server at IP: xxx.xx.xxx.xx:14141`. This will open the website for the server. From this website, you can interact with the server. This is where you can monitor the state of the simulation, verify the display of your system, and virtually interact with the EVA devices like you will be using during test week.

![Image of the user interface of the main page of TSS](frontend/images/tss-main-page.png)

## Peripheral Devices

The devices listed below are physical devices that will be used during test week to create a realistic scenario for both the EVA teams. The sensor data listed below will be synced with the telemetry server and can be fetched for use within your interface.

### UIA

The umbilical interface assembly (UIA) is a component used at the beginning of an EVA to transfer power and fluids to the suit.

<img src="documents/peripherals/uia.jpeg" style="height: 400px"/>

| Sensor       | Value True | Value False | Description                        |
| ------------ | ---------- | ----------- | ---------------------------------- |
| EMU1 POWER   | ON         | OFF         | Remotely powers the suit for EVA 1 |
| EV1 SUPPLY   | OPEN       | CLOSED      | Fills EVA 1's liquid coolant       |
| EV1 WASTE    | OPEN       | CLOSED      | Flushes EVA 1's liquid coolant     |
| EV1 OXYGEN   | OPEN       | CLOSED      | Fills EVA 1's oxygen tanks         |
| EMU2 POWER   | ON         | OFF         | Remotely powers the suit for EVA 2 |
| EV2 SUPPLY   | OPEN       | CLOSED      | Fills EVA 2's liquid coolant       |
| EV2 WASTE    | OPEN       | CLOSED      | Flushes EVA 2's liquid coolant     |
| EV2 OXYGEN   | OPEN       | CLOSED      | Fills EVA 2's oxygen tanks         |
| O2 Vent      | OPEN       | CLOSED      | Flushes both EVAs oxygen tanks     |
| DEPRESS PUMP | ON         | OFF         | Pressurizes both EVA suits         |

### DCU

The display and control unit (DCU) used for this challenge is a component that allows the user to control various settings of their suit's operation during an EVA. For example, if scrubber A's CO2 storage fills up, you could flip a switch on the DCU to flush it while switching to scrubber B.

<img src="documents/peripherals/dcu_front_new.jpg" style="height: 200px"/> <img src="documents/peripherals/dcu_top_new.jpg" style="height: 200px"/>

| Sensor  | Value True | Value False     | Description                                                                                           |
| ------- | ---------- | --------------- | ----------------------------------------------------------------------------------------------------- |
| BATTERY | SUIT BATT  | UMBILICAL POWER | Describes if the suit is running off its local battery or UIA power                                   |
| OXYGEN  | PRI TANK   | SEC TANK        | Describes if the suit is pulling from primary or secondary oxygen tanks                               |
| BATTERY   | PRI BATT  | SEC BATT       | Describes if the suit is connected to primary or secondary battery                                 |
| FAN     | PRI FAN    | SEC FAN         | Describes if the suit is using the primary fan or secondary fan                                       |
| PUMP    | OPEN       | CLOSED          | Describes if the coolant pump for the suit is open or closed (allows water to be flushed or supplied) |
| CO2     | Scrubber A | Scrubber B      | Describes which scrubber is currently filling with CO2 (other is venting)                             |

### LTV Task Board
 NASA SUITS aims to have a physical activity box with various panels and mock sensors for the EV to troubleshoot and repair. This physical box is representative of the Lunar Terrain Vehicle (LTV) External Control Panel and will be referred to as the `LTV Task Board`. Each item within the LTV Task Board will have a unique procedure for troubleshooting and repair. These procedures may be found in [documents/procedures/ltv-repair-procedures.pdf](documents/procedures/ltv-repair-procedures.pdf).

 Later on, we intend to release high definitions pictures of the LTV Task Board for teams pursuing computer vision solutions.


### Connecting to the simulator


<img src="frontend/images/dust-onboarding.png" style="height: 400px"/>

After opening the simulator on a Windows PC, a screen prompting you to enter an IP address will show up. This is prompting you to enter the website address for the TSS server, which is used to communicate back and forth with the simulator. Type in the network address without the port number. For example, if your instance of TSS is running on `172.20.182.43:14141`, then you will type in `172.20.182.43`. Note that you will want the server to be running on the same network or computer as the device running DUST so that they can commmunciate with each other.

## Development

The telemetry server is an important part of the challenge as it will serve as the main way to fetch telemetry for the EVA. You'll take this telemetry data and use it within the respective interfaces that you are designing and developing ahead of test week. This section will outline how to connect to the data stream, data formats, and other helpful information for development.

### UDP socket communication

To create a more realistic scenario, we require that you request and send commands over the [user datagram protocol](https://www.cloudflare.com/learning/ddos/glossary/user-datagram-protocol-udp/) (UDP) instead of a HTTPS connection. For fetching data and issuing commands, you will use a specified command number, more details can be found below. Please note that all requests should be formatted in [big endian](https://www.geeksforgeeks.org/dsa/little-and-big-endian-mystery/) format.

The request packet should contain two different integers, the first is a UNIX timestamp, and the second is a command number. If you are requesting to change the value of a field, then you will use an additional 4 bytes to set a new value for that field.

| Timestamp (unit32) | Command number (uint32) | Input Data (float) |
| ------------------ | ----------------------- | ------------------ |
| 4 bytes            | 4 bytes                 | 4 bytes (optional) |

The server will always respond back with a UDP packet to acknowledge a request or change. If you are sending a packet to change a value , then you will receive a 4 byte response where a successful change will be indicated as true `(01000000)` and false as `(00000000)`. If requesting a JSON file (command numbers 0, 1, and 2), then the UDP response will be a variable number of bytes based on the JSON file length. You can convert these bytes back to JSON for use within your interfaces.

| Output Data    |
| -------------- |
| Variable # of bytes |

These are the commands you can send to the server to fetch the telemetry data as JSON files. They directly correspond to the JSON files in `/data` folder in the root directory, and will be listed as such. Please note that the response from the UDP socket will be in byte format and you will need to convert it back to a human readable JSON format for processing.

| Command number | Referenced .json file          |
| -------------- | ------------------------------ |
| 0              | [EVA.json](/data/EVA.json)     |
| 1              | [LTV_ERRORS.json](data/LTV_ERRORS.json) |

<mark>NOTE on the LTV_ERRORS.json file: you will not be able to access any error codes nor procedures besides the Recovery Mode error code and procedures until the Recovery Mode error is resolved. Once the Recovery Mode error is resolved, you can see the Recovery Mode error code and procedures as well as other error codes and procedures. Be sure to keep polling for errors as other error codes and procedures may arise once others are resolved.</mark>

When fetching data, we recommend doing so in one second intervals. Telemetry data is calculated and updated in one second increments, so increasing the request rate in your programs will not make any difference.

Here is an example packet you can could send to fetch the EVA.json file:

```
Timestamp: 1763414183 -> bytes: 691b90a7
Command: 0 -> bytes: 00000000

Full packet bytes: 691b90a700000000
Response bytes: 6041a0c26f7400007b0a092274656c656d65747279223a097b0 (+742 more bytes, decode the bytes as JSON)
```

## Testing

It is incredibly important to test your hardware and software ahead of test week in May. The interface for TSS is intended to allow you to debug certain parts of your design in the absence of the physical <a href="#peripheral-devices">peripheral devices</a>. In the web interface, you'll note sections for both the UIA and DCU, with switches you can flip. These can be enabled and disabled to test your systems and note how they can impact telemetry values.

### Scripts

We have created various scripts to support testing and simulate real world values ahead of test week. Please run the following commands below in the <a href="/scripts/">scripts</a> folder.

- Simulate position values: `python simulate_position.py <tss_server_address>`
- Reset all telemetry data: `./reset_data.bat`

## JSC Test Week

![](https://www.nasa.gov/wp-content/uploads/2023/02/jsc2025e046380.jpg?w=1200)
_24-25 SUITS group photo at the rock yard located at JSC_

Test week at The Lyndon B. Johnson Space Center is the culmination of your teams effort and a chance to test and highlight your work to NASA team members. During the week, you should expect two sessions to showcase your work in a complete scenario. They will be spaced apart on separate days to allow additional time for fixing any issues that arise during your first session. A list of the procedures and expected timing (subject to change) that will be tested can be found <a href="/documents/procedures/">here</a>.

### What is the rock yard?

The rock yard is a physical location on-site at Johnson Space Center where your work will be tested. This is the same location used for several ongoing projects at NASA to validate equipment for lunar rovers and other devices. We have provided 3D scans of the rock yard which can be found [here](/documents/3d-scans), they might be useful for development, or just to see the environment that testing will take place in.

### What to expect for testing

<img src="frontend/images/suits-test-week.png">

During test week, we will be hosting an official instance of TSS. This will be deployed on a local network and you will connect to it via a network address. Provided that you are connected to the same Wi-Fi network as the server, you should be able to connect and issue commands in the exact same way. You should expect and plan that the network address for the server will differ from your development instances, so we suggest making it easy to change in your interface or code.

## Questions

If this README or the additional documents provided in the <a href="/documents/">documents folder</a> are not enough to answer a specific question, please reach the SUITS tech team at `nasa-suits@mail.nasa.gov`.

If there are any notable bugs with TSS blocking development, we welcome you to create a new issue within the GitHub repository.

If you have reached the very end of this document and still want to learn more about how TSS works, feel free to read the more technical documentation located in the `src` folder: <a href="/src/README.md">TSS Development Documentation</a>

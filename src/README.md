# TSS Development Documentation

This document is intended to provide an overview of the current telemetry stream server repository, allowing new developers or interested students to understand the structure and how to make changes/contribute to the telemetry stream server.

![](/frontend/images/tss-structure.png)
_NASA SUITS block diagram of all peripherals and software_

## Introduction

### Navigation

- <a href="#frontend-development">Frontend Development</a>
- <a href="#server-development">Server Development</a>
- <a href="#telemetry-simulation-development">Telemetry Simulation Development</a>
- <a href="#peripheral-devices">Peripheral Devices</a>

### Background

The NASA SUITS project has taken on several iterations since its start in 2018. This telemetry server marks a significant rewrite from the previous iteration of [TSS 2025](https://github.com/SUITS-Techteam/TSS-2025). We identified several key improvement points including a new interface, better simulated telemetry values, and other features that we wanted to focus on.

TSS was developed for the NASA SUITS challenge with the goal of providing a reliable stream of data to participating teams to use within their interfaces and hardware designs. This started out with several basic linear decay and growth algorithms to model basic resource consumption, but has since been expanded to accommodate complex relationships between multiple telemetry values (e.g. EVA location values calculating speed, increasing heart rate). These complex telemetry relationships help to create a more realistic scenario for students to develop with.

### Structure

The project is separated into several folders:

- `/data`: contains the JSON files that are read/written to for telemetry storage
- `/documents`: contains supporting documentation, images, etc. for students to use during development
- `/frontend`: all frontend assets, primarily HTML, JS, and CSS
- `/scripts`: supporting Python scripts to allow users to test various parts of their implementation
- `/src`: all C code that runs the server infrastructure and simulation engine

## Frontend Development

![Image of the user interface of the main page of TSS](../frontend/images/tss-main-page.png)
_Primary interface for interacting with TSS_

The frontend for TSS was developed in an extremely simple stack using HTML, CSS, and JavaScript. Through several iterations of the server, we have opted to keep the majority of the backend written in C. This meant that using more modern languages or libraries (e.g. React) would significantly complicate the development of the frontend.

### UI elements

The frontend is built with a minimal interface and reusable styling. All of the style code is consolidated into a singular stylesheet file that is titled <a href="../frontend/index.css">index.css</a>. The styling has slowly morphed into a large file than was originally planned and could likely be due for refactoring and simplification.

The primary color scheme is:

- Black (background): `#000000ff`
- Blue (title card background): `#06212cff`
- Light Blue (borders): `#3889abff`
- Green (buttons): `#28ae5f`
- Dark Gray (buttons): `#2d2d2d`
- Light Gray (borders and switches): `#8e8e8eff`

### Data syncing

Throughout the infrastructure, you'll note that we primarily use UDP sockets to communicate back and forth to make data changes and issue commands. The frontend is the one section of the tech stack that we have opted to still use HTTPS for data fetching. The server still serves the JSON files over HTTPS, and can be fetched with a relative URL such as: `/data/EVA.json`.

The frontend JavaScript fetches the JSON file `EVA.json` every second. A snippet of that code is referenced here:

```js
// Fetch fresh data from the backend every one second
setInterval(() => {
  fetchData();
  updateClock();
  updateConnectionStatus();
}, 1000);
```

After fetching the data from the backend, we still need to take the fresh data and display it on the interface. This is done with a nifty setup that allows us to add new telemetry values and elements to the HTML without creating repetitive code in JavaScript. You'll note that every value being updated has a HTML attribute labeled `data-path`, here is an example below:

```html
<div class="telemetry-value">
  <span
    class="telemetry-data"
    data-path="eva.telemetry.eva1.scrubber_a_co2_storage"
    data-units="%"
  >
    ------
  </span>
</div>
```

The data path, in this instance `eva.telemetry.eva1.scrubber_a_co2_storage`, directly corresponds to a field in a resulting JSON file. For example this data path would be for the `EVA.json` file and uses a period as a delimeter to denote a field name. As a result, we could expect to find the value to update this HTML element in a JSON file structured like below:

`EVA.json`

```json
{
  "telemetry": {
    "eva1": {
      "scrubber_a_co2_storage": "new value!"
    }
  }
}
```

Here is a snippet from the <a href="../frontend/index.js">index.js</a> file that quickly identifies all of the frontend elements that need to be updated with new telemetry values.

```js
const elements = document.querySelectorAll("[data-path]");
elements.forEach((el) => {
  // Process fetching the correct field from the JSON file and updating the HTML element's value
});
```

## Server Development

### UDP sockets

The server was converted to communicate primarily over the UDP protocol for the 24-25 challenge, after having bad experiences with several teams sending a high number of HTTP requests that brought the small server hosted on the local network down during test week. Moving to the UDP protocol helped remedy those issues and created an additional unique challenge for teams to develop with a networking protocol that they don't typically encounter.

The user datagram protocol is connectionless, which means that it can be much faster than TCP but can also be less reliable since there is no confirmation of delivery. The diagram below illustrates how the UDP protocol is used in C. If you are interested in learning more, I highly recommend this article by Cloudflare: [Everything you ever wanted to know about UDP sockets but were afraid to ask](https://blog.cloudflare.com/everything-you-ever-wanted-to-know-about-udp-sockets-but-were-afraid-to-ask-part-1/).

![](https://www.cs.dartmouth.edu/~campbell/cs60/UDPsockets.jpg)

_Image Credit: <a href="https://www.cs.dartmouth.edu/~campbell/cs60/socketprogramming.html">Dartmouth Computer Science</a>_

### Server structure

- `data.c`: Majority of the data handling, processing UDP requests, routing to JSON files, other data helpers, etc
- `network.c`: Core networking functionality, creating socket connections, etc
- `server.c`: Sets up the frontend HTTP server, UDP sockets, sim engine, and other helper functions to communicate with peripherals.

### Data handling

Requests to change a value can be done over HTTP (from the frontend) or via UDP (peripherals, student devices, etc). In both cases, they are eventually converted into a string format that represents a file name and field path to update the resulting JSON field with a new value. For example, if someone flips the EVA 1 power switch on the physical UIA, it will send a UDP packet to the server with the command number `2003`, this command number will be converted to a data path based on the hard coded table found in <a href="/src/data.h">data.h: udp_command_mappings</a>, in this case that would be `eva.uia.eva1_power`. This is a very similar mechanism done in reverse to the frontend data update code highlighted above.

## Telemetry Simulation Development

### Overview

A simulation engine was developed in C to help create a realistic scenario that incorporated the live telemetry values from the EVA, and create complex relationships with various other biometrics and environmental values.

The code for the library can be found in <a href="/src/lib/simulation">src/lib/simulation</a>. It is divided into two main files, `sim_algorithms.c` which specifies the supported algorithms for the simulation, and `sim_engine.c` which includes all of the helper functions and main code to initiate a simulation (this is called upon within the backend server to create a new simulation instance).

The supported algorithms are:

- Linear Growth
- Linear Decay
- Sine Wave
- Dependent Value (custom algorithm)
- External Value

The first three are quite basic and used for extremely simple time based telemetry calculations. What makes this powerful is the use of the final two types of algorithms, which can be used to pull in external values (e.g. the location of an EVA which is a real world value and not simulated), and custom equations that are dependent on both simuilated values and external values.

### Configuration

Instead of hardcoding every field that we wanted to simulate, we opted to create a format that was easily configurable. This took the form of JSON files that live within a <a href="/src/lib/simulation/config">config folder</a> in the root of the simulation library folder. Each file is representative of a single "component" that you want to simulate; in our case that would be `eva1` and `eva2`. Below is an example of a config file with some of the supported algorithms mentioned above:

NOTE: For custom algorithms, to support order of operations with parentheses, the parser was implemented with the expectation that everything would have a space in between it including parentheses. For example, you would write an equation like so: `( external_temp * 2 ) + 16

```json
{
  "component_name": "eva1",
  "fields": {
    "primary_battery_level": {
      "type": "float",
      "algorithm": "linear_decay",
      "min_value": 0.0,
      "max_value": 100.0,
      "rate": 0.05,
      "start_value": 100.0
      
    },
    "secondary_battery_level": {
      "type": "float",
      "algorithm": "linear_decay",
      "min_value": 0.0,
      "max_value": 100.0,
      "rate": 0.05,
      "start_value": 100.0
    },
    "oxy_pri_storage": {
      "type": "float",
      "algorithm": "linear_decay",
      "min_value": 0.0,
      "max_value": 100.0,
      "rate": 0.013,
      "start_value": 100.0
    },
    "oxy_sec_storage": {
      "type": "float",
      "algorithm": "linear_decay",
      "min_value": 0.0,
      "max_value": 100.0,
      "rate": 0.013,
      "start_value": 100.0
    }
  }
}
```

## Peripheral Devices

The peripheral devices used during test week communicate with TSS over the UDP protocol. The code for these devices are not available publicly.

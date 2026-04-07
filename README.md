# STM32F103_NRF24L01_Dual_OLED_RX

A continuation of my earlier STM32 embedded integration project, this repository focuses on **wireless data reception and display handling** using an **STM32F103**, **nRF24L01**, **PCA9548A I2C multiplexer**, and **dual SSD1306 OLED displays**.

This project is part of my personal embedded systems learning and demonstration journey.  
While my previous repository was focused on integrating multiple peripherals, sensors, displays, ADC, DMA, PWM, UART, and RTC on a single STM32F411 system, this new project moves in the next direction: **splitting the system into multiple nodes and building inter-board communication**.

In simple words, this project acts as a **wireless receiver and display node**.

---

## Project Background

This repository is a follow-up to my earlier project:

- **Previous project:** `STM32F411_TFT_OLED_ADXL355_UART`

That earlier project demonstrated my ability to integrate multiple peripherals and embedded modules into a single bare-metal STM32 firmware application.

This new repository continues that work by introducing:

- **multi-board architecture**
- **wireless communication**
- **message reconstruction from received packets**
- **dual-display output using two OLEDs**
- a more modular design approach for future embedded expansion

This project is an important step toward my long-term goal of building a more structured and scalable embedded system, and later converting the broader system into an **RTOS-based real-time design**.

---

## What This Project Does

This STM32F103-based firmware works as a **receiver-side embedded node**.

Its main job is to:

1. listen for incoming wireless packets using **nRF24L01**
2. receive segmented text/message data
3. rebuild the received frame from multiple packets
4. distribute the reconstructed content across **two OLED displays**
5. show how multiple modules can work together in a communication-driven embedded application

This project is less about local sensor acquisition and more about **receiving, organizing, and presenting data coming from another embedded system**.

---

## Main Hardware Used

- **STM32F103** development board
- **nRF24L01** 2.4 GHz wireless transceiver
- **2 x SSD1306 128x64 OLED displays**
- **PCA9548A I2C multiplexer**
- status LED
- SPI interface for nRF24L01
- I2C interface for OLED displays through multiplexer

---

## Main Features

### 1. Wireless Reception with nRF24L01
This project uses the **nRF24L01** module as a wireless receiver.

The STM32F103 is configured in **RX mode**, continuously listening for incoming packets from a transmitter node.  
Packets are received over SPI through a custom driver layer built around the nRF24L01 module.

This demonstrates:
- wireless embedded communication
- SPI peripheral handling
- packet-based data transfer
- receive-side message management

---

### 2. Multi-Packet Frame Reconstruction
Instead of receiving a full message in one packet, the project receives data in **multiple smaller packet segments**.

Each packet includes:
- a token index
- total token count
- frame ID
- text payload

The receiver stores each incoming token in the correct position and tracks which parts of the frame have already arrived.

Once all expected tokens for the current frame are received, the project considers the message complete and updates the displays.

This demonstrates:
- structured packet handling
- indexed message reconstruction
- frame tracking
- simple communication protocol design

---

### 3. Dual OLED Display Output
The received content is displayed across **two OLED screens**.

Since both OLEDs share the same I2C address, a **PCA9548A I2C multiplexer** is used to select and drive each display independently.

The reconstructed message is split into two parts:
- first half goes to **OLED 1**
- second half goes to **OLED 2**

This allows the receiver node to show more information than a single small OLED screen can normally present.

This demonstrates:
- I2C bus expansion
- working with multiple displays of the same address
- screen management
- display partitioning for received content

---

### 4. Custom SSD1306 Driver Usage
The project uses an SSD1306 display driver with:
- framebuffer-based screen updates
- string drawing
- pixel drawing
- line and rectangle drawing helpers
- contrast and invert support
- compact 5x7 font rendering

This keeps the display side reusable and organized while also helping me better understand low-level OLED communication.

---

### 5. Modular Receiver Architecture
Compared to my earlier all-in-one STM32F411 project, this repository is more focused and modular.

The firmware is clearly centered around:
- wireless receive logic
- frame management
- I2C display routing
- dual-OLED rendering

This is an important step toward designing larger embedded systems where different controllers are responsible for different roles.

---

## How the Project Works

### Startup Flow
When the board powers up, the firmware:

1. initializes GPIO
2. initializes I2C1
3. initializes RTC
4. initializes SPI1
5. initializes the receiver-side nRF24L01 driver
6. initializes OLED 1 through PCA9548A
7. initializes OLED 2 through PCA9548A
8. shows startup text on both OLED displays
9. enters an infinite loop waiting for wireless packets

---

### Runtime Flow
During normal operation:

1. the nRF24L01 listens for a packet
2. when a packet is received, the STM32 reads the payload
3. the payload is interpreted as one indexed token from a larger frame
4. the token is stored in the correct buffer position
5. the firmware keeps track of how many tokens of the current frame have arrived
6. once the full frame is received, the message is considered complete
7. the reconstructed content is split into two display ranges
8. OLED 1 shows the first section
9. OLED 2 shows the remaining section

This creates a simple but effective wireless display system.

---

## Packet Structure

From the current implementation, each received payload is treated approximately as:

- **Byte 0** → token index
- **Byte 1** → total token count
- **Byte 2** → frame ID
- **Byte 3 onward** → text/token data

This structure allows the receiver to:
- identify which piece of the message was received
- know how many total pieces belong to the frame
- detect a new frame when the frame ID changes
- avoid mixing old and new message content

---

## Why This Project Matters

This repository is important to me because it represents a move from:

- single-board embedded integration  
to
- **distributed embedded design**

My earlier STM32F411 project showed that I could make many peripherals work together on one microcontroller.

This new STM32F103 project shows that I am now extending that knowledge toward:
- wireless communication between boards
- modular node design
- structured packet handling
- separated display responsibility
- scalable future system architecture

It is a small but meaningful step toward more advanced embedded systems.

---

## Relationship to My Previous Project

This project is directly connected to my earlier repository:

- **Previous project:** `STM32F411_TFT_OLED_ADXL355_UART`
- https://github.com/roxinfi/STM32F411_TFT_OLED_ADXL355_UART

That project demonstrated:
- multi-peripheral STM32 integration
- sensor reading
- TFT + OLED + LCD handling
- ADC + DMA
- PWM LED control
- UART output
- RTC usage

This new repository continues that journey by focusing on the **communication and receiver/display side** of a larger system.

You can think of it as the next stage in the same learning path:
- first, integrate everything on one MCU
- then, start dividing responsibilities across multiple MCUs
- next, build more structured and scalable inter-controller systems

---

## Libraries and References Used

This project was built with the help of open-source libraries, examples, and reference repositories that I used to learn, understand implementation details, and improve my own code.

### nRF24L01 Reference / Learning Source
- **STM32 HAL nRF24 Library**  
  https://github.com/HardwareLevel/stm32_hal_nrf24_library

### OLED Reference / Display Learning
- **SSD1306 Library / Reference**  
  Based on my adapted SSD1306 implementation and earlier learning references used in my previous display work.

I appreciate the work of the original authors and maintainers whose open-source resources helped me understand and build this project.

---

## What This Project Demonstrates

This project demonstrates my practical ability to work with:

- STM32F103 firmware development
- SPI communication
- I2C communication
- wireless packet reception
- message/frame reconstruction
- multi-display output
- I2C multiplexing using PCA9548A
- modular embedded design
- receiver-side communication logic
- structured bare-metal embedded programming

More importantly, it shows that I am not only building isolated demos, but also working toward systems where multiple embedded devices can cooperate together.

---

## Current Design Style

The current implementation is still **bare-metal / super-loop based**.

That means:
- the main loop continuously checks for available wireless data
- packet handling and display updating happen directly in firmware logic
- the design is intentionally simple and transparent for learning and debugging

This is useful for understanding the communication flow clearly before moving to a more advanced RTOS-based architecture in future work.

---

## Open Source / Usage Note

This project is shared as a personal learning and demonstration project and is intended to be **free to study, use, modify, and build upon**.

If this code helps someone learn something, improve their own project, or understand embedded communication better, that is always welcome.

At the same time, please respect the licenses of any third-party libraries, examples, or references that contributed to this project.

This repository is shared in the spirit of:
- learning
- experimentation
- open knowledge
- embedded systems practice

---

## Closing

This project represents the next step in my embedded systems journey.

My earlier work focused on integrating many peripherals into one STM32-based system.  
This repository builds on that foundation and begins exploring how embedded devices can exchange and present information across separate nodes.

It is a simple receiver project on the surface, but for me it represents something bigger:  
the shift from single-board experiments toward more connected, modular, and scalable embedded system design.

# Microcontroller-Based Inverse Time Overcurrent Relay ⚡

## Overview
This repository contains the source code and logic implementation for a **Microcontroller-Based Overcurrent Protection Relay** with **Inverse Time Characteristics**. The system is designed to continuously monitor AC current and protect electrical circuits by calculating a precise tripping time based on the fault current magnitude, adhering to standard IEEE protection curves.

This project specifically implements the **Very Inverse** characteristic, ensuring a faster trip response for severe short circuits and a delayed trip for minor overloads, allowing for optimal system stability and protection.

## Features
* **Real-Time Current Monitoring:** Utilizes the PZEM-004T sensor for highly accurate AC current measurement.
* **Dynamic Trip Calculation:** Implements the IEEE Very Inverse mathematical model to calculate the exact trip delay dynamically.
* **Non-Blocking Logic:** The microcontroller evaluates fault conditions continuously using `millis()` instead of `delay()`, ensuring the system never freezes during a fault.
* **Visual Status Interface:** Displays real-time current, fault status, and calculated trip time on a 16x2 I2C LCD.
* **Hardware Isolation & Alarm:** Triggers a relay to disconnect the load and sounds a buzzer upon tripping.
* **Manual Reset:** Features a hardware push-button to restore the system only after the fault has been safely cleared.

## Hardware Components
To replicate this project, the following hardware components are required:
* Arduino Uno (or compatible microcontroller)
* PZEM-004T AC Current/Voltage Sensor
* 1-Channel Relay Module (5V)
* 16x2 LCD Display with I2C adapter
* Active Buzzer
* Push Button (for hardware reset)

## Mathematical Model
The tripping time ($t$) is calculated dynamically inside the microcontroller using the standard IEEE Very Inverse curve equation:

`t = (Beta * TMS) / ((I / Is)^Alpha - 1)`

Where:
* **`I`** = Measured Fault Current (A)
* **`Is`** = Pickup Current Setting (Set to 2.0 A by default)
* **`TMS`** = Time Multiplier Setting (Set to 0.05 s by default)
* **`Alpha`** = 1.0 (IEEE constant for Very Inverse)
* **`Beta`** = 13.5 (IEEE constant for Very Inverse)

## Software Dependencies
To compile and run this code, you must install the following libraries in your Arduino IDE via the Library Manager:
1. `PZEM004Tv30` (For UART communication with the current sensor)
2. `LiquidCrystal_I2C` (For the LCD display)

## How It Works
1. Upon startup, the Arduino initializes the relay (keeping the load connected) and the LCD.
2. The PZEM-004T sensor continuously feeds the RMS current value to the Arduino.
3. If the current exceeds the `Is` threshold (2.0 A), the system detects a fault and begins calculating the allowable trip time based on the fault's severity.
4. If the fault persists beyond the calculated trip time, the Arduino sends a signal to open the relay (disconnecting the load) and activates the buzzer.
5. The system halts and remains in the "TRIPPED" state until the user manually presses the Reset button.

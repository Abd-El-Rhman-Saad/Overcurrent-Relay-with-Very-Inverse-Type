# Microcontroller-Based Inverse Time Overcurrent Relay ⚡
### 12V AC Safe Prototyping Architecture

## Overview
This repository contains the source code and documentation for a **Microcontroller-Based Overcurrent Protection Relay** featuring **Inverse Time Characteristics**. The system continuously monitors AC current and calculates a dynamic tripping time based on the fault current's magnitude, adhering to the standard IEEE **Very Inverse** protection curve.

To eliminate the severe electrical hazards associated with 220V mains prototyping on breadboards, this project scales the load circuit down to a safe **12V AC** environment. This allows for rigorous, repeatable physical simulations of real-world overcurrents and short circuits without risk of component destruction or arc flashes.

## Key Features
* **Safe AC Prototyping:** Utilizes a 220V to 12V step-down transformer to safely simulate industrial faults.
* **Custom RMS Calculation:** Samples the analog waveform via the ZMCT103C sensor and calculates true RMS current entirely in software.
* **Dynamic Trip Calculation:** Implements the IEEE Very Inverse mathematical model to calculate the exact trip delay dynamically without using blocking `delay()` functions.
* **Fault Matrix Simulation:** Uses high-wattage power resistors switched in parallel to inject precise minor and major faults.
* **Hardware Isolation & Alarm:** Triggers an active-low 5V relay to disconnect the 12V load and indicates status via an I2C LCD and LEDs.

## Hardware Components
* **Arduino Uno** (Central 5V Logic and ADC)
* **Step-Down Transformer** (220V to 12V-0-12V, 1A rating)
* **ZMCT103C AC Current Sensor** (5A rating, analog output)
* **1-Channel Relay Module** (5V, Active Low)
* **Fault Matrix Resistors:**
  * 47Ω (5W) for Normal Load
  * 10Ω (30W) for Major Fault injection
* **16x2 LCD Display** (with I2C adapter)
* **LEDs** (Green/Red) & **Push Button** (for hardware reset)

## Mathematical Model
The dynamic tripping time ($t$) is calculated inside the Arduino using the IEEE Very Inverse curve equation:

`t = (Beta * TMS) / ((I / Is)^Alpha - 1)`

Where:
* **`I`** = Measured Fault Current (A)
* **`Is`** = Pickup Current Setting (Scaled to **0.4 A** for the 1A transformer)
* **`TMS`** = Time Multiplier Setting (**0.05 s**)
* **`Alpha`** = 1.0
* **`Beta`** = 13.5

## Fault Matrix Testing Protocol
The system is tested using a parallel resistor matrix to safely simulate grid faults:
1. **Normal Operation (~0.25A):** A single 47Ω resistor is connected. The current remains below the 0.4A pickup threshold.
2. **Minor Fault (~0.51A):** A second 47Ω resistor is switched in parallel. The Arduino detects the overcurrent, calculates a relatively long trip delay, and trips the relay if the fault persists.
3. **Major Fault / Short Circuit (~1.46A):** A 10Ω resistor is switched in parallel. The Arduino calculates a near-instantaneous trip time and isolates the circuit immediately, protecting the 1A transformer.

## Software Dependencies
Install the following library via the Arduino IDE Library Manager:
* `LiquidCrystal_I2C` (For the LCD display)

*Note: The PZEM-004T library is no longer required as the RMS calculation is now handled natively via the Arduino's 10-bit ADC.*

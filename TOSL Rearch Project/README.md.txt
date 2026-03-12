# Automated Dual Optical Filter Wheel System using Arduino

## 📌 Project Overview
This project demonstrates a precise motion control system built with an Arduino microcontroller to drive a custom **Dual Optical Filter Wheel Mechanism**. The system is designed to automate the selection of optical filters in a light path, which is a fundamental requirement in automated optical setups, spectroscopy, and advanced physics experiments.

## 🛠️ Hardware Components
* **Microcontroller:** Arduino (Uno/Nano/Mega)
* **Motor Driver:** L298N Dual H-Bridge Motor Driver
* **Actuator:** 28BYJ-48 Stepper Motor
* **Mechanism:** Two synchronized filter wheels, each containing 4 distinct optical filters (allowing for up to 16 unique filter combinations).
* **Power Supply:** External DC power source for the motor driver.

## 💻 Software & Libraries
* **Language:** C++ (Arduino IDE)
* **Core Library:** `AccelStepper.h` - Utilized for advanced non-blocking motor control, allowing for configurable acceleration, deceleration, and highly precise micro-stepping.

## ⚙️ System Architecture & Working Principle
The system uses the L298N module to provide the necessary current to drive the coils of the 28BYJ-48 stepper motor. By leveraging the `AccelStepper` library, the code implements continuous acceleration profiles, ensuring that the heavy filter wheels ramp up to speed and slow down smoothly. This prevents mechanical jerks and step loss, ensuring the filters perfectly align with the optical axis every single time.

## 🚀 Key Features
* **Precise Optical Alignment:** Moves the 4-slot filter wheels to exact micro-step locations for perfect beam alignment.
* **Smooth Acceleration Profiles:** Prevents mechanical stress on the wheel axis and guarantees repeatable positioning.
* **Non-Blocking Code:** The main loop remains free to read sensors (e.g., photo-detectors) or communicate via Serial while the wheels are in motion.

## 📁 Repository Contents
* `filter_wheel_control.ino`: The main Arduino sketch containing the AccelStepper configurations and rotational logic.
* *(Optional)* Add a photo of your physical dual filter wheel setup here to showcase the mechanical build!
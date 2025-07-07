# Wake Clock - ESP32 Multi-Function Clock

This project provides code for a digital wake-clock using an ESP32 with multiple input and output components. The clock includes rotary encoder input (w/ switch), RGB LED state indicator, alphanumeric display, and an RTC with battery backup so the time is not lost when power is.

A schedule can be set for each day of the week, with the following time windows:

* Wind Down (Blue)
* Sleep (Red)
* Quiet (Yellow)
* Wake (White)

Depending on which window the current time falls in for the current day's schedule the RGB LED will change colors.

# ESP32-NetFind
An ESP32 program for finding nearby networks, visible or hidden, that meet certain criteria.

## A quick rundown of how to use this code
Several constant variables are used in this code. You will likely need to change
the values of these constant variables to suit your needs.

* `LED_PINS`
  * An array of pin numbers on which LEDs are connected.
  * The LEDs will be illuminated in the order they're defined in the array.
* `NUM_PINS`
  * An integer that stores the **number** of LEDs defined in the `LED_PINS` list. If this value
    doesn't equal the number of LEDs defined, you may have trouble.
* `MAC_FILTER`
  * An array of three integers (written as hexadecimal values) that constitute the filter
    for the MAC address. In order for a device or access point to pass the filter, the value of
	the first segment in the device's MAC address must match the _first_ value in the array, the
	value of the second segment in the MAC address must match the second value in the array, etc.
* `DISP_LEDS`
  * Display the count on connected LEDs. Can be `true` or `false`.
* `DISP_OLED`
  * Display the count using a connected OLED display. Can be `true` or `false`.
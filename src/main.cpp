#include <Arduino.h>
#include <WiFi.h>

#include <string.h>
#include <map>

// For a connection via I2C using the Arduino Wire include:
#include <Wire.h>               // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"

/*
 * This is an array of pins for the LEDs you connect to the board.
 * You can add or remove pins at any time. The code has be written
 * to use an arbitrary number of pins, just make sure you update
 * the NUMPINS variable to reflect the number of defined pins.
 * This code will light them up in the order they're defined in
 * the array, so if {15, 16, 17} is used, and one network is found,
 * the LED connected to pin 15 will be lit. If two networks are found,
 * then the LEDs connected to pins 15 and 16 will be lit.
 */
const uint8_t LED_PINS[] = {32, 33, 25};
const uint8_t NUM_PINS = 3;

/*
 * This is your MAC address filter. The code is configured to use a
 * three-part filter to match against the first three segments of
 * a device's MAC address.
 * Note that MAC addresses are in hexadecimal, so each segment is really
 * an integer (between 0 and 255), not a string. When defining this filter,
 * use hexadecimal numbers by adding a '0x' before the two characters from
 * the MAC address. (e.g. 1A:2B:3C:4D:5E:6F to {0x1A, 0x2B, 0x3C, ...}).
 */
const uint8_t MAC_FILTER[3] = {0x00, 0x0D, 0x97};

/*
 * Flags for device count display methods. 
 * DISP_LEDS: display the count using discrete LEDs, connected to LED_PINS pins.
 * DISP_OLED: display the count using a connected OLED display.
 */
const bool DISP_LEDS = true;
const bool DISP_OLED = true;

/*
 * Definition for an attached OLED display. If the MELIFE ESP32 board from Amazon
 * is used, the defined pins should be correct. If nothing displays despite the
 * DISP_OLED flag being on, you may want to tune the second and third argument
 * pins in this definition.
 */
SSD1306Wire display(0x3c, 5, 4);  // ADDRESS, SDA, SCL  -  If not, they can be specified manually.

/*
 * A map that corresponds the signal strength to a given device to the
 * String version of its MAC address. You can use an OLED screen to
 * display the signal strengths, which may be useful for triangulating
 * the physical location of the device.
 */
std::map<String, int32_t> deviceSignalMap;

/*
 * A variable to keep track of the number of devices that passed the filter. 
 */
uint16_t num_devices = 0;

/**
 * Searches for WiFi access points that meets the 
 * @param filter The first three segments of the MAC address to filter by.
 * Only devices with a MAC address whose first three segments match the provided
 * filter are considered in the count.
 * @returns The number of devices found that match the filter.
 */
uint16_t count_devices(const uint8_t filter[3]) {
	// Scan for local devices.
	int16_t scanResults = WiFi.scanNetworks(false, true);
	// Maintain a count of found devices that pass the filter.
	uint16_t num_filtered = 0;

	// If no devices were found, print a message.
	if (scanResults == 0) Serial.println("No WiFi devices in AP Mode found");

	// If some devices were found, filter and count them.
	else {
		Serial.print("Found "); Serial.print(scanResults); Serial.println(" devices.");

		// For each of the scan results...
		for (int i = 0; i < scanResults; ++i) {
			// Retrieve further information about this results.
			String SSID = WiFi.SSID(i);
			int32_t RSSI = WiFi.RSSI(i);
			uint8_t* BSSID = WiFi.BSSID(i);
			String BSSIDstr = WiFi.BSSIDstr(i);

			Serial.print(i + 1);
			Serial.print(": ");
			Serial.print(SSID);
			Serial.print(" - ");
			Serial.print(BSSIDstr);
			Serial.print(" (");
			Serial.print(RSSI);
			Serial.print(")");
			Serial.println("");

			// Only consider devices whose MAC address passes the provided filter.
			if(BSSID[0] == filter[0] && BSSID[1] == filter[1] && BSSID[2] == filter[2]) {
				// Map the BSSID (MAC address) to the measured signal strength.
				std::map<String, int32_t>::iterator it = deviceSignalMap.find(BSSIDstr);
				if(it != deviceSignalMap.end() && it->second != std::abs(RSSI))
					it->second = std::abs(RSSI);
				else
					deviceSignalMap.insert(std::make_pair(BSSIDstr, std::abs(RSSI)));

				// The device has passed the filter. Increment the number of found devices.
				num_filtered++;
			}

			// delay(10);
		}
	}

	// Clean up RAM by deleting the scan results.
	WiFi.scanDelete();

	return num_filtered;
}

/**
 * Display, either on an OLED display or using the LEDs defined by the LED_PINS pins,
 * the provided count of devices that passed the filter.
 */
void disp_count(const uint16_t count) {
	// If displaying the count using LEDs has been enabled, do so.
	if(DISP_LEDS) {
		for(uint8_t i = 0; i < NUM_PINS; i++) {
			if(i < count) {
				// Serial.println("Setting pin " + String(i) + " HIGH.");
				digitalWrite(LED_PINS[i], HIGH);
			}
			else {
				// Serial.println("Setting pin " + String(i) + " LOW.");
				digitalWrite(LED_PINS[i], LOW);
			}
		}
	}

	// If displaying the count using an OLED has been enabled, do so.
	if(DISP_OLED) {
		// Clear the display.
		display.clear();

		display.drawStringMaxWidth(0, 8, 128,
			"Found " + String(count) + " devices that passed the filter."
		);

		display.display();
	}
}

void setup() {
	Serial.begin(115200);
	Serial.println();

	WiFi.persistent(false);

	// If device counts are to be displayed on LEDS, initialize the associated pins.
	if(DISP_LEDS) {
		for(uint8_t i = 0; i < NUM_PINS; i++) {
			pinMode(LED_PINS[i], OUTPUT);
			digitalWrite(LED_PINS[i], LOW);
		}
	}

	// If device counts are to be displayed on an OLED display, initialize it.
	if(DISP_OLED) {
		display.init();
		// Remove this if the text on your display is upside down.
		display.flipScreenVertically();
		display.setFont(ArialMT_Plain_10);
		display.setTextAlignment(TEXT_ALIGN_LEFT);
		display.display();
	}
}

void loop() {

	// Scan networks and count how many pass the filter.
	num_devices = count_devices(MAC_FILTER);

	Serial.println("Counted " + String(num_devices) + " devices.");

	// Display the number of devices that passed the filter.
	disp_count(num_devices);
	
	delay(100);
}
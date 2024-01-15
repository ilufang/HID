/*
  Copyright (c) 2024 ilufang
  See the readme for credit to other people.

  IO4 Simple Example


*/

#include <HID-Project.h>

static unsigned long time_test = 0, time_service = 0;

void setup() {
	Serial.begin(115200);
}

void loop() {
	unsigned long now = millis();
	// Get command from serial
	while (Serial.available()) {
		switch(Serial.read()) {
			case '0':
				Serial.println("Hello");
				break;
			case '1':
				SingleIO4.out.buttons[0] |= IO4_BUTTON_TEST;
				time_test = now;
				break;
			case '2':
				SingleIO4.out.buttons[0] |= IO4_BUTTON_SERVICE;
				time_service = now;
				break;
			case '3':
				SingleIO4.out.coin[0] += 0x100;
				break;
		}
	}

	// Release pressed buttons after 200ms
	if (time_test && now - time_test > 200) {
		SingleIO4.out.buttons[0] &= ~IO4_BUTTON_TEST;
		time_test = 0;
	}
	if (time_service && now - time_service > 200) {
		SingleIO4.out.buttons[0] &= ~IO4_BUTTON_SERVICE;
		time_service = 0;
	}

	// Process IO4 H2D
	switch(SingleIO4.recv()) {
		case IO4_CMD_SET_COMM_TIMEOUT:
			Serial.println("IO4_CMD_SET_COMM_TIMEOUT");
			break;
		case IO4_CMD_SET_SAMPLING_COUNT:
			Serial.println("IO4_CMD_SET_SAMPLING_COUNT");
			break;
		case IO4_CMD_CLEAR_BOARD_STATUS:
			Serial.println("IO4_CMD_CLEAR_BOARD_STATUS");
			break;
		case IO4_CMD_SET_GENERAL_OUTPUT:
			Serial.println("General write");
			break;
		case IO4_CMD_SET_PWM_OUTPUT:
			Serial.println("PWM write");
			break;
		case IO4_CMD_UNIQUE:
			Serial.println("UQ1 write");
			break;
		case IO4_CMD_UPDATE_FIRMWARE:
			Serial.println("Firmware update");
			break;
	}

	// Process IO4 D2H.
	// Call on each update. Maintain 120Hz+ report rate to avoid error 6401
	SingleIO4.send();
}

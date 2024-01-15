/*
Copyright (c) 2024 ilufang
See the readme for credit to other people.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

// Include guard
#pragma once

enum io4_btn {
	// System buttons in button[0]
	IO4_BUTTON_TEST     = 1 << 9,
	IO4_BUTTON_SERVICE  = 1 << 6,
};

enum io4_cmd {
	IO4_CMD_SET_COMM_TIMEOUT   = 0x01,
	IO4_CMD_SET_SAMPLING_COUNT = 0x02,
	IO4_CMD_CLEAR_BOARD_STATUS = 0x03,
	IO4_CMD_SET_GENERAL_OUTPUT = 0x04,
	IO4_CMD_SET_PWM_OUTPUT     = 0x05,
	IO4_CMD_UNIQUE             = 0x41,
	IO4_CMD_UPDATE_FIRMWARE    = 0x85,
};

typedef struct ATTRIBUTE_PACKED {
	// uint8_t _report_id; // = 0x01
	uint16_t analog[8];
	uint16_t rotary[4];
	uint16_t coin[2];
	uint16_t buttons[2];
	uint8_t system_status;
	uint8_t usb_status;
	// uint8_t unknown[29];
} io4_out_t;

class IO4API
{
public:

	inline IO4API();

	io4_out_t out;
	// TODO: general and pwm output states

	inline void setDirty();
	inline int send(bool force=false);
	inline int recv();

	/// Send generated report. Needs to be implemented in a lower level
	virtual int sendReport(void *report, int length) = 0;
	virtual int recvCommand() = 0;
	virtual int recvReport(void *report, int length) = 0;
private:
	unsigned long _report_time;
};

// Implementation is inline
#include "IO4API.hpp"

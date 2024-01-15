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

#define IO4_KEEPALIVE_INTVL 4 // 250 Hz report rate

IO4API::IO4API() {
	memset(&out, 0, sizeof(out));
	// out._report_id = HID_REPORTID_IO4_IN;
	out.system_status = 0x02;
	_report_time = 0;
}

void IO4API::setDirty() {
	_report_time = millis() - IO4_KEEPALIVE_INTVL - 1;
}

int IO4API::send(bool force) {
	unsigned long now = millis();
	if (force || (now - _report_time) > IO4_KEEPALIVE_INTVL) {
		if (sendReport(&out, sizeof(out)) > 0) {
			_report_time = now;
			return 1;
		}
	}
	return 0;
}

int IO4API::recv() {
	int cmd = recvCommand();
	if (cmd < 0) {
		return cmd;
	}
	switch (cmd) {
		case IO4_CMD_SET_COMM_TIMEOUT:
		case IO4_CMD_SET_SAMPLING_COUNT:
			recvReport(NULL, 0);
			out.system_status = 0x30;
			setDirty();
			return cmd;
		case IO4_CMD_CLEAR_BOARD_STATUS:
			recvReport(NULL, 0);
			out.system_status = 0;
			setDirty();
			return cmd;
		case IO4_CMD_SET_GENERAL_OUTPUT:
			// TODO: implement
			recvReport(NULL, 0);
			return cmd;
		case IO4_CMD_SET_PWM_OUTPUT:
			// TODO: implement
			recvReport(NULL, 0);
			return cmd;
		case IO4_CMD_UNIQUE:
			// TODO: implement
			recvReport(NULL, 0);
			return cmd;
		// case IO4_CMD_UPDATE_FIRMWARE:
		default:
			recvReport(NULL, 0);
			return cmd;
	}
}

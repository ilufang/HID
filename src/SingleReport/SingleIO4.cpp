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

#include "SingleIO4.h"

static const uint8_t  _hidReportDescriptorSingleIO4[] PROGMEM = {
	// D2H descriptor
	0x06, 0x00, 0xFF,  // Usage Page (Vendor Defined 0xFF00)
	0x0A, 0x01, 0x00,  // Usage (0x01)
	0xA1, 0x01,        // Collection (Application)
	0xA1, 0x01,        //   Collection (Application)
	0x85, 0x01,        //     Report ID (0x01)
	0x75, 0x08,        //     Report Size (8)
	0x15, 0x00,        //     Logical Minimum (0)
	0x26, 0xFF, 0x00,  //     Logical Maximum (255)
	0x95, 0x3f,        //     Report Count (63)
	0x09, 0x01,        //     Usage (0x01)
	0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0xC0,              //   End Collection
	// H2D descriptor
	0xA1, 0x01,        //   Collection (Application)
	0x85, 0x10,        //     Report ID (0x10)
	0x75, 0x08,        //     Report Size (8)
	0x15, 0x00,        //     Logical Minimum (0)
	0x26, 0xFF, 0x00,  //     Logical Maximum (255)
	0x95, 0x3f,        //     Report Count (63)
	0x09, 0x01,        //     Usage (0x01)
	0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
	0xC0,              //   End Collection
	0xC0,              // End Collection
};

SingleIO4_::SingleIO4_() : PluggableUSBModule(2, 1, epType) {
	protocol = HID_REPORT_PROTOCOL;
	idle = 1;
	_recv_avail = 0;
	epType[0] = EP_TYPE_INTERRUPT_IN;
	epType[1] = EP_TYPE_INTERRUPT_OUT;
	PluggableUSB().plug(this);
}

typedef struct {
  InterfaceDescriptor hid;
  HIDDescDescriptor   desc;
  EndpointDescriptor  in, out;
} HID2Descriptor;

int SingleIO4_::getInterface(uint8_t* interfaceCount) {
	*interfaceCount += 1; // uses 1
	HID2Descriptor hidInterface = {
		D_INTERFACE(pluggedInterface, 2, USB_DEVICE_CLASS_HUMAN_INTERFACE, HID_SUBCLASS_NONE, HID_PROTOCOL_NONE),
		D_HIDREPORT(sizeof(_hidReportDescriptorSingleIO4)),
		D_ENDPOINT(USB_ENDPOINT_IN (pluggedEndpoint), USB_ENDPOINT_TYPE_INTERRUPT, 64, 1),
		D_ENDPOINT(USB_ENDPOINT_OUT(pluggedEndpoint+1), USB_ENDPOINT_TYPE_INTERRUPT, 64, 1)
	};
	return USB_SendControl(0, &hidInterface, sizeof(hidInterface));
}

int SingleIO4_::getDescriptor(USBSetup& setup) {
	// Check if this is a HID Class Descriptor request
	if (setup.bmRequestType != REQUEST_DEVICETOHOST_STANDARD_INTERFACE) { return 0; }
	if (setup.wValueH != HID_REPORT_DESCRIPTOR_TYPE) { return 0; }

	// In a HID Class Descriptor wIndex cointains the interface number
	if (setup.wIndex != pluggedInterface) { return 0; }

	// Reset the protocol on reenumeration. Normally the host should not assume the state of the protocol
	// due to the USB specs, but Windows and Linux just assumes its in report mode.
	protocol = HID_REPORT_PROTOCOL;

	return USB_SendControl(TRANSFER_PGM, _hidReportDescriptorSingleIO4, sizeof(_hidReportDescriptorSingleIO4));
}

bool SingleIO4_::setup(USBSetup& setup) {
	if (pluggedInterface != setup.wIndex) {
		return false;
	}

	uint8_t request = setup.bRequest;
	uint8_t requestType = setup.bmRequestType;

	if (requestType == REQUEST_DEVICETOHOST_CLASS_INTERFACE)
	{
		if (request == HID_GET_REPORT) {
			// TODO: HID_GetReport();
			return true;
		}
		if (request == HID_GET_PROTOCOL) {
			// TODO: Send8(protocol);
			return true;
		}
	}

	if (requestType == REQUEST_HOSTTODEVICE_CLASS_INTERFACE)
	{
		if (request == HID_SET_PROTOCOL) {
			protocol = setup.wValueL;
			return true;
		}
		if (request == HID_SET_IDLE) {
			idle = setup.wValueH;
			return true;
		}
		if (request == HID_SET_REPORT)
		{
			// Use USBRecv on H2D IRQ endpoint to get report from host
			return true;
		}
	}

	return false;
}

uint8_t SingleIO4_::getProtocol() {
	return protocol;
}

int SingleIO4_::sendReport(void *report, int length) {
	if (USB_SendSpace(pluggedEndpoint) < length + 1) {
		return 0;
	}
	uint8_t id = HID_REPORTID_IO4_IN;
	USB_Send(pluggedEndpoint, &id, 1);
	USB_Send(pluggedEndpoint, report, length);
	USB_Send(pluggedEndpoint | TRANSFER_RELEASE | TRANSFER_ZERO, NULL, 63-length);
	return 64;
}

int SingleIO4_::recvCommand() {
	if (_recv_avail != 0) {
		return -2;
	}
	_recv_avail = USB_Available(pluggedEndpoint+1);
	if (_recv_avail < 2) {
		return -1;
	}
	uint8_t hdr[2];
	USB_Recv(pluggedEndpoint+1, hdr, 2);
	if (hdr[0] != HID_REPORTID_IO4_OUT) {
		return -3;
	}
	return hdr[1];
}

int SingleIO4_::recvReport(void *report, int length) {
	if (_recv_avail < length) {
		return 0;
	}
	if (length) {
		USB_Recv(pluggedEndpoint+1, report, length);
		_recv_avail -= length;
	}
	uint8_t buf[8];
	while (_recv_avail > 0) {
		if (_recv_avail < 8) {
			USB_Recv(pluggedEndpoint+1, buf, _recv_avail);
			break;
		} else {
			USB_Recv(pluggedEndpoint+1, buf, 8);
			_recv_avail -= 8;
		}
	}
	_recv_avail = 0;
	return length;
}

void SingleIO4_::wakeupHost() {
#ifdef __AVR__
	USBDevice.wakeupHost();
#endif
}

SingleIO4_ SingleIO4;

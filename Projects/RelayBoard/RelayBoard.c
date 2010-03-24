/*
             LUFA Library
     Copyright (C) Dean Camera, 2010.

  dean [at] fourwalledcubicle [dot] com
      www.fourwalledcubicle.com
*/

/*
  Copyright 2010  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaim all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *
 *  Main source file for the RelayBoard program. This file contains the main tasks of
 *  the project and is responsible for the initial application hardware configuration.
 */

#include "RelayBoard.h"


/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	SetupHardware();

	for (;;)
	  USB_USBTask();
}

/** Configures the board hardware and chip peripherals for the project's functionality. */
void SetupHardware(void)
{
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);

	/* Hardware Initialization */
	USB_Init();

	/* Initialize Relays */
	DDRC  |= ALL_RELAYS;
	PORTC |= ALL_RELAYS;
}


/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	USB_Device_EnableSOFEvents();
}

/** Event handler for the library USB Unhandled Control Packet event. */
void EVENT_USB_Device_UnhandledControlRequest(void)
{
    const uint8_t serial[5] = { 0, 0, 0, 0, 1 };
	uint8_t data[2]         = { 0, 0 };

    switch (USB_ControlRequest.bRequest)
	{
		case 0x09:
			if (USB_ControlRequest.bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE))
			{
				LEDs_ToggleLEDs(LEDS_LED1);

				Endpoint_ClearSETUP();

				Endpoint_Read_Control_Stream_LE(data, sizeof(data));
				Endpoint_ClearIN();

				switch (USB_ControlRequest.wValue)
				{
					case 0x303:
						if (data[1]) PORTC |= RELAY1; else PORTC &= ~RELAY1; break;
					case 0x306:
						if (data[1]) PORTC |= RELAY2; else PORTC &= ~RELAY2; break;
					case 0x309:
						if (data[1]) PORTC |= RELAY3; else PORTC &= ~RELAY3; break;
					case 0x30c:
						if (data[1]) PORTC |= RELAY4; else PORTC &= ~RELAY4; break;
					default:
						break;
				}
			}
			
			break;
		case 0x01:
			if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE))
			{
				LEDs_ToggleLEDs(LEDS_LED1);

				Endpoint_ClearSETUP();

				switch (USB_ControlRequest.wValue)
				{
					case 0x301:
						Endpoint_Write_Control_Stream_LE(serial, sizeof(serial));
						break;
					case 0x303:
						if (PORTC & RELAY1) data[1]=3; else data[1]=2; break;
					case 0x306:
						if (PORTC & RELAY2) data[1]=3; else data[1]=2; break;
					case 0x309:
						if (PORTC & RELAY3) data[1]=3; else data[1]=2; break;
					case 0x30c:
						if (PORTC & RELAY4) data[1]=3; else data[1]=2; break;
					default:
						break;
				}
				
				if (data[1])
				  Endpoint_Write_Control_Stream_LE(data, sizeof(data));

				Endpoint_ClearOUT();
			}

			break;
	}
}

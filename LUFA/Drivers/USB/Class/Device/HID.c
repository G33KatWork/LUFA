/*
             LUFA Library
     Copyright (C) Dean Camera, 2009.
              
  dean [at] fourwalledcubicle [dot] com
      www.fourwalledcubicle.com
*/

/*
  Copyright 2009  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, and distribute this software
  and its documentation for any purpose and without fee is hereby
  granted, provided that the above copyright notice appear in all
  copies and that both that the copyright notice and this
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

#include "../../HighLevel/USBMode.h"
#if defined(USB_CAN_BE_DEVICE)

#include "HID.h"

void HID_Device_ProcessControlPacket(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo)
{
	if (!(Endpoint_IsSETUPReceived()))
	  return;
	  
	if ((USB_ControlRequest.wIndex   != HIDInterfaceInfo->Config.InterfaceNumber) &&
	    (USB_ControlRequest.bRequest != REQ_SetIdle))
	{
		return;
	}

	switch (USB_ControlRequest.bRequest)
	{
		case REQ_GetReport:
			if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE))
			{
				Endpoint_ClearSETUP();	

				uint8_t  ReportINData[HID_MAX_REPORT_SIZE];
				uint16_t ReportINSize;
				uint8_t  ReportID = (USB_ControlRequest.wValue & 0xFF);

				memset(ReportINData, 0, sizeof(ReportINData));
				
				ReportINSize = CALLBACK_HID_Device_CreateHIDReport(HIDInterfaceInfo, &ReportID, ReportINData);

				Endpoint_Write_Control_Stream_LE(ReportINData, ReportINSize);
				Endpoint_ClearOUT();
			}
		
			break;
		case REQ_SetReport:
			if (USB_ControlRequest.bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE))
			{
				Endpoint_ClearSETUP();
				
				uint16_t ReportOUTSize = USB_ControlRequest.wLength;
				uint8_t  ReportOUTData[ReportOUTSize];
				uint8_t  ReportID = (USB_ControlRequest.wValue & 0xFF);

				Endpoint_Read_Control_Stream_LE(ReportOUTData, ReportOUTSize);
				Endpoint_ClearIN();
				
				CALLBACK_HID_Device_ProcessHIDReport(HIDInterfaceInfo, ReportID, ReportOUTData, ReportOUTSize);
			}
			
			break;
		case REQ_GetProtocol:
			if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE))
			{
				Endpoint_ClearSETUP();

				Endpoint_Write_Byte(HIDInterfaceInfo->State.UsingReportProtocol);
				Endpoint_ClearIN();

				Endpoint_ClearStatusStage();
			}
			
			break;
		case REQ_SetProtocol:
			if (USB_ControlRequest.bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE))
			{
				Endpoint_ClearSETUP();

				HIDInterfaceInfo->State.UsingReportProtocol = (USB_ControlRequest.wValue != 0x0000);
				
				Endpoint_ClearStatusStage();
			}
			
			break;
		case REQ_SetIdle:
			if (USB_ControlRequest.bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE))
			{
				if ((USB_ControlRequest.wIndex         == HIDInterfaceInfo->Config.InterfaceNumber) ||
				    (USB_ControlRequest.wValue & 0xFF) == 0)
				{
					Endpoint_ClearSETUP();
					
					HIDInterfaceInfo->State.IdleCount = ((USB_ControlRequest.wValue & 0xFF00) >> 6);
					
					Endpoint_ClearStatusStage();
				}
			}
			
			break;
		case REQ_GetIdle:
			if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE))
			{		
				Endpoint_ClearSETUP();
				
				Endpoint_Write_Byte(HIDInterfaceInfo->State.IdleCount >> 2);
				Endpoint_ClearIN();

				Endpoint_ClearStatusStage();
			}

			break;
	}
}

bool HID_Device_ConfigureEndpoints(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo)
{
	memset(&HIDInterfaceInfo->State, 0x00, sizeof(HIDInterfaceInfo->State));
	HIDInterfaceInfo->State.IdleCount = 500;
	HIDInterfaceInfo->State.UsingReportProtocol = true;

	if (!(Endpoint_ConfigureEndpoint(HIDInterfaceInfo->Config.ReportINEndpointNumber, EP_TYPE_INTERRUPT,
									 ENDPOINT_DIR_IN, HIDInterfaceInfo->Config.ReportINEndpointSize, ENDPOINT_BANK_SINGLE)))
	{
		return false;
	}
	
	return true;
}
		
void HID_Device_USBTask(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo)
{
	if (USB_DeviceState != DEVICE_STATE_Configured)
	  return;

	Endpoint_SelectEndpoint(HIDInterfaceInfo->Config.ReportINEndpointNumber);
	
	if (Endpoint_IsReadWriteAllowed())
	{
		uint8_t  ReportINData[HID_MAX_REPORT_SIZE];
		uint8_t  ReportID = 0;
		uint16_t ReportINSize;

		memset(ReportINData, 0, sizeof(ReportINData));

		ReportINSize  = CALLBACK_HID_Device_CreateHIDReport(HIDInterfaceInfo, &ReportID, ReportINData);

		bool StatesChanged     = (memcmp(ReportINData, HIDInterfaceInfo->State.PreviousReportINData, ReportINSize) != 0);
		bool IdlePeriodElapsed = (HIDInterfaceInfo->State.IdleCount && !(HIDInterfaceInfo->State.IdleMSRemaining));
		
		memcpy(HIDInterfaceInfo->State.PreviousReportINData, ReportINData, ReportINSize);

		if (ReportINSize && (StatesChanged || IdlePeriodElapsed))
		{
			HIDInterfaceInfo->State.IdleMSRemaining = HIDInterfaceInfo->State.IdleCount;

			if (ReportID)
			  Endpoint_Write_Stream_LE(&ReportID, sizeof(ReportID), NO_STREAM_CALLBACK);

			Endpoint_Write_Stream_LE(ReportINData, ReportINSize, NO_STREAM_CALLBACK);
		}
		
		Endpoint_ClearIN();
	}
}

void HID_Device_MillisecondElapsed(USB_ClassInfo_HID_Device_t* HIDInterfaceInfo)
{
	if (HIDInterfaceInfo->State.IdleMSRemaining)
	  HIDInterfaceInfo->State.IdleMSRemaining--;
}

#endif

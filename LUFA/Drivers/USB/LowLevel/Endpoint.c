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

#define  __INCLUDE_FROM_USB_DRIVER
#include "../HighLevel/USBMode.h"

#if defined(USB_CAN_BE_DEVICE)

#include "Endpoint.h"

#if !defined(FIXED_CONTROL_ENDPOINT_SIZE)
uint8_t USB_ControlEndpointSize = ENDPOINT_CONTROLEP_DEFAULT_SIZE;
#endif

bool Endpoint_ConfigureEndpoint_Prv(const uint8_t Number,
                                    const uint8_t UECFG0XData,
                                    const uint8_t UECFG1XData)
{
#if defined(CONTROL_ONLY_DEVICE)
	Endpoint_SelectEndpoint(ENDPOINT_CONTROLEP);
	Endpoint_EnableEndpoint();

	UECFG1X = 0;
	UECFG0X = UECFG0XData;
	UECFG1X = UECFG1XData;

	return Endpoint_IsConfigured();
#else
	uint8_t UECFG0XTemp[ENDPOINT_TOTAL_ENDPOINTS];
	uint8_t UECFG1XTemp[ENDPOINT_TOTAL_ENDPOINTS];
	uint8_t UEIENXTemp[ENDPOINT_TOTAL_ENDPOINTS];
	
	for (uint8_t EPNum = 0; EPNum < ENDPOINT_TOTAL_ENDPOINTS; EPNum++)
	{
		Endpoint_SelectEndpoint(EPNum);
		UECFG0XTemp[EPNum] = UECFG0X;
		UECFG1XTemp[EPNum] = UECFG1X;
		UEIENXTemp[EPNum]  = UEIENX;
	}
	
	UECFG0XTemp[Number] = UECFG0XData;
	UECFG1XTemp[Number] = UECFG1XData;
	UEIENXTemp[Number]  = 0;
	
	for (uint8_t EPNum = 1; EPNum < ENDPOINT_TOTAL_ENDPOINTS; EPNum++)
	{
		Endpoint_SelectEndpoint(EPNum);	
		UEIENX  = 0;
		UEINTX  = 0;
		UECFG1X = 0;
		Endpoint_DisableEndpoint();
	}

	for (uint8_t EPNum = 0; EPNum < ENDPOINT_TOTAL_ENDPOINTS; EPNum++)
	{
		if (!(UECFG1XTemp[EPNum] & (1 << ALLOC)))
		  continue;
		
		Endpoint_SelectEndpoint(EPNum);		
		Endpoint_EnableEndpoint();

		UECFG0X = UECFG0XTemp[EPNum];
		UECFG1X = UECFG1XTemp[EPNum];
		UEIENX  = UEIENXTemp[EPNum];
		
		if (!(Endpoint_IsConfigured()))
		  return false;
	}
	
	Endpoint_SelectEndpoint(Number);
	return true;
#endif
}

void Endpoint_ClearEndpoints(void)
{
	UEINT = 0;

	for (uint8_t EPNum = 0; EPNum < ENDPOINT_TOTAL_ENDPOINTS; EPNum++)
	{
		Endpoint_SelectEndpoint(EPNum);	
		UEIENX  = 0;
		UEINTX  = 0;
		UECFG1X = 0;
		Endpoint_DisableEndpoint();
	}
}

void Endpoint_ClearStatusStage(void)
{
	if (USB_ControlRequest.bmRequestType & REQDIR_DEVICETOHOST)
	{
		while (!(Endpoint_IsOUTReceived()))
		{
			if (USB_DeviceState == DEVICE_STATE_Unattached)
			  return;
		}

		Endpoint_ClearOUT();
	}
	else
	{
		while (!(Endpoint_IsINReady()))
		{
			if (USB_DeviceState == DEVICE_STATE_Unattached)
			  return;
		}
		
		Endpoint_ClearIN();
	}
}

#if !defined(CONTROL_ONLY_DEVICE)
uint8_t Endpoint_WaitUntilReady(void)
{
	#if (USB_STREAM_TIMEOUT_MS < 0xFF)
	uint8_t  TimeoutMSRem = USB_STREAM_TIMEOUT_MS;	
	#else
	uint16_t TimeoutMSRem = USB_STREAM_TIMEOUT_MS;
	#endif

	uint16_t PreviousFrameNumber = USB_Device_GetFrameNumber();

	for (;;)
	{
		if (Endpoint_GetEndpointDirection() == ENDPOINT_DIR_IN)
		{
			if (Endpoint_IsINReady())
			  return ENDPOINT_READYWAIT_NoError;
		}
		else
		{
			if (Endpoint_IsOUTReceived())
			  return ENDPOINT_READYWAIT_NoError;
		}
		
		if (USB_DeviceState == DEVICE_STATE_Unattached)
		  return ENDPOINT_READYWAIT_DeviceDisconnected;
		else if (USB_DeviceState == DEVICE_STATE_Suspended)
		  return ENDPOINT_READYWAIT_BusSuspended;
		else if (Endpoint_IsStalled())
		  return ENDPOINT_READYWAIT_EndpointStalled;

		uint16_t CurrentFrameNumber = USB_Device_GetFrameNumber();

		if (CurrentFrameNumber != PreviousFrameNumber)
		{
			PreviousFrameNumber = CurrentFrameNumber;

			if (!(TimeoutMSRem--))
			  return ENDPOINT_READYWAIT_Timeout;
		}
	}
}
#endif

#endif

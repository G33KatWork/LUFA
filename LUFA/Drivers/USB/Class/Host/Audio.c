/*
             LUFA Library
     Copyright (C) Dean Camera, 2011.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2011  Dean Camera (dean [at] fourwalledcubicle [dot] com)

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
#include "../../Core/USBMode.h"

#if defined(USB_CAN_BE_HOST)

#define  __INCLUDE_FROM_AUDIO_DRIVER
#define  __INCLUDE_FROM_AUDIO_HOST_C
#include "Audio.h"

uint8_t Audio_Host_ConfigurePipes(USB_ClassInfo_Audio_Host_t* const AudioInterfaceInfo,
                                  uint16_t ConfigDescriptorSize,
                                  void* ConfigDescriptorData)
{
	USB_Descriptor_Endpoint_t*  DataINEndpoint          = NULL;
	USB_Descriptor_Endpoint_t*  DataOUTEndpoint         = NULL;
	USB_Descriptor_Interface_t* AudioControlInterface   = NULL;
	USB_Descriptor_Interface_t* AudioStreamingInterface = NULL;

	memset(&AudioInterfaceInfo->State, 0x00, sizeof(AudioInterfaceInfo->State));

	if (DESCRIPTOR_TYPE(ConfigDescriptorData) != DTYPE_Configuration)
	  return AUDIO_ENUMERROR_InvalidConfigDescriptor;

	while (!(DataINEndpoint) || !(DataOUTEndpoint))
	{
		if (!(AudioControlInterface) ||
		    USB_GetNextDescriptorComp(&ConfigDescriptorSize, &ConfigDescriptorData,
		                              DComp_NextAudioInterfaceDataEndpoint) != DESCRIPTOR_SEARCH_COMP_Found)
		{
			if (!(AudioControlInterface) ||
			    USB_GetNextDescriptorComp(&ConfigDescriptorSize, &ConfigDescriptorData,
			                              DComp_NextAudioStreamInterface) != DESCRIPTOR_SEARCH_COMP_Found)
			{
				if (USB_GetNextDescriptorComp(&ConfigDescriptorSize, &ConfigDescriptorData,
											  DComp_NextAudioControlInterface) != DESCRIPTOR_SEARCH_COMP_Found)
				{
					return AUDIO_ENUMERROR_NoCompatibleInterfaceFound;
				}

				AudioControlInterface = DESCRIPTOR_PCAST(ConfigDescriptorData, USB_Descriptor_Interface_t);			

				if (USB_GetNextDescriptorComp(&ConfigDescriptorSize, &ConfigDescriptorData,
										  DComp_NextAudioStreamInterface) != DESCRIPTOR_SEARCH_COMP_Found)
				{
					return AUDIO_ENUMERROR_NoCompatibleInterfaceFound;
				}
			}

			AudioStreamingInterface = DESCRIPTOR_PCAST(ConfigDescriptorData, USB_Descriptor_Interface_t);

			continue;
		}

		USB_Descriptor_Endpoint_t* EndpointData = DESCRIPTOR_PCAST(ConfigDescriptorData, USB_Descriptor_Endpoint_t);

		if (EndpointData->EndpointAddress & ENDPOINT_DESCRIPTOR_DIR_IN)
		  DataINEndpoint  = EndpointData;
		else
		  DataOUTEndpoint = EndpointData;
	}

	for (uint8_t PipeNum = 1; PipeNum < PIPE_TOTAL_PIPES; PipeNum++)
	{
		uint16_t Size;
		uint8_t  Type;
		uint8_t  Token;
		uint8_t  EndpointAddress;
		bool     DoubleBanked;

		if (PipeNum == AudioInterfaceInfo->Config.DataINPipeNumber)
		{
			Size            = DataINEndpoint->EndpointSize;
			EndpointAddress = DataINEndpoint->EndpointAddress;
			Token           = PIPE_TOKEN_IN;
			Type            = EP_TYPE_BULK;
			DoubleBanked    = true;

			AudioInterfaceInfo->State.DataINPipeSize = DataINEndpoint->EndpointSize;
		}
		else if (PipeNum == AudioInterfaceInfo->Config.DataOUTPipeNumber)
		{
			Size            = DataOUTEndpoint->EndpointSize;
			EndpointAddress = DataOUTEndpoint->EndpointAddress;
			Token           = PIPE_TOKEN_OUT;
			Type            = EP_TYPE_ISOCHRONOUS;
			DoubleBanked    = true;

			AudioInterfaceInfo->State.DataOUTPipeSize = DataOUTEndpoint->EndpointSize;
		}
		else
		{
			continue;
		}
		
		if (!(Pipe_ConfigurePipe(PipeNum, Type, Token, EndpointAddress, Size,
		                         DoubleBanked ? PIPE_BANK_DOUBLE : PIPE_BANK_SINGLE)))
		{
			return AUDIO_ENUMERROR_PipeConfigurationFailed;
		}
	}

	AudioInterfaceInfo->State.ControlInterfaceNumber    = AudioControlInterface->InterfaceNumber;
	AudioInterfaceInfo->State.StreamingInterfaceNumber  = AudioStreamingInterface->InterfaceNumber;
	AudioInterfaceInfo->State.EnabledStreamingAltIndex  = AudioStreamingInterface->AlternateSetting;
	AudioInterfaceInfo->State.IsActive = true;
	
	return AUDIO_ENUMERROR_NoError;
}

static uint8_t DComp_NextAudioControlInterface(void* CurrentDescriptor)
{
	USB_Descriptor_Header_t* Header = DESCRIPTOR_PCAST(CurrentDescriptor, USB_Descriptor_Header_t);

	if (Header->Type == DTYPE_Interface)
	{
		USB_Descriptor_Interface_t* Interface = DESCRIPTOR_PCAST(CurrentDescriptor, USB_Descriptor_Interface_t);

		if ((Interface->Class    == AUDIO_CSCP_AudioClass) &&
		    (Interface->SubClass == AUDIO_CSCP_ControlSubclass) &&
		    (Interface->Protocol == AUDIO_CSCP_ControlProtocol))
		{
			return DESCRIPTOR_SEARCH_Found;
		}
	}

	return DESCRIPTOR_SEARCH_NotFound;
}

static uint8_t DComp_NextAudioStreamInterface(void* CurrentDescriptor)
{
	USB_Descriptor_Header_t* Header = DESCRIPTOR_PCAST(CurrentDescriptor, USB_Descriptor_Header_t);

	if (Header->Type == DTYPE_Interface)
	{
		USB_Descriptor_Interface_t* Interface = DESCRIPTOR_PCAST(CurrentDescriptor, USB_Descriptor_Interface_t);

		if ((Interface->Class    == AUDIO_CSCP_AudioClass) &&
		    (Interface->SubClass == AUDIO_CSCP_AudioStreamingSubclass) &&
		    (Interface->Protocol == AUDIO_CSCP_StreamingProtocol))
		{
			return DESCRIPTOR_SEARCH_Found;
		}
	}

	return DESCRIPTOR_SEARCH_NotFound;
}

static uint8_t DComp_NextAudioInterfaceDataEndpoint(void* CurrentDescriptor)
{
	USB_Descriptor_Header_t* Header = DESCRIPTOR_PCAST(CurrentDescriptor, USB_Descriptor_Header_t);

	if (Header->Type == DTYPE_Endpoint)
	{
		USB_Descriptor_Endpoint_t* Endpoint = DESCRIPTOR_PCAST(CurrentDescriptor, USB_Descriptor_Endpoint_t);

		if ((Endpoint->Attributes & EP_TYPE_MASK) == EP_TYPE_ISOCHRONOUS)
		  return DESCRIPTOR_SEARCH_Found;
	}
	else if (Header->Type == DTYPE_Interface)
	{
		return DESCRIPTOR_SEARCH_Fail;
	}

	return DESCRIPTOR_SEARCH_NotFound;
}

uint8_t AUDIO_Host_StartStopStreaming(USB_ClassInfo_Audio_Host_t* const AudioInterfaceInfo,
			                          bool EnableStreaming)
{
	if (!(AudioInterfaceInfo->State.IsActive))
	  return HOST_SENDCONTROL_DeviceDisconnected;

	return USB_Host_SetInterfaceAltSetting(AudioInterfaceInfo->State.StreamingInterfaceNumber,
	                                       EnableStreaming ? AudioInterfaceInfo->State.EnabledStreamingAltIndex : 0);
}

#endif


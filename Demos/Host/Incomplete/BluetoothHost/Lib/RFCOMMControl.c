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
 *  RFCOMM multiplexer control layer module. This module handles multiplexer
 *  channel commands to the control DLCI in the RFCOMM layer, to open, configure,
 *  test and close logical RFCOMM channels.
 */

#define  INCLUDE_FROM_RFCOMM_CONTROL_C
#include "RFCOMMControl.h"

void RFCOMM_ProcessControlCommand(const uint8_t* Command, Bluetooth_Channel_t* const Channel)
{
	const RFCOMM_Command_t* CommandHeader  = (const RFCOMM_Command_t*)Command;
	const uint8_t*          CommandData    = (const uint8_t*)Command + sizeof(RFCOMM_Command_t);
	uint16_t                ControlDataLen = RFCOMM_GetFrameDataLength(CommandData);

	/* Adjust the command data pointer to skip over the variable size field */
	CommandData += (ControlDataLen < 128) ? 1 : 2;

	switch (CommandHeader->Command)
	{
		case RFCOMM_Control_Test:
			RFCOMM_ProcessTestCommand(CommandHeader, CommandData, Channel);
			break;
		case RFCOMM_Control_FlowControlEnable:
			RFCOMM_ProcessFCECommand(CommandHeader, CommandData, Channel);
			break;
		case RFCOMM_Control_FlowControlDisable:
			RFCOMM_ProcessFCDCommand(CommandHeader, CommandData, Channel);
			break;
		case RFCOMM_Control_ModemStatus:
			RFCOMM_ProcessMSCommand(CommandHeader, CommandData, Channel);
			break;
		case RFCOMM_Control_RemotePortNegotiation:
			RFCOMM_ProcessRPNCommand(CommandHeader, CommandData, Channel);
			break;
		case RFCOMM_Control_RemoteLineStatus:
			RFCOMM_ProcessRLSCommand(CommandHeader, CommandData, Channel);
			break;
		case RFCOMM_Control_DLCParameterNegotiation:
			RFCOMM_ProcessDPNCommand(CommandHeader, CommandData, Channel);
			break;
		default:
			BT_RFCOMM_DEBUG(1, "<< Unknown Command");		
			break;
	}
}

static void RFCOMM_ProcessTestCommand(const RFCOMM_Command_t* const CommandHeader, const uint8_t* CommandData,
			                          Bluetooth_Channel_t* const Channel)
{
	BT_RFCOMM_DEBUG(1, "<< TEST Command");
}

static void RFCOMM_ProcessFCECommand(const RFCOMM_Command_t* const CommandHeader, const uint8_t* CommandData,
			                         Bluetooth_Channel_t* const Channel)
{
	BT_RFCOMM_DEBUG(1, "<< FCE Command");
}

static void RFCOMM_ProcessFCDCommand(const RFCOMM_Command_t* const CommandHeader, const uint8_t* CommandData,
			                         Bluetooth_Channel_t* const Channel)
{
	BT_RFCOMM_DEBUG(1, "<< FCD Command");
}

static void RFCOMM_ProcessMSCommand(const RFCOMM_Command_t* const CommandHeader, const uint8_t* CommandData,
			                        Bluetooth_Channel_t* const Channel)
{
	BT_RFCOMM_DEBUG(1, "<< MS Command");
}

static void RFCOMM_ProcessRPNCommand(const RFCOMM_Command_t* const CommandHeader, const uint8_t* CommandData,
			                         Bluetooth_Channel_t* const Channel)
{
	BT_RFCOMM_DEBUG(1, "<< RPN Command");
}

static void RFCOMM_ProcessRLSCommand(const RFCOMM_Command_t* const CommandHeader, const uint8_t* CommandData,
			                         Bluetooth_Channel_t* const Channel)
{
	BT_RFCOMM_DEBUG(1, "<< RLS Command");
}

static void RFCOMM_ProcessDPNCommand(const RFCOMM_Command_t* const CommandHeader, const uint8_t* CommandData,
			                         Bluetooth_Channel_t* const Channel)
{
	const RFCOMM_DPN_Parameters_t* Params = (const RFCOMM_DPN_Parameters_t*)CommandData;

	BT_RFCOMM_DEBUG(1, "<< DPN Command");
	BT_RFCOMM_DEBUG(2, "-- Config DLCI: 0x%02X", Params->DLCI);
	
	/* Ignore parameter negotiations to the control channel */
	if (Params->DLCI == RFCOMM_CONTROL_DLCI)
	  return;
	
	/* Retrieve existing channel configuration data, if already opened */
	RFCOMM_Channel_t* RFCOMMChannel = RFCOMM_GetChannelData(Params->DLCI);
	
	/* Check if the channel has no corresponding entry - remote did not open it first */
	if (RFCOMMChannel == NULL)
	{
		/* Find a free entry in the RFCOMM channel multiplexer state array */
		for (uint8_t i = 0; i < RFCOMM_MAX_OPEN_CHANNELS; i++)
		{
			/* If the channel's DLCI is zero, the channel state entry is free */
			if (!(RFCOMM_Channels[i].DLCI))
			{
				RFCOMMChannel       = &RFCOMM_Channels[i];
				RFCOMMChannel->DLCI = Params->DLCI;
				break;
			}
		}
		
		/* No free entry was found, discard the request */
		if (RFCOMMChannel == NULL)
		{
			BT_RFCOMM_DEBUG(2, "-- No Free Channel");
			return;
		}
	}
	
	/* Save the new channel configuration */
	RFCOMMChannel->State       = RFCOMM_Channel_Open;
	RFCOMMChannel->Priority    = Params->Priority;
	RFCOMMChannel->UseUIFrames = (Params->FrameType != 0);
	RFCOMMChannel->RemoteMTU   = Params->MaximumFrameSize;
	
	struct
	{
		RFCOMM_Command_t        CommandHeader;
		uint8_t                 Length;
		RFCOMM_DPN_Parameters_t Params;
	} DPNResponse;
	
	/* Fill out the DPN response data */
	DPNResponse.CommandHeader.Command = CommandHeader->Command;
	DPNResponse.CommandHeader.EA      = true;
	DPNResponse.Length                = (sizeof(DPNResponse.Params) << 1) | 0x01;
	DPNResponse.Params                = *Params;
	
	BT_RFCOMM_DEBUG(1, ">> DPN Response");

	/* Send the PDN response to acknowledge the command */
	RFCOMM_SendFrame(RFCOMM_CONTROL_DLCI, false, RFCOMM_Frame_UIH, sizeof(DPNResponse), &DPNResponse, Channel);
}

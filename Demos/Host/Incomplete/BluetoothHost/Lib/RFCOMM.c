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
 *  RFCOMM layer module. This module manages the RFCOMM layer of the
 *  stack, providing virtual serial port channels on top of the lower
 *  L2CAP layer.
 */

#define  INCLUDE_FROM_RFCOMM_C
#include "RFCOMM.h"

/** 8-Bit CRC table used by the FCS field of each RFCOMM encoded frame, sourced from the ETSI TS 101 369 V7.2.0
 *  specification document, upon which the RFCOMM specification is based.
 */
const uint8_t CRC8_Table[256] PROGMEM =
	{
		0x00, 0x91, 0xE3, 0x72, 0x07, 0x96, 0xE4, 0x75, 0x0E, 0x9F, 0xED, 0x7C, 0x09, 0x98, 0xEA, 0x7B,
		0x1C, 0x8D, 0xFF, 0x6E, 0x1B, 0x8A, 0xF8, 0x69, 0x12, 0x83, 0xF1, 0x60, 0x15, 0x84, 0xF6, 0x67,
		0x38, 0xA9, 0xDB, 0x4A, 0x3F, 0xAE, 0xDC, 0x4D, 0x36, 0xA7, 0xD5, 0x44, 0x31, 0xA0, 0xD2, 0x43,
		0x24, 0xB5, 0xC7, 0x56, 0x23, 0xB2, 0xC0, 0x51, 0x2A, 0xBB, 0xC9, 0x58, 0x2D, 0xBC, 0xCE, 0x5F,
		0x70, 0xE1, 0x93, 0x02, 0x77, 0xE6, 0x94, 0x05, 0x7E, 0xEF, 0x9D, 0x0C, 0x79, 0xE8, 0x9A, 0x0B,
		0x6C, 0xFD, 0x8F, 0x1E, 0x6B, 0xFA, 0x88, 0x19, 0x62, 0xF3, 0x81, 0x10, 0x65, 0xF4, 0x86, 0x17,
		0x48, 0xD9, 0xAB, 0x3A, 0x4F, 0xDE, 0xAC, 0x3D, 0x46, 0xD7, 0xA5, 0x34, 0x41, 0xD0, 0xA2, 0x33,
		0x54, 0xC5, 0xB7, 0x26, 0x53, 0xC2, 0xB0, 0x21, 0x5A, 0xCB, 0xB9, 0x28, 0x5D, 0xCC, 0xBE, 0x2F,
		0xE0, 0x71, 0x03, 0x92, 0xE7, 0x76, 0x04, 0x95, 0xEE, 0x7F, 0x0D, 0x9C, 0xE9, 0x78, 0x0A, 0x9B,
		0xFC, 0x6D, 0x1F, 0x8E, 0xFB, 0x6A, 0x18, 0x89, 0xF2, 0x63, 0x11, 0x80, 0xF5, 0x64, 0x16, 0x87,
		0xD8, 0x49, 0x3B, 0xAA, 0xDF, 0x4E, 0x3C, 0xAD, 0xD6, 0x47, 0x35, 0xA4, 0xD1, 0x40, 0x32, 0xA3,
		0xC4, 0x55, 0x27, 0xB6, 0xC3, 0x52, 0x20, 0xB1, 0xCA, 0x5B, 0x29, 0xB8, 0xCD, 0x5C, 0x2E, 0xBF,
		0x90, 0x01, 0x73, 0xE2, 0x97, 0x06, 0x74, 0xE5, 0x9E, 0x0F, 0x7D, 0xEC, 0x99, 0x08, 0x7A, 0xEB,
		0x8C, 0x1D, 0x6F, 0xFE, 0x8B, 0x1A, 0x68, 0xF9, 0x82, 0x13, 0x61, 0xF0, 0x85, 0x14, 0x66, 0xF7,
		0xA8, 0x39, 0x4B, 0xDA, 0xAF, 0x3E, 0x4C, 0xDD, 0xA6, 0x37, 0x45, 0xD4, 0xA1, 0x30, 0x42, 0xD3,
		0xB4, 0x25, 0x57, 0xC6, 0xB3, 0x22, 0x50, 0xC1, 0xBA, 0x2B, 0x59, 0xC8, 0xBD, 0x2C, 0x5E, 0xCF
	};

/** Initializes the RFCOMM service, ready for new connections from a SDP client. */
void RFCOMM_Initialize(void)
{
	/* Not currently used */
}

void RFCOMM_ProcessPacket(void* Data, Bluetooth_Channel_t* const Channel)
{
	const RFCOMM_Header_t* FrameHeader = (const RFCOMM_Header_t*)Data;
	
	/* Decode the RFCOMM frame type from the header */
	switch (FrameHeader->Control & ~FRAME_POLL_FINAL)
	{
		case RFCOMM_Frame_SABM:
			RFCOMM_ProcessSABM(FrameHeader, Channel);
			break;
		case RFCOMM_Frame_UA:
			RFCOMM_ProcessUA(FrameHeader, Channel);
			break;
		case RFCOMM_Frame_DM:
			RFCOMM_ProcessDM(FrameHeader, Channel);
			break;
		case RFCOMM_Frame_DISC:
			RFCOMM_ProcessDISC(FrameHeader, Channel);
			break;
		case RFCOMM_Frame_UIH:
			RFCOMM_ProcessUIH(FrameHeader, Channel);
			break;
		default:
			BT_RFCOMM_DEBUG(1, "<< Unknown Frame Type");
			break;
	}
}

static void RFCOMM_ProcessSABM(const RFCOMM_Header_t* const FrameHeader, Bluetooth_Channel_t* const Channel)
{
	BT_RFCOMM_DEBUG(1, "<< SABM Received");
	BT_RFCOMM_DEBUG(2, "-- DLCI 0x%02X", FrameHeader->Address.DLCI);
	
	// TODO: Reset channel send/receive state here
	
	BT_RFCOMM_DEBUG(1, ">> UA Sent");
	RFCOMM_SendFrame(FrameHeader->Address.DLCI, true, (RFCOMM_Frame_UA | FRAME_POLL_FINAL), 0, NULL, Channel);
}

static void RFCOMM_ProcessUA(const RFCOMM_Header_t* const FrameHeader, Bluetooth_Channel_t* const Channel)
{
	const uint8_t* CurrBufferPos = ((const uint8_t*)FrameHeader + sizeof(RFCOMM_Header_t));

	BT_RFCOMM_DEBUG(1, "<< UA Received");
	BT_RFCOMM_DEBUG(2, "-- Address 0x%02X", FrameHeader->Address);
}

static void RFCOMM_ProcessDM(const RFCOMM_Header_t* const FrameHeader, Bluetooth_Channel_t* const Channel)
{
	const uint8_t* CurrBufferPos = ((const uint8_t*)FrameHeader + sizeof(RFCOMM_Header_t));

	BT_RFCOMM_DEBUG(1, "<< DM Received");
	BT_RFCOMM_DEBUG(2, "-- Address 0x%02X", FrameHeader->Address);
}

static void RFCOMM_ProcessDISC(const RFCOMM_Header_t* const FrameHeader, Bluetooth_Channel_t* const Channel)
{
	const uint8_t* CurrBufferPos = ((const uint8_t*)FrameHeader + sizeof(RFCOMM_Header_t));

	BT_RFCOMM_DEBUG(1, "<< DISC Received");
	BT_RFCOMM_DEBUG(2, "-- Address 0x%02X", FrameHeader->Address);
}

static void RFCOMM_ProcessUIH(const RFCOMM_Header_t* const FrameHeader, Bluetooth_Channel_t* const Channel)
{
	const uint8_t* CurrBufferPos = ((const uint8_t*)FrameHeader + sizeof(RFCOMM_Header_t));

	BT_RFCOMM_DEBUG(1, "<< UIH Received");
	BT_RFCOMM_DEBUG(2, "-- Address 0x%02X", FrameHeader->Address);
}

static void RFCOMM_SendFrame(const uint8_t DLCI, const bool CommandResponse, const uint8_t Control, const uint16_t DataLen, const uint8_t* Data,
                             Bluetooth_Channel_t* const Channel)
{
	struct
	{
		RFCOMM_Header_t FrameHeader;
		uint8_t         Size[(DataLen < 128) ? 1 : 2];
		uint8_t         Data[DataLen];
		uint8_t         FCS;
	} ResponsePacket;
	
	/* Set the frame header values to the specified address and frame type */
	ResponsePacket.FrameHeader.Control = Control;
	ResponsePacket.FrameHeader.Address = (RFCOMM_Address_t)
		{
			.EA   = true,
			.CR   = CommandResponse,
			.DLCI = DLCI,
		};
	
	/* Set the lower 7 bits of the packet length */
	ResponsePacket.Size[0] = (DataLen << 1);
	
	/* Terminate the size field if size is 7 bits or lower, otherwise set the upper 8 bits of the length */
	if (DataLen < 128)
	  ResponsePacket.Size[0] |= 0x01;
	else
	  ResponsePacket.Size[1]  = (DataLen >> 7);
	
	/* Copy over the packet data from the source buffer to the response packet buffer */
	memcpy(ResponsePacket.Data, Data, DataLen);
	
	/* Calculate the frame checksum from the appropriate fields */
	if ((Control & ~FRAME_POLL_FINAL) == RFCOMM_Frame_UIH)
	  ResponsePacket.FCS = RFCOMM_GetFCSValue(&ResponsePacket, sizeof(ResponsePacket.FrameHeader));
	else
	  ResponsePacket.FCS = RFCOMM_GetFCSValue(&ResponsePacket, sizeof(ResponsePacket.FrameHeader) + sizeof(ResponsePacket.Size));

	/* Send the completed response packet to the sender */
	Bluetooth_SendPacket(&ResponsePacket, sizeof(ResponsePacket), Channel);
}

static uint8_t RFCOMM_GetFCSValue(const void* FrameStart, uint16_t Length)
{
	const uint8_t* CurrPos = FrameStart;
	uint8_t        FCS     = 0xFF;
	
	while (Length--)
	{
		FCS = pgm_read_byte(&CRC8_Table[FCS ^ *CurrPos]);
		CurrPos++;
	}

	return (0xFF - FCS);
}

static uint16_t RFCOMM_GetFrameDataLength(const uint8_t** BufferPos)
{
	uint8_t FirstOctet  = *((*BufferPos)++);
	uint8_t SecondOctet = 0;
	
	if (!(FirstOctet & 0x01))
	  SecondOctet = *((*BufferPos)++);
	
	return (((uint16_t)SecondOctet << 7) | (FirstOctet >> 1));
}


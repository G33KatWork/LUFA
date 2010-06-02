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

#include "SDPServices.h"

const struct
{
	uint8_t  Header;
	uint32_t Data;
} PROGMEM RFCOMM_Attribute_ServiceHandle =
	{
		(SDP_DATATYPE_UnsignedInt | SDP_DATASIZE_32Bit),
		SWAPENDIAN_32(0x00010001),
	};

const struct
{
	uint8_t    Header;
	uint16_t   Size;
	ItemUUID_t UUIDList[];
} PROGMEM RFCOMM_Attribute_ServiceClassIDs =
	{
		(SDP_DATATYPE_Sequence | SDP_DATASIZE_Variable16Bit),
		SWAPENDIAN_16(sizeof(ItemUUID_t) * 1),
		{
			{(SDP_DATATYPE_UUID | SDP_DATASIZE_128Bit), SP_CLASS_UUID}
		}
	};

const struct
{
	uint8_t        Header;
	uint16_t       Size;

	ItemProtocol_t ProtocolList[];
} PROGMEM RFCOMM_Attribute_ProtocolDescriptor =
	{
		(SDP_DATATYPE_Sequence | SDP_DATASIZE_Variable16Bit),
		SWAPENDIAN_16(sizeof(ItemProtocol_t) * 2),
		{
			{
				(SDP_DATATYPE_Sequence | SDP_DATASIZE_Variable8Bit),
				sizeof(ItemUUID_t),
				{
					{(SDP_DATATYPE_UUID | SDP_DATASIZE_128Bit), L2CAP_UUID},
				}
			},
			{
				(SDP_DATATYPE_Sequence | SDP_DATASIZE_Variable8Bit),
				sizeof(ItemUUID_t),
				{
					{(SDP_DATATYPE_UUID | SDP_DATASIZE_128Bit), RFCOMM_UUID},
				}
			},
		}
	};

const struct
{
	uint8_t    Header;
	uint16_t   Size;
	ItemUUID_t UUIDList[];
} PROGMEM RFCOMM_Attribute_BrowseGroupList =
	{
		(SDP_DATATYPE_Sequence | SDP_DATASIZE_Variable16Bit),
		SWAPENDIAN_16(sizeof(ItemUUID_t) * 1),
		{
			{(SDP_DATATYPE_UUID | SDP_DATASIZE_128Bit), PUBLICBROWSEGROUP_CLASS_UUID}
		}
	};
	
const struct
{
	uint8_t      Header;
	uint8_t      Size;
	ItemLangID_t OffsetList[];
} PROGMEM RFCOMM_Attribute_LanguageBaseIDOffset =
	{
		.Header = (SDP_DATATYPE_Sequence | SDP_DATASIZE_Variable8Bit),
		.Size   = (sizeof(ItemLangID_t) * 1),
		.OffsetList =
			{
				{
					{(SDP_DATATYPE_UnsignedInt | SDP_DATASIZE_16Bit), SWAPENDIAN_16(0x454E)},
					{(SDP_DATATYPE_UnsignedInt | SDP_DATASIZE_16Bit), SWAPENDIAN_16(0x006A)},
					{(SDP_DATATYPE_UnsignedInt | SDP_DATASIZE_16Bit), SWAPENDIAN_16(0x0100)},
				}
			}
	};	
	
const struct
{
	uint8_t Header;
	uint8_t Size;
	char    Text[];
} PROGMEM RFCOMM_Attribute_ServiceName =
	{
		(SDP_DATATYPE_String | SDP_DATASIZE_Variable8Bit),
		sizeof("Serial Port") - 1,
		"Serial Port",
	};

const struct
{
	uint8_t Header;
	uint8_t Size;
	char    Text[];
} PROGMEM RFCOMM_Attribute_ServiceDescription =
	{
		(SDP_DATATYPE_String | SDP_DATASIZE_Variable8Bit),
		sizeof("Wireless Serial Port Service") - 1,
		"Wireless Serial Port Service",
	};

const ServiceAttributeTable_t PROGMEM RFCOMM_Attribute_Table[] =
	{
		{.AttributeID = SDP_ATTRIBUTE_ID_SERVICERECORDHANDLE,    .Data = &RFCOMM_Attribute_ServiceHandle       },
		{.AttributeID = SDP_ATTRIBUTE_ID_SERVICECLASSIDS,        .Data = &RFCOMM_Attribute_ServiceClassIDs     },
		{.AttributeID = SDP_ATTRIBUTE_ID_PROTOCOLDESCRIPTORLIST, .Data = &RFCOMM_Attribute_ProtocolDescriptor  },
		{.AttributeID = SDP_ATTRIBUTE_ID_BROWSEGROUPLIST,        .Data = &RFCOMM_Attribute_BrowseGroupList     },
		{.AttributeID = SDP_ATTRIBUTE_ID_LANGUAGEBASEATTROFFSET, .Data = &RFCOMM_Attribute_LanguageBaseIDOffset},
		{.AttributeID = SDP_ATTRIBUTE_ID_SERVICENAME,            .Data = &RFCOMM_Attribute_ServiceName         },
		{.AttributeID = SDP_ATTRIBUTE_ID_SERVICEDESCRIPTION,     .Data = &RFCOMM_Attribute_ServiceDescription  },

		SERVICE_ATTRIBUTE_TABLE_TERMINATOR
	};

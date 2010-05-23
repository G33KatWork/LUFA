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

#define  INCLUDE_FROM_SERVICEDISCOVERYPROTOCOL_C
#include "ServiceDiscoveryProtocol.h"

const struct
{
	uint8_t  Header;
	uint32_t Data;
} PROGMEM ServiceDiscoveryServer_Attribute_ServiceHandle = {(SDP_DATATYPE_UnsignedInt | SDP_DATASIZE_32Bit), 0x00000000};

const struct
{
	uint8_t     Header;
	uint16_t    Size;
	ClassUUID_t UUIDList[];
} PROGMEM ServiceDiscoveryServer_Attribute_ServiceClassIDs =
	{
		(SDP_DATATYPE_Sequence | SDP_DATASIZE_Variable16Bit), (sizeof(ClassUUID_t) * 1),
		{
			{.Header = (SDP_DATATYPE_UUID | SDP_DATASIZE_128Bit), .UUID = {BASE_96BIT_UUID, 0x01, 0x00, 0x00, 0x00}}
		}
	};

const struct
{
	uint8_t  Header;
	uint16_t Data;
} PROGMEM ServiceDiscoveryServer_Attribute_Version = {(SDP_DATATYPE_UnsignedInt | SDP_DATASIZE_16Bit), 0x0100};

/** Service Discovery Protocol attribute table, listing all supported attributes of the service. */
const ServiceAttributeTable_t ServiceDiscoveryServer_Attribute_Table[] PROGMEM =
	{
		{.AttributeID = SDP_ATTRIBUTE_ID_SERVICERECORDHANDLE, .Data = &ServiceDiscoveryServer_Attribute_ServiceHandle   },
		{.AttributeID = SDP_ATTRIBUTE_ID_SERVICECLASSIDS,     .Data = &ServiceDiscoveryServer_Attribute_ServiceClassIDs },
		{.AttributeID = SDP_ATTRIBUTE_ID_VERSION,             .Data = &ServiceDiscoveryServer_Attribute_Version         },

		SERVICE_ATTRIBUTE_TABLE_TERMINATOR
	};

/** Service Discovery Protocol attribute, indicationg the service's name. */
const struct
{
	uint8_t Header;
	uint8_t Length;
	uint8_t Data[];
} PROGMEM SDP_Attribute_Name = {(SDP_DATATYPE_String | SDP_DATASIZE_Variable8Bit), sizeof("SDP"), "SDP"};

/** Service Discovery Protocol attribute, indicationg the service's description. */
const struct
{
	uint8_t Header;
	uint8_t Length;
	uint8_t Data[];
} PROGMEM SDP_Attribute_Description = {(SDP_DATATYPE_String | SDP_DATASIZE_Variable8Bit), sizeof("BT Service Discovery"), "BT Service Discovery"};

/** Service Discovery Protocol attribute, indicationg the service's availability. */
const struct
{
	uint8_t Header;
	uint8_t Data;
} PROGMEM SDP_Attribute_Availability = {(SDP_DATATYPE_UnsignedInt | SDP_DATASIZE_8Bit), 0xFF};

const struct
{
	uint8_t  Header;
	uint16_t Data;
} PROGMEM SDP_Attribute_LanguageOffset = {(SDP_DATATYPE_UnsignedInt | SDP_DATASIZE_16Bit), SDP_ATTRIBUTE_LANGOFFSET};

const struct
{
	uint8_t  Header;
	uint32_t Data;
} PROGMEM SDP_Attribute_ServiceHandle = {(SDP_DATATYPE_UnsignedInt | SDP_DATASIZE_32Bit), 0x00000001};

const struct
{
	uint8_t     Header;
	uint16_t    Size;
	ClassUUID_t UUIDList[];
} PROGMEM SDP_Attribute_ServiceClassIDs =
	{
		(SDP_DATATYPE_Sequence | SDP_DATASIZE_Variable16Bit), (sizeof(ClassUUID_t) * 1),
		{
			{.Header = (SDP_DATATYPE_UUID | SDP_DATASIZE_128Bit), .UUID = {BASE_96BIT_UUID, 0x00, 0x00, 0x00, 0x01}}
		}
	};

/** Service Discovery Protocol attribute table, listing all supported attributes of the service. */
const ServiceAttributeTable_t SDP_Attribute_Table[] PROGMEM =
	{
		{.AttributeID = SDP_ATTRIBUTE_ID_SERVICERECORDHANDLE, .Data = &SDP_Attribute_ServiceHandle    },
		{.AttributeID = SDP_ATTRIBUTE_ID_SERVICECLASSIDS,     .Data = &SDP_Attribute_ServiceClassIDs  },
		{.AttributeID = SDP_ATTRIBUTE_ID_LANGIDOFFSET,        .Data = &SDP_Attribute_LanguageOffset   },
		{.AttributeID = SDP_ATTRIBUTE_ID_NAME,                .Data = &SDP_Attribute_Name             },
		{.AttributeID = SDP_ATTRIBUTE_ID_DESCRIPTION,         .Data = &SDP_Attribute_Description      },

		SERVICE_ATTRIBUTE_TABLE_TERMINATOR
	};

/** Master service table, listing all supported services (and their attribute tables) of the device, including
 *  each service's UUID.
 */
const ServiceTable_t SDP_Services_Table[] PROGMEM =
	{
		{   // 128-bit UUID for the Service Discovery Server Service
			.UUID  = {BASE_96BIT_UUID, 0x01, 0x00, 0x00, 0x00},
			.AttributeTable = ServiceDiscoveryServer_Attribute_Table,
		},
		{   // 128-bit UUID for the SDP service
			.UUID  = {BASE_96BIT_UUID, 0x00, 0x00, 0x00, 0x01},
			.AttributeTable = SDP_Attribute_Table,
		},
#if 0
		{   // 128-bit UUID for the RFCOMM service
			.UUID  = {BASE_96BIT_UUID, 0x03, 0x00, 0x00, 0x00},
			.AttributeTable = RFCOMM_Attribute_Table,
		},
#endif
	};

/** Base UUID value common to all standardized Bluetooth services */
const uint8_t BaseUUID[] PROGMEM = {BASE_96BIT_UUID, 0x00, 0x00, 0x00, 0x00};


/** Main Service Discovery Protocol packet processing routine. This function processes incomming SDP packets from
 *  a connected Bluetooth device, and sends back appropriate responses to allow other devices to determine the
 *  services the local device exposes.
 *
 *  \param[in]  Data     Incomming packet data containing the SDP request
 *  \param[in]  Channel  Channel the request was issued to by the remote device
 */
void SDP_ProcessPacket(void* Data, Bluetooth_Channel_t* Channel)
{
	SDP_PDUHeader_t* SDPHeader = (SDP_PDUHeader_t*)Data;
	SDPHeader->ParameterLength = SwapEndian_16(SDPHeader->ParameterLength);

	BT_SDP_DEBUG(1, "SDP Packet Received");
	BT_SDP_DEBUG(2, "-- PDU ID: 0x%02X", SDPHeader->PDU);
	BT_SDP_DEBUG(2, "-- Param Length: 0x%04X", SDPHeader->ParameterLength);

	switch (SDPHeader->PDU)
	{
		case SDP_PDU_SERVICESEARCHREQUEST:
			SDP_ProcessServiceSearch(SDPHeader, Channel);
			break;		
		case SDP_PDU_SERVICEATTRIBUTEREQUEST:
			SDP_ProcessServiceAttribute(SDPHeader, Channel);
			break;
		case SDP_PDU_SERVICESEARCHATTRIBUTEREQUEST:
			SDP_ProcessServiceSearchAttribute(SDPHeader, Channel);
			break;
	}
}

/** Internal processing routine for SDP Service Search Requests.
 *
 *  \param[in] SDPHeader  Pointer to the start of the issued SDP request
 *  \param[in] Channel    Pointer to the Bluetooth channel structure the request was issued to
 */
static void SDP_ProcessServiceSearch(const SDP_PDUHeader_t* const SDPHeader, Bluetooth_Channel_t* const Channel)
{
	BT_SDP_DEBUG(1, "<< Service Search");
}

/** Internal processing routine for SDP Service Attribute Requests.
 *
 *  \param[in] SDPHeader  Pointer to the start of the issued SDP request
 *  \param[in] Channel    Pointer to the Bluetooth channel structure the request was issued to
 */
static void SDP_ProcessServiceAttribute(const SDP_PDUHeader_t* const SDPHeader, Bluetooth_Channel_t* const Channel)
{
	BT_SDP_DEBUG(1, "<< Service Attribute");
}

/** Internal processing routine for SDP Service Search Attribute Requests.
 *
 *  \param[in] SDPHeader  Pointer to the start of the issued SDP request
 *  \param[in] Channel    Pointer to the Bluetooth channel structure the request was issued to
 */
static void SDP_ProcessServiceSearchAttribute(const SDP_PDUHeader_t* const SDPHeader, Bluetooth_Channel_t* const Channel)
{
	const void* CurrentParameter = ((void*)SDPHeader + sizeof(SDP_PDUHeader_t));
	
	BT_SDP_DEBUG(1, "<< Service Search Attribute");

	/* Retrieve the list of search UUIDs from the request */
	uint8_t UUIDList[12][UUID_SIZE_BYTES];
	uint8_t TotalUUIDs = SDP_GetUUIDList(UUIDList, &CurrentParameter);
	BT_SDP_DEBUG(2, "-- Total UUIDs: %d", TotalUUIDs);
	
	/* Retrieve the maximum Attribute reponse size from the request */
	uint16_t MaxAttributeSize = *((uint16_t*)CurrentParameter);
	CurrentParameter += sizeof(uint16_t);
	BT_SDP_DEBUG(2, "-- Max Return Attribute Bytes: 0x%04X", MaxAttributeSize);
	
	/* Retrieve the list of Attributes from the request */
	uint16_t AttributeList[15][2];
	uint8_t  TotalAttributes = SDP_GetAttributeList(AttributeList, &CurrentParameter);
	BT_SDP_DEBUG(2, "-- Total Attributes: %d", TotalAttributes);
	
	struct
	{
		SDP_PDUHeader_t SDPHeader;
		uint16_t        AttributeListByteCount;
		uint8_t         ResponseData[100];
	} ResponsePacket;
	
	/* Create a pointer to the buffer to indicate the current location for response data to be added */
	void* CurrResponsePos = ResponsePacket.ResponseData;

	/* Clamp the maximum attribute size to the size of the allocated buffer */
	if (MaxAttributeSize > sizeof(ResponsePacket.ResponseData))
	  MaxAttributeSize = sizeof(ResponsePacket.ResponseData);

	/* Add the outer Data Element Sequence header for the retrieved Attributes */
	uint16_t* TotalResponseSize = SDP_AddDataElementHeader16(&CurrResponsePos, SDP_DATATYPE_Sequence);
	
	/* Search through the list of UUIDs one at a time looking for matching search Attributes */
	for (uint8_t CurrUUIDItem = 0; CurrUUIDItem < TotalUUIDs; CurrUUIDItem++)
	{
		/* Retrieve the attribute table of the current search UUID from the global UUID table if it exists */
		ServiceAttributeTable_t* AttributeTable = SDP_GetAttributeTable(UUIDList[CurrUUIDItem]);
		
		/* If the UUID does not exist in the global UUID table, continue on to the next search UUID */
		if (AttributeTable == NULL)
		  continue;
		  
		BT_SDP_DEBUG(2, " -- Found UUID %d in table", CurrUUIDItem);

		/* Add an inner Data Element Sequence header for the current UUID's found Attributes */
		uint16_t* CurrentUUIDResponseSize = SDP_AddDataElementHeader16(&CurrResponsePos, SDP_DATATYPE_Sequence);
		
		/* Search through the list of Attributes one at a time looking for values in the current UUID's Attribute table */
		for (uint8_t CurrAttribute = 0; CurrAttribute < TotalAttributes; CurrAttribute++)
		{
			uint16_t* AttributeIDRange = AttributeList[CurrAttribute];
		
			/* Look in the current Attribute Range for a matching Attribute ID in the UUID's Attribute table */
			for (uint32_t CurrAttributeID = AttributeIDRange[0]; CurrAttributeID <= AttributeIDRange[1]; CurrAttributeID++)
			{
				/* Retrieve a PROGMEM pointer to the value of the current Attribute ID, if it exists in the UUID's Attribute table */
				const void* AttributeValue = SDP_GetAttributeValue(AttributeTable, CurrAttributeID);
				
				/* If the Attribute does not exist in the current UUID's Attribute table, continue to the next Attribute ID */
				if (AttributeValue == NULL)
				  continue;
				
				BT_SDP_DEBUG(2, " -- Add Attribute 0x%04X", CurrAttributeID);

				/* Increment the current UUID's returned Attribute container size by the number of added bytes */
				*CurrentUUIDResponseSize += SDP_AddAttributeToResponse(CurrAttributeID, AttributeValue, &CurrResponsePos);
			}

			/* Increment the outer container size by the number of added bytes */
			*TotalResponseSize += 3 + *CurrentUUIDResponseSize;
		}
	}

	/* Set the total response list size to the size of the outer container plus its header size */
	ResponsePacket.AttributeListByteCount    = 3 + *TotalResponseSize;

	/* Fill in the response packet's header */
	ResponsePacket.SDPHeader.PDU             = SDP_PDU_SERVICESEARCHATTRIBUTERESPONSE;
	ResponsePacket.SDPHeader.TransactionID   = SDPHeader->TransactionID;
	ResponsePacket.SDPHeader.ParameterLength = (ResponsePacket.AttributeListByteCount + sizeof(ResponsePacket.AttributeListByteCount));

	BT_SDP_DEBUG(1, ">> Service Search Attribute Response");
	BT_SDP_DEBUG(2, "-- Total Parameter Length: 0x%04X", ResponsePacket.SDPHeader.ParameterLength);

	/* Send the completed response packet to the sender */
	Bluetooth_SendPacket(&ResponsePacket, (sizeof(ResponsePacket.SDPHeader) + ResponsePacket.SDPHeader.ParameterLength),
	                     Channel);
}

/** Adds the given attribute ID and value to the reponse buffer, and advances the response buffer pointer past the added data.
 *
 *  \param[in] AttributeID          Attribute ID to add to the response buffer
 *  \param[in] AttributeValue       Pointer to the start of the Attribute's value, located in PROGMEM
 *  \param[in, out] ResponseBuffer  Pointer to a buffer where the Attribute and Attribute Value is to be added
 *
 *  \return Number of bytes added to the response buffer
 */
static uint16_t SDP_AddAttributeToResponse(const uint16_t AttributeID, const void* AttributeValue, void** ResponseBuffer)
{
	/* Retrieve the size of the attribute value from its container header */
	uint32_t AttributeValueLength = SDP_GetLocalAttributeContainerSize(AttributeValue);

	/* Add a Data Element header to the response for the Attribute ID */
	*((uint8_t*)*ResponseBuffer) = (SDP_DATATYPE_UnsignedInt | SDP_DATASIZE_16Bit);
	*ResponseBuffer += sizeof(uint8_t);
	
	/* Add the Attribute ID to the created Data Element */
	*((uint16_t*)*ResponseBuffer) = AttributeID;
	*ResponseBuffer += sizeof(uint16_t);
	
	/* Copy over the Attribute value Data Element container to the response */
	memcpy_P(*ResponseBuffer, AttributeValue, AttributeValueLength);
	*ResponseBuffer += AttributeValueLength;
	
	return (sizeof(uint8_t) + sizeof(uint16_t) + AttributeValueLength);
}

/** Retrieves a pointer to the value of the given Attribute ID from the given Attribute table.
 *
 *  \param[in] AttributeTable  Pointer to the Attribute table to search in
 *  \param[in] AttributeID     Attribute ID to search for within the table
 *
 *  \return Pointer to the start of the Attribute's value if found within the table, NULL otherwise
 */
static void* SDP_GetAttributeValue(const ServiceAttributeTable_t* AttributeTable, const uint16_t AttributeID)
{
	void* CurrTableItemData;
	
	/* Search through the current Attribute table, abort when the terminator item has been reached */
	while ((CurrTableItemData = (void*)pgm_read_word(&AttributeTable->Data)) != NULL)
	{
		/* Check if the current Attribute ID matches the search ID - if so return a pointer to it */
		if (pgm_read_word(&AttributeTable->AttributeID) == AttributeID)
		  return CurrTableItemData;
		
		AttributeTable++;
	}
			
	return NULL;
}

/** Retrieves the Attribute table for the given UUID if it exists.
 *
 *  \param[in] UUID  UUID to search for
 *
 *  \return Pointer to the UUID's associated Attribute table if found in the global UUID table, NULL otherwise
 */
static ServiceAttributeTable_t* SDP_GetAttributeTable(const uint8_t* const UUID)
{
	/* Search through the global UUID list an item at a time */
	for (uint8_t CurrTableItem = 0; CurrTableItem < (sizeof(SDP_Services_Table) / sizeof(ServiceTable_t)); CurrTableItem++)
	{
		/* If the current table item's UUID matches the search UUID, return a pointer the table item's Attribute table */
		if (!(memcmp_P(UUID, SDP_Services_Table[CurrTableItem].UUID, UUID_SIZE_BYTES)))
		  return (ServiceAttributeTable_t*)pgm_read_word(&SDP_Services_Table[CurrTableItem].AttributeTable);
	}
	
	return NULL;
}

/** Reads in the collection of Attribute ranges from the input buffer's Data Element Sequence container, into the given 
 *  Attribute list for later use. Once complete, the input buffer pointer is advanced to the end of the Attribute container.
 *
 *  \param[out] AttributeList     Pointer to a buffer where the list of Attribute ranges are to be stored
 *  \param[in]  CurrentParameter  Pointer to a Buffer containing a Data Element Sequence of Attribute and Attribute Range elements
 *
 *  \return Total number of Attribute ranges stored in the Data Element Sequence
 */
static uint8_t SDP_GetAttributeList(uint16_t AttributeList[][2], const void** const CurrentParameter)
{
	uint8_t ElementHeaderSize;
	uint8_t TotalAttributes = 0;

	/* Retrieve the total size of the Attribute container, and unwrap the outer Data Element Sequence container */
	uint16_t AttributeIDListLength = SDP_GetDataElementSize(CurrentParameter, &ElementHeaderSize);
	BT_SDP_DEBUG(2, "-- Total Attribute Length: 0x%04X", AttributeIDListLength);
	while (AttributeIDListLength)
	{
		/* Retrieve the size of the next Attribute in the container and get a pointer to the next free Attribute element in the list */
		uint16_t* CurrentAttributeRange = AttributeList[TotalAttributes++];
		uint8_t   AttributeLength       = SDP_GetDataElementSize(CurrentParameter, &ElementHeaderSize);
		
		/* Copy over the starting Attribute ID and (if it the current element is a range) the ending Attribute ID */
		memcpy(&CurrentAttributeRange[0], *CurrentParameter, AttributeLength);
		
		/* If the element is not an Attribute Range, copy over the starting ID to the ending ID to make a range of 1 */
		if (AttributeLength == 2)
		  memcpy(&CurrentAttributeRange[1], *CurrentParameter, 2);

		BT_SDP_DEBUG(2, "-- Attribute: 0x%04X-0x%04X", CurrentAttributeRange[0], CurrentAttributeRange[1]);
		
		AttributeIDListLength -= (AttributeLength + ElementHeaderSize);
		*CurrentParameter     += AttributeLength;
	}
	
	return TotalAttributes;
}

/** Reads in the collection of UUIDs from the input buffer's Data Element Sequence container, into the given 
 *  UUID list for later use. Once complete, the input buffer pointer is advanced to the end of the UUID container.
 *
 *  \param[out] UUIDList          Pointer to a buffer where the list of UUIDs are to be stored
 *  \param[in]  CurrentParameter  Pointer to a Buffer containing a Data Element Sequence of UUID elements
 *
 *  \return Total number of UUIDs stored in the Data Element Sequence
 */
static uint8_t SDP_GetUUIDList(uint8_t UUIDList[][UUID_SIZE_BYTES], const void** const CurrentParameter)
{
	uint8_t ElementHeaderSize;
	uint8_t TotalUUIDs = 0;

	/* Retrieve the total size of the UUID container, and unwrap the outer Data Element Sequence container */
	uint16_t ServicePatternLength = SDP_GetDataElementSize(CurrentParameter, &ElementHeaderSize);
	BT_SDP_DEBUG(2, "-- Total UUID Length: 0x%04X", ServicePatternLength);
	while (ServicePatternLength)
	{
		/* Retrieve the size of the next UUID in the container and get a pointer to the next free UUID element in the list */
		uint8_t* CurrentUUID = UUIDList[TotalUUIDs++];
		uint8_t  UUIDLength  = SDP_GetDataElementSize(CurrentParameter, &ElementHeaderSize);
		
		/* Copy over the base UUID value to the free UUID slot in the list */
		memcpy_P(CurrentUUID, BaseUUID, sizeof(BaseUUID));

		/* Copy over UUID from the container to the free slot - if a short UUID (<= 4 bytes) it replaces the lower
		   4 bytes of the base UUID, otherwise it replaces the UUID completely */
		memcpy(&CurrentUUID[(UUIDLength <= 4) ? (UUID_SIZE_BYTES - 4) : 0], *CurrentParameter, UUIDLength);
		
		BT_SDP_DEBUG(2, "-- UUID (%d): 0x%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
		                UUIDLength,
		                CurrentUUID[15], CurrentUUID[14], CurrentUUID[13], CurrentUUID[12],
		                CurrentUUID[11], CurrentUUID[10], CurrentUUID[9],  CurrentUUID[8],
		                CurrentUUID[7],  CurrentUUID[6],  CurrentUUID[5],  CurrentUUID[4],
		                CurrentUUID[3],  CurrentUUID[2],  CurrentUUID[1],  CurrentUUID[0]);

		ServicePatternLength -= (UUIDLength + ElementHeaderSize);
		*CurrentParameter    += UUIDLength;
	}
	
	return TotalUUIDs;
}

/** Retrieves the total size of the given locally stored (in PROGMEM) attribute Data Element container.
 *
 *  \param[in] AttributeData  Pointer to the start of the Attribute container, located in PROGMEM
 *
 *  \return Size in bytes of the entire attribute container, including the header
 */
static uint32_t SDP_GetLocalAttributeContainerSize(const void* const AttributeData)
{
	/* Fetch the size of the Data Element structure from the header */
	uint8_t SizeIndex = (pgm_read_byte(AttributeData) & 0x07);
	
	/* Convert the Data Element size index into a size in bytes */
	switch (SizeIndex)
	{
		case SDP_DATASIZE_Variable8Bit:
			return (1 + sizeof(uint8_t))  + pgm_read_byte(AttributeData + 1);
		case SDP_DATASIZE_Variable16Bit:
			return (1 + sizeof(uint16_t)) + pgm_read_word(AttributeData + 1);
		case SDP_DATASIZE_Variable32Bit:
			return (1 + sizeof(uint32_t)) + pgm_read_dword(AttributeData + 1);
		default:
			return (1 + (1 << SizeIndex));
	}

	return 0;
}

/** Retrieves the size of a Data Element container from the current input buffer, and advances the input buffer
 *  pointer to the start of the Data Element's contents.
 *
 *  \param[in, out] DataElementHeader  Pointer to the start of a Data Element header
 *  \param[out]     ElementHeaderSize  Size in bytes of the header that was skipped
 *
 *  \return Size in bytes of the Data Element container's contents, minus the header
 */
static uint32_t SDP_GetDataElementSize(const void** const DataElementHeader, uint8_t* const ElementHeaderSize)
{
	/* Fetch the size of the Data Element structure from the header, increment the current buffer pos */
	uint8_t SizeIndex = (*((uint8_t*)*DataElementHeader) & 0x07);
	*DataElementHeader += sizeof(uint8_t);
	
	uint32_t ElementValue;

	/* Convert the Data Element size index into a size in bytes */
	switch (SizeIndex)
	{
		case SDP_DATASIZE_Variable8Bit:
			ElementValue = *((uint8_t*)*DataElementHeader);
			*DataElementHeader += sizeof(uint8_t);
			*ElementHeaderSize  = (1 + sizeof(uint8_t));
			break;
		case SDP_DATASIZE_Variable16Bit:
			ElementValue = *((uint16_t*)*DataElementHeader);
			*DataElementHeader += sizeof(uint16_t);
			*ElementHeaderSize  = (1 + sizeof(uint16_t));
			break;
		case SDP_DATASIZE_Variable32Bit:
			ElementValue = *((uint32_t*)*DataElementHeader);
			*DataElementHeader += sizeof(uint32_t);
			*ElementHeaderSize  = (1 + sizeof(uint32_t));
			break;
		default:
			ElementValue = (1 << SizeIndex);
			*ElementHeaderSize = 1;
			break;
	}
	
	return ElementValue;
}

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

#if defined(ENABLE_PDI_PROTOCOL) || defined(__DOXYGEN__)

#warning PDI Programming Protocol support is incomplete and not currently suitable for use.

/** \file
 *
 *  PDI Protocol handler, to process V2 Protocol wrapped PDI commands used in Atmel programmer devices.
 */

#define  INCLUDE_FROM_PDIPROTOCOL_C
#include "PDIProtocol.h"

uint32_t XPROG_Param_NVMBase;
uint32_t XPROG_Param_EEPageSize;

/** Handler for the CMD_XPROG_SETMODE command, which sets the programmer-to-target protocol used for PDI
 *  XMEGA programming (either PDI or JTAG). Only PDI programming is supported.
 */
void PDIProtocol_XPROG_SetMode(void)
{
	struct
	{
		uint8_t Protocol;
	} SetMode_XPROG_Params;
	
	Endpoint_Read_Stream_LE(&SetMode_XPROG_Params, sizeof(SetMode_XPROG_Params));

	Endpoint_ClearOUT();
	Endpoint_SetEndpointDirection(ENDPOINT_DIR_IN);
	
	Endpoint_Write_Byte(CMD_XPROG_SETMODE);
	Endpoint_Write_Byte(SetMode_XPROG_Params.Protocol ? STATUS_CMD_FAILED : STATUS_CMD_OK);
	Endpoint_ClearIN();	
}

/** Handler for the CMD_XPROG command, which wraps up XPROG commands in a V2 wrapper which need to be
 *  removed and processed so that the underlying XPROG command can be handled.
 */
void PDIProtocol_XPROG_Command(void)
{
	uint8_t XPROGCommand = Endpoint_Read_Byte();

	switch (XPROGCommand)
	{
		case XPRG_CMD_ENTER_PROGMODE:
			PDIProtocol_EnterXPROGMode();
			break;
		case XPRG_CMD_LEAVE_PROGMODE:
			PDIProtocol_LeaveXPROGMode();
			break;
		case XPRG_CMD_ERASE:
			PDIProtocol_Erase();
			break;
		case XPRG_CMD_WRITE_MEM:
			PDIProtocol_WriteMemory();
			break;
		case XPRG_CMD_READ_MEM:
			PDIProtocol_ReadMemory();
			break;
		case XPRG_CMD_CRC:
			PDIProtocol_ReadCRC();
			break;
		case XPRG_CMD_SET_PARAM:
			PDIProtocol_SetParam();
			break;
	}
}

/** Handler for the XPROG ENTER_PROGMODE command to establish a PDI connection with the attached device. */
static void PDIProtocol_EnterXPROGMode(void)
{
	Endpoint_ClearOUT();
	Endpoint_SetEndpointDirection(ENDPOINT_DIR_IN);
	
	/* Enable PDI programming mode with the attached target */
	PDITarget_EnableTargetPDI();
	
	/* Store the RESET key into the RESET PDI register to keep the XMEGA in reset */
	PDITarget_SendByte(PDI_CMD_STCS | PDI_RESET_REG);	
	PDITarget_SendByte(PDI_RESET_KEY);

	/* Enable access to the XPROG NVM bus by sending the documented NVM access key to the device */
	PDITarget_SendByte(PDI_CMD_KEY);	
	for (uint8_t i = sizeof(PDI_NVMENABLE_KEY); i > 0; i--)
	  PDITarget_SendByte(PDI_NVMENABLE_KEY[i - 1]);

	/* Wait until the NVM bus becomes active */
	bool NVMBusEnabled = PDITarget_WaitWhileNVMBusBusy();
	
	Endpoint_Write_Byte(CMD_XPROG);
	Endpoint_Write_Byte(XPRG_CMD_ENTER_PROGMODE);
	Endpoint_Write_Byte(NVMBusEnabled ? XPRG_ERR_OK : XPRG_ERR_FAILED);
	Endpoint_ClearIN();
}

/** Handler for the XPROG LEAVE_PROGMODE command to terminate the PDI programming connection with
 *  the attached device.
 */
static void PDIProtocol_LeaveXPROGMode(void)
{
	Endpoint_ClearOUT();
	Endpoint_SetEndpointDirection(ENDPOINT_DIR_IN);
	
	/* Clear the RESET key into the RESET PDI register to allow the XMEGA to run */
	PDITarget_SendByte(PDI_CMD_STCS | PDI_RESET_REG);	
	PDITarget_SendByte(0x00);

	PDITarget_DisableTargetPDI();

	Endpoint_Write_Byte(CMD_XPROG);
	Endpoint_Write_Byte(XPRG_CMD_LEAVE_PROGMODE);
	Endpoint_Write_Byte(XPRG_ERR_OK);
	Endpoint_ClearIN();
}

/** Handler for the XPRG ERASE command to erase a specific memory address space in the attached device. */
static void PDIProtocol_Erase(void)
{
	uint8_t ReturnStatus = XPRG_ERR_OK;

	struct
	{
		uint8_t  MemoryType;
		uint32_t Address;
	} Erase_XPROG_Params;

	Endpoint_Read_Stream_LE(&Erase_XPROG_Params, sizeof(Erase_XPROG_Params));

	Endpoint_ClearOUT();
	Endpoint_SetEndpointDirection(ENDPOINT_DIR_IN);
	
	// TODO: Send erase command here via PDI protocol
	
	Endpoint_Write_Byte(CMD_XPROG);
	Endpoint_Write_Byte(XPRG_CMD_ERASE);
	Endpoint_Write_Byte(ReturnStatus);
	Endpoint_ClearIN();	
}

/** Handler for the XPROG WRITE_MEMORY command to write to a specific memory space within the attached device. */
static void PDIProtocol_WriteMemory(void)
{
	uint8_t ReturnStatus = XPRG_ERR_OK;

	struct
	{
		uint8_t  MemoryType;
		uint32_t Address;
		uint16_t Length;
		uint8_t  ProgData[256];
	} WriteMemory_XPROG_Params;
	
	Endpoint_Read_Stream_LE(&WriteMemory_XPROG_Params, (sizeof(WriteMemory_XPROG_Params) -
	                                                    sizeof(WriteMemory_XPROG_Params).ProgData));
	WriteMemory_XPROG_Params.Address = SwapEndian_32(WriteMemory_XPROG_Params.Address);
	WriteMemory_XPROG_Params.Length  = SwapEndian_16(WriteMemory_XPROG_Params.Length);
	Endpoint_Read_Stream_LE(&WriteMemory_XPROG_Params.ProgData, WriteMemory_XPROG_Params.Length);

	Endpoint_ClearOUT();
	Endpoint_SetEndpointDirection(ENDPOINT_DIR_IN);
	
	// TODO: Send program command here via PDI protocol
	
	Endpoint_Write_Byte(CMD_XPROG);
	Endpoint_Write_Byte(XPRG_CMD_READ_MEM);
	Endpoint_Write_Byte(ReturnStatus);	
	Endpoint_ClearIN();
}

/** Handler for the XPROG READ_MEMORY command to read data from a specific address space within the
 *  attached device.
 */
static void PDIProtocol_ReadMemory(void)
{
	uint8_t ReturnStatus = XPRG_ERR_OK;

	struct
	{
		uint8_t  MemoryType;
		uint32_t Address;
		uint16_t Length;
	} ReadMemory_XPROG_Params;
	
	Endpoint_Read_Stream_LE(&ReadMemory_XPROG_Params, sizeof(ReadMemory_XPROG_Params));
	ReadMemory_XPROG_Params.Address = SwapEndian_32(ReadMemory_XPROG_Params.Address);
	ReadMemory_XPROG_Params.Length  = SwapEndian_16(ReadMemory_XPROG_Params.Length);

	Endpoint_ClearOUT();
	Endpoint_SetEndpointDirection(ENDPOINT_DIR_IN);
	
	if (ReadMemory_XPROG_Params.MemoryType == XPRG_MEM_TYPE_USERSIG)
	{
		PDITarget_SendByte(PDI_CMD_STS | (PDI_DATSIZE_1BYTE << 2));
		PDITarget_SendAddress(DATAMEM_BASE | DATAMEM_NVM_CMD);
		PDITarget_SendByte(NVM_CMD_READUSERSIG);

		
	}
	
	Endpoint_Write_Byte(CMD_XPROG);
	Endpoint_Write_Byte(XPRG_CMD_READ_MEM);
	Endpoint_Write_Byte(ReturnStatus);
	
	Endpoint_ClearIN();
}

/** Handler for the XPROG CRC command to read a specific memory space's CRC value for comparison between the
 *  attached device's memory and a data set on the host.
 */
static void PDIProtocol_ReadCRC(void)
{
	uint8_t ReturnStatus = XPRG_ERR_OK;
	
	struct
	{
		uint8_t CRCType;
	} ReadCRC_XPROG_Params;
	
	Endpoint_Read_Stream_LE(&ReadCRC_XPROG_Params, sizeof(ReadCRC_XPROG_Params));
	Endpoint_ClearOUT();
	Endpoint_SetEndpointDirection(ENDPOINT_DIR_IN);
	
	uint32_t MemoryCRC = 0;
	uint8_t  CRCReadCommand;

	if (ReadCRC_XPROG_Params.CRCType == XPRG_CRC_APP)
	  CRCReadCommand = NVM_CMD_APPCRC;
	else if (ReadCRC_XPROG_Params.CRCType == XPRG_CRC_BOOT)
	  CRCReadCommand = NVM_CMD_BOOTCRC;
	else
	  CRCReadCommand = NVM_CMD_FLASHCRC;
	
	/* Set the NVM command to the correct CRC read command */
	PDITarget_SendByte(PDI_CMD_STS | (PDI_DATSIZE_1BYTE << 2));
	PDITarget_SendAddress(DATAMEM_BASE | DATAMEM_NVM_CMD);
	PDITarget_SendByte(CRCReadCommand);

	/* Set CMDEX bit in NVM CTRLA register to start the CRC generation */
	PDITarget_SendByte(PDI_CMD_STS | (PDI_DATSIZE_1BYTE << 2));
	PDITarget_SendAddress(DATAMEM_BASE | DATAMEM_NVM_CTRLA);
	PDITarget_SendByte(1 << 0);

	/* Wait until the NVM bus and controller is no longer busy */
	PDITarget_WaitWhileNVMBusBusy();
	PDITarget_WaitWhileNVMControllerBusy();
	
	/* Read the three byte generated CRC value */
	PDITarget_SendByte(PDI_CMD_LDS | (PDI_DATSIZE_3BYTES << 2));
	PDITarget_SendAddress(DATAMEM_BASE | DATAMEM_NVM_DAT0);
	MemoryCRC  = PDITarget_ReceiveByte();
	MemoryCRC |= ((uint16_t)PDITarget_ReceiveByte() << 8);
	MemoryCRC |= ((uint32_t)PDITarget_ReceiveByte() << 16);
	
	Endpoint_Write_Byte(CMD_XPROG);
	Endpoint_Write_Byte(XPRG_CMD_CRC);
	Endpoint_Write_Byte(ReturnStatus);
	
	if (ReturnStatus == XPRG_ERR_OK)
	{
		Endpoint_Write_Byte(MemoryCRC >> 16);
		Endpoint_Write_Word_LE(MemoryCRC & 0xFFFF);		
	}
	
	Endpoint_ClearIN();	
}

/** Handler for the XPROG SET_PARAM command to set a PDI parameter for use when communicating with the
 *  attached device.
 */
static void PDIProtocol_SetParam(void)
{
	uint8_t ReturnStatus = XPRG_ERR_OK;

	uint8_t XPROGParam = Endpoint_Read_Byte();
	
	if (XPROGParam == XPRG_PARAM_NVMBASE)
	  XPROG_Param_NVMBase = Endpoint_Read_DWord_LE();
	else if (XPROGParam == XPRG_PARAM_EEPPAGESIZE)
	  XPROG_Param_EEPageSize = Endpoint_Read_Word_LE();
	else
	  ReturnStatus = XPRG_ERR_FAILED;
	
	Endpoint_ClearOUT();
	Endpoint_SetEndpointDirection(ENDPOINT_DIR_IN);
		  
	Endpoint_Write_Byte(CMD_XPROG);
	Endpoint_Write_Byte(XPRG_CMD_SET_PARAM);
	Endpoint_Write_Byte(ReturnStatus);
	Endpoint_ClearIN();
}

#endif

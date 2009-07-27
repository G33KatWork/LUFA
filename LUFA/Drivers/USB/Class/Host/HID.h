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

/** \ingroup Group_USBClassHID
 *  @defgroup Group_USBClassHIDHost HID Class Host Mode Driver
 *
 *  \section Sec_Dependencies Module Source Dependencies
 *  The following files must be built with any user project that uses this module:
 *    - LUFA/Drivers/USB/Class/Host/HID.c
 *
 *  \section Module Description
 *  Host Mode USB Class driver framework interface, for the HID USB Class driver.
 *
 *  @{
 */

#ifndef __HID_CLASS_HOST_H__
#define __HID_CLASS_HOST_H__

	/* Includes: */
		#include "../../USB.h"
		#include "../Common/HID.h"
		
	/* Enable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
			extern "C" {
		#endif

	/* Public Interface - May be used in end-application: */
		/* Type Defines: */
			/** Class state structure. An instance of this structure should be made within the user application,
			 *  and passed to each of the HID class driver functions as the HIDInterfaceInfo parameter. This
			 *  stores each HID interface's configuration and state information.
			 */
			typedef struct
			{
				const struct
				{
					uint8_t  DataINPipeNumber; /**< Pipe number of the HID interface's IN data pipe */
					uint8_t  DataOUTPipeNumber; /**< Pipe number of the HID interface's OUT data pipe */
					
					bool     MatchInterfaceProtocol;
					uint8_t  HIDInterfaceProtocol;
				} Config; /**< Config data for the USB class interface within the device. All elements in this section
				           *   <b>must</b> be set or the interface will fail to enumerate and operate correctly.
				           */
				struct
				{
					uint16_t DataINPipeSize; /**< Size in bytes of the HID interface's IN data pipe */
					uint16_t DataOUTPipeSize;  /**< Size in bytes of the HID interface's OUT data pipe */
				} State; /**< State data for the USB class interface within the device. All elements in this section
						  *   <b>may</b> be set to initial values, but may also be ignored to default to sane values when
						  *   the interface is enumerated.
						  */
			} USB_ClassInfo_HID_Host_t;

		/* Enums: */
			enum
			{
				HID_ENUMERROR_NoError                    = 0, /**< Configuration Descriptor was processed successfully */
				HID_ENUMERROR_InvalidConfigDescriptor    = 1, /**< The device returned an invalid Configuration Descriptor */
				HID_ENUMERROR_NoHIDInterfaceFound        = 2, /**< A compatible HID interface was not found in the device's Configuration Descriptor */
				HID_ENUMERROR_EndpointsNotFound          = 3, /**< Compatible HID endpoints were not found in the device's CDC interface */
			} CDCHost_EnumerationFailure_ErrorCodes_t;
	
		/* Function Prototypes: */
			void HID_Host_USBTask(USB_ClassInfo_HID_Host_t* HIDInterfaceInfo);
			uint8_t HID_Host_ConfigurePipes(USB_ClassInfo_HID_Host_t* HIDInterfaceInfo, uint16_t ConfigDescriptorLength,
			                                uint8_t* DeviceConfigDescriptor);
		
	/* Private Interface - For use in library only: */
	#if !defined(__DOXYGEN__)
		/* Macros: */
			#define HID_INTERFACE_CLASS             0x03
			
			#define HID_FOUND_DATAPIPE_IN           (1 << 0)
			#define HID_FOUND_DATAPIPE_OUT          (1 << 1)

		/* Function Prototypes: */
			#if defined(INCLUDE_FROM_HID_CLASS_HOST_C)
				static uint8_t DComp_HID_Host_NextHIDInterface(void* CurrentDescriptor);
				static uint8_t DComp_HID_Host_NextInterfaceHIDDataEndpoint(void* CurrentDescriptor);
			#endif	
	#endif	
	
	/* Disable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
			}
		#endif

#endif

/** @} */

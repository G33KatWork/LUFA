/*
             LUFA Library
     Copyright (C) Dean Camera, 2010.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
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
 *  \brief Common definitions and declarations for the library USB HID Class driver.
 *
 *  Common definitions and declarations for the library USB HID Class driver.
 *
 *  \note This file should not be included directly. It is automatically included as needed by the USB module driver
 *        dispatch header located in LUFA/Drivers/USB.h.
 */

/** \ingroup Group_USBClassHID
 *  @defgroup Group_USBClassHIDCommon  Common Class Definitions
 *
 *  \section Module Description
 *  Constants, Types and Enum definitions that are common to both Device and Host modes for the USB
 *  HID Class.
 *
 *  @{
 */

#ifndef _HID_CLASS_COMMON_H_
#define _HID_CLASS_COMMON_H_

	/* Includes: */
		#include "../../HighLevel/StdDescriptors.h"

		#include <string.h>

	/* Preprocessor Checks: */
		#if !defined(__INCLUDE_FROM_HID_DRIVER)
			#error Do not include this file directly. Include LUFA/Drivers/USB.h instead.
		#endif

	/* Macros: */
		/** Constant for a keyboard report modifier byte, indicating that the keyboard's left control key is currently pressed. */
		#define HID_KEYBOARD_MODIFER_LEFTCTRL   (1 << 0)

		/** Constant for a keyboard report modifier byte, indicating that the keyboard's left shift key is currently pressed. */
		#define HID_KEYBOARD_MODIFER_LEFTSHIFT  (1 << 1)

		/** Constant for a keyboard report modifier byte, indicating that the keyboard's left alt key is currently pressed. */
		#define HID_KEYBOARD_MODIFER_LEFTALT    (1 << 2)

		/** Constant for a keyboard report modifier byte, indicating that the keyboard's left GUI key is currently pressed. */
		#define HID_KEYBOARD_MODIFER_LEFTGUI    (1 << 3)

		/** Constant for a keyboard report modifier byte, indicating that the keyboard's right control key is currently pressed. */
		#define HID_KEYBOARD_MODIFER_RIGHTCTRL  (1 << 4)

		/** Constant for a keyboard report modifier byte, indicating that the keyboard's right shift key is currently pressed. */
		#define HID_KEYBOARD_MODIFER_RIGHTSHIFT (1 << 5)

		/** Constant for a keyboard report modifier byte, indicating that the keyboard's right alt key is currently pressed. */
		#define HID_KEYBOARD_MODIFER_RIGHTALT   (1 << 6)

		/** Constant for a keyboard report modifier byte, indicating that the keyboard's right GUI key is currently pressed. */
		#define HID_KEYBOARD_MODIFER_RIGHTGUI   (1 << 7)

		/** Constant for a keyboard output report LED byte, indicating that the host's NUM LOCK mode is currently set. */
		#define HID_KEYBOARD_LED_NUMLOCK        (1 << 0)

		/** Constant for a keyboard output report LED byte, indicating that the host's CAPS LOCK mode is currently set. */
		#define HID_KEYBOARD_LED_CAPSLOCK       (1 << 1)

		/** Constant for a keyboard output report LED byte, indicating that the host's SCROLL LOCK mode is currently set. */
		#define HID_KEYBOARD_LED_SCROLLLOCK     (1 << 2)

		/** Constant for a keyboard output report LED byte, indicating that the host's KATANA mode is currently set. */
		#define HID_KEYBOARD_LED_KATANA         (1 << 3)

	/* Type Defines: */
		/** Enum for possible Class, Subclass and Protocol values of device and interface descriptors relating to the HID
		 *  device class.
		 */
		enum HID_Descriptor_ClassSubclassProtocol_t
		{
			HID_CSCP_HIDClass             = 0x03, /**< Descriptor Class value indicating that the device or interface
			                                       *   belongs to the HID class.
			                                       */
			HID_CSCP_NonBootSubclass      = 0x00, /**< Descriptor Subclass value indicating that the device or interface
			                                       *   does not implement a HID boot protocol.
			                                       */
			HID_CSCP_BootSubclass         = 0x01, /**< Descriptor Subclass value indicating that the device or interface
			                                       * implements a HID boot protocol.
			                                       */
			HID_CSCP_NonBootProtocol      = 0x00, /**< Descriptor Protocol value indicating that the device or interface
			                                       *   does not belong to a HID boot protocol.
			                                       */
			HID_CSCP_KeyboardBootProtocol = 0x01, /**< Descriptor Protocol value indicating that the device or interface
			                                       *   belongs to the Keyboard HID boot protocol.
			                                       */
			HID_CSCP_MouseBootProtocol    = 0x02, /**< Descriptor Protocol value indicating that the device or interface
			                                       *   belongs to the Mouse HID boot protocol.
			                                       */
		};
	
		/** Enum for the HID class specific control requests that can be issued by the USB bus host. */
		enum HID_ClassRequests_t
		{
			HID_REQ_GetReport   = 0x01, /**< HID class-specific Request to get the current HID report from the device. */
			HID_REQ_GetIdle     = 0x02, /**< HID class-specific Request to get the current device idle count. */
			HID_REQ_SetReport   = 0x09, /**< HID class-specific Request to set the current HID report to the device. */
			HID_REQ_SetIdle     = 0x0A, /**< HID class-specific Request to set the device's idle count. */
			HID_REQ_GetProtocol = 0x03, /**< HID class-specific Request to get the current HID report protocol mode. */
			HID_REQ_SetProtocol = 0x0B, /**< HID class-specific Request to set the current HID report protocol mode. */
		};

		/** Enum for the HID class specific descriptor types. */
		enum HID_DescriptorTypes_t
		{
			HID_DTYPE_HID    = 0x21, /**< Descriptor header type value, to indicate a HID class HID descriptor. */
			HID_DTYPE_Report = 0x22, /**< Descriptor header type value, to indicate a HID class HID report descriptor. */
		};

		/** Enum for the different types of HID reports. */
		enum HID_ReportItemTypes_t
		{
			HID_REPORT_ITEM_In      = 0, /**< Indicates that the item is an IN report type. */
			HID_REPORT_ITEM_Out     = 1, /**< Indicates that the item is an OUT report type. */
			HID_REPORT_ITEM_Feature = 2, /**< Indicates that the item is a FEATURE report type. */
		};

		/** \brief HID class-specific HID Descriptor (LUFA naming conventions).
		 *
		 *  Type define for the HID class-specific HID descriptor, to describe the HID device's specifications. Refer to the HID
		 *  specification for details on the structure elements.
		 *
		 *  \see \ref USB_HID_StdDescriptor_HID_t for the version of this type with standard element names.
		 */
		typedef struct
		{
			USB_Descriptor_Header_t Header; /**< Regular descriptor header containing the descriptor's type and length. */

			uint16_t                HIDSpec; /**< BCD encoded version that the HID descriptor and device complies to. */
			uint8_t                 CountryCode; /**< Country code of the localized device, or zero if universal. */

			uint8_t                 TotalReportDescriptors; /**< Total number of HID report descriptors for the interface. */

			uint8_t                 HIDReportType; /**< Type of HID report, set to \ref HID_DTYPE_Report. */
			uint16_t                HIDReportLength; /**< Length of the associated HID report descriptor, in bytes. */
		} USB_HID_Descriptor_HID_t;

		/** \brief HID class-specific HID Descriptor (USB-IF naming conventions).
		 *
		 *  Type define for the HID class-specific HID descriptor, to describe the HID device's specifications. Refer to the HID
		 *  specification for details on the structure elements.
		 *
		 *  \see \ref USB_HID_Descriptor_HID_t for the version of this type with non-standard LUFA specific
		 *       element names.
		 */
		typedef struct
		{
			uint8_t  bLength; /**< Size of the descriptor, in bytes. */
			uint8_t  bDescriptorType; /**< Type of the descriptor, either a value in \ref USB_DescriptorTypes_t or a value
			                           *   given by the specific class.
			                           */

			uint16_t bcdHID; /**< BCD encoded version that the HID descriptor and device complies to. */
			uint8_t  bCountryCode; /**< Country code of the localized device, or zero if universal. */

			uint8_t  bNumDescriptors; /**< Total number of HID report descriptors for the interface. */

			uint8_t  bDescriptorType2; /**< Type of HID report, set to \ref HID_DTYPE_Report. */
			uint16_t wDescriptorLength; /**< Length of the associated HID report descriptor, in bytes. */
		} USB_HID_StdDescriptor_HID_t;

		/** \brief Standard HID Boot Protocol Mouse Report.
		 *
		 *  Type define for a standard Boot Protocol Mouse report
		 */
		typedef struct
		{
			uint8_t Button; /**< Button mask for currently pressed buttons in the mouse. */
			int8_t  X; /**< Current delta X movement of the mouse. */
			int8_t  Y; /**< Current delta Y movement on the mouse. */
		} USB_MouseReport_Data_t;

		/** \brief Standard HID Boot Protocol Keyboard Report.
		 *
		 *  Type define for a standard Boot Protocol Keyboard report
		 */
		typedef struct
		{
			uint8_t Modifier; /**< Keyboard modifier byte, indicating pressed modifier keys (a combination of
			                   *   HID_KEYBOARD_MODIFER_* masks).
			                   */
			uint8_t Reserved; /**< Reserved for OEM use, always set to 0. */
			uint8_t KeyCode[6]; /**< Key codes of the currently pressed keys. */
		} USB_KeyboardReport_Data_t;

		/** Type define for the data type used to store HID report descriptor elements. */
		typedef uint8_t USB_Descriptor_HIDReport_Datatype_t;

#endif

/** @} */


/* Origin: composite_desc.c from nxp_lpcxpresso_11u14_usbd_lib_composite example
 * Descriptors rewritten for Selfbus-USB-Interface
 *
 * @brief This file contains USB composite class example using USBD_lib stack.
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2012
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include "app_usbd_cfg.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/**
 * HID Report Descriptor
 */
const uint8_t HidIf_ReportDescriptor[] = {
        0x06, 0x00, 0xFF,       // Usage Page = 0xFF00 (Vendor Defined Page 1)
        0x09, 0x01,             // Usage (Vendor Usage 1)
        0xA1, 0x01,             // Collection (Application)
        0x15, 0x00,             // Logical Minimum (data bytes in the report may have minimum value = 0x00)
        0x26, 0xFF, 0x00,       // Logical Maximum (data bytes in the report may have maximum value = 0x00FF = unsigned 255)
        0x35, 0x00,             // Physical Minimum
        0x46, 0xFF, 0x00,       // Physical Maximum
        0x75, 0x08,             // Report Size: 8-bit field size
        0x85, 0x01,             // Report ID: 1, place ID in every report at the beginning
        0x95, 0x3F,             // Report Count: Make sixty-three 8-bit fields (the next time the parser hits an "Input", "Output", or "Feature" item)
        0x09, 0x01,             // Usage (Vendor Usage 1)
        0x81, 0x00,             // Input (Data, Array, Abs): Instantiates input packet fields based on the above report size, count, logical min/max, and usage.
//        0x75, 0x08,             // Report Size: 8-bit field size
//        0x85, 0x01,             // Report ID: 1, place ID in every report at the beginning
//        0x95, 0x3F,             // Report Count: Make sixty-three 8-bit fields (the next time the parser hits an "Input", "Output", or "Feature" item)
        0x09, 0x01,             // Usage (Vendor Usage 1)
        0x91, 0x00,             // Output (Data, Array, Abs): Instantiates output packet fields.  Uses same report size and count as "Input" fields, since nothing new/different was specified to the parser since the "Input" item.
        0xC0                    // End Collection
};
const uint16_t HidIf_ReportDescSize = sizeof(HidIf_ReportDescriptor);

/**
 * USB Standard Device Descriptor
 */
ALIGNED(4) const uint8_t USB_DeviceDescriptor[] = {
    USB_DEVICE_DESC_SIZE,       /* bLength */
    USB_DEVICE_DESCRIPTOR_TYPE, /* bDescriptorType */
    WBVAL(0x0200),              /* bcdUSB : 2.00*/
    0xEF,                       /* bDeviceClass */
    0x02,                       /* bDeviceSubClass */
    0x01,                       /* bDeviceProtocol */
    USB_MAX_PACKET0,            /* bMaxPacketSize0 */
    WBVAL(0x147B),              /* idVendor */
    WBVAL(0x5120),              /* idProduct */
    WBVAL(0x0100),              /* bcdDevice : 1.00 */
    0x01,                       /* iManufacturer */
    0x02,                       /* iProduct */
    0x03,                       /* iSerialNumber */
    0x01                        /* bNumConfigurations */
};

/**
 * USB FSConfiguration Descriptor
 * All Descriptors (Configuration, Interface, Endpoint, Class, Vendor)
 * Descriptor for composite device
 */
ALIGNED(4) uint8_t USB_FsConfigDescriptor[] = {
    /* Configuration 1 */
    USB_CONFIGURATION_DESC_SIZE,            /* bLength */
    USB_CONFIGURATION_DESCRIPTOR_TYPE,      /* bDescriptorType */
    WBVAL(                                  /* wTotalLength */
        USB_CONFIGURATION_DESC_SIZE     +
        /* HID class related descriptors */
        USB_INTERFACE_DESC_SIZE         +
        HID_DESC_SIZE                   +
        USB_ENDPOINT_DESC_SIZE          +
        USB_ENDPOINT_DESC_SIZE          +
        /* CDC class related descriptors */
        USB_INTERFACE_ASSOC_DESC_SIZE   +   /* interface association descriptor */
        USB_INTERFACE_DESC_SIZE         +   /* communication control interface */
        0x0013                          +   /* CDC functions */
        2 * USB_ENDPOINT_DESC_SIZE      +   /* interrupt endpoint */
        USB_INTERFACE_DESC_SIZE         +   /* communication data interface */
        2 * USB_ENDPOINT_DESC_SIZE      +   /* bulk endpoints */
        0
        ),
    0x03,                                   /* bNumInterfaces */
    0x01,                                   /* bConfigurationValue */
    0x00,                                   /* iConfiguration */
    USB_CONFIG_BUS_POWERED,                 /* bmAttributes */
    USB_CONFIG_POWER_MA(100),               /* bMaxPower */

    /* Interface 0, Alternate Setting 0, HID Class */
    USB_INTERFACE_DESC_SIZE,                /* bLength */
    USB_INTERFACE_DESCRIPTOR_TYPE,          /* bDescriptorType */
    USB_HID_IF_NUM,                         /* bInterfaceNumber */
    0x00,                                   /* bAlternateSetting */
    0x02,                                   /* bNumEndpoints */
    USB_DEVICE_CLASS_HUMAN_INTERFACE,       /* bInterfaceClass */
    HID_SUBCLASS_NONE,                      /* bInterfaceSubClass */
    HID_PROTOCOL_NONE,                      /* bInterfaceProtocol */
    0x04,                                   /* iInterface */
    /* HID Class Descriptor */
    /* HID_DESC_OFFSET = 0x0012 */
    HID_DESC_SIZE,                          /* bLength */
    HID_HID_DESCRIPTOR_TYPE,                /* bDescriptorType */
    WBVAL(0x0111),                          /* bcdHID : 1.11*/
    0x00,                                   /* bCountryCode */
    0x01,                                   /* bNumDescriptors */
    HID_REPORT_DESCRIPTOR_TYPE,             /* bDescriptorType */
    WBVAL(sizeof(HidIf_ReportDescriptor)),  /* wDescriptorLength */
    /* Endpoint, HID Interrupt In */
    USB_ENDPOINT_DESC_SIZE,                 /* bLength */
    USB_ENDPOINT_DESCRIPTOR_TYPE,           /* bDescriptorType */
    HID_EP_IN,                              /* bEndpointAddress */
    USB_ENDPOINT_TYPE_INTERRUPT,            /* bmAttributes */
    WBVAL(0x0040),                          /* wMaxPacketSize */
    HID_REPORT_INTERVAL,                    /* bInterval */
    /* Endpoint, HID Interrupt Out */
    USB_ENDPOINT_DESC_SIZE,                 /* bLength */
    USB_ENDPOINT_DESCRIPTOR_TYPE,           /* bDescriptorType */
    HID_EP_OUT,                             /* bEndpointAddress */
    USB_ENDPOINT_TYPE_INTERRUPT,            /* bmAttributes */
    WBVAL(0x0040),                          /* wMaxPacketSize */
    HID_REPORT_INTERVAL,                    /* bInterval */

    /* Interface association descriptor IAD*/
    USB_INTERFACE_ASSOC_DESC_SIZE,          /* bLength */
    USB_INTERFACE_ASSOCIATION_DESCRIPTOR_TYPE, /* bDescriptorType */
    USB_CDC_CIF_NUM,                        /* bFirstInterface */
    0x02,                                   /* bInterfaceCount */
    CDC_COMMUNICATION_INTERFACE_CLASS,      /* bFunctionClass */
    CDC_ABSTRACT_CONTROL_MODEL,             /* bFunctionSubClass */
    0x00,                                   /* bFunctionProtocol: no protocol used */
    0x05,                                   /* iFunction: Interface string*/

    /* Interface 1, Alternate Setting 0, Communication class interface descriptor */
    USB_INTERFACE_DESC_SIZE,                /* bLength */
    USB_INTERFACE_DESCRIPTOR_TYPE,          /* bDescriptorType */
    USB_CDC_CIF_NUM,                        /* bInterfaceNumber: Number of Interface */
    0x00,                                   /* bAlternateSetting: Alternate setting */
    0x01,                                   /* bNumEndpoints: One endpoint used */
    CDC_COMMUNICATION_INTERFACE_CLASS,      /* bInterfaceClass: Communication Interface Class */
    CDC_ABSTRACT_CONTROL_MODEL,             /* bInterfaceSubClass: Abstract Control Model */
    0x00,                                   /* bInterfaceProtocol: no protocol used */
    0x05,                                   /* iInterface: Interface string */
    /* Header Functional Descriptor*/
    0x05,                                   /* bLength: CDC header Descriptor size */
    CDC_CS_INTERFACE,                       /* bDescriptorType: CS_INTERFACE */
    CDC_HEADER,                             /* bDescriptorSubtype: Header Func Desc */
    WBVAL(CDC_V1_10),                       /* bcdCDC 1.10 */
    /* Call Management Functional Descriptor*/
    0x05,                                   /* bFunctionLength */
    CDC_CS_INTERFACE,                       /* bDescriptorType: CS_INTERFACE */
    CDC_CALL_MANAGEMENT,                    /* bDescriptorSubtype: Call Management Func Desc */
    0x01,                                   /* bmCapabilities: device handles call management */
    USB_CDC_DIF_NUM,                        /* bDataInterface: CDC data IF ID */
    /* Abstract Control Management Functional Descriptor*/
    0x04,                                   /* bFunctionLength */
    CDC_CS_INTERFACE,                       /* bDescriptorType: CS_INTERFACE */
    CDC_ABSTRACT_CONTROL_MANAGEMENT,        /* bDescriptorSubtype: Abstract Control Management desc */
    0x02,                                   /* bmCapabilities: SET_LINE_CODING, GET_LINE_CODING, SET_CONTROL_LINE_STATE supported */
    /* Union Functional Descriptor*/
    0x05,                                   /* bFunctionLength */
    CDC_CS_INTERFACE,                       /* bDescriptorType: CS_INTERFACE */
    CDC_UNION,                              /* bDescriptorSubtype: Union func desc */
    USB_CDC_CIF_NUM,                        /* bMasterInterface: Communication class interface is master */
    USB_CDC_DIF_NUM,                        /* bSlaveInterface0: Data class interface is slave 0 */
    /* Endpoint 1 Descriptor*/
    USB_ENDPOINT_DESC_SIZE,                 /* bLength */
    USB_ENDPOINT_DESCRIPTOR_TYPE,           /* bDescriptorType */
    USB_CDC_INT_EP,                         /* bEndpointAddress */
    USB_ENDPOINT_TYPE_INTERRUPT,            /* bmAttributes */
    WBVAL(0x0010),                          /* wMaxPacketSize */
    0x02,            /* 2ms */              /* bInterval */

    /* Interface 2, Alternate Setting 0, Data class interface descriptor*/
    USB_INTERFACE_DESC_SIZE,                /* bLength */
    USB_INTERFACE_DESCRIPTOR_TYPE,          /* bDescriptorType */
    USB_CDC_DIF_NUM,                        /* bInterfaceNumber: Number of Interface */
    0x00,                                   /* bAlternateSetting: no alternate setting */
    0x02,                                   /* bNumEndpoints: two endpoints used */
    CDC_DATA_INTERFACE_CLASS,               /* bInterfaceClass: Data Interface Class */
    0x00,                                   /* bInterfaceSubClass: no subclass available */
    0x00,                                   /* bInterfaceProtocol: no protocol used */
    0x05,                                   /* iInterface: Interface string*/
    /* Endpoint, EP Bulk Out */
    USB_ENDPOINT_DESC_SIZE,                 /* bLength */
    USB_ENDPOINT_DESCRIPTOR_TYPE,           /* bDescriptorType */
    USB_CDC_OUT_EP,                         /* bEndpointAddress */
    USB_ENDPOINT_TYPE_BULK,                 /* bmAttributes */
    WBVAL(USB_FS_MAX_BULK_PACKET),          /* wMaxPacketSize */
    0x00,                                   /* bInterval: ignore for Bulk transfer */
    /* Endpoint, EP Bulk In */
    USB_ENDPOINT_DESC_SIZE,                 /* bLength */
    USB_ENDPOINT_DESCRIPTOR_TYPE,           /* bDescriptorType */
    USB_CDC_IN_EP,                          /* bEndpointAddress */
    USB_ENDPOINT_TYPE_BULK,                 /* bmAttributes */
    WBVAL(USB_FS_MAX_BULK_PACKET),          /* wMaxPacketSize */
    0x00,                                   /* bInterval: ignore for Bulk transfer */

    /* Terminator */
    0                                       /* bLength */
};

// Descriptor for single function device
ALIGNED(4) uint8_t USBSngl_FsConfigDescriptor[] = {
    /* Configuration 1 */
    USB_CONFIGURATION_DESC_SIZE,            /* bLength */
    USB_CONFIGURATION_DESCRIPTOR_TYPE,      /* bDescriptorType */
    WBVAL(                                  /* wTotalLength */
        USB_CONFIGURATION_DESC_SIZE     +
        /* HID class related descriptors */
        USB_INTERFACE_DESC_SIZE         +
        HID_DESC_SIZE                   +
        USB_ENDPOINT_DESC_SIZE          +
        USB_ENDPOINT_DESC_SIZE
        ),
    0x01,                                   /* bNumInterfaces */
    0x01,                                   /* bConfigurationValue */
    0x00,                                   /* iConfiguration */
    USB_CONFIG_BUS_POWERED,                 /* bmAttributes */
    USB_CONFIG_POWER_MA(100),               /* bMaxPower */

    /* Interface 0, Alternate Setting 0, HID Class */
    USB_INTERFACE_DESC_SIZE,                /* bLength */
    USB_INTERFACE_DESCRIPTOR_TYPE,          /* bDescriptorType */
    USB_HID_IF_NUM,                         /* bInterfaceNumber */
    0x00,                                   /* bAlternateSetting */
    0x02,                                   /* bNumEndpoints */
    USB_DEVICE_CLASS_HUMAN_INTERFACE,       /* bInterfaceClass */
    HID_SUBCLASS_NONE,                      /* bInterfaceSubClass */
    HID_PROTOCOL_NONE,                      /* bInterfaceProtocol */
    0x04,                                   /* iInterface */
    /* HID Class Descriptor */
    /* HID_DESC_OFFSET = 0x0012 */
    HID_DESC_SIZE,                          /* bLength */
    HID_HID_DESCRIPTOR_TYPE,                /* bDescriptorType */
    WBVAL(0x0111),                          /* bcdHID : 1.11*/
    0x00,                                   /* bCountryCode */
    0x01,                                   /* bNumDescriptors */
    HID_REPORT_DESCRIPTOR_TYPE,             /* bDescriptorType */
    WBVAL(sizeof(HidIf_ReportDescriptor)),  /* wDescriptorLength */
    /* Endpoint, HID Interrupt In */
    USB_ENDPOINT_DESC_SIZE,                 /* bLength */
    USB_ENDPOINT_DESCRIPTOR_TYPE,           /* bDescriptorType */
    HID_EP_IN,                              /* bEndpointAddress */
    USB_ENDPOINT_TYPE_INTERRUPT,            /* bmAttributes */
    WBVAL(0x0040),                          /* wMaxPacketSize */
    HID_REPORT_INTERVAL,                    /* bInterval */
    /* Endpoint, HID Interrupt Out */
    USB_ENDPOINT_DESC_SIZE,                 /* bLength */
    USB_ENDPOINT_DESCRIPTOR_TYPE,           /* bDescriptorType */
    HID_EP_OUT,                             /* bEndpointAddress */
    USB_ENDPOINT_TYPE_INTERRUPT,            /* bmAttributes */
    WBVAL(0x0040),                          /* wMaxPacketSize */
    HID_REPORT_INTERVAL,                    /* bInterval */

    /* Terminator */
    0                                       /* bLength */
};

/**
 * USB String Descriptor
 */
const uint8_t USB_StringDescriptor[] = {
    /* Index 0x00: LANGID Codes */
    0x04,                           /* bLength (2 Byte (wLANGID) + Type + length) */
    USB_STRING_DESCRIPTOR_TYPE,     /* bDescriptorType */
    WBVAL(0x0409),                  /* wLANGID : US English */
    /* Index 0x01: Manufacturer */
    (7 * 2 + 2),                    /* bLength (7 Char + Type + length) */
    USB_STRING_DESCRIPTOR_TYPE,     /* bDescriptorType */
    'S', 0,
    'e', 0,
    'l', 0,
    'f', 0,
    'b', 0,
    'u', 0,
    's', 0,
    /* Index 0x02: Product */
    (13 * 2 + 2),                   /* bLength (13 Char + Type + length) */
    USB_STRING_DESCRIPTOR_TYPE,     /* bDescriptorType */
    'K', 0,
    'N', 0,
    'X', 0,
    '-', 0,
    'I', 0,
    'n', 0,
    't', 0,
    'e', 0,
    'r', 0,
    'f', 0,
    'a', 0,
    'c', 0,
    'e', 0,
    /* Index 0x03: Serial Number */
    (4 * 2 + 2),                    /* bLength (4 Char + Type + length) */
    USB_STRING_DESCRIPTOR_TYPE,     /* bDescriptorType */
    'A', 0,
    '0', 0,
    '0', 0,
    '0', 0,
    /* Index 0x04: Interface 0, Alternate Setting 0 */
    (13 * 2 + 2),                   /* bLength (13 Char + Type + length) */
    USB_STRING_DESCRIPTOR_TYPE,     /* bDescriptorType */
    'K', 0,
    'N', 0,
    'X', 0,
    '-', 0,
    'I', 0,
    'n', 0,
    't', 0,
    'e', 0,
    'r', 0,
    'f', 0,
    'a', 0,
    'c', 0,
    'e', 0,
    /* Index 0x05: Interface 1, Alternate Setting 0 */
    (11 * 2 + 2),                   /* bLength (11 Char + Type + length) */
    USB_STRING_DESCRIPTOR_TYPE,     /* bDescriptorType */
    'B', 0,
    'u', 0,
    's', 0,
    '-', 0,
    'M', 0,
    'o', 0,
    'n', 0,
    'i', 0,
    't', 0,
    'o', 0,
    'r', 0,
};


/* This file contains several functions from different NXP examples and libraries.
 * See following headers for origin and copyright information.
 * The functions may contain modifications and extensions.
 */
#include "chip.h"
#include "usbd_rom_api.h"
#include "nxp.h"
#include "hid_knx.h"
#include "cdc_vcom.h"
#include "cdc_dbg.h"
#include <string.h>


/* Origin: board_sysinit.c from nxp_lpcxpresso_11u14_board_lib
 *
 * @brief NXP LPCXpresso 11U14 Sysinit file
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2013
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

/* Setup system clocking */
void SystemSetupClocking(void)
{
  volatile int i;

  /* Powerup main oscillator */
  Chip_SYSCTL_PowerUp(SYSCTL_POWERDOWN_SYSOSC_PD);

  /* Wait 200us for OSC to be stablized, no status
     indication, dummy wait. */
  for (i = 0; i < 0x100; i++) {}

  /* Set system PLL input to main oscillator */
  Chip_Clock_SetSystemPLLSource(SYSCTL_PLLCLKSRC_MAINOSC);

  /* Power down PLL to change the PLL divider ratio */
  Chip_SYSCTL_PowerDown(SYSCTL_POWERDOWN_SYSPLL_PD);

  /* Setup PLL for main oscillator rate (FCLKIN = 12MHz) * 4 = 48MHz
     MSEL = 3 (this is pre-decremented), PSEL = 1 (for P = 2)
     FCLKOUT = FCLKIN * (MSEL + 1) = 12MHz * 4 = 48MHz
     FCCO = FCLKOUT * 2 * P = 48MHz * 2 * 2 = 192MHz (within FCCO range) */
  Chip_Clock_SetupSystemPLL(3, 1);

  /* Powerup system PLL */
  Chip_SYSCTL_PowerUp(SYSCTL_POWERDOWN_SYSPLL_PD);

  /* Wait for PLL to lock */
  while (!Chip_Clock_IsSystemPLLLocked()) {}

  /* Set system clock divider to 1 */
  Chip_Clock_SetSysClockDiv(1);

  /* Setup FLASH access to 3 clocks */
  Chip_FMC_SetFLASHAccess(FLASHTIM_50MHZ_CPU);

  /* Set main clock source to the system PLL. This will drive 48MHz
     for the main clock and 48MHz for the system clock */
  Chip_Clock_SetMainClockSource(SYSCTL_MAINCLKSRC_PLLOUT);

  /* Set USB PLL input to main oscillator */
  Chip_Clock_SetUSBPLLSource(SYSCTL_PLLCLKSRC_MAINOSC);
  /* Setup USB PLL  (FCLKIN = 12MHz) * 4 = 48MHz
     MSEL = 3 (this is pre-decremented), PSEL = 1 (for P = 2)
     FCLKOUT = FCLKIN * (MSEL + 1) = 12MHz * 4 = 48MHz
     FCCO = FCLKOUT * 2 * P = 48MHz * 2 * 2 = 192MHz (within FCCO range) */
  Chip_Clock_SetupUSBPLL(3, 1);

  /* Powerup USB PLL */
  Chip_SYSCTL_PowerUp(SYSCTL_POWERDOWN_USBPLL_PD);

  /* Wait for PLL to lock */
  while (!Chip_Clock_IsUSBPLLLocked()) {}
}

/* Origin: composite_main.c from nxp_lpcxpresso_11u14_usbd_lib_composite example
 * @brief This file contains USB HID Mouse example using USB ROM Drivers.
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

extern const  USBD_HW_API_T hw_api;
extern const  USBD_CORE_API_T core_api;
extern const  USBD_HID_API_T hid_api;
extern const  USBD_CDC_API_T cdc_api;
static const  USBD_API_T g_usbApi = {
    &hw_api,
    &core_api,
    0,
    0,
    &hid_api,
    &cdc_api,
    0,
    0x02221101,
};

const  USBD_API_T *g_pUsbApi = &g_usbApi;

/* Find the address of interface descriptor for given class type.  */
USB_INTERFACE_DESCRIPTOR *find_IntfDesc(const uint8_t *pDesc, uint32_t intfClass)
{
  USB_COMMON_DESCRIPTOR *pD;
  USB_INTERFACE_DESCRIPTOR *pIntfDesc = 0;
  uint32_t next_desc_adr;

  pD = (USB_COMMON_DESCRIPTOR *) pDesc;
  next_desc_adr = (uint32_t) pDesc;

  while (pD->bLength) {
    /* is it interface descriptor */
    if (pD->bDescriptorType == USB_INTERFACE_DESCRIPTOR_TYPE) {

      pIntfDesc = (USB_INTERFACE_DESCRIPTOR *) pD;
      /* did we find the right interface descriptor */
      if (pIntfDesc->bInterfaceClass == intfClass) {
        break;
      }
    }
    pIntfDesc = 0;
    next_desc_adr = (uint32_t) pD + pD->bLength;
    pD = (USB_COMMON_DESCRIPTOR *) next_desc_adr;
  }

  return pIntfDesc;
}

// Origin: Parts of the main() function
ErrorCode_t usb_init(USBD_HANDLE_T *g_hUsb, bool use_singledevonly)
{
  USBD_API_INIT_PARAM_T usb_param;
  USB_CORE_DESCS_T desc;
  ErrorCode_t ret = LPC_OK;

  /* initialize call back structures */
  memset((void *) &usb_param, 0, sizeof(USBD_API_INIT_PARAM_T));
  usb_param.usb_reg_base = LPC_USB0_BASE;
  usb_param.max_num_ep = 5;
  usb_param.mem_base = USB_STACK_MEM_BASE;
  usb_param.mem_size = USB_STACK_MEM_SIZE;
  usb_param.USB_SOF_Event = USB_sof_event;
  // usb_param.USB_Error_Event = USB_Error_Event; ///\todo maybe helpfull for virtual com port debugging
  // usb_param.USB_Configure_Event = USB_Configure_Event; ///\todo maybe helpfull for virtual com port debugging

  /* Set the USB descriptors */
  desc.device_desc = (uint8_t *) USB_DeviceDescriptor;
  desc.string_desc = (uint8_t *) USB_StringDescriptor;

  if (use_singledevonly)
  {
    desc.high_speed_desc = USBSngl_FsConfigDescriptor;
    desc.full_speed_desc = USBSngl_FsConfigDescriptor;
  } else {
    desc.high_speed_desc = USB_FsConfigDescriptor;
    desc.full_speed_desc = USB_FsConfigDescriptor;
  }
  desc.device_qualifier = 0;

  /* USB Initialization */
  ret = USBD_API->hw->Init(g_hUsb, &desc, &usb_param);
  if (ret == LPC_OK) {

    ret = HidIf_Init(*g_hUsb,
        (USB_INTERFACE_DESCRIPTOR *) find_IntfDesc(desc.high_speed_desc,
            USB_DEVICE_CLASS_HUMAN_INTERFACE),
            &usb_param.mem_base, &usb_param.mem_size);
    if (ret == LPC_OK)
    {
      ret = USBD_API->hw->EnableEvent(*g_hUsb, 0, USB_EVT_SOF, 1);
      if (!use_singledevonly) {
        /* Init VCOM interface */
        ret = vcom_init(*g_hUsb, &desc, &usb_param);
        if (ret == LPC_OK) {
          CdcIfSet_hUsb(*g_hUsb);
        }
      }
      if (ret == LPC_OK) {
        /*  enable USB interrupts */
        NVIC_EnableIRQ(USB0_IRQn);
        /* now connect */
        USBD_API->hw->Connect(*g_hUsb, 1);
      }
    }
  }
  return ret;
}

/* Origin: hid_mouse.c from the nxp_lpcxpresso_11u14_usbd_lib_composite example
 * File contains modifications and extensions
 *
 * @brief This file contains USB HID Mouse example using USB ROM Drivers.
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

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

extern const uint8_t HidIf_ReportDescriptor[];
extern const uint16_t HidIf_ReportDescSize;

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/* HID-Interface init routine. */
ErrorCode_t HidIf_Init(USBD_HANDLE_T hUsb,
             USB_INTERFACE_DESCRIPTOR *pIntfDesc,
             uint32_t *mem_base,
             uint32_t *mem_size)
{
  USBD_HID_INIT_PARAM_T hid_param;
  USB_HID_REPORT_T reports_data[1];
  ErrorCode_t ret = LPC_OK;

  /* Do a quick check of if the interface descriptor passed is the right one. */
  if ((pIntfDesc == 0) || (pIntfDesc->bInterfaceClass != USB_DEVICE_CLASS_HUMAN_INTERFACE)) {
    return ERR_FAILED;
  }

  /* Init HID params */
  memset((void *) &hid_param, 0, sizeof(USBD_HID_INIT_PARAM_T));
  hid_param.max_reports = 1;
  hid_param.mem_base = *mem_base;
  hid_param.mem_size = *mem_size;
  hid_param.intf_desc = (uint8_t *) pIntfDesc;
  /* user defined functions */
  hid_param.HID_GetReport = HidIf_GetReport;
  hid_param.HID_SetReport = HidIf_SetReport;
  hid_param.HID_EpIn_Hdlr = HidIf_Ep_Hdlr;
  hid_param.HID_EpOut_Hdlr = HidIf_Ep_Hdlr;
  hid_param.HID_Ep0_Hdlr = HidIf_Ep_Hdlr;
  /* Init reports_data */
  reports_data[0].len = HidIf_ReportDescSize;
  reports_data[0].idle_time = 0;
  reports_data[0].desc = (uint8_t *) &HidIf_ReportDescriptor[0];
  hid_param.report_data  = reports_data;

  ret = USBD_API->hid->init(hUsb, &hid_param);

  /* update memory variables */
  *mem_base = hid_param.mem_base;
  *mem_size = hid_param.mem_size;
  /* store stack handle for later use. */
  HidIfSet_hUsb(hUsb);

  return ret;
}


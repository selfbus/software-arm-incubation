
#ifndef _NXP_H_
#define _NXP_H_

#ifdef __cplusplus
extern "C"
{
#endif

void USB_IRQHandler(void);

ErrorCode_t USB_sof_event(USBD_HANDLE_T hUsb);

void SystemSetupClocking(void);

USB_INTERFACE_DESCRIPTOR *find_IntfDesc(const uint8_t *pDesc, uint32_t intfClass);

/**
 * @brief HID interface init routine.
 * @param hUsb    : Handle to USB device stack
 * @param pIntfDesc : Pointer to HID interface descriptor
 * @param mem_base  : Pointer to memory address which can be used by HID driver
 * @param mem_size  : Size of the memory passed
 * @return  On success returns LPC_OK. Params mem_base and mem_size are updated
 *      to point to new base and available size.
 */
ErrorCode_t HidIf_Init(USBD_HANDLE_T hUsb,
    USB_INTERFACE_DESCRIPTOR *pIntfDesc,
    uint32_t *mem_base,
    uint32_t *mem_size);

ErrorCode_t usb_init(USBD_HANDLE_T *g_hUsb, bool use_singledevonly);

#ifdef __cplusplus
}
#endif


#endif /* _NXP_H_ */

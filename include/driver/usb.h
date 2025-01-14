#ifndef __DRIVER_USB_H__
#define __DRIVER_USB_H__
/* ================================================================================================================== */
/* [INCL] Includes                                                                                                    */
/* ================================================================================================================== */
/* ----------------------------------  Standard  -------------------------------------------------------------------- */
#include <stdint.h>
/* ------------------------------------  SDK  ----------------------------------------------------------------------- */

/* -----------------------------------  Plejd  ---------------------------------------------------------------------- */

/* -----------------------------------  Local  ---------------------------------------------------------------------- */

/* ================================================================================================================== */
/* [DEFS] Defines                                                                                                     */
/* ================================================================================================================== */
#define MAX_USB_RX_PAYLOAD_LENGTH (26)
/* ================================================================================================================== */
/* [TYPE] Type definitions                                                                                            */
/* ================================================================================================================== */
typedef void (*usb_to_mesh_callback_t)(uint8_t*);
/* ================================================================================================================== */
/* [FDEF] Function declarations                                                                                       */
/* ================================================================================================================== */
void usb_init(void);

void usb_send(uint8_t *data);

void usb_clear(void);

void set_usb_to_mesh_callback(usb_to_mesh_callback_t usb_mesh_cb);
#endif /* __DRIVER_USB_H__ */

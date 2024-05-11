/* ================================================================================================================== */
/* [INCL] Includes                                                                                                    */
/* ================================================================================================================== */
/* ----------------------------------  Standard  -------------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
/* ------------------------------------  SDK  ----------------------------------------------------------------------- */
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/zephyr.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/printk.h>
/* -----------------------------------  Plejd  ---------------------------------------------------------------------- */
#include <compact_fifo.h>
/* -----------------------------------  Local  ---------------------------------------------------------------------- */
#include <driver/usb.h>
#include <packet.h>
LOG_MODULE_REGISTER(cdc_acm_echo, LOG_LEVEL_DBG);
/* ================================================================================================================== */
/* [DEFS] Defines                                                                                                     */
/* ================================================================================================================== */

/* ================================================================================================================== */
/* [TYPE] Type definitions                                                                                            */
/* ================================================================================================================== */
typedef struct usb_packet_st {
	char start_delimiter[5];
	radio_packet_t radio_packet;
	char end_delimiter[3];
} usb_packet_t;
/* ================================================================================================================== */
/* [GLOB] Global variables                                                                                            */
/* ================================================================================================================== */
static uint8_t tx_buffer[300 * sizeof(usb_packet_t)];
static compact_fifo_t tx;
static usb_to_mesh_callback_t cb;
static uint8_t* start = (uint32_t *)0x20079000;
/* ================================================================================================================== */
/* [PFDE] Private functions declaration                                                                               */
/* ================================================================================================================== */
static void interrupt_handler(const struct device *dev, void *user_data);
static const struct device *dev;
/* ================================================================================================================== */
/* [PFUN] Private functions implementations                                                                           */
/* ================================================================================================================== */

/* ================================================================================================================== */
/* [FUNC] Functions implementations                                                                                   */
/* ================================================================================================================== */
void usb_init(void)
{
	uint32_t baudrate, dtr = 0U;
	int ret;

    compact_fifo_init(&tx, tx_buffer, sizeof(usb_packet_t), sizeof(tx_buffer));

	dev = DEVICE_DT_GET_ONE(zephyr_cdc_acm_uart);
	if (!device_is_ready(dev)) {
		LOG_ERR("CDC ACM device not ready");
		return;
	}

	ret = usb_enable(NULL);
	if (ret != 0) {
		LOG_ERR("Failed to enable USB");
		return;
	}

	LOG_INF("Wait for DTR");

	while (true) {
		uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
		if (dtr) {
			break;
		} else {
			/* Give CPU resources to low priority threads. */
			k_sleep(K_MSEC(100));
		}
	}

	LOG_INF("DTR set");

    /* They are optional, we use them to test the interrupt endpoint */
	ret = uart_line_ctrl_set(dev, UART_LINE_CTRL_DCD, 1);
	if (ret) {
		LOG_WRN("Failed to set DCD, ret code %d", ret);
	}

	ret = uart_line_ctrl_set(dev, UART_LINE_CTRL_DSR, 1);
	if (ret) {
		LOG_WRN("Failed to set DSR, ret code %d", ret);
	}

	/* Wait 1 sec for the host to do all settings */
	k_busy_wait(1000000);

	ret = uart_line_ctrl_get(dev, UART_LINE_CTRL_BAUD_RATE, &baudrate);
	if (ret) {
		LOG_WRN("Failed to get baudrate, ret code %d", ret);
	} else {
		LOG_INF("Baudrate detected: %d", baudrate);
	}

	uart_irq_callback_set(dev, interrupt_handler);

    uart_irq_rx_enable(dev);
}

void usb_send(uint8_t *data)
{
	usb_packet_t usb_packet;

	memcpy(usb_packet.start_delimiter, "PLEJD", 5);
	memcpy(usb_packet.end_delimiter, "END", 3);
	memcpy(&usb_packet.radio_packet, data, sizeof(radio_packet_t));

    uint32_t res = compact_fifo_push(&tx, &usb_packet);

	if (res != CFIFO_OK) {
		printk("Failed to push data to FIFO for sending to PC\n");
	}

    while(uart_irq_tx_complete(dev) == 0);

    if (uart_irq_tx_complete(dev)) {
        uart_irq_tx_enable(dev);
    }
}

void usb_clear(void)
{
	compact_fifo_clear(&tx);
}

void set_usb_to_mesh_callback(usb_to_mesh_callback_t usb_mesh_cb) 
{
    cb = usb_mesh_cb;
}
/* ================================================================================================================== */
/* [INTR] Interrupts                                                                                                  */
/* ================================================================================================================== */
static void interrupt_handler(const struct device *dev, void *user_data)
{
	ARG_UNUSED(user_data);

	while (uart_irq_update(dev) && uart_irq_is_pending(dev)) {

		// if (*start == 0) uart_tx_abort(dev);

		if (uart_irq_rx_ready(dev)) {
			int recv_len;
			uint8_t buffer[MAX_USB_RX_PAYLOAD_LENGTH];

			recv_len = uart_fifo_read(dev, buffer, MAX_USB_RX_PAYLOAD_LENGTH);
            
			if (recv_len < 0) {
				printk("Failed to read UART FIFO\n");
				recv_len = 0;
			}
            else {
                // Callback to send received USB data to Network core
                cb(buffer);
            }
		}

		if (uart_irq_tx_ready(dev)) {

            uint8_t packet[sizeof(usb_packet_t)];

            if (compact_fifo_pop(&tx, packet) == CFIFO_OK){
                uint16_t len = uart_fifo_fill(dev, packet, sizeof(usb_packet_t));
				if (len != sizeof(usb_packet_t)) {
					printk("Failed to fill UART FIFO\n");
				}
            }
			else {
				printk("Failed to pop data from Network core FIFO for USB sending\n");
			}
            uart_irq_tx_disable(dev);
		}
	}
}
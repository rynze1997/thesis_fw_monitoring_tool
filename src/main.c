/* ================================================================================================================== */
/* [INCL] Includes                                                                                                    */
/* ================================================================================================================== */
/* ----------------------------------  Standard  -------------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* ------------------------------------  SDK  ----------------------------------------------------------------------- */
#include <zephyr/sys/printk.h>
/* -----------------------------------  Plejd  ---------------------------------------------------------------------- */

/* -----------------------------------  Local  ---------------------------------------------------------------------- */
#include <packet.h>
#include <compact_fifo_mutex.h>
#include <driver/usb.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main_module, LOG_LEVEL_DBG);
/* ================================================================================================================== */
/* [DEFS] Defines                                                                                                     */
/* ================================================================================================================== */

/* ================================================================================================================== */
/* [TYPE] Type definitions                                                                                            */
/* ================================================================================================================== */
#define SEND_KEYS_PROTOCOL     		0xFF
#define MESH_COMMAND_PROTOCOL 		0xFE
#define MESH_START_STOP_PROTOCOL 	0xFD
/* ================================================================================================================== */
/* [GLOB] Global variables                                                                                            */
/* ================================================================================================================== */
static compact_fifo_t* rx_fifo = (uint32_t *)0x20070000;
static compact_fifo_t* tx_fifo = (uint32_t *)0x20075000;
static uint8_t* start = (uint32_t *)0x20079000;
static uint8_t* end = (uint32_t *)0x20079004;
/* ================================================================================================================== */
/* [PFDE] Private functions declaration                                                                               */
/* ================================================================================================================== */
static void usb_to_mesh_callback(uint8_t *data);
/* ================================================================================================================== */
/* [PFUN] Private functions implementations                                                                           */
/* ================================================================================================================== */
static void append_rbc_header(mesh_package_format_t* packet, uint8_t* payload_length, uint16_t index, uint16_t version)
{
	
}

static void append_phy_header(mesh_package_format_t* packet, uint8_t* payload_length)
{
	
}

static void usb_to_mesh_callback(uint8_t *data)
{
	if (data[0] == SEND_KEYS_PROTOCOL) {
		printk("Keys received\n");
		uint32_t res = compact_fifo_mutex_push(tx_fifo, (void*)data);

		if (res != CFIFO_OK) {
			printk("Failed to push keys data to usb_mesh FIFO\n");
		}
		return;
	} 
	else if (data[0] == MESH_COMMAND_PROTOCOL) {
		// Payload length = mesh_version + flags + command + payload + crc
		// NOTE! Payload is encrypted
		uint8_t payload_length = data[1];

		mesh_package_format_t packet;
		memset(&packet, 0, sizeof(packet));
		memcpy(&packet.plejd, &data[2], payload_length);

		uint16_t index, version;
		memcpy(&index, &data[2 + payload_length], sizeof(packet.rbc_header.version));
		memcpy(&version, &data[2 + payload_length + sizeof(index)], sizeof(packet.rbc_header.version));

		append_rbc_header(&packet, &payload_length, index, version);
		append_phy_header(&packet, &payload_length);

		// Push to shared mem for NET core to access it
		uint32_t res = compact_fifo_mutex_push(tx_fifo, (void*)&packet);

		if (res != CFIFO_OK) {
			printk("Failed to push mesh packet to usb_mesh FIFO\n");
		}
	}
	else if (data[0] == MESH_START_STOP_PROTOCOL) {
		printk("Start/Stop received\n");
		*start = data[1];
	}
	else {
		printk("Unknown protocol\n");
	}
	
}
/* ================================================================================================================== */
/* [FUNC] Functions implementations                                                                                   */
/* ================================================================================================================== */
int main(void)
{
	printk("APP CORE: Starting application thread!\n");
	*start = 0;
	*end = 0;

	usb_init();

	set_usb_to_mesh_callback(&usb_to_mesh_callback);
	
	radio_packet_t packet;
	k_msleep(1000);

	uint32_t cntr = 0;
	while(true){

		if (*start == 0) {
			compact_fifo_mutex_clear(rx_fifo);
			usb_clear();
			cntr = 0;
		}

		uint32_t res = compact_fifo_mutex_pop(rx_fifo, &packet);

		if (res == CFIFO_OK) {
			cntr++;
			if (cntr != packet.counter) {
				printk("APP: Counter mismatch: %d != %d\n", cntr, packet.counter);
			}
			usb_send((uint8_t*)packet.raw);
		}
		
	}

	return 0;
}
/* ================================================================================================================== */
/* [INTR] Interrupts                                                                                                  */
/* ================================================================================================================== */
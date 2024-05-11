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
#include <radio.h>
#include <settings.h>
#include <compact_fifo_mutex.h>
/* ================================================================================================================== */
/* [DEFS] Defines                                                                                                     */
/* ================================================================================================================== */
#define RADIO_RX_BUFFER_NUM_ELEMENTS (100)
#define RADIO_TX_BUFFER_NUM_ELEMENTS (10)
/* ================================================================================================================== */
/* [TYPE] Type definitions                                                                                            */
/* ================================================================================================================== */

/* ================================================================================================================== */
/* [GLOB] Global variables                                                                                            */
/* ================================================================================================================== */
static uint8_t my_mac[6];
static uint8_t my_mac_fake[6] = {0x11, 0x11, 0x11, 0x11, 0x11, 0x11};
static compact_fifo_t* rx_fifo = (uint32_t *)0x20070000;
static compact_fifo_t* tx_fifo = (uint32_t *)0x20075000;
static uint8_t* start = (uint32_t *)0x20079000;
static uint8_t* end = (uint32_t *)0x20079004;
/* ================================================================================================================== */
/* [PFDE] Private functions declaration                                                                               */
/* ================================================================================================================== */
static void send_tx_packet();
static void apply_keys(uint8_t* data);
static bool send_rx_packet(radio_packet_t* packet);
/* ================================================================================================================== */
/* [PFUN] Private functions implementations                                                                           */
/* ================================================================================================================== */
static void apply_keys(uint8_t* data)
{
	settings_set_key(&data[4]);
	uint32_t mesh_address = 0;
	// Convert from little endian to big endian
	mesh_address = ((mesh_address >> 24) & 0xff) |      // Move byte 3 to byte 0
					((mesh_address << 8) & 0xff0000) |   // Move byte 1 to byte 2
					((mesh_address >> 8) & 0xff00) |     // Move byte 2 to byte 1
					((mesh_address << 24) & 0xff000000); // Move byte 0 to byte 3

	memcpy(&mesh_address, &data[0], 4);
	settings_set_address(mesh_address);
	printk("Keys received\n");
	// Disable radio to apply new keys
	NRF_RADIO->TASKS_DISABLE = 1;
}

static void send_tx_packet()
{
    radio_packet_t packet;

	uint32_t res = compact_fifo_mutex_pop(tx_fifo, &packet);

	if (res != CFIFO_OK){
		return;
	}

	if(packet.raw[0] == 0xFF){
		apply_keys(&packet.raw[1]);
	}
	else{
		memcpy(&packet.raw[3], my_mac_fake, 6);
		radio_send(&packet);  
		// printk("Sent\n");
	}
		      
}

static bool send_rx_packet(radio_packet_t* packet)
{
	uint32_t res = compact_fifo_mutex_push(rx_fifo, packet);

	if (res != CFIFO_OK){
		printk("Failed to push packet shared mem queue, res: %d\n", res);
		return false;
	}

	return true;
}
/* ================================================================================================================== */
/* [FUNC] Functions implementations                                                                                   */
/* ================================================================================================================== */
int main(void)
{      
    printk("NET CORE: Main %s\n", CONFIG_BOARD);

	memcpy(my_mac, NRF_FICR->DEVICEADDR, 6);

	uint32_t res = 0;

    res = compact_fifo_mutex_init(rx_fifo, 0x20070064, sizeof(radio_packet_t), RADIO_RX_BUFFER_NUM_ELEMENTS * sizeof(radio_packet_t));
    if (res != CFIFO_OK){
        printk("NET: Failed to initialize rx queue\n");
        return res;
    }
	res = compact_fifo_mutex_init(tx_fifo, 0x20075064, sizeof(radio_packet_t), RADIO_TX_BUFFER_NUM_ELEMENTS * sizeof(radio_packet_t));
	if (res != CFIFO_OK){
        printk("NET: Failed to initialize tx queue\n");
        return res;
    }

    // /* INIT */
    radio_init();
	radio_set_callback(&send_rx_packet);
    // /* INIT END */

	while(true){
		radio_process();
		send_tx_packet();
	}

    return 0;
}
/* ================================================================================================================== */
/* [INTR] Interrupts                                                                                                  */
/* ================================================================================================================== */



















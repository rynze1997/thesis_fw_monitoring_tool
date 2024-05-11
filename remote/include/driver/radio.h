#ifndef __DRIVER_RADIO_H__
#define __DRIVER_RADIO_H__
/* ================================================================================================================== */
/* [INCL] Includes                                                                                                    */
/* ================================================================================================================== */
/* ----------------------------------  Standard  -------------------------------------------------------------------- */
#include <stdint.h>
/* ------------------------------------  SDK  ----------------------------------------------------------------------- */

/* -----------------------------------  Plejd  ---------------------------------------------------------------------- */

/* -----------------------------------  Local  ---------------------------------------------------------------------- */
#include "../packet.h"
/* ================================================================================================================== */
/* [DEFS] Defines                                                                                                     */
/* ================================================================================================================== */

#define S0_FIELD_SIZE 1             

#define LENGTH_FIELD_SIZE 1        

#define S1_FIELD_SIZE 1             

#define OVERHEAD_LENGTH             (S0_FIELD_SIZE + LENGTH_FIELD_SIZE + S1_FIELD_SIZE)

#define MAX_PACKET_PAYLOAD_LENGTH   (35)
#define MAX_RADIO_PACKET_LEN        (MAX_PACKET_PAYLOAD_LENGTH + OVERHEAD_LENGTH)
/* ================================================================================================================== */
/* [TYPE] Type definitions                                                                                            */
/* ================================================================================================================== */
typedef struct radio_packet_st {
    uint8_t raw[MAX_RADIO_PACKET_LEN];
    uint32_t timestamp;
    uint32_t counter;
    uint8_t channel;
} __attribute__((packed)) radio_packet_t;

typedef bool (*radio_packet_callback_t)(radio_packet_t*);
/* ================================================================================================================== */
/* [FDEF] Function declarations                                                                                       */
/* ================================================================================================================== */
/**
 * @brief Init radio
 *
 * @return uint32_t
 */
uint32_t radio_init();
/**
 * @brief Register radio Rx packet validation callback
 *
 * @param validation_cb Packet validation callback
 * @return uint32_t, NRF_SUCCESS if BLE and RBC headers valid
 */
uint32_t radio_set_callback(radio_packet_callback_t validation_cb);
/**
 * @brief Send radio packet. Preempt RX if active.
 *
 * @param packet    Persistent pointer to packet.
 * @return uint32_t
 */
uint32_t radio_send(radio_packet_t *packet);

/**
 * @brief Pop radio packets from queue
 * 
 * @return uint32_t 
 */
uint32_t radio_process();
#endif /* __DRIVER_RADIO_H__ */

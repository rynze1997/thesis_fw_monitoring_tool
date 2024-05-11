#ifndef __PACKET_H__
#define __PACKET_H__
/* ======================================================================== */
/* [INCL] Includes                                                          */
/* ======================================================================== */
/* ------------------------  Standard  ------------------------------------ */
#include <stdint.h>
/* --------------------------  SDK  --------------------------------------- */

/* -------------------------  Plejd  -------------------------------------- */

/* -------------------------  Local  -------------------------------------- */

/* ======================================================================== */
/* [DEFS] Defines                                                           */
/* ======================================================================== */
#define __packed_gcc __attribute__((packed))
#define RBC_MESH_VALUE_MAX_LEN              (23UL)
#define BLE_ADV_PACKET_PAYLOAD_MAX_LENGTH   (31)
#define BLE_GAP_ADDR_LEN                    (6)

#define S0_FIELD_SIZE               (1)     /* This is set to 1 byte in the PCNF0 register in radio.c */

#define LENGTH_FIELD_SIZE           (1)     /* This is set to 6 bits in the PCNF0 register in radio.c.
                                            * For the field sizes defined in bits, the occupation in RAM will always be rounded
                                            * up to the next full byte size
                                            */

#define S1_FIELD_SIZE               (1)     /* This is set to 2 bits in the PCNF0 register in radio.c. Total length of LENGTH
                                            * and S1LEN fields must be 8 bits always.
                                            */

#define OVERHEAD_LENGTH             (S0_FIELD_SIZE + LENGTH_FIELD_SIZE + S1_FIELD_SIZE)

#define MAX_PACKET_PAYLOAD_LENGTH   (35)
#define MAX_RADIO_PACKET_LEN        (MAX_PACKET_PAYLOAD_LENGTH + OVERHEAD_LENGTH)
/* ======================================================================== */
/* [TYPE] Type definitions                                                  */
/* ======================================================================== */
typedef struct radio_packet_st {
    uint8_t raw[MAX_RADIO_PACKET_LEN];
    uint32_t timestamp;
    uint32_t counter;
    uint8_t channel;
} __attribute__((packed)) radio_packet_t;

typedef struct mesh_package_format_st {
    ble_header_t ble_header;
    
    rbc_header_t rbc_header;

    app_data_t plejd;
} __attribute__((packed)) mesh_package_format_t;
/* ======================================================================== */
/* [FDEF] Function declarations                                             */
/* ======================================================================== */
#endif /* __PACKET_H__ */
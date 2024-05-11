/* ================================================================================================================== */
/* [INCL] Includes                                                                                                    */
/* ================================================================================================================== */
/* ----------------------------------  Standard  -------------------------------------------------------------------- */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
/* ------------------------------------  SDK  ----------------------------------------------------------------------- */
#include <hal/nrf_radio.h>
#include <zephyr/irq.h>
#include <zephyr/sys/printk.h>
/* -----------------------------------  Plejd  ---------------------------------------------------------------------- */

/* -----------------------------------  Local  ---------------------------------------------------------------------- */
#include <radio.h>
#include <compact_fifo.h>
#include <settings.h>
/* ================================================================================================================== */
/* [DEFS] Defines                                                                                                     */
/* ================================================================================================================== */
#define RADIO_TX_BUFFER_NUM_ELEMENTS (10)
#define RADIO_RX_BUFFER_NUM_ELEMENTS (50)

#define RADIO_STATE_RX_IDLE     (NRF_RADIO->STATE == RADIO_STATE_STATE_RxIdle)
#define RADIO_STATE_DISABLED    (NRF_RADIO->STATE == RADIO_STATE_STATE_Disabled)
#define RADIO_STATE_RX          (NRF_RADIO->STATE == RADIO_STATE_STATE_Rx)


#define MESH_MAX_CHANNELS       (4)
#define INVALID_RF_CHANNEL      (0x3F) 

#define COUNT(X) ((sizeof(X))/(sizeof(X[0])))
/* ================================================================================================================== */
/* [TYPE] Type definitions                                                                                            */
/* ================================================================================================================== */
typedef struct mesh_channel_st {
    uint8_t number : 6;  // BLE channels 0-39 and 40, 41 as special frequencies
    uint8_t speed  : 2;  // Radio data rate and modulation setting, see. register RADIO->MODE
} mesh_channel_t;

typedef struct radio_st {
    radio_packet_t  tx;
    radio_packet_t  rx;
    compact_fifo_t  incoming;
    compact_fifo_t  outgoing;
    mesh_channel_t  channels[MESH_MAX_CHANNELS];
    mesh_channel_t  channels_tx[MESH_MAX_CHANNELS];
    volatile uint32_t        rx_counter;

    struct {
        uint8_t tx, rx;
    } channel_counter;

    radio_packet_callback_t callback;
} radio_t;
/* ================================================================================================================== */
/* [GLOB] Global variables                                                                                            */
/* ================================================================================================================== */
static uint8_t outgoing_buffer[RADIO_TX_BUFFER_NUM_ELEMENTS * sizeof(radio_packet_t)];
static uint8_t incoming_buffer[RADIO_RX_BUFFER_NUM_ELEMENTS * sizeof(radio_packet_t)];
static radio_t radio;
static volatile bool inhibit_rx_preemption = false;
static uint8_t* start = (uint32_t *)0x20079000;
static uint8_t* end = (uint32_t *)0x20079004;
/* ================================================================================================================== */
/* [PFDE] Private functions declaration                                                                               */
/* ================================================================================================================== */
/**
 * @brief   Callback for disable event. Occurs on DISABLED event from radio.
 *          Used to change radio state. TX mode is activated only when there is something to transmit.
 */
static inline void on_event_disabled();
/**
 * @brief   Push RX packets to queue
 */
static inline void enqueue_rx_packet();
/**
 * @brief   Initialize radio hardware. This function is called on timeslot start.
 *          After setup, DISABLE task is triggered which triggers radio interrupt that will start radio from disabled state.
 */
static inline void radio_init_hardware();
/**
 * @brief   Set up radio for TX mode and initialize ramp-up.
 *
 */
static inline void radio_start_sending();
/**
 * @brief   Set up radio for RX mode and initialize ramp-up.
 *
 */
static inline void radio_start_listening();
/**
 * @brief Setup for a radio channel
 *
 * @param channel BLE channel number
 * @param speed   Data rate and modulation, see RADIO->MODE register
 */
static inline void radio_channel_set(uint32_t channel, uint32_t speed);
/**
 * @brief Increment a channel configuration counter
 *
 * @param counter
 * @return incremented, or wrapped around, counter
 */
static inline uint8_t increment_channel_counter(uint8_t counter);
/**
 * @brief Increment a channel configuration counter
 *
 * @param counter
 * @return incremented, or wrapped around, counter
 */
static inline uint8_t increment_channel_counter_tx(uint8_t counter);
/**
 * @brief Clear and disable radio interrupts.
 *
 */
static inline void clear_and_disable_radio_irq();

/**
 * @brief Radio interrupt handler
 * 
 */
static void radio_on_radio_interrupt();

static void configure_timer();

static uint32_t time_now();
/* ================================================================================================================== */
/* [PFUN] Private functions implementations                                                                           */
/* ================================================================================================================== */
static inline void clear_and_disable_radio_irq()
{
    NRF_RADIO->INTENCLR = 0xFFFFFFFF;
    NVIC_DisableIRQ(RADIO_IRQn);
    NVIC_ClearPendingIRQ(RADIO_IRQn);
}

static inline void on_event_disabled()
{
    
    if (compact_fifo_queued(&radio.outgoing) > 0) {
        radio_start_sending();
    }
    else {
        radio_start_listening();
    }
}

static inline void enqueue_rx_packet()
{
    bool crc_status = NRF_RADIO->CRCSTATUS;

    if (crc_status) {
        if (*start == 1) {
            radio.rx_counter++;
            radio.rx.timestamp = time_now();
            radio.rx.counter = radio.rx_counter;
            radio.rx.channel = radio.channels[radio.channel_counter.rx].number;
        } else {
            radio.rx_counter = 0;
            NRF_TIMER0->TASKS_CLEAR = 1;
            return;
        }
     
        uint32_t res = compact_fifo_push(&radio.incoming, (void*)&radio.rx);
        if (res != CFIFO_OK){
            printk("Failed to push packet to incoming queue in equeue_rx_packet\n");
            return;
        }
    }
}

static inline void radio_start_sending()
{
    
}

static inline void radio_start_listening()
{
    
}

static inline void radio_init_hardware()
{
    printk("Initializing radio hardware\n");

    
}

static inline void radio_channel_set(uint32_t channel, uint32_t speed)
{
   
}

static inline uint8_t increment_channel_counter(uint8_t counter)
{
    counter++;

    /* The system is able to tune in to up to four channels. If the value of the next channel is INVALID,
     * the channel counter should be reset and the system should begin again at the first channel.
     */
    if (counter == COUNT(radio.channels) || radio.channels[counter].number == INVALID_RF_CHANNEL) {
        counter = 0;
    }

    return counter;
}

static inline uint8_t increment_channel_counter_tx(uint8_t counter)
{
    counter++;

    /* The system is able to tune in to up to four channels. If the value of the next channel is INVALID,
     * the channel counter should be reset and the system should begin again at the first channel.
     */
    if (counter == COUNT(radio.channels_tx) || radio.channels_tx[counter].number == INVALID_RF_CHANNEL) {
        counter = 0;
    }

    return counter;
}

static uint32_t time_now()
{
    NRF_TIMER0->TASKS_CAPTURE[0] = 1;
    return NRF_TIMER0->CC[0];
}


static void configure_timer()
{
    NRF_TIMER0->TASKS_STOP = 1;
    NRF_TIMER0->INTENCLR = 0xFFFFFFFF;
    NRF_TIMER0->TASKS_CLEAR = 1;
    NRF_TIMER0->MODE = TIMER_MODE_MODE_Timer;
    NRF_TIMER0->BITMODE = TIMER_BITMODE_BITMODE_32Bit;
    // 1 us per tick
    NRF_TIMER0->PRESCALER = 4;
    NRF_TIMER0->TASKS_START = 1;
  

}
/* ================================================================================================================== */
/* [FUNC] Functions implementations                                                                                   */
/* ================================================================================================================== */

uint32_t radio_init()
{
    memset(&radio, 0, sizeof(radio));
    uint32_t res = 0;

    res = compact_fifo_init(&radio.incoming, incoming_buffer, sizeof(radio_packet_t), sizeof(incoming_buffer));
    if (res != CFIFO_OK){
        printk("Failed to initialize incoming queue\n");
        return res;
    }
    res = compact_fifo_init(&radio.outgoing, outgoing_buffer, sizeof(radio_packet_t), sizeof(outgoing_buffer));
    if (res != CFIFO_OK){
        printk("Failed to initialize outgoing queue\n");
        return res;
    }

    // Register and connect radio interrupt with Zephyr IRQ handler
    IRQ_CONNECT(RADIO_IRQn, 0, radio_on_radio_interrupt, 0, IRQ_ZERO_LATENCY);

    configure_timer();

    // Initialize radio hardware, this will also start the radio.
    radio_init_hardware();

    return 0;
}

uint32_t radio_set_callback(radio_packet_callback_t callback)
{
    radio.callback = callback;
    return 0;
}

uint32_t radio_send(radio_packet_t *packet)
{
    uint32_t res = compact_fifo_push(&radio.outgoing, packet);
    if (res != CFIFO_OK){
        printk("Failed to push packet to outgoing queue\n");
        return res;
    }

    // If we are in Rx flow and we need to send something, preempt the Rx flow to change to Tx.
    if (NRF_RADIO->STATE >= RADIO_STATE_STATE_RxRu && NRF_RADIO->STATE <= RADIO_STATE_STATE_Rx) {
        NVIC_SetPendingIRQ(RADIO_IRQn);
    }

    return NRFX_SUCCESS;
}

uint32_t cntr = 0;
uint32_t radio_process()
{

    if (*start == 0) {
        radio.rx_counter = 0;
        cntr = 0;
        NRF_TIMER0->TASKS_CLEAR = 1;
    }
    
    radio_packet_t packet;
    // Do not pop the packet if push to shared memory queue failed
    if (compact_fifo_peek(&radio.incoming, &packet) == CFIFO_OK){
       
        if (radio.callback != NULL){
            if(radio.callback(&packet)){
                cntr++;
                // printk("Packet counter %d\n", cntr);
                if (cntr != packet.counter){
                    printk("Packet counter mismatch %d != %d\n", packet.counter, cntr);
                }
                compact_fifo_pop(&radio.incoming, &packet);
            }
        }
    }

    return 0;
}
/* ================================================================================================================== */
/* [INTR] Interrupts                                                                                                  */
/* ================================================================================================================== */
static void radio_on_radio_interrupt()
{

    if (compact_fifo_queued(&radio.outgoing) > 0) {
        // Preempt RX flow only when we are in RX mode and not actively receiving a packet.
        if (NRF_RADIO->STATE >= RADIO_STATE_STATE_RxRu && NRF_RADIO->STATE <= RADIO_STATE_STATE_Rx && !inhibit_rx_preemption) {
            // Disable radio to start in TX mode.
            NRF_RADIO->TASKS_DISABLE = 1;
        }
    }

    if (NRF_RADIO->EVENTS_ADDRESS) {
        NRF_RADIO->EVENTS_ADDRESS = 0;
        /* Inhibit RX preemption if we are in RX mode and have already received address of incoming packet.
         * We want to finish receiving the packet before we start transmitting. */
        // printk("Address received\n");
        if (RADIO_STATE_RX) {
            inhibit_rx_preemption = true;
        }
    }
    else if (NRF_RADIO->EVENTS_END) {
        NRF_RADIO->EVENTS_END = 0;

        inhibit_rx_preemption = false;
        uint16_t tx_queued = compact_fifo_queued(&radio.outgoing);
         
        enqueue_rx_packet();

        if (tx_queued > 0) {
            NRF_RADIO->TASKS_DISABLE = 1;
        }
       
    } else if (NRF_RADIO->EVENTS_DISABLED) {
        // printk("Disabled event\n");
        NRF_RADIO->EVENTS_DISABLED = 0;

        if (RADIO_STATE_DISABLED) {
            on_event_disabled();
        }
    }
}

#ifndef __LIBS_COMPACT_FIFO_MUTEX_H__
#define __LIBS_COMPACT_FIFO_MUTEX_H__

/* ================================================================================================================= */
/* [INCL] Includes                                                                                                   */
/* ================================================================================================================= */
/* ----------------------------------  Standard  ------------------------------------------------------------------- */
#include <stdint.h>

/* ------------------------------------  SDK  ---------------------------------------------------------------------- */
#include <zephyr/kernel.h>
/* -----------------------------------  Plejd  --------------------------------------------------------------------- */

/* -----------------------------------  Local  --------------------------------------------------------------------- */

/* ================================================================================================================= */
/* [DEFS] Defines                                                                                                    */
/* ================================================================================================================= */
#define CFIFO_OK                0x00
#define CFIFO_FULL              0x01
#define CFIFO_EMPTY             0x02
#define CFIFO_INVALID_ARG       0x03
#define CFIFO_INVALID_ARG_NULL  0x04
#define CFIFO_INVALID_ARG_VAL   0x05

/* ================================================================================================================= */
/* [TYPE] Type definitions                                                                                           */
/* ================================================================================================================= */

typedef struct {
    void * buffer;
    uint16_t wpos;
    uint16_t rpos;
    uint16_t queued;
    uint16_t buf_len;
    uint16_t max_len;
    uint8_t item_size;
    struct k_sem sem_lock;
} compact_fifo_t;

/* ================================================================================================================= */
/* [FDEF] Function declarations                                                                                      */
/* ================================================================================================================= */

/**
 * @brief Initialize the given compact fifo handle
 * 
 * @param fifo Reference to the fifo to be initialized
 * @param buffer Buffer to store the fifo data in
 * @param item_size Size of each element to be put in the buffer
 * @param buf_len Length of the buffer in bytes
 * @return uint32_t Success/Error code
 */
uint32_t compact_fifo_mutex_init(compact_fifo_t * fifo, void * buffer, uint16_t item_size, uint16_t buf_len);

/**
 * @brief Clear the buffer, discarding all pushed elements
 * 
 * @param fifo The fifo to clear
 * @return uint32_t Success/Error code
 */
uint32_t compact_fifo_mutex_clear(compact_fifo_t * fifo);

/**
 * @brief Push an item to the fifo
 * 
 * @param fifo The fifo to push the item to
 * @param item The item to push
 * @return uint32_t Success/Error code
 */
uint32_t compact_fifo_mutex_push(compact_fifo_t * fifo, void * item);

/**
 * @brief Pop an item from the fifo
 * 
 * @param fifo The fifo to pop the item from 
 * @param item Pointer to memory location to store the popped item
 * @return uint32_t Success/Error code
 */
uint32_t compact_fifo_mutex_pop(compact_fifo_t * fifo, void * item);

/**
 * @brief Peek at the next item in the fifo, without removing it
 * 
 * @param fifo The fifo to peek at
 * @param item Pointer to memory location to store the peeked item
 * @return uint32_t Success/Error code
 */
uint32_t compact_fifo_mutex_peek(compact_fifo_t * fifo, void * item);

/**
 * @brief Get the number of queued elements in the fifo
 * 
 * @param fifo The fifo to check
 * @return cf_ptr Number of elements currently in the fifo
 */
uint16_t compact_fifo_mutex_queued(compact_fifo_t * fifo);
#endif /* __LIBS_COMPACT_FIFO_MUTEX_H__ */

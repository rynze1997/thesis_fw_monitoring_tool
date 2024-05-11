
/* ================================================================================================================= */
/* [INCL] Includes                                                                                                   */
/* ================================================================================================================= */
/* ----------------------------------  Standard  ------------------------------------------------------------------- */
#include <stdlib.h>
#include <critical.h>
/* ------------------------------------  SDK  ---------------------------------------------------------------------- */
#include <zephyr/kernel.h>
/* -----------------------------------  Plejd  --------------------------------------------------------------------- */

/* -----------------------------------  Local  --------------------------------------------------------------------- */
#include <libs/compact_fifo_mutex.h>
/* ================================================================================================================= */
/* [DEFS] Defines                                                                                                    */
/* ================================================================================================================= */

/* ================================================================================================================= */
/* [TYPE] Type definitions                                                                                           */
/* ================================================================================================================= */

/* ================================================================================================================= */
/* [GLOB] Global variables                                                                                           */
/* ================================================================================================================= */

/* ================================================================================================================= */
/* [PFDE] Private functions declaration                                                                              */
/* ================================================================================================================= */

/* ================================================================================================================= */
/* [PFUN] Private functions implementations                                                                          */
/* ================================================================================================================= */

/* ================================================================================================================= */
/* [FUNC] Functions implementations                                                                                  */
/* ================================================================================================================= */

uint32_t compact_fifo_mutex_init(compact_fifo_t * fifo, void * buffer, uint16_t item_size, uint16_t buf_len){
    if (fifo == NULL){
        return CFIFO_INVALID_ARG_NULL;
    }
    if (buffer == NULL){
        return CFIFO_INVALID_ARG_NULL;
    }
    if (buf_len == 0 || item_size == 0){
        return CFIFO_INVALID_ARG_VAL;
    }
    if (buf_len % item_size != 0){
        return CFIFO_INVALID_ARG_VAL;
    }

    fifo->buffer = buffer;
    fifo->item_size = item_size;
    fifo->buf_len = buf_len;
    fifo->queued = 0;
    fifo->rpos = 0;
    fifo->wpos = 0;
    fifo->max_len = (fifo->buf_len / fifo->item_size);

    k_sem_init(&fifo->sem_lock, 1, 1);

    return CFIFO_OK;
}

uint32_t compact_fifo_mutex_clear(compact_fifo_t * fifo)
{
    if (k_sem_take(&fifo->sem_lock, K_NO_WAIT) == 0){
        // printk("Mutex locked in push\n");
    } else {
        // printk("Mutex lock failed in push\n");
        return CFIFO_INVALID_ARG_NULL;
    }

    fifo->queued = 0;
    fifo->rpos = 0;
    fifo->wpos = 0;

    k_sem_give(&fifo->sem_lock);
    return CFIFO_OK;
}

uint32_t compact_fifo_mutex_push(compact_fifo_t * fifo, void * item){
    if (fifo == NULL || item == NULL){
        return CFIFO_INVALID_ARG_NULL;
    }
    if (fifo->queued >= fifo->max_len){
        return CFIFO_FULL;
    }

    if (k_sem_take(&fifo->sem_lock, K_NO_WAIT) == 0){
        // printk("Mutex locked in push\n");
    } else {
        // printk("Mutex lock failed in push\n");
        return CFIFO_INVALID_ARG_NULL;
    }

    uint8_t * buf;

    buf = (uint8_t*)(fifo->buffer + fifo->wpos);
    fifo->wpos = (fifo->wpos + fifo->item_size) % fifo->buf_len;
    fifo->queued++;

    for (uint32_t i = 0; i < fifo->item_size; i++){
        *buf++ = *(uint8_t*)item++;
    }
    k_sem_give(&fifo->sem_lock);

    return CFIFO_OK;
}

uint32_t compact_fifo_mutex_pop(compact_fifo_t * fifo, void * item){
    if (fifo == NULL || item == NULL){
        return CFIFO_INVALID_ARG_NULL;
    }
    if (fifo->queued == 0){
        return CFIFO_EMPTY;
    }
    if (k_sem_take(&fifo->sem_lock, K_NO_WAIT) == 0){
        // printk("Mutex locked in pop\n");
    } else {
        // printk("Mutex lock failed in pop\n");
        return CFIFO_INVALID_ARG_NULL;
    }
    uint8_t * buf;

    buf = (uint8_t*)(fifo->buffer + fifo->rpos);
    fifo->rpos = (fifo->rpos + fifo->item_size) % fifo->buf_len;
    fifo->queued--;
    
    for (uint32_t i = 0; i < fifo->item_size; i++){
        *(uint8_t*)item++ = *buf++;
    }
    k_sem_give(&fifo->sem_lock);

    return CFIFO_OK;
}

uint32_t compact_fifo_mutex_peek(compact_fifo_t * fifo, void * item){
    if (fifo == NULL || item == NULL){
        return CFIFO_INVALID_ARG_NULL;
    }
    if (fifo->queued == 0){
        return CFIFO_EMPTY;
    }

    uint8_t * buf = (uint8_t*)(fifo->buffer + fifo->rpos);
    for (uint32_t i = 0; i < fifo->item_size; i++){
        *(uint8_t*)item++ = *buf++;
    }
    
    return CFIFO_OK;
}

uint16_t compact_fifo_mutex_queued(compact_fifo_t * fifo){
    if (fifo == NULL){
        return 0;
    }
    return fifo->queued;
}

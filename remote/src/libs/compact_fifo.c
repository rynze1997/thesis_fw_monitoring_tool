/* ================================================================================================================= */
/* [INCL] Includes                                                                                                   */
/* ================================================================================================================= */
/* ----------------------------------  Standard  ------------------------------------------------------------------- */
#include <stdlib.h>
#include <critical.h>
/* ------------------------------------  SDK  ---------------------------------------------------------------------- */

/* -----------------------------------  Plejd  --------------------------------------------------------------------- */

/* -----------------------------------  Local  --------------------------------------------------------------------- */
#include <libs/compact_fifo.h>
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

uint32_t compact_fifo_init(compact_fifo_t * fifo, void * buffer, uint16_t item_size, uint16_t buf_len){
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

    return CFIFO_OK;
}

uint32_t compact_fifo_clear(compact_fifo_t * fifo)
{
    fifo->queued = 0;
    fifo->rpos = 0;
    fifo->wpos = 0;
    return CFIFO_OK;
}

uint32_t compact_fifo_push(compact_fifo_t * fifo, void * item){
    if (fifo == NULL || item == NULL){
        return CFIFO_INVALID_ARG_NULL;
    }
    if (fifo->queued >= fifo->max_len){
        return CFIFO_FULL;
    }
    uint8_t * buf;

    buf = (uint8_t*)(fifo->buffer + fifo->wpos);
    fifo->wpos = (fifo->wpos + fifo->item_size) % fifo->buf_len;
    fifo->queued++;

    for (uint32_t i = 0; i < fifo->item_size; i++){
        *buf++ = *(uint8_t*)item++;
    }

    return CFIFO_OK;
}

uint32_t compact_fifo_pop(compact_fifo_t * fifo, void * item){
    if (fifo == NULL || item == NULL){
        return CFIFO_INVALID_ARG_NULL;
    }
    if (fifo->queued == 0){
        return CFIFO_EMPTY;
    }

    uint8_t * buf;

    buf = (uint8_t*)(fifo->buffer + fifo->rpos);
    fifo->rpos = (fifo->rpos + fifo->item_size) % fifo->buf_len;
    fifo->queued--;
    
    for (uint32_t i = 0; i < fifo->item_size; i++){
        *(uint8_t*)item++ = *buf++;
    }

    return CFIFO_OK;
}

uint32_t compact_fifo_peek(compact_fifo_t * fifo, void * item){
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

uint16_t compact_fifo_queued(compact_fifo_t * fifo){
    if (fifo == NULL){
        return 0;
    }
    return fifo->queued;
}

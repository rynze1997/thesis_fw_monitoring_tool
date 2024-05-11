
/* ================================================================================================================== */
/* [INCL] Includes                                                                                                    */
/* ================================================================================================================== */
/* ----------------------------------  Standard  -------------------------------------------------------------------- */
#include <stdint.h>
#include <string.h>
/* ------------------------------------  SDK  ----------------------------------------------------------------------- */

/* -----------------------------------  Plejd  ---------------------------------------------------------------------- */

/* -----------------------------------  Local  ---------------------------------------------------------------------- */
#include <settings/settings.h>
#include <util/access_address.h>
/* ================================================================================================================== */
/* [DEFS] Defines                                                                                                     */
/* ================================================================================================================== */

/* ================================================================================================================== */
/* [TYPE] Type definitions                                                                                            */
/* ================================================================================================================== */

/* ================================================================================================================== */
/* [GLOB] Global variables                                                                                            */
/* ================================================================================================================== */

/* ================================================================================================================== */
/* [PFDE] Private functions declaration                                                                               */
/* ================================================================================================================== */

/* ================================================================================================================== */
/* [PFUN] Private functions implementations                                                                           */
/* ================================================================================================================== */

/* ================================================================================================================== */
/* [FUNC] Functions implementations                                                                                   */
/* ================================================================================================================== */
uint32_t settings_get_address()
{
    return access_address;
}

void settings_get_key(uint8_t *key)
{
    memcpy(key, crypto_key, 16);
}

void settings_set_address(uint32_t address)
{
    access_address = address;
}

void settings_set_key(uint8_t *key)
{
    memcpy(crypto_key, key, 16);
}

/* ================================================================================================================== */
/* [INTR] Interrupts                                                                                                  */
/* ================================================================================================================== */

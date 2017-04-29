/*
 * Author: Jon Trulson <jtrulson@ics.com>
 * Copyright (c) 2017 Intel Corporation.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "rn2903.h"

#include "upm_utilities.h"

// milliseconds
#define RN2903_MAX_WAIT   (1000)

// milliseconds
#define RN2903_DEFAULT_RESP_DELAY   (250)

// some useful macros to save on typing and text wrapping
#undef _SHIFT
#define _SHIFT(x) (_RN2903_##x##_SHIFT)

#undef _MASK
#define _MASK(x) (_RN2903_##x##_MASK)

#undef _SHIFTMASK
#define _SHIFTMASK(x) (_MASK(x) << _SHIFT(x))

static bool validate_hex_str(const char *hex)
{
    assert(hex != NULL);

    int len = strlen(hex);
    if ((len % 2) != 0)
    {
        printf("%s: strlen(hex) must be a multiple of 2\n",
               __FUNCTION__);
        return false;
    }

    for (int i=0; i<len; i++)
    {
        if ( !((hex[i] >= '0' && hex[i] <= '9') ||
               (tolower(hex[i]) >= 'a' && tolower(hex[i]) <= 'f')) )
        {
            printf("%s: invalid hex character at position %d\n",
                   __FUNCTION__, i);
            return false;
        }
    }

    return true;
}

static rn2903_context _rn2903_preinit()
{
    // make sure MRAA is initialized
    int mraa_rv;
    if ((mraa_rv = mraa_init()) != MRAA_SUCCESS)
    {
        printf("%s: mraa_init() failed (%d).\n", __FUNCTION__, mraa_rv);
        return NULL;
    }

    rn2903_context dev =
        (rn2903_context)malloc(sizeof(struct _rn2903_context));

    if (!dev)
        return NULL;

    // zero out context
    memset((void *)dev, 0, sizeof(struct _rn2903_context));

    dev->cmd_resp_wait_ms = RN2903_DEFAULT_RESP_DELAY;

    return dev;
}

static rn2903_context _rn2903_postinit(rn2903_context dev,
                                       unsigned int baudrate)
{
    assert(dev != NULL);

    if (rn2903_set_baudrate(dev, baudrate))
    {
        printf("%s: rn2903_set_baudrate() failed.\n", __FUNCTION__);
        rn2903_close(dev);
        return NULL;
    }

    if (rn2903_set_flow_control(dev, RN2903_FLOW_CONTROL_NONE))
    {
        printf("%s: rn2903_set_flow_control() failed.\n", __FUNCTION__);
        rn2903_close(dev);
        return NULL;
    }

    // turn off debugging
    rn2903_set_debug_cmd(dev, false);

    // send an initial command, which will always fail the first time
    // (invalid_param)
    rn2903_command(dev, "sys get ver");

    // reset the device
    if (rn2903_reset(dev))
    {
        printf("%s: rn2903_reset() failed.\n", __FUNCTION__);
        rn2903_close(dev);
        return NULL;
    }

    // now get and store our hardware EUI
    if (rn2903_command(dev, "sys get hweui"))
    {
        printf("%s: rn2903_command(sys get hweui) failed.\n", __FUNCTION__);
        rn2903_close(dev);
        return NULL;
    }
    strncpy(dev->hardware_eui, dev->resp_data, RN2903_MAX_HEX_EUI64);

    return dev;
}

// uart init
rn2903_context rn2903_init(unsigned int uart, unsigned int baudrate)
{
    rn2903_context dev;

    if (!(dev = _rn2903_preinit()))
        return NULL;

    // initialize the MRAA context

    // uart, default should be 8N1
    if (!(dev->uart = mraa_uart_init(uart)))
    {
        printf("%s: mraa_uart_init() failed.\n", __FUNCTION__);
        rn2903_close(dev);
        return NULL;
    }

    return _rn2903_postinit(dev, baudrate);
}

// uart tty init
rn2903_context rn2903_init_tty(const char *uart_tty, unsigned int baudrate)
{
    rn2903_context dev;

    if (!(dev = _rn2903_preinit()))
        return NULL;

    // initialize the MRAA context

    // uart, default should be 8N1
    if (!(dev->uart = mraa_uart_init_raw(uart_tty)))
    {
        printf("%s: mraa_uart_init_raw() failed.\n", __FUNCTION__);
        rn2903_close(dev);
        return NULL;
    }

    return _rn2903_postinit(dev, baudrate);
}

void rn2903_close(rn2903_context dev)
{
    assert(dev != NULL);

    if (dev->to_hex_buf)
        free(dev->to_hex_buf);
    if (dev->from_hex_buf)
        free(dev->from_hex_buf);

    if (dev->uart)
        mraa_uart_stop(dev->uart);

    free(dev);
}

int rn2903_read(const rn2903_context dev, char *buffer, size_t len)
{
    assert(dev != NULL);

    // uart
    return mraa_uart_read(dev->uart, buffer, len);
}

int rn2903_write(const rn2903_context dev, const char *buffer, size_t len)
{
    assert(dev != NULL);

    int rv = mraa_uart_write(dev->uart, buffer, len);
    mraa_uart_flush(dev->uart);

    return rv;
}

bool rn2903_data_available(const rn2903_context dev, unsigned int millis)
{
    assert(dev != NULL);

    if (mraa_uart_data_available(dev->uart, millis))
        return true;
    else
        return false;
}

upm_result_t rn2903_set_baudrate(const rn2903_context dev,
                                 unsigned int baudrate)
{
    assert(dev != NULL);

    if (mraa_uart_set_baudrate(dev->uart, baudrate))
    {
        printf("%s: mraa_uart_set_baudrate() failed.\n", __FUNCTION__);
        return UPM_ERROR_OPERATION_FAILED;
    }

    return UPM_SUCCESS;
}

void rn2903_set_debug_cmd(const rn2903_context dev, bool enable)
{
    assert(dev != NULL);

    dev->debug_cmd_resp = enable;
}

void rn2903_set_response_wait_time(const rn2903_context dev,
                                   unsigned int wait_time)
{
    assert(dev != NULL);

    dev->cmd_resp_wait_ms = wait_time;
}

void rn2903_drain(const rn2903_context dev)
{
    assert(dev != NULL);

    char resp[RN2903_MAX_BUFFER];
    int rv;
    while (rn2903_data_available(dev, 0))
    {
        rv = rn2903_read(dev, resp, RN2903_MAX_BUFFER);
        if (rv < 0)
        {
            printf("%s: read failed\n", __FUNCTION__);
            return;
        }
        // printf("%s: Tossed %d bytes\n", __FUNCTION__, rv);
    }

    return;
}

RN2903_RESPONSE_T rn2903_waitfor_response(const rn2903_context dev,
                                          int wait_ms)
{
    assert(dev != NULL);

    memset(dev->resp_data, 0, RN2903_MAX_BUFFER);
    dev->resp_len = 0;

    upm_clock_t clock;
    upm_clock_init(&clock);
    uint32_t elapsed = 0;

    do
    {
        if (rn2903_data_available(dev, 0))
        {
            int rv = rn2903_read(dev, &(dev->resp_data[dev->resp_len]), 1);

            if (rv < 0)
                return RN2903_RESPONSE_UPM_ERROR;

            // discard CR's
            if (dev->resp_data[dev->resp_len] == '\r')
                continue;

            // got a LF, we are done - discard and finish
            if (dev->resp_data[dev->resp_len] == '\n')
            {
                dev->resp_data[dev->resp_len] = 0;
                break;
            }

            // too much data?
            if (dev->resp_len >= RN2903_MAX_BUFFER - 1)
                break;

            dev->resp_len++;
        }
    } while ( (elapsed = upm_elapsed_ms(&clock)) < wait_ms);

    if (dev->debug_cmd_resp)
        printf("\tRESP (%ld): '%s'\n", dev->resp_len,
               (dev->resp_len) ? dev->resp_data : "");

    if (elapsed >= wait_ms)
        return RN2903_RESPONSE_TIMEOUT;
    else if (rn2903_find(dev, dev->resp_data, RN2903_PHRASE_INV_PARAM))
        return RN2903_RESPONSE_INVALID_PARAM;
    else
        return RN2903_RESPONSE_OK; // either data or "ok"
}

RN2903_RESPONSE_T rn2903_command(const rn2903_context dev, const char *cmd)
{
    assert(dev != NULL);
    assert(cmd != NULL);

    rn2903_drain(dev);

    if (dev->debug_cmd_resp)
        printf("CMD: '%s'\n", cmd);

    if (rn2903_write(dev, cmd, strlen(cmd)) < 0)
    {
        printf("%s: rn2903_write(cmd) failed\n", __FUNCTION__);
        return RN2903_RESPONSE_UPM_ERROR;
    }

    // now write the termination string (CR/LF)
    if (rn2903_write(dev, RN2903_PHRASE_TERM, RN2903_PHRASE_TERM_LEN) < 0)
    {
        printf("%s: rn2903_write(TERM) failed\n", __FUNCTION__);
        return RN2903_RESPONSE_UPM_ERROR;
    }

    return rn2903_waitfor_response(dev, dev->cmd_resp_wait_ms);
}

const char *rn2903_get_response(const rn2903_context dev)
{
    assert(dev != NULL);

    return dev->resp_data;
}

size_t rn2903_get_response_len(const rn2903_context dev)
{
    assert(dev != NULL);

    return dev->resp_len;
}

const char *rn2903_to_hex(const rn2903_context dev, const void *src, int len)
{
    assert(dev != NULL);
    assert(src != NULL);

    static const char hdigits[16] = "0123456789ABCDEF";

    // first free previous destination hex buffer if allocated
    if (dev->to_hex_buf)
    {
        free(dev->to_hex_buf);
        dev->to_hex_buf = NULL;
    }

    if (len == 0)
        return NULL;

    int dlen = (len * 2) + 1;

    if (!(dev->to_hex_buf = malloc(dlen)))
    {
        printf("%s: malloc(%d) failed\n", __FUNCTION__, dlen);
        return NULL;
    }
    memset(dev->to_hex_buf, 0, dlen);

    char *dptr = dev->to_hex_buf;
    char *sptr = (char *)src;
    for (int i=0; i<len; i++)
    {
        *dptr++ = hdigits[(sptr[i] >> 4) & 0x0f];
        *dptr++ = hdigits[(sptr[i] & 0x0f)];
    }

    // the memset() will have ensured the last byte is 0
    return dev->to_hex_buf;
}

const char *rn2903_from_hex(const rn2903_context dev,
                      const void *src)
{
    assert(dev != NULL);
    assert(src != NULL);

    // first free previous destination hex buffer if allocated
    if (dev->from_hex_buf)
    {
        free(dev->from_hex_buf);
        dev->from_hex_buf = NULL;
    }

    int len = strlen(src);
    if (len == 0)
        return NULL;

    if (!validate_hex_str(src))
        return NULL;

    // add a byte for 0 termination, just in case we're dealing with a
    // string
    int dlen = (len / 2) + 1;

    if (!(dev->from_hex_buf = malloc(dlen)))
    {
        printf("%s: malloc(%d) failed\n", __FUNCTION__, dlen);
        return NULL;
    }
    memset(dev->from_hex_buf, 0, dlen);

    char *dptr = dev->from_hex_buf;
    char *sptr = (char *)src;
    for (int i=0; i<(dlen - 1); i++)
    {
        char tbuf[3] = { sptr[i*2], sptr[(i*2)+1], 0 };
        *dptr++ = (char)strtol(tbuf, NULL, 16);
    }

    // the memset() will ensure the last byte is 0
    return dev->from_hex_buf;
}

const char *rn2903_get_hardware_eui(const rn2903_context dev)
{
    assert(dev != NULL);

    return dev->hardware_eui;
}

upm_result_t rn2903_update_mac_status(const rn2903_context dev)
{
    assert(dev != NULL);

    if (rn2903_command(dev, "mac get status"))
    {
        printf("%s: rn2903_command(mac get status) failed.\n", __FUNCTION__);
        return UPM_ERROR_OPERATION_FAILED;
    }

    // make sure we actually got a hex value of 4 bytes
    if (!validate_hex_str(dev->resp_data) || dev->resp_len != 4)
    {
        printf("%s: invalid mac status.\n", __FUNCTION__);
        return UPM_ERROR_OPERATION_FAILED;
    }

    // convert it
    const char *statPtr = rn2903_from_hex(dev, dev->resp_data);
    if (!statPtr)
    {
        printf("%s: from_hex conversion failed.\n", __FUNCTION__);
        return UPM_ERROR_OPERATION_FAILED;
    }

    // now play pointer games.  We should have 2 bytes (uint16_t)
    // which we will stuff into our mac_status field in the context.
    uint16_t status16 = *(uint16_t *)statPtr;

    // now with that data, decode the mac_status_mac_status (no, I'm
    // not stuttering) bitfield.  Then set our mac_status members
    // accordingly.

    dev->mac_status_word = status16;
    dev->mac_mac_status =
        (RN2903_MAC_STATUS_T)((status16 & _SHIFTMASK(MAC_STATUS_MAC_STATUS))
                              >> _SHIFT(MAC_STATUS_MAC_STATUS));

    return UPM_SUCCESS;
}

uint16_t rn2903_get_mac_status_word(const rn2903_context dev)
{
    assert(dev != NULL);

    return dev->mac_status_word;
}

RN2903_MAC_STATUS_T rn2903_get_mac_status(const rn2903_context dev)
{
    assert(dev != NULL);

    return dev->mac_mac_status;
}

upm_result_t rn2903_reset(const rn2903_context dev)
{
    assert(dev != NULL);

    if (rn2903_command(dev, "sys reset"))
        return UPM_ERROR_OPERATION_FAILED;

    return UPM_SUCCESS;
}

RN2903_JOIN_STATUS_T rn2903_join(const rn2903_context dev,
                                 RN2903_JOIN_TYPE_T type)
{
    assert(dev != NULL);

    // first, do a couple of initial checks...

    // get the mac status and ensure that 1) we are not already
    // joined, 2) the mac status is idle, 3) we have not been
    // silenced, and 4) MAC has not been paused.

    if (rn2903_update_mac_status(dev))
    {
        printf("%s: rn2903_update_mac_status() failed\n", __FUNCTION__);
        return RN2903_JOIN_STATUS_UPM_ERROR;
    }

    uint16_t status = rn2903_get_mac_status_word(dev);
    RN2903_MAC_STATUS_T mac_status = rn2903_get_mac_status(dev);

    if (status & RN2903_MAC_STATUS_JOINED)
        return RN2903_JOIN_STATUS_ALREADY_JOINED;
    else if (status & RN2903_MAC_STATUS_SILENT)
        return RN2903_JOIN_STATUS_SILENT;
    else if (status & RN2903_MAC_STATUS_PAUSED)
        return RN2903_JOIN_STATUS_MAC_PAUSED;
    else if (mac_status != RN2903_MAC_STAT_IDLE)
        return RN2903_JOIN_STATUS_BUSY;

    // so far, so good... now build the command

    char cmd[16] = {};
    snprintf(cmd, 16, "mac join %s",
             (type == RN2903_JOIN_TYPE_OTAA) ? "otaa" : "abp");

    // now run the command.  We will get two responses back - one
    // immediately if there is an error or if the join operation was
    // successfully submitted to the radio for transmission, and
    // another indicating whether the join was granted, or failed.
    // ABP joins will always succeed immediately.

    RN2903_RESPONSE_T rv;
    if ((rv = rn2903_command(dev, cmd)))
    {
        // a failure of some sort.  We've already screened for most of
        // them, but there are a couple that we can't detect until we
        // try.
        printf("%s: join command failed (%d).\n", __FUNCTION__, rv);
        return RN2903_JOIN_STATUS_UPM_ERROR;
    }

    // if we are here, then we either got an "ok" or another error we
    // didn't screen for.

    const char *resp = rn2903_get_response(dev);
    if (rn2903_find(dev, resp, "no_free_ch"))
        return RN2903_JOIN_STATUS_NO_CHAN;
    else if (rn2903_find(dev, resp, "keys_not_init"))
        return RN2903_JOIN_STATUS_BAD_KEYS;

    // ok, so now we wait for awhile for another response indicating
    // whether the join request was accepted or not

    // FIXME - 60secs?
    if ((rv = rn2903_waitfor_response(dev, 60000)))
    {
        printf("%s: join response failed (%d).\n", __FUNCTION__, rv);
        return RN2903_JOIN_STATUS_UPM_ERROR;
    }

    resp = rn2903_get_response(dev);
    if (rn2903_find(dev, resp, "denied"))
        return RN2903_JOIN_STATUS_DENIED;
    else if (rn2903_find(dev, resp, "accepted"))
        return RN2903_JOIN_STATUS_ACCEPTED;

    // if it's anything else, we failed
    printf("%s: unexpected response to join request (%s).\n",
           __FUNCTION__, (rn2903_get_response_len(dev)) ? resp : "");

    return RN2903_JOIN_STATUS_UPM_ERROR;
}

upm_result_t rn2903_set_flow_control(const rn2903_context dev,
                                     RN2903_FLOW_CONTROL_T fc)
{
    assert(dev != NULL);

    mraa_result_t rv = MRAA_SUCCESS;

    switch(fc)
    {
    case RN2903_FLOW_CONTROL_NONE:
        rv = mraa_uart_set_flowcontrol(dev->uart, false, false);
        break;

    case RN2903_FLOW_CONTROL_HARD:
        rv = mraa_uart_set_flowcontrol(dev->uart, false, true);
        break;

    case RN2903_FLOW_CONTROL_SOFT:
        rv = mraa_uart_set_flowcontrol(dev->uart, true, false);
        break;

    default:
        return UPM_ERROR_INVALID_PARAMETER;
    }

    if (rv == MRAA_SUCCESS)
        return UPM_SUCCESS;
    else
        return UPM_ERROR_OPERATION_FAILED;
}

bool rn2903_find(const rn2903_context dev, const char *buffer, const char *str)
{
    assert(dev != NULL);
    assert(buffer != NULL);
    assert(str != NULL);

    return ((strstr(buffer, str)) ? true : false);
}


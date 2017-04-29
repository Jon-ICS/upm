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

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// maximum buffer size
#define RN2903_MAX_BUFFER (512)

// size of hex encoded 64b EUI (IPV6 Extended Unique Identifier)
#define RN2903_MAX_HEX_EUI64  (16) 


    // This byte sequence must follow all commands.  All responses
    // will also be followed by these bytes (\r\n - CR LF).
    const char *RN2903_PHRASE_TERM = "\r\n";
    const size_t RN2903_PHRASE_TERM_LEN = 2;

    // invalid parameter
    const char *RN2903_PHRASE_INV_PARAM = "invalid_param";

    // ok
    const char *RN2903_PHRASE_OK = "ok";

    // RN2903_MAC_STATUS_BITS_T from "mac get status" cmd
    typedef enum {
        RN2903_MAC_STATUS_JOINED          = 0x0001,

        RN2903_MAC_STATUS_MAC_STATUS0     = 0x0002,
        RN2903_MAC_STATUS_MAC_STATUS1     = 0x0004,
        RN2903_MAC_STATUS_MAC_STATUS2     = 0x0008,
        _RN2903_MAC_STATUS_MAC_STATUS_MASK = 7,
        _RN2903_MAC_STATUS_MAC_STATUS_SHIFT = 1,

        RN2903_MAC_STATUS_AUTO_REPLY      = 0x0010,
        RN2903_MAC_STATUS_ADR             = 0x0020,
        RN2903_MAC_STATUS_SILENT          = 0x0040,
        RN2903_MAC_STATUS_PAUSED          = 0x0080,
        RN2903_MAC_STATUS_RFU             = 0x0100,
        RN2903_MAC_STATUS_LINK_CHK        = 0x0200,

        RN2903_MAC_STATUS_CHAN_UPD        = 0x0400,
        RN2903_MAC_STATUS_OUT_PWR_UPD     = 0x0800,
        RN2903_MAC_STATUS_NBREP_UPD       = 0x1000,
        RN2903_MAC_STATUS_PRESCALER_UPD   = 0x2000,
        RN2903_MAC_STATUS_SECOND_RX_UPD   = 0x4000,
        RN2903_MAC_STATUS_TX_TIMING_UPD   = 0x8000,
    } RN2903_MAC_STATUS_BITS_T;

    // RN2903_MAC_STATUS_MAC_STATUS values
    typedef enum {
        RN2903_MAC_STAT_IDLE                  = 0,
        RN2903_MAC_STAT_TX_IN_PROGESS         = 1,
        RN2903_MAC_STAT_BEFORE_RX_WIN1        = 2,
        RN2903_MAC_STAT_RX_WIN1_OPEN          = 3,
        RN2903_MAC_STAT_BETWEEN_RX_WIN1_WIN2  = 4,
        RN2903_MAC_STAT_RX_WIN2_OPEN          = 5,
        RN2903_MAC_STAT_ACK_TIMEOUT           = 6,
    } RN2903_MAC_STATUS_T;

    // Join types
    typedef enum {
        RN2903_JOIN_TYPE_OTAA                 = 0, // over-the-air-activation
        RN2903_JOIN_TYPE_ABP                  = 1, // activation-by
                                                   // personalization
    } RN2903_JOIN_TYPE_T;

    // Join status
    typedef enum {
        RN2903_JOIN_STATUS_ACCEPTED           = 0,
        RN2903_JOIN_STATUS_BAD_KEYS,
        RN2903_JOIN_STATUS_NO_CHAN,
        RN2903_JOIN_STATUS_SILENT,
        RN2903_JOIN_STATUS_BUSY,
        RN2903_JOIN_STATUS_MAC_PAUSED,
        RN2903_JOIN_STATUS_DENIED,
        RN2903_JOIN_STATUS_ALREADY_JOINED,
        RN2903_JOIN_STATUS_UPM_ERROR,
    } RN2903_JOIN_STATUS_T;

    // possible flow control methods
    typedef enum {
        RN2903_FLOW_CONTROL_NONE              = 0,
        RN2903_FLOW_CONTROL_HARD,          // hardware flow control
        RN2903_FLOW_CONTROL_SOFT           // software flow control
    } RN2903_FLOW_CONTROL_T;

    // last command status
    typedef enum {
        RN2903_RESPONSE_OK                    = 0, // "ok", or data
        RN2903_RESPONSE_INVALID_PARAM         = 1, // "invalid_param"
        RN2903_RESPONSE_TIMEOUT               = 3,
        RN2903_RESPONSE_UPM_ERROR             = 4,
    } RN2903_RESPONSE_T;

#ifdef __cplusplus
}
#endif

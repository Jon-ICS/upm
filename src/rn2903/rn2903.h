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

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include <upm.h>
#include <mraa/uart.h>

#include "rn2903_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * @file rn2903.h
     * @library rn2903
     * @brief Generic API for the Microchip RN2903 LoRa Radio
     *
     */

    /**
     * Device context
     */
    typedef struct _rn2903_context {
        mraa_uart_context        uart;

        // contents of last command response, stripped of CR/LF
        char                     resp_data[RN2903_MAX_BUFFER];
        // length of response data
        size_t                   resp_len;

        // These are allocated buffers we use reuse internally to
        // store conversion from/to hex and back.  We manage these
        // internally so that users don't have to.
        char                     *to_hex_buf;
        char                     *from_hex_buf;

        // maximum time to wait for a response after a command is
        // submitted
        int                      cmd_resp_wait_ms;

        // if enabled, print all commands and responses
        bool                     debug_cmd_resp;

        // Our hardware hex encoded EUI + terminating NULL
        char                     hardware_eui[RN2903_MAX_HEX_EUI64 + 1];

        // 16b mac status word
        uint16_t                 mac_status_word;
        // this is the mac_status bitfield of the mac status 16b word
        RN2903_MAC_STATUS_T      mac_mac_status;

    } *rn2903_context;

    /**
     * RN2903 Initializer for generic UART operation using a UART index.
     *
     * @param uart Specify which uart to use.
     * @param baudrate Specify the baudrate to use.
     * @return an initialized device context on success, NULL on error.
     */
    rn2903_context rn2903_init(unsigned int uart, unsigned int baudrate);

    /**
     * RN2903 Initializer for generic UART operation using a filesystem
     * tty path (eg. /dev/ttyUSB0).
     *
     * @param uart_tty character string representing a filesystem path to a
     * serial tty device.
     * @param baudrate Specify the baudrate to use.
     * @return an initialized device context on success, NULL on error.
     */
    rn2903_context rn2903_init_tty(const char *uart_tty, unsigned int baudrate);

    /**
     * RN2903 sensor close function
     *
     * @param dev Device context
     */
    void rn2903_close(rn2903_context dev);

    /**
     * Read character data from the device.
     *
     * @param dev Device context
     * @param buffer The character buffer to read data into.
     * @param len The maximum size of the buffer
     * @return The number of bytes successfully read, or -1 on error
     */
    int rn2903_read(const rn2903_context dev, char *buffer, size_t len);

    /**
     * Write character data to the device.
     *
     * @param dev Device context
     * @param buffer The character buffer containing data to write.
     * @param len The number of bytes to write.
     * @return The number of bytes successfully written, or -1 on error.
     */
    int rn2903_write(const rn2903_context dev, const char *buffer, size_t len);

    /**
     * Set the baudrate of the device.
     *
     * @param dev Device context
     * @param baudrate The baud rate to set for the device.
     * @return UPM result
     */
    upm_result_t rn2903_set_baudrate(const rn2903_context dev,
                                     unsigned int baudrate);

    /**
     * Set the default time, in milliseconds, to wait for data to
     * arrive after sending a command.
     *
     * @param dev Device context
     * @param wait_ms The response delay to set, in milliseconds.
     */
    void rn2903_set_response_wait_time(const rn2903_context dev,
                                       unsigned int wait_ms);

    /**
     * Determine whether there is data available to be read.  This
     * function will wait up to "millis" milliseconds for data to
     * become available.
     *
     * @param dev Device context
     * @param millis The number of milliseconds to wait for data to
     * become available.
     * @return true if data is available to be read, false otherwise.
     */
    bool rn2903_data_available(const rn2903_context dev,
                               unsigned int millis);

    /**
     * Read and throw away any data currently available to be read.
     * This is useful to avoid reading data that might have been the
     * result of a previous command interfering with data you
     * currently want to read.  This function is automatically called
     * by rn2903_command(), and rn2903_command_waitfor() prior to
     * writing the requested command to the device.
     *
     * @param dev Device context
     */
    void rn2903_drain(const rn2903_context dev);

    /**
     * Send an AT command and optionally return a response.
     *
     * @param dev Device context
     * @param cmd A character string containing the AT command to
     * send, including the "AT" prefix and a terminating carriage
     * return ("\r").
     * @param resp A pointer to a buffer that will contain the
     * response.  If NULL is specified, the response is ignored.  The
     * returned string buffer will be 0 terminated like any ordinary C
     * string.
     * @param resp_len The length of the supplied response buffer.  If
     * 0, then any response will be ignored.  No more than resp_len
     * characters (including the trailing 0 byte) will be returned.
     * @return The number of bytes read, or -1 on error.
     */
    RN2903_RESPONSE_T rn2903_command(const rn2903_context dev,
                                       const char *cmd);
    RN2903_RESPONSE_T rn2903_waitfor_response(const rn2903_context dev,
                                              int wait_ms);

    const char *rn2903_get_response(const rn2903_context dev);
    size_t rn2903_get_response_len(const rn2903_context dev);

    const char *rn2903_to_hex(const rn2903_context dev,
                              const void *src, int len);
    const char *rn2903_from_hex(const rn2903_context dev,
                                const void *src);

    RN2903_JOIN_STATUS_T rn2903_join(const rn2903_context dev,
                                     RN2903_JOIN_TYPE_T type);

    const char *rn2903_get_hardware_eui(const rn2903_context dev);

    upm_result_t rn2903_update_mac_status(const rn2903_context dev);
    uint16_t rn2903_get_mac_status_word(const rn2903_context dev);
    RN2903_MAC_STATUS_T rn2903_get_mac_status(const rn2903_context dev);

    upm_result_t rn2903_reset(const rn2903_context dev);
    void rn2903_set_debug_cmd(const rn2903_context dev, bool enable);



    /**
     * Set a flow control method for the UART.  By default, during
     * initialization, flow control is disabled.
     *
     * @param dev Device context
     * @param fc One of the RN2903_FLOW_CONTROL_T values.
     * @return the UPM result.
     */
    upm_result_t rn2903_set_flow_control(const rn2903_context dev,
                                         RN2903_FLOW_CONTROL_T fc);

    /**
     * Look for a string in a buffer.  This is a utility function that
     * can be used to indicate if a given string is present in a
     * supplied buffer.  The search is case sensitive.
     *
     * @param dev Device context
     * @param buffer The 0 teminated buffer in which to search.
     * @param str The 0 teminated string to search for.
     * @return true if the string was found, false otherwise.
     */
    bool rn2903_find(const rn2903_context dev, const char *buffer,
                     const char *str);

#ifdef __cplusplus
}
#endif

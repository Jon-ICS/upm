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

#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "rn2903.h"
#include "upm_utilities.h"

int main(int argc, char **argv)
{
//! [Interesting]

    char *defaultDev = "/dev/ttyUSB0";
    if (argc > 1)
        defaultDev = argv[1];

    printf("Using device: %s\n", defaultDev);
    // Instantiate a RN2903 sensor on defaultDev at 57600 baud.
    rn2903_context sensor = rn2903_init_tty(defaultDev, 57600);

    if (!sensor)
    {
        printf("rn2903_init_tty() failed.\n");
        return 1;
    }

    // enable cmd/resp output debugging
    rn2903_set_debug_cmd(sensor, true);

    // get version
    if (rn2903_command(sensor, "sys get ver"))
    {
        printf("Failed to retrieve device version string\n");
        rn2903_close(sensor);
        return 1;
    }
    printf("Firmware version: %s\n", rn2903_get_response(sensor));

    printf("Hardware EUI: %s\n", rn2903_get_hardware_eui(sensor));


#if 1
    rn2903_command(sensor, "sys get vdd");
#endif

    rn2903_update_mac_status(sensor);
    printf("status %04x, mac_mac_status %d\n", sensor->mac_status_word,
           sensor->mac_mac_status);


// test some conversions...
#if 1

    char *str = "Hi there big guy!";
    const char *hptr = rn2903_to_hex(sensor, str, strlen(str));
    printf("convert string (%s) to hex: '%s'\n", str, hptr);
    const char *dptr = rn2903_from_hex(sensor, hptr);
    printf("convert back to str: '%s'\n", dptr);

    int i = 12345;
    const char *iptr = rn2903_to_hex(sensor, &i, sizeof(int));
    printf("convert int (%d) to hex: '%s'\n", i, iptr);
    int *idptr = (int *)rn2903_from_hex(sensor, iptr);
    printf("convert back to int: '%d'\n", *idptr);

    float f = -12345.67;
    const char *fptr = rn2903_to_hex(sensor, &f, sizeof(float));
    printf("convert float (%f) to hex: '%s'\n", f, fptr);
    float *fdptr = (float *)rn2903_from_hex(sensor, fptr);
    printf("convert back to float: '%f'\n", *fdptr);
#endif

    printf("\n\n");

#if 0
    // for OTAA, need dev eui, app eui, app key
    rn2903_set_device_eui(sensor, "0011223344556677");
    rn2903_set_application_eui(sensor, "0011223344556677");
    rn2903_set_application_key(sensor, "01234567012345670123456701234567");
    RN2903_JOIN_STATUS_T rv = rn2903_join(sensor, RN2903_JOIN_TYPE_OTAA);
    printf("JOIN: got rv %d\n", rv);
#endif

    // for ABP, need dev addr, net session key, app session key

    rn2903_set_device_addr(sensor, "00112233");
    rn2903_set_network_session_key(sensor, "00112233001122330011223300112233");
    rn2903_set_application_session_key(sensor,
                                       "00112233001122330011223300112233");
    RN2903_JOIN_STATUS_T rv = rn2903_join(sensor, RN2903_JOIN_TYPE_ABP);
    printf("JOIN: got rv %d\n", rv);

    rn2903_update_mac_status(sensor);
    printf("status %04x, mac_mac_status %d\n", sensor->mac_status_word,
           sensor->mac_mac_status);

    if (rn2903_get_mac_status_word(sensor) & RN2903_MAC_STATUS_JOINED)
        printf("JOINED\n");
    else
        printf("NOT JOINED\n");

    printf("Transmitting a packet.... \"AABBCCDDEEFF\"\n");

    RN2903_MAC_TX_STATUS_T trv;
    trv = rn2903_mac_tx(sensor, RN2903_MAC_MSG_TYPE_UNCONFIRMED,
                        20, "AABBCCDDEEFF");
    printf("MAC TX: got trv %d\n", trv);


    printf("Exiting\n");

    rn2903_close(sensor);

//! [Interesting]

    return 0;
}

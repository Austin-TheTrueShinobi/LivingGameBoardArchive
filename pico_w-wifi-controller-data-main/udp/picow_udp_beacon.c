/**
 * @file udp_beacon.c
 * @brief UDP Beacon Sender for Pico_W using CYW43 Wi-Fi module.
 *
 * This program demonstrates UDP communication using lwIP stack and CYW43 Wi-Fi module.
 * It sends UDP packets to a specified target IP address and port at regular intervals.
 *
 */

#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"

#define UDP_PORT 4444 ///< UDP port to send packets to.
#define BEACON_MSG_LEN_MAX 127 ///< Maximum length of the beacon message.
#define BEACON_TARGET "255.255.255.255" ///< Target IP address for broadcasting UDP packets. THIS IS THE STATIC METHOD
#define BEACON_INTERVAL_MS 1000 ///< Interval between UDP packets in milliseconds.

/**
 * @brief Sends UDP beacon packets to a specified IP address and port.
 *
 * This function initializes a UDP socket and sends UDP packets with a counter value to
 * the specified target IP address and port at regular intervals.
 */
void run_udp_beacon() {
    struct udp_pcb* pcb = udp_new(); // Create a new UDP control block.

    ip_addr_t addr;
    ipaddr_aton(BEACON_TARGET, &addr); // Convert target IP address from string to IP format.

    int counter = 0; // Counter for the UDP packets.

    while (true) {
        // Allocate a new pbuf (packet buffer) for the UDP packet.
        struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, BEACON_MSG_LEN_MAX + 1, PBUF_RAM);
        char *req = (char *)p->payload; // Get pointer to the payload of the pbuf.
        memset(req, 0, BEACON_MSG_LEN_MAX + 1); // Clear the payload buffer.

        // Format the payload with the counter value and a newline character.
        snprintf(req, BEACON_MSG_LEN_MAX, "%d\n", counter);

        // Send the UDP packet to the specified address and port.
        err_t er = udp_sendto(pcb, p, &addr, UDP_PORT);
        pbuf_free(p); // Free the allocated pbuf.

        if (er != ERR_OK) {
            printf("Failed to send UDP packet! error=%d", er);
        } else {
            printf("Sent packet %d\n", counter);
            counter++;
        }

        // Note: In practice, the end result for both background and poll methods is the same.

#if PICO_CYW43_ARCH_POLL
        // If using pico_cyw43_arch_poll, poll periodically from the main loop
        // to check for Wi-Fi driver or lwIP work that needs to be done.
        cyw43_arch_poll();
        sleep_ms(BEACON_INTERVAL_MS);
#else
        // If not using pico_cyw43_arch_poll, Wi-Fi driver and lwIP work
        // is done via interrupt in the background. This sleep represents
        // an example of some (blocking) work you might be doing.
        sleep_ms(BEACON_INTERVAL_MS);
#endif
    }
}

/**
 * @brief Main function.
 *
 * This function initializes the Wi-Fi module, establishes a connection to the
 * specified Wi-Fi network, and then starts sending UDP beacon packets.
 *
 * @return 0 if successful, non-zero on failure.
 */
int main() {
    stdio_init_all();

    if (cyw43_arch_init()) {
        printf("Failed to initialize CYW43 Wi-Fi module\n");
        return 1;
    }

    cyw43_arch_enable_sta_mode(); // Enable station mode for Wi-Fi module.

    printf("Connecting to Wi-Fi...\n");

    // Attempt to connect to the specified Wi-Fi network with WPA2 security.
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("Failed to connect to Wi-Fi\n");
        return 1;
    } else {
        printf("Connected to Wi-Fi\n");
    }

    run_udp_beacon(); // Start sending UDP beacon packets.
    
    cyw43_arch_deinit(); // Deinitialize the Wi-Fi module.
    return 0;
}

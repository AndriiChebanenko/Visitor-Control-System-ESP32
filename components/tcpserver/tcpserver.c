#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <esp_wifi.h>
#include "esp_netif.h"
#include "esp_log.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "tcpserver.h"
#include "components_log_props.h"

int tcpserver_init() {
    // Create the server socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        return -1;
    }

    // Define the server address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8083);	
    server_address.sin_addr.s_addr = inet_addr("192.168.43.43");

    // Bind the socket to the IP and port
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Bind failed");
        close(server_socket); // Clean up on error
        return -1;
    }

    // Start listening for connections
    if (listen(server_socket, 5) < 0) {
        perror("Listen failed");
        close(server_socket); // Clean up on error
        return -1;
    }
	#ifdef COMPONENTS_LOG_ON
    	printf("Server is listening on 10.10.10.251:8083\n");
    #endif
    return server_socket;
}

void tcpserver_deinit(int server_socket) {
	close(server_socket);
}

esp_ip4_addr_t get_ip_address() {
    esp_netif_ip_info_t ip_info;
    esp_ip4_addr_t ip_address = {0};
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");

    if (netif == NULL) {
		#ifdef COMPONENTS_LOG_ON
        	ESP_LOGE("IP_INFO", "Failed to get netif handle");
        #endif
        return ip_address; // Повертаємо 0.0.0.0
    }

    esp_err_t ret = esp_netif_get_ip_info(netif, &ip_info);
    if (ret == ESP_OK) {
        ip_address = ip_info.ip; // Повертаємо IP-адресу
    } else {
		#ifdef COMPONENTS_LOG_ON
        	ESP_LOGE("IP_INFO", "Failed to get IP info: %s", esp_err_to_name(ret));
        #endif
    }

    return ip_address;
}
/*
 * tcpserver.c
 *
 *  Created on: 2 груд. 2024 р.
 *      Author: andri
 */

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

int tcpserver_init() {
	// Створення серверного сокета
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // Визначення адреси сервера
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8083);
    
    esp_ip4_addr_t ip = get_ip_address();
	char ip_str[16];
	ip_to_string(&ip, ip_str, sizeof(ip_str));
	ESP_LOGI("IP", "IP Address as string: %s", ip_str);

    server_address.sin_addr.s_addr = inet_addr("192.168.43.43");

    // Прив'язка сокета до IP та порта
    bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address));

    // Прослуховування підключень
    listen(server_socket, 5);

    printf("Server is listening on 10.10.10.251:8083\n");
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
        ESP_LOGE("IP_INFO", "Failed to get netif handle");
        return ip_address; // Повертаємо 0.0.0.0
    }

    esp_err_t ret = esp_netif_get_ip_info(netif, &ip_info);
    if (ret == ESP_OK) {
        ip_address = ip_info.ip; // Повертаємо IP-адресу
    } else {
        ESP_LOGE("IP_INFO", "Failed to get IP info: %s", esp_err_to_name(ret));
    }

    return ip_address;
}

void ip_to_string(const esp_ip4_addr_t *ip, char *ip_str, size_t len) {
    if (ip == NULL || ip_str == NULL || len < 16) { // IPv4 максимум 15 символів + термінатор
        if (ip_str != NULL && len > 0) {
            snprintf(ip_str, len, "Invalid");
        }
        return;
    }
    snprintf(ip_str, len, IPSTR, IP2STR(ip));
}

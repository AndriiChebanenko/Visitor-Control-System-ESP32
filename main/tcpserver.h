/*
 * tcpserver.h
 *
 *  Created on: 2 груд. 2024 р.
 *      Author: andri
 */

 #pragma once
 
 #include "esp_netif_ip_addr.h"
 
int tcpserver_init();
void tcpserver_deinit(int server_socket);
esp_ip4_addr_t get_ip_address();
void ip_to_string(const esp_ip4_addr_t *ip, char *ip_str, size_t len);
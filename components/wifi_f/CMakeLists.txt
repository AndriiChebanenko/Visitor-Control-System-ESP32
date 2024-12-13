idf_component_register(
	SRCS "wifi_f.c"
    INCLUDE_DIRS "." ".."
    REQUIRES esp_netif esp_wifi nvs_flash
    )

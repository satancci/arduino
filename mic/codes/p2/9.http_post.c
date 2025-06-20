#include <stdio.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG "Wi-Fi STA"
#define WIFI_SSID "Nome da rede"
#define WIFI_PASS "Senha da rede"

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data){
    if (event_base == WIFI_EVENT)
    {
        switch (event_id)
        {
        case WIFI_EVENT_STA_START:
        {
            ESP_LOGI(TAG, "Wi-Fi iniciado. Conectando-se...");
            esp_wifi_connect();
            break;
        };

        case WIFI_EVENT_STA_DISCONNECTED:
        {
            ESP_LOGW(TAG, "Wi-Fi desconectado. Tentando reconectar...");
            esp_wifi_connect();
            break;
        };

        default:
        {
            ESP_LOGI(TAG, "Evento Wi-Fi nao tratado: %d", (int)event_id);
            break;
        }
        }
    }
    else if (event_base == IP_EVENT)
    {
        switch (event_id)
        {
        case IP_EVENT_STA_GOT_IP:
        {
            ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
            ESP_LOGI(TAG, "Conectado com IP: " IPSTR,
                     IP2STR(&event->ip_info.ip));
            break;
        };

        default:
            break;
        }
    }
}

void nvs_init(){
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        nvs_flash_erase();
        ret = nvs_flash_init();
    }
};

void event_loop_init(){
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                        &wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                        &wifi_event_handler, NULL, NULL);
};

void wifi_init(){
    wifi_init_config_t wifi_cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_cfg);
    esp_wifi_set_mode(WIFI_MODE_STA);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS},
    };

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();
};

void setup(){
    nvs_init();
    event_loop_init();
    wifi_init();
}

void loop(){
    vTaskDelay(pdMS_TO_TICKS(1000));
}

void app_main()
{
    setup();
    while(1){loop();}
};
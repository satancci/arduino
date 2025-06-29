// Os ESP_LOG ESTÃO TROCADOS para usar o simulador:
// ESP_LOGE está como ESP_LOGW
// ESP_LOGI está como ESP_LOGE
// Para consertar, altere:
// ESP_LOGE -> ESP_LOGI
// ESP_LOGW -> ESP_LOGE



#include "stdio.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/i2c_master.h"
#include <string.h>
#include <stdlib.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_http_client.h"

#define TAG_HTTP "HTTP_CLIENT"
#define TAG "Wi-Fi STA"
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASS ""
#define WIFI_CONNECTED_BIT BIT0

i2c_master_bus_handle_t master_handle;
i2c_master_dev_handle_t slave_cartao_handle;
i2c_master_dev_handle_t slave_lcd_handle;

static EventGroupHandle_t eventos_i2c;
const int i2c_event_cfg_master = BIT0;
const int i2c_event_cfg_slave_cartao = BIT1;
const int i2c_event_cfg_slave_lcd = BIT2;

static EventGroupHandle_t eventos_configs;
const int config_event_io_out = BIT0;
const int config_event_io_in = BIT1;
const int config_event_i2c = BIT2;
const int config_event_wifi = BIT3;

static EventGroupHandle_t wifi_event_group;
static char *response_buffer = NULL;
static int response_len = 0;

static EventGroupHandle_t eventos_wifi_config;
const int nvs_cfg = BIT0;
const int event_loop_cfg = BIT1;
const int wifi_init_cfg = BIT2;

SemaphoreHandle_t mutex;
SemaphoreHandle_t semaf_bin;
TaskHandle_t lido = NULL;

uint8_t data_wr[] = {0x00};
uint8_t data_rd[8];

void configura_master(void *master_params){
    // Configuração do barramento I2C mestre
    i2c_master_bus_config_t config_mst={
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = 3,
        .sda_io_num = 4,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true
    };
    esp_err_t status = i2c_new_master_bus(&config_mst, &master_handle);
    if (status != ESP_OK) {
        ESP_LOGW("SETUP -> CONFIG -> I2C", "Falha ao criar o barramento mestre!");
        vTaskDelete(NULL);
    } 
    ESP_LOGE("SETUP -> CONFIG -> I2C", "Sucesso ao criar o barramento mestre!");
    xEventGroupSetBits(eventos_i2c, i2c_event_cfg_master);
    vTaskDelete(NULL);
}

void configura_slave_cartao(void *slave_params){
    // Configuração do dispositivo escravo I2C
    xEventGroupWaitBits(eventos_i2c, i2c_event_cfg_master, pdFALSE, pdFALSE, portMAX_DELAY);
    i2c_device_config_t config_dev = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x50,
        .scl_speed_hz = 100000
    };
    if (i2c_master_bus_add_device(master_handle, &config_dev, &slave_cartao_handle) != ESP_OK) {
        ESP_LOGW("SETUP -> CONFIG -> I2C", "Falha ao adicionar o dispositivo escravo LEITOR DE CARTÃO!");
        vTaskDelete(NULL);
    }
    ESP_LOGE("SETUP -> CONFIG -> I2C", "Sucesso ao adicionar o dispositivo escravo LEITOR DE CARTÃO!");
    xEventGroupSetBits(eventos_i2c, i2c_event_cfg_slave_cartao);
    vTaskDelete(NULL);
}

void configura_slave_lcd(void *slave_params){
    // Configuração do dispositivo escravo I2C para o LCD
    xEventGroupWaitBits(eventos_i2c, i2c_event_cfg_master, pdFALSE, pdFALSE, portMAX_DELAY);
    i2c_device_config_t config_dev = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x27,
        .scl_speed_hz = 100000
    };
    if (i2c_master_bus_add_device(master_handle, &config_dev, &slave_lcd_handle) != ESP_OK) {
        ESP_LOGW("SETUP -> CONFIG -> I2C", "Falha ao adicionar o dispositivo escravo LIQUID CRYSTAL DISPLAY!");
        vTaskDelete(NULL);
    }
    ESP_LOGE("SETUP -> CONFIG -> I2C", "Sucesso ao adicionar o dispositivo escravo LIQUID CRYSTAL DISPLAY!");
    xEventGroupSetBits(eventos_i2c, i2c_event_cfg_slave_lcd);
    vTaskDelete(NULL);
}

/*ignore, por enquanto!
void escrita_leitura_i2c(){
    i2c_master_transmit_receive(slave_cartao_handle, data_wr, sizeof(data_wr), data_rd, sizeof(data_rd), portMAX_DELAY);
}*/

void configura_io_out(void *io_params){
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_NUM_0) | (1ULL << GPIO_NUM_9),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    if(gpio_config(&io_conf) != ESP_OK){
        ESP_LOGW("SETUP -> CONFIG -> SAÍDAS", "Falha ao configurar GPIO das SAÍDAS!");
        vTaskDelete(NULL);
    }
    ESP_LOGE("SETUP -> CONFIG", "Configuração de GPIO das SAÍDAS concluída com sucesso!");
    xEventGroupSetBits(eventos_configs, config_event_io_out);
    vTaskDelete(NULL);
}

void configura_io_in(void *io_params){
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_NUM_2),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    esp_err_t ret = gpio_config(&io_conf);

    if(ret != ESP_OK){
        ESP_LOGW("SETUP -> CONFIG -> ENTRADAS", "Falha ao configurar GPIO das ENTRADAS!");
        vTaskDelete(NULL);
    }
    ESP_LOGE("SETUP -> CONFIG", "Configuração de GPIO das ENTRADAS concluída com sucesso!");
    xEventGroupSetBits(eventos_configs, config_event_io_in);
    vTaskDelete(NULL);
}

void configura_i2c(void *i2c_params){
    eventos_i2c = xEventGroupCreate();
    if (eventos_i2c == NULL) {
        ESP_LOGW("SETUP -> CONFIG", "Falha ao criar o grupo de eventos I2C!");
        vTaskDelete(NULL);
    }
    ESP_LOGE("SETUP -> CONFIG", "Grupo de eventos I2C criado com sucesso!");

    xTaskCreate(configura_master, "Configura Master I2C", 2048, NULL, 2, NULL);
    xTaskCreate(configura_slave_cartao, "Configura Slave Cartão I2C", 2048, NULL, 1, NULL);
    xTaskCreate(configura_slave_lcd, "Configura Slave LCD I2C", 2048, NULL, 1, NULL);
    
    xEventGroupWaitBits(eventos_i2c, i2c_event_cfg_slave_cartao | i2c_event_cfg_slave_lcd, pdFALSE, pdTRUE, portMAX_DELAY);
    ESP_LOGE("SETUP -> CONFIG", "Configuração do I2C concluída com sucesso!");
    xEventGroupSetBits(eventos_configs, config_event_i2c);
    vTaskDelete(NULL);
}

void nvs_init(void *nvs_params){
    init_nvs:
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        nvs_flash_erase();
        ESP_LOGW("SETUP -> CONFIG -> WIFI -> NVS", "Flash sem memória não inicializado ou versão nova encontrada.");
        ESP_LOGE("SETUP -> CONFIG -> WIFI -> NVS", "Flash apagado e reinicializado!");
        goto init_nvs;

    }
    ESP_LOGE("SETUP -> CONFIG -> WIFI", "NVS Flash inicializado com sucesso!");
    xEventGroupSetBits(eventos_wifi_config, nvs_cfg);
    vTaskDelete(NULL);
    
};

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data){
    if (event_base == WIFI_EVENT)
    {
        switch (event_id)
        {
        case WIFI_EVENT_STA_START:
        {
            ESP_LOGE(TAG, "Wi-Fi iniciado. Conectando-se...");
            esp_wifi_connect();
            break;
        };

        case WIFI_EVENT_STA_DISCONNECTED:
        {
            xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
            ESP_LOGW(TAG, "Wi-Fi desconectado. Tentando reconectar...");
            esp_wifi_connect();
            break;
        };

        default:
        {
            ESP_LOGE(TAG, "Evento Wi-Fi nao tratado: %d", (int)event_id);
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
            ESP_LOGE(TAG, "Conectado com IP: " IPSTR,
                     IP2STR(&event->ip_info.ip));
            xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
            break;
        };

        default:
            break;
        }
    }
};

void event_loop_init(void *event_loop_params){
    if(xEventGroupWaitBits(eventos_wifi_config, nvs_cfg, pdFALSE, pdTRUE, pdMS_TO_TICKS(1000)) == 0){
        ESP_LOGW("SETUP -> CONFIG -> WIFI -> EVENT_LOOP", "Falha (TIMEOUT) ao esperar pelo evento de NVS!");
        vTaskDelete(NULL);
    }

    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                        &wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                        &wifi_event_handler, NULL, NULL);

    ESP_LOGE("SETUP -> CONFIG -> WIFI", "Loop de eventos Wi-Fi inicializado com sucesso!");
    xEventGroupSetBits(eventos_wifi_config, event_loop_cfg);
    vTaskDelete(NULL);
};

void wifi_init(void *wifi_params){
    if (xEventGroupWaitBits(eventos_wifi_config, event_loop_cfg, pdFALSE, pdTRUE, pdMS_TO_TICKS(1000)) == 0)
    {
        ESP_LOGW("SETUP -> CONFIG -> WIFI -> WIFI_INIT", "Falha (TIMEOUT) ao esperar pelo evento de loop de eventos do wifi!");
        vTaskDelete(NULL);
    }
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
    ESP_LOGE("SETUP -> CONFIG -> WIFI", "Wi-Fi iniciado com sucesso!");
    xEventGroupSetBits(eventos_wifi_config, wifi_init_cfg);
    vTaskDelete(NULL);
};

esp_err_t http_post_request_event_handler(esp_http_client_event_t *evt){
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        if (evt->data && evt->data_len > 0)
        {
            char *new_buf = realloc(response_buffer,
                                    response_len + evt->data_len + 1);
            if (!new_buf)
            {
                ESP_LOGW(TAG_HTTP, "Falha ao realocar buffer");
                free(response_buffer);
                response_buffer = NULL;
                response_len = 0;
                return ESP_FAIL;
            }

            response_buffer = new_buf;
            memcpy(response_buffer + response_len, evt->data, evt->data_len);
            response_len += evt->data_len;
            response_buffer[response_len] = '\0';
        };
        break;

    case HTTP_EVENT_ON_FINISH:
        if (response_buffer)
        {
            ESP_LOGE(TAG_HTTP, "Resposta recebida (%d bytes):\n%s",
                     response_len, response_buffer);
            free(response_buffer);
            response_buffer = NULL;
            response_len = 0;
        };
        break;

    case HTTP_EVENT_DISCONNECTED:
    case HTTP_EVENT_ERROR:
        if (response_buffer)
        {
            free(response_buffer);
            response_buffer = NULL;
            response_len = 0;
            ESP_LOGW(TAG_HTTP, "Requisicao terminada com erro ou desconexao");
        }
        break;

    default:
        break;
    }

    return ESP_OK;
};

void http_post_request_task(){
    while (1)
    {
        xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE,
                            pdFALSE, portMAX_DELAY);

        const char *post_data = "{\"nome\": \"Aluno\", \"nota\": 10}";

        esp_http_client_config_t config = {
            .url = "http://httpbin.org/post",
            .method = HTTP_METHOD_POST,
            .event_handler = http_post_request_event_handler,
            .timeout_ms = 5000,
            .user_agent = "esp-idf/5.x",
        };

        esp_http_client_handle_t client = esp_http_client_init(&config);
        esp_http_client_set_header(client, "Content-Type", "application/json");
        esp_http_client_set_post_field(client, post_data, strlen(post_data));

        esp_err_t err = esp_http_client_perform(client);
        if (err != ESP_OK)
        {
            ESP_LOGW(TAG_HTTP, "Erro na requisicao POST: %s", esp_err_to_name(err));
        }

        esp_http_client_cleanup(client);

        vTaskDelay(pdMS_TO_TICKS(1000 * 60 * 60)); // Repetir a cada 1 hora
    }
};

void configura_wifi(void *params){
    xEventGroupWaitBits(eventos_configs, config_event_i2c, pdFALSE, pdTRUE, portMAX_DELAY);
    wifi_event_group = xEventGroupCreate();
    if (wifi_event_group == NULL) {
        ESP_LOGW("SETUP -> CONFIG -> WIFI", "Falha ao criar o grupo de eventos Wi-Fi!");
        vTaskDelete(NULL);
    }
    eventos_wifi_config = xEventGroupCreate();
    if (eventos_wifi_config == NULL) {
        ESP_LOGW("SETUP -> CONFIG", "Falha ao criar o grupo de eventos de configuração Wi-Fi!");
        vTaskDelete(NULL);
    }
    ESP_LOGE("SETUP -> CONFIG", "Grupo de eventos Wi-Fi criado com sucesso!");
    xTaskCreate(nvs_init, "NVS Init", 4096, NULL, 2, NULL);
    xTaskCreate(event_loop_init, "Event Loop Init", 4096, NULL, 2, NULL);
    xTaskCreate(wifi_init, "Wi-Fi Init", 4096, NULL, 2, NULL);
    xEventGroupWaitBits(eventos_wifi_config, nvs_cfg | event_loop_cfg | wifi_init_cfg, 
                        pdFALSE, pdTRUE, portMAX_DELAY);
    /*xTaskCreate(&http_post_request_task, "http_post_request_task", 4096, 
                NULL, 5, NULL);*/
    
    ESP_LOGE("SETUP -> CONFIG", "Configuração do Wi-Fi concluída com sucesso!");
    xEventGroupSetBits(eventos_configs, config_event_wifi);
    vTaskDelete(NULL);
}

void verifica_cartao(void *params){
    if(xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE){
        ESP_LOGE("-> verificador", "Mutex adquirido com sucesso!");
        if (i2c_master_bus_read(slave_cartao_handle, data_rd, sizeof(data_rd), portMAX_DELAY) != ESP_OK) {
            ESP_LOGW("-> verificador", "Falha ao ler dados do cartão!");
            xSemaphoreGive(mutex);
            vTaskDelete(NULL);
            //VERICAR MELHOR ESSE IF MAIS TARDE       } 
        ESP_LOGE("-> verificador", "Dados lidos do cartão: %s", data_rd);
        xSemaphoreGive(mutex);
    } 
}
void loop(void *params){
    mutex = xSemaphoreCreateMutex();
    xSemaphoreTake(semaf_bin, portMAX_DELAY);
    xTaskCreate(verifica_cartao, "Verifica Cartão", 2048, NULL, 2, NULL);
    ESP_LOGE("LOOP", "As tarefas estão em funcionamento!");
    while(1){
        if (gpio_get_level(GPIO_NUM_2) == 1) {
            xSemaphoreGive(mutex);
            gpio_set_level(GPIO_NUM_0, 1);
            vTaskDelay(pdMS_TO_TICKS(1000));
            gpio_set_level(GPIO_NUM_0, 0);
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // Delay de 1 segundo
    }
    vTaskDelete(NULL);
}

void setup(void *params){
    eventos_configs = xEventGroupCreate();
    if (eventos_configs == NULL) {
        ESP_LOGW("SETUP", "Falha ao criar o grupo de eventos de configuração!");
        return;
    }
    ESP_LOGE("SETUP", "Grupo de eventos de configuração criado com sucesso!");

    xTaskCreate(configura_io_out, "Configura IO OUT", 2048, NULL, 2, NULL);
    xTaskCreate(configura_io_in, "Configura IO IN", 2048, NULL, 2, NULL);
    xTaskCreate(configura_i2c, "Configura I2C", 2048, NULL, 2, NULL);
    xTaskCreate(configura_wifi, "Configura Wi-Fi", 2048, NULL, 2, NULL);

    xEventGroupWaitBits(eventos_configs, config_event_io_out |
         config_event_io_in | config_event_i2c | config_event_wifi, pdFALSE, pdTRUE, portMAX_DELAY);
    ESP_LOGE("SETUP", "Configurações iniciais concluídas com sucesso!");
    xSemaphoreGive(semaf_bin);
    
    vTaskDelete(NULL);
}

void app_main(){
    semaf_bin = xSemaphoreCreateBinary();
    ESP_LOGE("MAIN", "Iniciando o aplicativo principal...");
    xTaskCreate(setup, "Setup", 4096, NULL, 10, NULL);
    xTaskCreate(loop, "Loop", 2048, NULL, 5, NULL);
}

#include "stdio.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/i2c_master.h"


i2c_master_bus_handle_t master_handle;
i2c_master_dev_handle_t slave_cartao_handle;
i2c_master_dev_handle_t slave_lcd_handle;

EventGroupHandle_t eventos_i2c;
const int i2c_event_cfg_master = BIT0;
const int i2c_event_cfg_slave_cartao = BIT1;
const int i2c_event_cfg_slave_lcd = BIT2;

EventGroupHandle_t eventos_configs;
const int config_event_io_out = BIT0;
const int config_event_io_in = BIT1;
const int config_event_i2c = BIT2;

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
        ESP_LOGE("I2C", "Falha ao criar o barramento mestre!");
        vTaskDelete(NULL);
    }
    ESP_LOGI("I2C", "Sucesso ao criar o barramento mestre!");
    xEventGroupSetBits(eventos_i2c, i2c_event_cfg_master);
    vTaskDelete(NULL);
}

void configura_slave_cartao(void *slave_params){
    // Configuração do dispositivo escravo I2C
    xEventGroupWaitBits(eventos_i2c, i2c_event_cfg_master, pdTRUE, pdFALSE, portMAX_DELAY);
    i2c_device_config_t config_dev = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x50,
        .scl_speed_hz = 100000
    };
    if (i2c_master_bus_add_device(master_handle, &config_dev, &slave_cartao_handle) != ESP_OK) {
        ESP_LOGE("I2C", "Falha ao adicionar o dispositivo escravo LEITOR DE CARTÃO!");
        vTaskDelete(NULL);
    }
    ESP_LOGI("I2C", "Sucesso ao adicionar o dispositivo escravo LEITOR DE CARTÃO!");
    xEventGroupSetBits(eventos_i2c, i2c_event_cfg_slave_cartao);
    vTaskDelete(NULL);
}

void configura_slave_lcd(void *slave_params){
    // Configuração do dispositivo escravo I2C para o LCD
    xEventGroupWaitBits(eventos_i2c, i2c_event_cfg_slave_cartao, pdTRUE, pdFALSE, portMAX_DELAY);
    i2c_device_config_t config_dev = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x27,
        .scl_speed_hz = 100000
    };
    if (i2c_master_bus_add_device(master_handle, &config_dev, &slave_lcd_handle) != ESP_OK) {
        ESP_LOGE("I2C", "Falha ao adicionar o dispositivo escravo LIQUID CRYSTAL DISPLAY!");
        vTaskDelete(NULL);
    }
    ESP_LOGI("I2C", "Sucesso ao adicionar o dispositivo escravo LIQUID CRYSTAL DISPLAY!");
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
        ESP_LOGE("CONFIG", "Falha ao configurar GPIO das SAÍDAS!");
        vTaskDelete(NULL);
    }
    ESP_LOGI("CONFIG", "Configuração de GPIO das SAÍDAS concluída com sucesso!");
    xEventGroupSetBits(eventos_configs, config_event_io_out);
    vTaskDelete(NULL);
}

void configura_io_in(void *io_params){
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_NUM_2),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    if(gpio_config(&io_conf)!= ESP_OK){
        ESP_LOGE("CONFIG", "Falha ao configurar GPIO das ENTRADAS!");
        vTaskDelete(NULL);
    }
    ESP_LOGI("CONFIG", "Configuração de GPIO das ENTRADAS concluída com sucesso!");
    xEventGroupSetBits(eventos_configs, config_event_io_in);
    vTaskDelete(NULL);
}

void configura_i2c(void *i2c_params){
    eventos_i2c = xEventGroupCreate();
    if (eventos_i2c == NULL) {
        ESP_LOGE("CONFIG", "Falha ao criar o grupo de eventos I2C!");
        vTaskDelete(NULL);
    }
    ESP_LOGI("CONFIG", "Grupo de eventos I2C criado com sucesso!");

    xTaskCreate(configura_master, "Configura Master I2C", 2048, NULL, 2, NULL);
    xTaskCreate(configura_slave_cartao, "Configura Slave Cartão I2C", 2048, NULL, 1, NULL);
    xTaskCreate(configura_slave_lcd, "Configura Slave LCD I2C", 2048, NULL, 1, NULL);

    xEventGroupWaitBits(eventos_i2c, i2c_event_cfg_slave_cartao | i2c_event_cfg_slave_lcd, pdTRUE, pdTRUE, portMAX_DELAY);
    ESP_LOGI("CONFIG", "Configuração do I2C concluída com sucesso!");
    xEventGroupSetBits(eventos_configs, config_event_i2c);
    vTaskDelete(NULL);
}

void setup(){
    eventos_configs = xEventGroupCreate();
    if (eventos_configs == NULL) {
        ESP_LOGE("SETUP", "Falha ao criar o grupo de eventos de configuração!");
        return;
    }
    ESP_LOGI("SETUP", "Grupo de eventos de configuração criado com sucesso!");

    xTaskCreate(configura_io_out, "Configura IO OUT", 2048, NULL, 2, NULL);
    xTaskCreate(configura_io_in, "Configura IO IN", 2048, NULL, 2, NULL);
    xTaskCreate(configura_i2c, "Configura I2C", 2048, NULL, 2, NULL);

    xEventGroupWaitBits(eventos_configs, config_event_io_out | config_event_io_in | config_event_i2c, pdTRUE, pdTRUE, portMAX_DELAY);
    ESP_LOGI("SETUP", "Configurações iniciais concluídas com sucesso!");
}

void loop(){

}

void app_main(){
    setup();
    while(1){loop();}
}

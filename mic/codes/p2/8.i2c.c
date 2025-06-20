#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "stdio.h"

i2c_master_master_handle_t master_handle;
i2c_master_dev_handle_t slave_handle;

uint8_t data_wr[] = {0x00};
uint8_t data_rd[8];

void configura_master(){
    i2c_master_bus_config_t config_mst={
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = 3,
        .sda_io_num = 4,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true
    };
    i2c_new_master_bus(&config_mst, &master_handle);
}

void configura_slave(){
    i2c_device_config_t config_dev = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x50,
        .scl_speed_hz = 100000
    };
    i2c_master_bus_add_device(master_handle, &config_dev, &slave_handle);
}

void wr(){
    i2c_master_transmit_receive(slave_handle, data_wr, sizeof(data_wr), data_rd, sizeof(data_rd), portMAX_DELAY);
}

void setup(){
    configura_master();
    configura_slave();
    wr();
    printf("dados: %s\n", data_rd);
}

void loop(){}

void app_main(void)
{
    setup();
    while (1) {
        loop();
    }
}
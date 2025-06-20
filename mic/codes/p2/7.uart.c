#include "driver/uart.h"
#include "stdio.h"
#include "string.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

QueueHandle_t uart_queue;

void inicia_uart(void){
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, 8 , 10, -1 , -1);
    uart_driver_install(UART_NUM_1, 256, 0, 10, &uart_queue, 0);
}

void uart_event_task(void *pvParameters){
    uart_event_t evento;
    uint8_t data[256];
    while(1){
        if(xQueueReceive(uart_queue, (void *)&evento, portMAX_DELAY)){
        switch(evento.type){
            case UART_DATA:
                int len = uart_read_bytes(UART_NUM_1, data, evento.size, pdMS_TO_TICKS(20));
                if(len>0){
                    data[len]='\0';
                    printf("received: %s\n", (char *)data);
                }
                break;

            case UART_FIFO_OVF:
                uart_flush_input(UART_NUM_1);
                xQueueReset(uart_queue);
                break;

            case UART_BUFFER_FULL:
                uart_flush_input(UART_NUM_1);
                xQueueReset(uart_queue);
                break;
                
            default: break;
            }
        }
    }
}

void uart_enviador(){
    while(1){
        char zap[] = "roleta russa lgbt: 6. Voce eh muito gay";
        uart_write_bytes(UART_NUM_1, zap, strlen(zap));
        vTaskDelay(100);
    }
}

void app_main(void){
    inicia_uart();
    xTaskCreate(uart_event_task, "UART Recebe", 2048, NULL, 2, NULL);
    xTaskCreate(uart_enviador, "UART Envia", 2048, NULL, 1, NULL);
}
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"

xSemaphoreHandle_t semaf_bin;

void task_one(void *task1_params) {
    while (1) {
        ESP_LOGI("Task One", "Tentando adquirir o semaf_bin na tarefa 1...");
        xSemaphoreTake(semaf_bin, portMAX_DELAY) == pdTRUE ? (
            ESP_LOGI("Task One", "Mutex adquirido pela tarefa 1!"), xSemaphoreGive(semaf_bin)):
            ESP_LOGE("Task One", "Falha ao adquirir o semaf_bin!");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
void task_two(void *task2_params) {
    while (1) {
        ESP_LOGI("Task Two", "Mutex adquirido pela tarefa 2!");
        xSemaphoreGive(semaf_bin);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
void app_main(void) {
    semaf_bin = xSemaphoreCreateBinary();
    semaf_bin == NULL ? (
        ESP_LOGE("Main", "Falha ao criar o semaf_bin!"), return) : 
        ESP_LOGI("Main", "Semaf_bin criado com sucesso!");
    
    xTaskCreate(task_one, "Task One", 2048, NULL, 5, NULL);
    xTaskCreate(task_two, "Task Two", 2048, NULL, 5, NULL);
    ESP_LOGI("Main", "Tarefas criadas com sucesso e em funcionamento!");
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000)); 
    }
}
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

void task_one(void *task1_params) {
    while (1) {
        ESP_LOGI("Task One", "Rodando a tarefa 1 agora!");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
void task_two(void *task2_params) {
    while (1) {
        ESP_LOGI("Task Two", "Rodando a tarefa 2 agora!");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
void app_main(void) {
    xTaskCreate(task_one, "Task One", 2048, NULL, 5, NULL);
    xTaskCreate(task_two, "Task Two", 2048, NULL, 5, NULL);
    ESP_LOGI("Main", "Tarefas criadas com sucesso e em funcionamento!");
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000)); 
    }
}
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

TaskHandle_t task2_handle = NULL;

void task_one(void *task1_params) {
    while (1) {
        ESP_LOGI("Task One", "Rodando a tarefa 1 agora!");
        xTaskNotifyGive(task2_handle);
        ESP_LOGI("Task One", "Notificação enviada para a tarefa 2!");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void task_two(void *task2_params) {
    while (1) {
        unsigned long quantidade_notificacoes = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        ESP_LOGI("Task Two", "Rodando a tarefa 2 agora!");
        quantidade_notificacoes > 0 ?
            ESP_LOGI("Task Two", "Notificações recebidas: %lu", quantidade_notificacoes):
            ESP_LOGE("Task Two", "Nenhuma notificação recebida!");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void app_main(void) {
    xTaskCreate(task_one, "Task One", 2048, NULL, 5, NULL);
    xTaskCreate(task_two, "Task Two", 2048, NULL, 5, &task2_handle);
    ESP_LOGI("Main", "Tarefas criadas com sucesso e em funcionamento!");
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000)); 
    }
}
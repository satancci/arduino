#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

QueueHandle_t fila;
unsigned int valor;

void task_one(void *task1_params) {
    while (1) {
        valor = 42; // Exemplo de valor a ser enviado
        ESP_LOGI("Task One", "Enviando valor %d para a fila...", valor);
        xQueueSend(fila, &valor, portMAX_DELAY) == pdTRUE ? (
            ESP_LOGI("Task One", "Valor %d enviado com sucesso!", valor)):
            ESP_LOGE("Task One", "Falha ao enviar o valor para a fila!");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void task_two(void *task2_params) {
    while (1) {
        xQueueReceive(fila, &valor, portMAX_DELAY) == pdTRUE ? (
            ESP_LOGI("Task Two", "Valor recebido da fila: %d", valor)):
            ESP_LOGE("Task Two", "Falha ao receber o valor da fila!");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void app_main(void) {
    fila = xQueueCreate(10, sizeof(int));
    fila == NULL ? (ESP_LOGE("Main", "Falha ao criar a fila!"), return): 
                    ESP_LOGI("Main", "Fila criada com sucesso!");
    xTaskCreate(task_one, "Task One", 2048, NULL, 5, NULL);
    xTaskCreate(task_two, "Task Two", 2048, NULL, 5, NULL);
    ESP_LOGI("Main", "Tarefas criadas com sucesso e em funcionamento!");
    while (1) {vTaskDelay(pdMS_TO_TICKS(10000));}
}
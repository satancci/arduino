#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"

EventGroupHandle_t eventos;
const int passou_task_one = BIT0;
const int passou_task_two = BIT1;

void task_one(void *task1_params) {
    while (1) {
        ESP_LOGI("Task One", "Entrou na tarefa 1...");
        // Simula a tarefa fazendo algo
        xEventGroupSetBits(eventos, passou_task_one);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void task_two(void *task2_params) {
    while (1) {
        ESP_LOGI("Task Two", "Entrou na tarefa 2...");
        if(xEventGroupWaitBits(eventos, passou_task_one, pdTRUE, pdFALSE, portMAX_DELAY) & passou_task_one) {
            ESP_LOGI("Task Two", "Iniciou a tarefa 2!");
            xEventGroupSetBits(eventos, passou_task_two);
            ESP_LOGI("Task Two", "Tarefa 2 sinalizada!");
        } else {
            ESP_LOGE("Task Two", "Falha ao aguardar a tarefa 1!");
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void task_three(void *task3_params) {
    while (1) {
        ESP_LOGI("Task Three", "Entrou na tarefa 3!");
        if(xEventGroupWaitBits(eventos, passou_task_two, pdTRUE, pdFALSE, portMAX_DELAY) & passou_task_two) {
            ESP_LOGI("Task Three", "Tarefa 3 foi iniciada!");
        } else {
            ESP_LOGE("Task Three", "Falha ao aguardar a tarefa 2!");
        }   
        vTaskDelay(pdMS_TO_TICKS(2000)); 
    }
} 

void app_main(void) {
    eventos = xEventGroupCreate();
    eventos == NULL? (ESP_LOGE("Main", "Falha ao criar o grupo de eventos!"), return): 
                      ESP_LOGI("Main", "Grupo de eventos criado com sucesso!");
    xTaskCreate(task_one, "Task One", 2048, NULL, 5, NULL);
    xTaskCreate(task_two, "Task Two", 2048, NULL, 5, NULL);
    xTaskCreate(task_three, "Task Three", 2048, NULL, 5, NULL);
    ESP_LOGI("Main", "Tarefas criadas com sucesso e em funcionamento!");
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000)); 
    }
}
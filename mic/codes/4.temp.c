#include "driver/gptimer.h" //temporizador
#include "stdint.h" //uints
#include "esp_attr.h" //IRAM_ATTR (alarme)
#include "esp_task_wdt.h" //watchdog task (alarme)

uint8_t centesimo = 0; //variavel de contagem de centesimos de segundo
uint8_t segundo = 0; //variavel de contagem de segundo  
uint8_t minuto = 0; //variavel de contagem de minuto

bool IRAM_ATTR alarme(gptimer_handle_t temporizador, const gptimer_alarm_event_data_t *edata, void *user_ctx) {
    centesimo++; 
    return true; 
}

void configura_alarme(uint64_t tempo_alarme){
    // Configura o temporizador
    gptimer_handle_t temporizador = NULL;
    gptimer_config_t config_temporizador = {
      .clk_src = GPTIMER_CLK_SRC_APB, // Fonte de clock APB
      .direction = GPTIMER_COUNT_UP, // Contagem crescente
      .resolution_hz = 1000000, // Resolucao em 1 MHz
    };
  
    //Configura o alarme
    gptimer_alarm_config_t config_alarme = {
      .alarm_count = tempo_alarme, // em microsegundos
      .reload_count = 0, // Reinicia do zero
      .flags.auto_reload_on_alarm = true, // Reinicia automaticamente
    };
  
    //Configura o callback do alarme
    gptimer_event_callbacks_t config_callback = {
      .on_alarm = alarme,
    };
  
    // Cria o temporizador, seta o alarme e registra o callback
    gptimer_new_timer(&config_temporizador, &temporizador);
    gptimer_register_event_callbacks(temporizador, &config_callback, NULL) ;
    gptimer_set_alarm_action(temporizador, &config_alarme);
    gptimer_enable(temporizador);
    gptimer_start(temporizador);
}

void principal(){
    centesimo >= 100 ? (segundo++, centesimo = 0) : centesimo;
    segundo >= 60 ? (minuto++, segundo = 0) : segundo;
    minuto >= 60 ? (minuto = 0) : minuto;
    //completar o restante do código aqui
}
void app_main(void) {
    esp_task_wdt_deinit();
    configura_alarme(10000); 
    while (1) {
        principal();
    }
}

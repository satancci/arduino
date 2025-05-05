#include "display_lcd.c" //lcd
#include "driver/gpio.h" //interrupcao
#include "driver/gptimer.h" //temporizador
#include "stdint.h" //uints
#include "esp_attr.h" //IRAM_ATTR (alarme)
#include "esp_task_wdt.h" //watchdog task (alarme)

//funcao de callback da interrupcao
static void interrupcao(void* args){
  printf("Interrupção ativada\n");
}

//funcao de callback do alarme
bool IRAM_ATTR alarme(gptimer_handle_t temporizador, const gptimer_alarm_event_data_t *edata, void *user_ctx) {
  return true; 
}

void configura_lcd(unsigned int pino_base){
  display_lcd_config_t config_display = {
    .D4 = pino_base,
    .D5 = pino_base+1,
    .D6 = pino_base+2,
    .D7 = pino_base+3,
    .RS = 8,
    .E = 9
  };

  lcd_config(&config_display);
}

void configura_isr(unsigned int pino_isr){
  gpio_config_t io_config = {
    .pin_bit_mask = (1 << pino_isr),
    .mode = GPIO_MODE_INPUT,
    .pull_down_en = GPIO_PULLDOWN_ENABLE,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .intr_type = GPIO_INTR_POSEDGE
  };

  gpio_config(&io_config);
  gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
  gpio_isr_handler_add(pino_isr, interrupcao, NULL);
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

void app_main(){
  configura_lcd(0); // Pinos 0 a 3 para LCD
  configura_isr(10); // Pino 10 para interrupção
  esp_task_wdt_deinit(); // Desativa o watchdog do task (necessário para o alarme funcionar corretamente)
  configura_alarme(10000000); // 1 segundo = 1000000 microsegundos
}

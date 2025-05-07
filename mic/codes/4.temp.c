#include "driver/gptimer.h" //temporizador
#include "driver/gpio.h"
#include "stdint.h" //uints
#include "esp_attr.h" //IRAM_ATTR (alarme)
#include "esp_task_wdt.h" //watchdog task (alarme)

#define GPIO_OUT_REG 0x60004004

unsigned int *pOut = (unsigned int *)GPIO_OUT_REG;

uint8_t centesimo = 0;
uint8_t segundo = 0;
uint8_t minuto = 0;


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
void delay(int ciclos){
    for (volatile int i = 0; i < ciclos * 10000; i++);
};

char show_number(int val){
   switch (val)
   {
        case 0:  return 0xC0;
        case 1:  return 0xF9;
        case 2:  return 0xA4;
        case 3:  return 0xB0;
        case 4:  return 0x99;
        case 5:  return 0x92;
        case 6:  return 0x82;
        case 7:  return 0xF8;
        case 8:  return 0x80;
        case 9:  return 0x90;
        default: return 0xC0;
   }
}

void setup(){
    esp_task_wdt_deinit();
    gpio_config_t config_gpio = {
    		.pin_bit_mask = 0x7FF,
			.mode = GPIO_MODE_OUTPUT
    };
    gpio_config(&config_gpio);

    configura_alarme(10000);
}

void loop(){
    centesimo >= 100 ? (segundo++, centesimo = 0) : centesimo;
    segundo >= 60 ? (minuto++, segundo = 0) : segundo;
    minuto >= 60 ? (minuto = 0) : minuto;

    *pOut = (0x0<<8) | show_number(centesimo%10);
    delay(2);
    *pOut = (0x1<<8) | show_number((int)(centesimo/10));
    delay(2);
    *pOut = (0x2<<8) | show_number(segundo%10);
    delay(2);
    *pOut = (0x3<<8) | show_number((int)(segundo/10));
    delay(2);
    *pOut = (0x4<<8) | show_number(minuto%10);
    delay(2);
    *pOut = (0x5<<8) | show_number((int)(minuto/10));
    delay(2);
}

void app_main(void) {
    setup();
    while (1) {
        loop();
    }
}

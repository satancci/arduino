#include "driver/gptimer.h" //temporizador
#include "display_lcd.c" //lcd
#include "driver/gpio.h" //interrupcao
#include "stdint.h" //uints
#include "esp_attr.h" //IRAM_ATTR (alarme)
#include "esp_task_wdt.h" //watchdog task (alarme)

uint16_t milesimo, vmilesimo;
uint8_t segundo, vsegundo;
uint8_t minuto, vminuto;

bool habilitado = false;

display_lcd_config_t config_display = {
    .D4 = 5,
    .D5 = 6,
    .D6 = 7,
    .D7 = 8,
    .RS = 9,
    .E = 10
};

static void interrupcao(void *arg){
    switch ((unsigned int) arg) {
        case 0: habilitado = true;  break;
        case 1: habilitado = false; break;
        case 2: vmilesimo = milesimo; vsegundo = segundo; vminuto = minuto;  break;
        case 3: if(!habilitado) (milesimo = segundo = minuto = vmilesimo = vsegundo = vminuto = 0);  break;
        default: break;
    }
}

bool IRAM_ATTR alarme(gptimer_handle_t temporizador, const gptimer_alarm_event_data_t *edata, void *user_ctx) {
    habilitado ? milesimo++ : milesimo;
    return true;
}

void configura_isr(unsigned int pino_isr){
    gpio_config_t io_config = {
      .pin_bit_mask = (1 << pino_isr),
      .mode = GPIO_MODE_INPUT,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .pull_up_en = GPIO_PULLUP_ENABLE,
      .intr_type = GPIO_INTR_POSEDGE
    };
  
    gpio_config(&io_config);
    gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
    gpio_isr_handler_add(pino_isr, interrupcao, (void* )pino_isr);
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

void setup(){
    esp_task_wdt_deinit();
    lcd_config(&config_display);
    for(int i=0; i<4; i++) configura_isr(i); 
    configura_alarme(1000);
}

void loop(){
    milesimo >= 1000 ? (segundo++, milesimo = 0) : milesimo;
    segundo >= 60 ? (minuto++, segundo = 0) : segundo;
    minuto >= 60 ? (minuto = 0) : minuto;

    vmilesimo >= 1000 ? (vsegundo++, vmilesimo = 0) : vmilesimo;
    vsegundo >= 60 ? (vminuto++, vsegundo = 0) : vsegundo;
    vminuto >= 60 ? (vminuto = 0) : vminuto;
    
    char btempo[20];
    char bvolta[20];
    snprintf(btempo, sizeof(btempo), "Tempo %02d:%02d:%03d", minuto, segundo, milesimo);
    snprintf(bvolta, sizeof(bvolta), "Volta %02d:%02d:%03d", vminuto, vsegundo, vmilesimo);
    lcd_escreve_2_linhas(btempo, bvolta);
    delay(10);
}

void app_main(void) {
    setup();
    while (1) {
        loop();
    }
}

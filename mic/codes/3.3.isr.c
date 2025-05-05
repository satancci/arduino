#include "driver/gpio.h"
#include "display_lcd.h"
#include "stdio.h"

int contador;

display_lcd_config_t config_display = {
  .D4 = 0,
  .D5 = 1,
  .D6 = 2,
  .D7 = 3,
  .RS = 8,
  .E = 9
};

void configura_pino(int pino){
  gpio_config_t io_config = {
    .pin_bit_mask = (1 << pino),
    .mode = 1,
    .pull_down_en = 0,
    .pull_up_en = 1,
    .intr_type = 1
  };
  gpio_config(&io_config);
}

static void interrupt(void* args){
  contador = 0;
};

static void principal(){
  contador++;
  char texto[16];
  snprintf(texto, 16, "%d", contador);
  lcd_escreve_1_linha(texto, 1);
}

void app_main(void)
{
  lcd_config(&config_display);
  configura_pino(10);
  gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
  gpio_isr_handler_add(10, interrupt, NULL);
  while(1){
    principal();
  }
}

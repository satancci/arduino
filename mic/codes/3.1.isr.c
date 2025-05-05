#include "display_lcd.h" // Exercicio 3
#include "driver/gpio.h" 

display_lcd_config_t config_display = { // Exercicio 3
	.D4 = 0,
	.D5 = 1,
	.D6 = 2,
	.D7 = 3,
	.RS = 8,
	.E = 9
};

gpio_config_t config_gpio = { // Exercicio 4
	.pin_bit_mask = (1 << 10),
	.mode = GPIO_MODE_INPUT,
	.pull_up_en = GPIO_PULLUP_ENABLE,
	.pull_down_en = GPIO_PULLDOWN_DISABLE,
	.intr_type = GPIO_INTR_POSEDGE
};

char isr_pino_10_status = 0; // Exercicio 5

void isr_pino_10(void *args); // Exercicio 4
void delay(int ciclos);

void app_main(void)
{
	lcd_config(&config_display); // Exercicio 3
	
	gpio_config(&config_gpio); // Exercicio 4
	gpio_install_isr_service(ESP_INTR_FLAG_IRAM); // Exercicio 4
	gpio_isr_handler_add(10, isr_pino_10, NULL); // Exercicio 4
	
	while(1){
		if(isr_pino_10_status){ // Exercicio 5
			lcd_escreve_2_linhas("Interrupcao", "Habilitada"); // Exercicio 5
			delay(10000); // Exercicio 5
			isr_pino_10_status = 0; // Exercicio 5
		}
		
		lcd_escreve_2_linhas("EEL7030", "Microprocessadores"); // Exercicio 5
		delay(200); // Delay para evitar escrita desnecessaria
	}
};

void isr_pino_10(void *args){ // Exercicio 4
	isr_pino_10_status = 1; // Exercicio 5
};

void delay(int ciclos){
	for(volatile int i = 0; i < 1000*ciclos; i++);
};

#include "hal/gpio_types.h"
#include "soc/gpio_periph.h"
#include "soc/io_mux_reg.h"
#include <hal/gpio_ll.h>
#include <string.h>
#include <sys/_types.h>

void pinMode(unsigned int pino, const char* modo){
	if(pino > 21) return;
	gpio_ll_iomux_func_sel(GPIO_PIN_MUX_REG[pino], PIN_FUNC_GPIO);
	gpio_ll_set_drive_capability(&GPIO, pino, GPIO_DRIVE_CAP_2);
	if (strcmp(modo, "output") == 0){
		gpio_ll_input_disable(&GPIO, pino);
    	gpio_ll_output_enable(&GPIO, pino);
    	return;
    }
    if (strcmp(modo, "input_pullup") == 0){
		gpio_ll_output_disable(&GPIO, pino);
		gpio_ll_input_enable(&GPIO, pino);
		gpio_ll_pullup_en(&GPIO, pino);
		gpio_ll_pulldown_dis(&GPIO, pino);
		return;
	}
	if(strcmp(modo, "input_pulldown") == 0){
		gpio_ll_output_disable(&GPIO, pino);
		gpio_ll_input_enable(&GPIO, pino);
		gpio_ll_pullup_dis(&GPIO, pino);
		gpio_ll_pulldown_en(&GPIO, pino);
		return;
	}
}

bool digitalRead(unsigned int pino){
	return gpio_ll_get_level(&GPIO, pino);
}

void digitalWrite(unsigned int pino, bool estado) {
    if (estado) {
        gpio_ll_set_level(&GPIO, pino, 1); 
        return;
    } 
    gpio_ll_set_level(&GPIO, pino, 0);
}

void digitalWrite8(uint8_t valor, uint8_t pino_base) {
    for (int i = 0; i < 8; i++) {
        gpio_ll_set_level(&GPIO, pino_base + i, (valor >> i) & 0x01);
    }
}

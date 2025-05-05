#define IO_MUX_GPIOn_REG(n) (0x60009004+4*n)
#define GPIO_FUNCn_OUT_SEL_CFG_REG(n) (0x60004554+4*n)

#define GPIO_ENABLE_REG 0x60004020
#define GPIO_OUT_W1TS_REG 0x60004008
#define GPIO_OUT_W1TC_REG 0x6000400C
#define GPIO_OUT_REG 0x60004004

unsigned int *pEnable = (unsigned int *)GPIO_ENABLE_REG;
unsigned int *pSet = (unsigned int *)GPIO_OUT_W1TS_REG;  
unsigned int *pClear = (unsigned int *)GPIO_OUT_W1TC_REG;
unsigned int *pOut = (unsigned int *)GPIO_OUT_REG;

void configura_io_mux(unsigned char pino){
    if (pino > 21) return;
    // Configura a função do pino
    int *ptr = IO_MUX_GPIOn_REG(pino);
    *ptr = 0x1800;
   
    // Desconecta os periféricos associados
    ptr = GPIO_FUNCn_OUT_SEL_CFG_REG(pino);
    *ptr = 0x280;
}

void delay(int ciclos) {
    for (volatile int i = 0; i < ciclos * 1000; i++);
}

char auxiliar_func(int val){
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
        case 10: return 0x88;
        case 11: return 0x83;
        case 12: return 0xC6;
        case 13: return 0xA1;
        case 14: return 0x86;
        case 15: return 0x8E;
        default: return 0x80;
   }
}

void app_main(void) {
    for (int i = 4; i < 8; i++) configura_io_mux(i);
    *pEnable = 0xFF; 
    while(1){
        for (int i = 0; i < 16; i++)
        {
           *pOut = auxiliar_func(i);
           delay(10000);
        }
    }
}

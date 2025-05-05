#define GPIO_ENABLE_REG 0x60004020
#define GPIO_OUT_W1TS_REG 0x60004008
#define GPIO_OUT_W1TC_REG 0x6000400C

unsigned int *pEnable = (unsigned int *)GPIO_ENABLE_REG;
unsigned int *pSet = (unsigned int *)GPIO_OUT_W1TS_REG;  
unsigned int *pClear = (unsigned int *)GPIO_OUT_W1TC_REG;

void delay(int ciclos);

void main(void) {

    *pEnable = (1 << 2);
    while (1) {
        *pSet = (1 << 2);
        delay(1000);
        *pClear = (1 << 2);
        delay(1000);
    }

}

void delay(int ciclos) {
    for (volatile int i = 0; i < ciclos * 1000; i++);
}

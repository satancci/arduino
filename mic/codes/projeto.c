#include "driver/gptimer.h" // Inclui driver do temporizador GPTIMER
#include "display_lcd.c"    // Inclui funções para controle do display LCD
#include "driver/gpio.h"    // Inclui driver para manipulação de GPIOs (pinos digitais)
#include "stdint.h"         // Inclui definições de tipos inteiros padrão (uint8_t, uint16_t, etc)
#include "esp_attr.h"       // Inclui atributos especiais para funções (ex: IRAM_ATTR)
#include "esp_task_wdt.h"   // Inclui funções para manipulação do watchdog timer

uint16_t milesimo, vmilesimo; // Variáveis para milésimos do cronômetro e da volta
uint8_t segundo, vsegundo;    // Variáveis para segundos do cronômetro e da volta
uint8_t minuto, vminuto;      // Variáveis para minutos do cronômetro e da volta

bool habilitado = false;      // Flag que indica se o cronômetro está rodando

// Estrutura de configuração dos pinos do display LCD
display_lcd_config_t config_display = {
    .D4 = 5,  // Pino D4 do LCD conectado ao GPIO 5
    .D5 = 6,  // Pino D5 do LCD conectado ao GPIO 6
    .D6 = 7,  // Pino D6 do LCD conectado ao GPIO 7
    .D7 = 8,  // Pino D7 do LCD conectado ao GPIO 8
    .RS = 9,  // Pino RS do LCD conectado ao GPIO 9
    .E  = 10  // Pino E do LCD conectado ao GPIO 10
};

// Função de interrupção chamada ao pressionar um botão
static void interrupcao(void *arg){
    switch ((unsigned int) arg) { // Converte o argumento para inteiro (número do botão)
        case 0: habilitado = true;  break; // Botão 0: inicia o cronômetro
        case 1: habilitado = false; break; // Botão 1: pausa o cronômetro
        case 2: (vmilesimo = milesimo, vsegundo = segundo, vminuto = minuto); break;
        case 3: if(!habilitado) (milesimo = segundo = minuto = vmilesimo = vsegundo = vminuto = 0); break;
        default: break; // Outros valores: não faz nada
    }
}

// Função de callback chamada pelo temporizador a cada alarme
bool IRAM_ATTR alarme(gptimer_handle_t temporizador, const gptimer_alarm_event_data_t *edata, void *user_ctx) {
    if(habilitado) milesimo++; // Incrementa milésimos se o cronômetro estiver habilitado
    return true; // Indica que o alarme foi tratado
}

// Configura a interrupção para um pino específico
void configura_isr(unsigned int pino_isr){
    gpio_config_t io_config = {
      .pin_bit_mask = (1 << pino_isr),           // Seleciona o pino
      .mode = GPIO_MODE_INPUT,                   // Define como entrada
      .pull_down_en = GPIO_PULLDOWN_DISABLE,     // Desabilita pull-down
      .pull_up_en = GPIO_PULLUP_ENABLE,          // Habilita pull-up
      .intr_type = GPIO_INTR_POSEDGE             // Interrupção na borda de subida
    };
  
    gpio_config(&io_config); // Aplica a configuração do pino
    gpio_install_isr_service(ESP_INTR_FLAG_IRAM); // Instala o serviço de interrupção
    gpio_isr_handler_add(pino_isr, interrupcao, (void* )pino_isr); // Associa a função de interrupção ao pino
}

// Configura o temporizador e o alarme periódico
void configura_alarme(uint64_t tempo_alarme){
    gptimer_handle_t temporizador = NULL; // Handle do temporizador
    gptimer_config_t config_temporizador = {
      .clk_src = GPTIMER_CLK_SRC_APB,     // Usa o clock APB
      .direction = GPTIMER_COUNT_UP,      // Contagem crescente
      .resolution_hz = 1000000,           // Resolução de 1 MHz (1 us)
    };

    gptimer_alarm_config_t config_alarme = {
      .alarm_count = tempo_alarme,        // Valor do alarme em us
      .reload_count = 0,                  // Reinicia do zero
      .flags.auto_reload_on_alarm = true, // Alarme automático (loop)
    };

    gptimer_event_callbacks_t config_callback = {
      .on_alarm = alarme,                 // Função chamada no alarme
    };

    gptimer_new_timer(&config_temporizador, &temporizador);           // Cria o temporizador
    gptimer_register_event_callbacks(temporizador, &config_callback, NULL); // Registra o callback
    gptimer_set_alarm_action(temporizador, &config_alarme);           // Configura o alarme
    gptimer_enable(temporizador);                                     // Habilita o temporizador
    gptimer_start(temporizador);                                      // Inicia o temporizador
}

// Função de atraso (delay) baseada em laço ocupado
void delay(int ciclos){
    for (volatile int i = 0; i < ciclos * 10000; i++); // Espera ocupada
}

// Função de inicialização do sistema
void setup(){
    esp_task_wdt_deinit();         // Desabilita o watchdog timer da task principal
    lcd_config(&config_display);   // Inicializa o display LCD
    for(int i=0; i<4; i++) configura_isr(i);    // Configura as interrupções dos 4 botões
    configura_alarme(1000);        // Configura o alarme do temporizador para 1 ms
}

// Função principal de repetição (loop)
void loop(){
    if(milesimo >= 1000) (segundo++, milesimo = 0); // 1000 ms = 1 s
    if(segundo >= 60)   (minuto++, segundo = 0);    // 60 s = 1 min
    if(minuto >= 60)    minuto = 0;                 // Limita minutos a 59

    if(vmilesimo >= 1000) (vsegundo++, vmilesimo = 0); // Atualiza volta: 1000 ms = 1 s
    if(vsegundo >= 60)    (vminuto++, vsegundo = 0);   // Atualiza volta: 60 s = 1 min
    if(vminuto >= 60)     vminuto = 0;                 // Limita minutos da volta a 59
    
    char btempo[20]; // Buffer para string do tempo principal
    char bvolta[20]; // Buffer para string do tempo de volta
    snprintf(btempo, sizeof(btempo), "Tempo %02d:%02d:%03d", minuto, segundo, milesimo); // Monta string do tempo
    snprintf(bvolta, sizeof(bvolta), "Volta %02d:%02d:%03d", vminuto, vsegundo, vmilesimo); // Monta string da volta
    lcd_escreve_2_linhas(btempo, bvolta); // Escreve as duas linhas no display LCD
    delay(10); // Pequeno atraso para evitar flicker
}

// Função principal da aplicação (ponto de entrada)
void app_main(void) {
    setup();         // Inicializa o sistema
    while (1) {      // Loop infinito
        loop();      // Executa o loop principal
    }
}

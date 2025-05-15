/**
 * @file projeto.c
 * @brief Cronômetro com display LCD e botões de controle
 * @details Este código implementa um cronômetro com controle de tempo de volta, utilizando um display LCD e botões para iniciar, pausar, reiniciar e zerar o cronômetro.
 * @note O código utiliza interrupções para detectar os botões e um temporizador para controlar o tempo.
 * @note O cronômetro é exibido em um display LCD, e o tempo de volta é atualizado a cada vez que o botão de volta é pressionado.
 * @author Ricson Paulo
 * @date 2025-05-14
 * @version 1.0
 */

#include "driver/gptimer.h" // Inclui driver do temporizador GPTIMER
#include "display_lcd.c"    // Inclui funções para controle do display LCD
#include "driver/gpio.h"    // Inclui driver para manipulação de GPIOs (pinos digitais)
#include "stdint.h"         // Inclui definições de tipos inteiros padrão (uint8_t, uint16_t, etc)
#include "esp_attr.h"       // Inclui atributos especiais para funções (ex: IRAM_ATTR)
#include "esp_task_wdt.h"   // Inclui funções para manipulação do watchdog timer
#include "esp_timer.h"      // Inclui funções para manipulação de temporizadores
#include "esp_log.h"        // Inclui funções de log do ESP-IDF

uint16_t milesimo, vmilesimo;   // Variáveis para milésimos do cronômetro e da volta
uint8_t minuto, vminuto, segundo, vsegundo;        // Variáveis para minutos e segundos do cronômetro e da volta

bool habilitado, reload;        // Flag que indica se o cronômetro está rodando, Flag que indica se o cronômetro de volta deve ser reiniciado

/**
 * @brief Configuração do display LCD
 * @details Define os pinos do display LCD e inicializa a configuração.sss
 */
display_lcd_config_t config_display = {
    .D4 = 5,  // Pino D4 do LCD conectado ao GPIO 5
    .D5 = 6,  // Pino D5 do LCD conectado ao GPIO 6
    .D6 = 7,  // Pino D6 do LCD conectado ao GPIO 7
    .D7 = 8,  // Pino D7 do LCD conectado ao GPIO 8
    .RS = 9,  // Pino RS do LCD conectado ao GPIO 9
    .E  = 10  // Pino E do LCD conectado ao GPIO 10
};

/**
 * @brief Função de interrupção para os botões
 * @param arg Argumento passado para a função de interrupção
 * @return void
 * @details Esta função é chamada quando um botão é pressionado. Ela desabilita a interrupção, inicia um temporizador de debounce e executa ações com base no botão pressionado.
 */
static void interrupcao(void *arg){
    void **args = (void **)arg;
    int botao = (int)(intptr_t)args[0];                     
    esp_timer_handle_t debounce_timer = args[1];

    gpio_intr_disable(botao);// Desabilita interrupcoes no pino
    esp_timer_start_once(debounce_timer, 70000) ; // Inicia o timer de debounce e espera 70ms

    switch (botao) { // Converte o argumento para inteiro (número do botão)
        case 0: habilitado = true;  break; // Botão 0: inicia o cronômetro
        case 1: habilitado = false; break; // Botão 1: pausa o cronômetro
        case 2: reload = true;	break;
        case 3: if(!habilitado) (milesimo = segundo = minuto = vmilesimo = vsegundo = vminuto = 0, reload = true); break;
        default: break; // Outros valores: não faz nada
    }
}

/**
 * @brief Função de alarme do temporizador
 * @param temporizador Handle do temporizador
 * @param edata Dados do evento de alarme
 * @param user_ctx Contexto do usuário
 * @return true, se o alarme foi tratado com sucesso
 * @details Esta função é chamada quando o alarme do temporizador é acionado. Ela incrementa os milésimos e segundos do cronômetro.
 */
bool IRAM_ATTR alarme(gptimer_handle_t temporizador, const gptimer_alarm_event_data_t *edata, void *user_ctx) {
    if(habilitado) (milesimo++, vmilesimo++); // Incrementa milésimos se o cronômetro estiver habilitado
    return true; // Indica que o alarme foi tratado
}

/**
 * @brief Função de callback do temporizador de debounce
 * @param arg Argumento passado para a função de callback
 * @return void
 * @details Esta função é chamada após o tempo de debounce. Ela reabilita a interrupção do botão.
 */
static void debounce_timer_callback(void *arg) {
    int pino = (int)(intptr_t)arg;
    while(gpio_get_level(pino) == 0) {} // Enquanto o botao ainda está pressionado
    gpio_intr_enable(pino); // Reabilita a interrupcao
}

/**
 * @brief Configura a interrupção para os botões
 * @param pino_isr Pino do botão a ser configurado
 * @return void
 * @details Esta função configura o pino do botão como entrada com pull-up, instala o serviço de interrupção e associa a função de interrupção ao pino.
 */
void configura_isr(unsigned int pino_isr){
    esp_timer_handle_t debounce_timer; // Instancia o manipulador do timer
    const esp_timer_create_args_t timer_args = {
        .callback = &debounce_timer_callback,
        .arg = (void *)(intptr_t)pino_isr,
        .name = "debounce_timer"
	 };

    esp_timer_create (&timer_args, &debounce_timer);
    void **args = malloc(2);
    args[0] = (void *)(intptr_t)pino_isr;    
    args[1] = (void *)debounce_timer;

    gpio_config_t io_config = {
      .pin_bit_mask = (1 << pino_isr),           // Seleciona o pino
      .mode = GPIO_MODE_INPUT,                   // Define como entrada
      .pull_down_en = GPIO_PULLDOWN_DISABLE,     // Desabilita pull-down
      .pull_up_en = GPIO_PULLUP_ENABLE,          // Habilita pull-up
      .intr_type = GPIO_INTR_POSEDGE             // Interrupção na borda de subida
    };
  
    gpio_config(&io_config); // Aplica a configuração do pino
    gpio_install_isr_service(ESP_INTR_FLAG_IRAM); // Instala o serviço de interrupção
    gpio_isr_handler_add(pino_isr, interrupcao, args); // Associa a função de interrupção ao pino, passando dois argumentos
}

/**
 * @brief Configura o temporizador e o alarme
 * @param tempo_alarme Tempo do alarme em microssegundos
 * @return void
 * @details Esta função configura o temporizador GPTIMER, define a contagem, a resolução e o alarme. Ela também registra a função de alarme e inicia o temporizador.
 */
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

/**
 * @brief Função de configuração inicial dos componentes
 * @param void
 * @return void
 * @details Esta função é chamada no início do programa. Ela desabilita o watchdog timer, inicializa o display LCD, configura as interrupções dos botões e configura o alarme do temporizador.
 */
void setup(){
    esp_task_wdt_deinit();         // Desabilita o watchdog timer da task principal
    lcd_config(&config_display);   // Inicializa o display LCD
    for(int i=0; i<4; i++) configura_isr(i);    // Configura as interrupções dos 4 botões (0, 1, 2 e 3)
    configura_alarme(1000);        // Configura o alarme do temporizador para 1 ms
    lcd_escreve_2_linhas("Tempo 00:00:000", "Volta 00:00:000");
}

/**
 * @brief Função principal do loop
 * @param void
 * @return void
 * @details Esta função é chamada repetidamente no loop principal. Ela atualiza o tempo do cronômetro e o tempo de volta, e exibe as informações no display LCD.
 */
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
    lcd_escreve_1_linha(btempo, 1);
    if(reload){
      snprintf(bvolta, sizeof(bvolta), "Volta %02d:%02d:%03d", vminuto, vsegundo, vmilesimo); // Monta string da volta
      (vminuto = vsegundo = vmilesimo = 0);
      lcd_escreve_1_linha(bvolta, 2);
      reload = false;
    }
}

/**
 * @brief Função principal do programa
 * @param void
 * @return void
 * @details Esta função é chamada no início do programa. Ela inicializa o sistema (setup) e entra em um loop infinito (loop).
 */
void app_main(void) {
    setup();        
    while(1) loop();
}

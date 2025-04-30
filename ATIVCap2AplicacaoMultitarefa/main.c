// Sistema de Monitoramento Multi-núcleo para BitDogLab
// Este projeto utiliza os dois núcleos da Raspberry Pi Pico na placa BitDogLab
// para monitorar o nível de atividade simulado por um joystick analógico.
// O Core 0 lê o joystick e envia o estado para o Core 1, que controla o LED RGB e o buzzer.

#include <stdio.h>          // Para funções de entrada/saída padrão (printf)
#include "pico/stdlib.h"   // Biblioteca padrão da Raspberry Pi Pico
#include "pico/multicore.h" // Biblioteca para utilizar os dois núcleos
#include "hardware/gpio.h"  // Biblioteca para controle dos pinos GPIO
#include "hardware/adc.h"   // Biblioteca para conversão analógico-digital (ADC)
#include "hardware/pwm.h"   // Biblioteca para modulação por largura de pulso (PWM)
#include "hardware/timer.h" // Biblioteca para usar timers e alarmes

// --- Definições da BitDogLab ---
// LED RGB 

#define LED_R_PIN 13
#define LED_G_PIN 11
#define LED_B_PIN 12
// Buzzers
#define BUZZER_A_PIN 21 // Pino do Buzzer A
// Joystick Analógico
#define JOY_X_PIN 27    // ADC 1
#define ADC_JOY_X 1

// --- Configurações ---
#define JOYSTICK_READ_INTERVAL_MS 2000    // Leitura a cada 2 segundos
#define ADC_MAX_VALUE 4095                // Valor máximo do ADC (12 bits)

// Limiares teóricos para os estados (aproximadamente 1/3 e 2/3 do ADC_MAX_VALUE)
#define LIMIAR_BAIXA (ADC_MAX_VALUE / 3)      // ~1365 // 4095 / 3 ≈ 1365 (33%)
#define LIMIAR_MODERADA (2 * ADC_MAX_VALUE / 3) // ~2730 // 4095 * 2/3 ≈ 2730 (66%)

#define BUZZER_WRAP_VALUE 62499           // Wrap para PWM do buzzer (~2kHz audível)

// Flag volatile para armazenar o estado global 
volatile uint8_t global_state = 0; // Estado compartilhado entre cores

// --- Protótipos ---
void core1_entry(); // Função que roda no Core 1: recebe dados do Core 0 e aciona buzzer/LED.
int64_t core0_joystick_read_callback(alarm_id_t id, void *user_data); // Callback chamado periodicamente para ler o joystick e enviar estado ao Core 1.
void setup_gpios(); // Configura todos os GPIOs usados no projeto.
void setup_adc(); // Inicializa o sistema ADC para leituras do joystick.
void setup_pwm(); // Inicializa o PWM para LEDs RGB e Buzzer.
void set_rgb_led(uint16_t r, uint16_t g, uint16_t b); // PWM duty cycle 0-65535 // Define níveis de PWM para o LED RGB
void update_rgb_led_from_state(uint8_t state); // Atualiza a cor do LED RGB conforme o estado recebido.

// --- Core 1 - Controle de Atuadores ---
void core1_entry() {
    printf("Core 1 Iniciado. Aguardando estado...\n");

    // Loop principal do Core 1 - Processa estado recebido da FIFO
    while (1) {
        // Bloqueia até receber um valor via FIFO (estado enviado pelo Core 0)
        uint8_t received_state = multicore_fifo_pop_blocking();
        printf("Core 1: Estado recebido = %u\n", received_state);

        // Atualiza a flag volatile global
        global_state = received_state;

        // --- Controle do Buzzer com PWM ---
        if (received_state == 3) {
            printf("Core 1: Estado ALTO! Ativando buzzer.\n");
            pwm_set_gpio_level(BUZZER_A_PIN, BUZZER_WRAP_VALUE / 2); // Ativa o buzzer
        } else {
            pwm_set_gpio_level(BUZZER_A_PIN, 0); // Desativa o buzzer
        }

        // --- Controle do LED RGB ---
        update_rgb_led_from_state(received_state);
    }
}

// --- Core 0 - Leitura de Sensor e Envio de Estado ---
int main() {
    stdio_init_all();     // Inicializa a serial para printf

    // --- Espera ativa pela conexão USB ---
    while (!stdio_usb_connected()) {
        sleep_ms(100); // Espera até o USB estar pronto
    }

    printf("BitDogLab Dual Core Monitor Iniciado!\n"); // Mensagem de confirmação

    // Inicializa os periféricos
    setup_gpios();
    setup_adc();
    setup_pwm();

    // Inicia o Core 1
    printf("Lançando Core 1...\n");
    multicore_launch_core1(core1_entry);
    printf("Core 1 Lançado.\n");

    // Configura alarme para leitura periódica do joystick no Core 0
    add_alarm_in_ms(JOYSTICK_READ_INTERVAL_MS, core0_joystick_read_callback, NULL, true);
    printf("Core 0: Alarme para leitura do Joystick configurado.\n");

    // Loop principal do Core 0
    while (1) {
        __wfi(); // Núcleo 0 dorme até o próximo alarme
    }

    return 0; // Nunca alcançado
}

// Callback do Alarme no Core 0 para ler o Joystick e enviar seu valor via FIFO
int64_t core0_joystick_read_callback(alarm_id_t id, void *user_data) {
    // Seleciona e lê o ADC do Joystick X
    adc_select_input(ADC_JOY_X);
    uint32_t joystick_value = adc_read(); // Leitura no intervalo 0-4095
    printf("Core 0: Lendo Joystick X = %lu.\n", joystick_value);

    uint8_t estado;
    if (joystick_value <= LIMIAR_BAIXA) {
        estado = 1; // Atividade Baixa
        printf("Core 0: Estado = 1 (Baixa). Enviando para Core 1...\n");
    } else if (joystick_value <= LIMIAR_MODERADA) {
        estado = 2; // Atividade Moderada
        printf("Core 0: Estado = 2 (Moderada). Enviando para Core 1...\n");
    } else {
        estado = 3; // Atividade Alta
        printf("Core 0: Estado = 3 (Alta). Enviando para Core 1...\n");
    }

    // Envia o estado para o Core 1 via FIFO
    if (multicore_fifo_wready()) {
        multicore_fifo_push_blocking(estado); // Agora enviamos o estado (uint8_t)
    } else {
        printf("Core 0: FIFO cheia ao tentar enviar estado!\n");
    }

    // Retorna o tempo (em ms) até a próxima chamada do alarme
    return JOYSTICK_READ_INTERVAL_MS * 1000; // Retorno em microssegundos
}

// --- Funções Auxiliares ---

// Configura os GPIOs utilizados
void setup_gpios() {
    gpio_init(BUZZER_A_PIN);
    gpio_set_dir(BUZZER_A_PIN, GPIO_OUT);
    // Pinos dos LEDs RGB e Buzzer serão configurados em setup_pwm()
    // Outros GPIOs podem ser inicializados aqui se necessário
}

// Configura o ADC para a leitura dos sensores (joystick)
void setup_adc() {
    adc_init();
    // Habilita o pino do joystick para ADC
    adc_gpio_init(JOY_X_PIN);
}

// Configura o PWM para LEDs RGB e para o buzzer
void setup_pwm() {
    // --- Configuração dos LEDs RGB ---
    gpio_set_function(LED_R_PIN, GPIO_FUNC_PWM);
    gpio_set_function(LED_G_PIN, GPIO_FUNC_PWM);
    gpio_set_function(LED_B_PIN, GPIO_FUNC_PWM);

    uint slice_r = pwm_gpio_to_slice_num(LED_R_PIN);
    uint slice_g = pwm_gpio_to_slice_num(LED_G_PIN);
    uint slice_b = pwm_gpio_to_slice_num(LED_B_PIN);

    uint16_t wrap_leds = 65535;  // Máximo para 16 bits
    pwm_set_wrap(slice_r, wrap_leds);
    pwm_set_wrap(slice_g, wrap_leds);
    pwm_set_wrap(slice_b, wrap_leds);

    pwm_set_chan_level(slice_r, pwm_gpio_to_channel(LED_R_PIN), 0);
    pwm_set_chan_level(slice_g, pwm_gpio_to_channel(LED_G_PIN), 0);
    pwm_set_chan_level(slice_b, pwm_gpio_to_channel(LED_B_PIN), 0);

    pwm_set_enabled(slice_r, true);
    pwm_set_enabled(slice_g, true);
    pwm_set_enabled(slice_b, true);

    // --- Configuração do Buzzer A com PWM ---
    gpio_set_function(BUZZER_A_PIN, GPIO_FUNC_PWM);
    uint slice_buzzer = pwm_gpio_to_slice_num(BUZZER_A_PIN);
    uint chan_buzzer = pwm_gpio_to_channel(BUZZER_A_PIN);

    pwm_set_wrap(slice_buzzer, BUZZER_WRAP_VALUE);
    pwm_set_chan_level(slice_buzzer, chan_buzzer, 0);
    pwm_set_enabled(slice_buzzer, true);
}

// Ajusta o brilho dos LEDs RGB utilizando PWM
void set_rgb_led(uint16_t r, uint16_t g, uint16_t b) {
    pwm_set_gpio_level(LED_R_PIN, r);
    pwm_set_gpio_level(LED_G_PIN, g);
    pwm_set_gpio_level(LED_B_PIN, b);
}

// Atualiza a cor dos LEDs RGB com base no valor do estado (1, 2 ou 3)
void update_rgb_led_from_state(uint8_t state) {
    uint16_t r = 0, g = 0, b = 0;
    uint16_t max_bright = 30000; // Limita o brilho

    switch (state) {
        case 1: // Atividade Baixa -> LED VERDE
            r = 0;
            g = max_bright;
            b = 0;
            printf("Core 1: LED Verde.\n");
            break;
        case 2: // Atividade Moderada -> LED AZUL
            r = 0;
            g = 0;
            b = max_bright;
            printf("Core 1: LED Azul.\n");
            break;
        case 3: // Atividade Alta -> LED VERMELHO
            r = max_bright;
            g = 0;
            b = 0;
            printf("Core 1: LED Vermelho.\n");
            break;
    }

    set_rgb_led(r, g, b);
}
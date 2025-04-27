#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/gpio.h" // Incluído explicitamente

#define LED_PIN 12
#define INTERVALO_MS 1000

// Não precisa mais da flag 'alternar_led' nem de 'estado_led' global

// Callback agora alterna o LED diretamente
// Retorna o *próximo* intervalo em *micro*segundos
int64_t temporizador_callback_direto(alarm_id_t id, void *user_data) {
    static bool led_state = false; // Variável estática para manter o estado entre chamadas
    led_state = !led_state;
    gpio_put(LED_PIN, led_state);

    // Opcional: Imprimir do callback (cuidado se o tempo for crítico)
    // printf("LED = %d (do callback)\n", led_state);

    // Retorna o *atraso* para o próximo alarme em *micro*segundos.
    // Um valor positivo reagenda o alarme.
    return (int64_t)INTERVALO_MS * 1000ll; // Usa 1000ll para literal long long
}

int main() {
    stdio_init_all(); // Para printf

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    printf("Iniciando pisca LED (método direto)...\n");

    // Adiciona o alarme. Note o último parâmetro 'false'.
    // O callback agora controla a repetição retornando um valor positivo.
    if (!add_alarm_in_ms(INTERVALO_MS, temporizador_callback_direto, NULL, false)) {
         printf("Erro: Falha ao adicionar alarme do temporizador!\n");
         while(true) tight_loop_contents();
    }

    // O loop principal pode fazer outras coisas ou apenas esperar/dormir
    while (true) {
        // Opção 1: Não fazer nada, o temporizador cuida de tudo
        tight_loop_contents();

        // Opção 2: Dormir para economizar energia (se não houver outras tarefas)
        // sleep_ms(INTERVALO_MS * 2); // Dorme por um tempo
    }
    // return 0; // Inalcançável em contexto embarcado
}

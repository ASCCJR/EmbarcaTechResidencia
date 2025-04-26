#include <stdio.h>
#include "pico/stdlib.h"

#define LED_PIN_RED 13      // Mudei para o Led Vermelho da BitDogLab (GPIO 13), antes era 12 (azul)
#define BUTTON_PIN 5    // Botão A da BitDogLab (GPIO 5)
#define DEBOUNCE_MS 20  // Tempo para debounce em MS

// Mensagem aparece no serial monitor mesmo sem ação no botão --> solução: alterar para mensagem aparecer somente quando o botão sofrer ação

// LED_PIN apesar de funcionar não tem cor definida na nomeclatura, 
// no manual da bitdoglab colocam que o 12 (utilizado no código original é azul), então deve-se alterar 
// para LED_PIN_BLUE ao invés de ficar LED_PIN que não tem indicação da cor

// O GitHub da bitdoglab  exemplifica que o red fica no 13, o green no 11, e o blue no 12

int main() {
    stdio_init_all();   // Inicializa UART (printf)
    
    // Configura LED
    gpio_init(LED_PIN_RED);
    gpio_set_dir(LED_PIN_RED, GPIO_OUT);
    
    // Configura Botão (com pull-up interno)
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    bool estado_anterior = false;
    
    while (true) {
        // Pisca o LED (500ms ligado, 500ms desligado)
        gpio_put(LED_PIN_RED, 1);
        sleep_ms(500);
        gpio_put(LED_PIN_RED, 0);
        sleep_ms(500);

        // Lê o botão (invertido porque o pull-up ativo LOW)
        bool pressionado = !gpio_get(BUTTON_PIN);
        
        // Debounce: espera para confirmar a pressão
        if (pressionado) {
            sleep_ms(DEBOUNCE_MS);
            pressionado = !gpio_get(BUTTON_PIN); // Lê novamente
        }

        // SOMENTE! imprime se o estado do botão mudar (importante para que não aja envio exacerbado de informação sem necessidade.
        // No codigo original o envio era constante, mesmo sem ação no botão)
        // O código original não tinha debounce, então o botão poderia ser pressionado e solto rapidamente, fazendo com que a mensagem aparecesse várias vezes
        // mesmo sem ação no botão, o que não é desejado.
        
        if (pressionado != estado_anterior) {
            estado_anterior = pressionado;
            if (pressionado) {
                printf("Botão PRESSIONADO!\n");
            } else {
                printf("BOTÃO NÃO PRESSIONADO\n");
            }
        }
    }
    return 0;
}


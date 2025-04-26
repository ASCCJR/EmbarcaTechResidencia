## Atividade Sala de aula dia 24/04/2025
## Melhorar o Código a seguir:

```c
#include <stdio.h>
#include "pico/stdlib.h"
#define LED_PIN 12
#define BUTTON_PIN 5

int main() {
    stdio_init_all();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
    while (true) {
        gpio_put(LED_PIN, 1);
        sleep_ms(500);
        gpio_put(LED_PIN, 0);
        sleep_ms(500);
        bool pressionado = !gpio_get(BUTTON_PIN);
        if (pressionado) {
            printf("Botão pressionado!\n");
        } else {
            printf("Botão não pressionado\n");
        }
    }
 return 0;
}
11
```

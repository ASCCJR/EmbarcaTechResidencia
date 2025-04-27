## Atividade Sala de aula dia 24/04/2025
## Melhorar o Código a seguir:

```c
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#define LED_PIN 12

volatile bool alternar_led = false;

bool estado_led = false;

int64_t temporizador_callback(alarm_id_t id, void *user_data) {
    alternar_led = true;
    return 1000;
}
int main() {
    stdio_init_all();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    add_alarm_in_ms(1000, temporizador_callback, NULL, true);
    
    while (true) {
        if (alternar_led) {
            estado_led = !estado_led;
            gpio_put(LED_PIN, estado_led);
            printf("LED = %d\n", estado_led);
            sleep_ms(100);
            alternar_led = false;
        }
        tight_loop_contents();
    }
}
```

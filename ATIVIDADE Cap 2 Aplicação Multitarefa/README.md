# Sistema de Monitoramento Multi-núcleo para BitDogLab

Este projeto implementa um sistema dual-core utilizando a placa **BitDogLab** (baseada no Raspberry Pi Pico), para **monitorar o joystick analógico** e **controlar LEDs RGB e buzzer** de forma inteligente.

## Descrição

- **Core 0:** Realiza a leitura periódica do valor do eixo X do joystick utilizando o ADC interno.  
- **Core 1:** Recebe o valor via FIFO, determina o estado atual do joystick (baixo, moderado, alto) e aciona os LEDs e o buzzer conforme o nível de atividade.

---

## Funcionalidades

- **Leitura do joystick:** a cada 2 segundos (`add_alarm_in_ms`).
- **Comunicação entre núcleos:** via **FIFO** (First In First Out).
- **Controle de LEDs RGB:** para indicar o estado do sistema.
- **Acionamento do buzzer:** em caso de atividade alta do joystick.
- **Uso de flags `volatile`** para compartilhamento seguro de estados entre cores.
- **Organização modular:** funções auxiliares bem separadas.

---

## Definições de Estado do Joystick

| Estado | Intervalo de valor ADC | LED           | Buzzer  |
|:------:|:----------------------:|:-------------:|:-------:|
| 1 - Baixo    | 0 a 1499                | Verde         | Desligado |
| 2 - Moderado | 1500 a 2999             | Azul          | Desligado |
| 3 - Alto     | 3000 a 4095             | Vermelho      | Ativado   |

---

## Organização do Código

- `main()`
  - Inicializa periféricos (ADC, GPIOs, PWM)
  - Lança o Core 1 (`multicore_launch_core1`)
  - Configura alarme periódico para leituras
- `core1_entry()`
  - Espera valores via FIFO
  - Atualiza estado global
  - Ativa LEDs e buzzer de acordo com o estado
- Funções auxiliares:
  - `setup_gpios()`
  - `setup_adc()`
  - `setup_pwm()`
  - `set_rgb_led()`
  - `determinar_estado()`
  - `atualizar_led_pelo_estado()`
- `core0_joystick_read_callback()`
  - Callback do alarme no Core 0 para leitura e envio do joystick

---

## Requisitos Técnicos Atendidos ✅

- [x] Uso de `volatile` para variáveis globais compartilhadas.
- [x] Uso de `add_alarm_in_ms()` no Core 0.
- [x] Comunicação inter-core usando FIFO.
- [x] Definição clara de limiares para os estados do joystick.
- [x] Controle de buzzer e LEDs via PWM.

---

## Diagrama Simplificado de Funcionamento

```plaintext
[Core 0]
 |-> add_alarm_in_ms -> Ler Joystick -> Enviar via FIFO
                          
[Core 1]
 <- FIFO <- Recebe valor
       |-> Determina estado
       |-> Atualiza LEDs
       |-> Liga/Desliga buzzer
```

---

## Hardware Utilizado

- **Placa:** BitDogLab
- **Componentes:**
  - Joystick analógico
  - LED RGB
  - Buzzer Piezoelétrico
  - Botões (não usados diretamente nesta aplicação)

---

## Compilação

Este projeto usa:
- **SDK Pico C**
- **CMake**
- **Visual Studio Code** com extensão Pico para desenvolvimento.

---

## Observação

- Atualmente, apenas o eixo **X do joystick** está sendo monitorado.
- A leitura do eixo Y poderia ser adicionada facilmente se desejado no futuro.
- O segundo buzzer (Buzzer B) não foi utilizado neste projeto.

---

## Autor

Projeto desenvolvido para estudo de sistemas embarcados multicore usando a plataforma BitDogLab.

---

- Vou explicar o propósito de cada um dos requisitos técnicos solicitados para o seu projeto:

## Especificações Técnicas

### 1. Flag volatile para armazenar o estado global

**Propósito:** A palavra-chave `volatile` indica ao compilador que uma variável pode mudar a qualquer momento, independentemente do fluxo de código visível. Em um sistema multicore, isso é essencial porque:

- Previne otimizações do compilador que poderiam eliminar leituras repetidas da variável
- Garante que o valor mais recente seja sempre lido da memória, não de registradores ou cache
- Evita problemas de sincronização entre os núcleos

Sem o `volatile`, um núcleo poderia não perceber mudanças feitas pelo outro núcleo, causando comportamentos imprevisíveis.

### 2. Utilizar add_alarm_in_ms() no Core 0

**Propósito:** Esta função cria um alarme de tempo único que:

- É mais flexível que temporizadores repetitivos, pois permite reagendar dinamicamente
- Consome menos recursos que um loop de polling
- Permite controle mais preciso sobre quando ocorrerá a próxima leitura
- O retorno da função determina quando o próximo alarme será disparado, permitindo ajustes dinâmicos do intervalo

Diferente do `add_repeating_timer_ms()`, o `add_alarm_in_ms()` força você a reagendar explicitamente, o que dá maior controle sobre o comportamento temporal.

### 3. Utilizar FIFO para enviar comandos ao Core 1

**Propósito:** A comunicação FIFO (First In, First Out) é uma forma segura de comunicação intercore porque:

- Implementa um buffer de mensagens ordenado
- Evita condições de corrida (race conditions)
- Funciona como uma fila, preservando a ordem das mensagens
- Tem operações atômicas gerenciadas pelo hardware
- Bloqueia o núcleo receptor até que haja dados disponíveis, evitando polling desnecessário

Outras formas de comunicação intercore poderiam exigir mecanismos de sincronização mais complexos.

### 4. Definir limiares da leitura do joystick para cada estado

**Propósito:** Os limiares fornecem um mapeamento entre valores analógicos brutos e estados compreensíveis:

- Facilita a interpretação dos dados em níveis discretos de severidade
- Permite respostas apropriadas a cada nível de entrada
- Simplifica a lógica de controle no sistema
- Possibilita feedback visual intuitivo através das cores do LED RGB

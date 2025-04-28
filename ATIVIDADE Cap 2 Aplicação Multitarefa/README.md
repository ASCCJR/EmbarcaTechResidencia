# EmbarcaTechResidencia

# 📟 BitDogLab - Sistema de Monitoramento Multi-núcleo

Este projeto implementa um sistema de monitoramento para a placa **BitDogLab** usando os dois núcleos da **Raspberry Pi Pico**.

- O **Core 0** lê periodicamente o valor do joystick analógico e envia o estado para o **Core 1** via FIFO.
- O **Core 1** interpreta o estado recebido, aciona o buzzer em situações críticas e ajusta a cor do LED RGB conforme o nível de alerta.

---

## 🚀 Funcionamento

- A cada **2 segundos**, o Core 0 lê o valor do eixo **X** do joystick.
- Se o valor ultrapassar um **limiar crítico**, o Core 1:
  - **Ativa** o **buzzer**.
  - **Acende** o **LED vermelho**.
- Caso contrário:
  - O buzzer permanece **desativado**.
  - O LED RGB muda de cor para **azul**, **verde** ou **amarelo**, dependendo da intensidade do valor lido.

---

## 🔧 Hardware Utilizado

- **BitDogLab** com Raspberry Pi Pico
- Joystick analógico (ligado aos pinos ADC 26 e 27)
- LED RGB conectado aos pinos:
  - Vermelho: GPIO13
  - Verde: GPIO11
  - Azul: GPIO12
- Buzzer ativo no GPIO21

---

## 🛠️ Configuração dos Pinos

| Componente | Pino GPIO |
|:-----------|:----------|
| LED Vermelho | 13 |
| LED Verde    | 11 |
| LED Azul     | 12 |
| Joystick X   | ADC 1 (GPIO27) |
| Joystick Y   | ADC 0 (GPIO26) |
| Buzzer A     | 21 |

---

## 🧩 Bibliotecas utilizadas

- `pico/stdlib.h` — Funções básicas de GPIO, delays e UART/USB
- `pico/multicore.h` — Controle de múltiplos núcleos
- `hardware/gpio.h` — Manipulação direta de GPIOs
- `hardware/adc.h` — Leitura dos pinos ADC
- `hardware/pwm.h` — Controle de LEDs RGB e buzzer via PWM
- `hardware/timer.h` — Timers para execução periódica

---

## 📈 Estados e Cores do LED

| Valor do Joystick | Estado | Cor do LED |
|:---|:---|:---|
| 0% - 30% | Baixo | Azul |
| 30% - 70% | Médio | Verde |
| 70% - 85% | Alto | Amarelo |
| >85% (acima do limiar crítico) | Crítico | Vermelho + buzzer |

---

## 🧪 Como usar

1. Conecte a BitDogLab ao computador via USB.
2. Compile e grave o firmware usando a extensão do Raspberry Pi Pico no Visual Studio Code.
3. Abra o monitor serial para acompanhar as leituras e mensagens de debug.
4. Movimente o joystick:
   - Para a **direita** (X alto), o sistema detectará estado crítico.
   - Para outras posições, o LED mudará de cor conforme a faixa.

---

## 📝 Observações

- O buzzer é ativado com PWM (~2kHz) quando o joystick indica situação crítica.
- O LED RGB usa PWM para controlar brilho e mistura de cores.
- O código foi otimizado para:
  - Uso correto dos dois cores
  - Acesso seguro à FIFO multicore
  - Controle eficiente dos atuadores
- Leitura do eixo Y do joystick não está sendo usada nesta versão, mas pode ser facilmente adicionada.

---

## 📋 Autor

Projeto desenvolvido para BitDogLab - **Escola 4.0 / Unicamp**  
**Desenvolvedor:** ASCCJR

---

## Perguntas e dúvidas:

1. **O joystick so atinge valor alto quando direcionado pra direita e assim acendendo a luz vermelha, é pra ser assim?**

Sim, **é pra ser assim mesmo**


**O código está lendo o eixo X do joystick** (`JOY_X_PIN`, ADC 1).

- Quando o joystick está **no meio** (posição de repouso), ele gera uma tensão média (~1,65V), que no ADC da BitDogLab vira **aproximadamente 2048** (lembrando que o ADC vai de 0 a 4095).
- **Quando você empurra o joystick para a direita**, a tensão no eixo X **aumenta** para perto de 3,3V → o ADC lê **valores altos** (perto de 4095).
- **Quando empurra para a esquerda**, a tensão **cai** para perto de 0V → o ADC lê **valores baixos** (perto de 0).

**Como o limiar crítico (`LIMIAR_CRITICO`) está em 3500**, só quando o joystick vai **fortemente para a direita** é que o `estado > LIMIAR_CRITICO`, e aí:

- O **LED fica vermelho** (estado crítico),
- O **buzzer toca**.

Se deixar o joystick no meio ou empurrar para a esquerda, o estado fica abaixo de 3500, e aí o LED fica **verde, azul** ou **amarelo** (depende da faixa).

---

### 📈 Em resumo:
- Direita = estado alto ➔ vermelho + buzzer
- Meio = estado médio ➔ verde
- Esquerda = estado baixo ➔ azul
- Entre eles = estado médio-alto ➔ amarelo

---

### ⚡ Se quiser que a "zona crítica" váe para outra direção:
- Precisa mudar qual ADC lê (`JOY_Y_PIN` para cima/baixo)  
- Ou inverter os valores no mapeamento de LED.
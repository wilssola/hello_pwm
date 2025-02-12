# Projeto EmbarcaTech PWM

Este projeto demonstra o uso de PWM (Pulse Width Modulation) em um microcontrolador Raspberry Pi Pico para controlar a iluminação de um LED e a posição de um servo motor.

## Estrutura do Projeto

```
.gitignore
.vscode/
build/
CMakeLists.txt
diagram.json
pwm.c
pico_sdk_import.cmake
wokwi.toml
```

## Arquivos Principais

- pwm.c: Contém o código fonte principal que configura e controla o PWM para o LED e o servo motor.
- CMakeLists.txt: Arquivo de configuração do CMake para compilar o projeto.
- diagram.json: Descreve a conexão dos componentes no circuito.
- pico_sdk_import.cmake: Importa o SDK do Raspberry Pi Pico.

## Dependências

- Raspberry Pi Pico SDK
- CMake

## Configuração do Ambiente

1. Clone o repositório.
2. Instale o Raspberry Pi Pico SDK seguindo as instruções da [documentação oficial](https://github.com/raspberrypi/pico-sdk).
3. Configure o ambiente de desenvolvimento com CMake.

## Compilação e Execução

1. Crie um diretório build e navegue até ele:
    ```sh
    mkdir build
    cd build
    ```
2. Execute o CMake para gerar os arquivos de construção:
    ```sh
    cmake ..
    ```
3. Compile o projeto:
    ```sh
    make
    ```
4. Carregue o binário gerado (`pwm.uf2`) no Raspberry Pi Pico.

## Código Fonte

### Configuração do PWM para o LED

A função `led_pwm_setup_irq` configura o PWM para controlar a iluminação do LED com interrupções:

```c
uint led_pwm_setup_irq() {
    gpio_set_function(PIN_LED, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(PIN_LED);
    pwm_clear_irq(slice_num);
    pwm_set_irq_enabled(slice_num, true);
    irq_set_exclusive_handler(PWM_IRQ_WRAP, wrap);
    irq_set_enabled(PWM_IRQ_WRAP, true);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 2.0f);
    pwm_init(slice_num, &config, true);
    return slice_num;
}
```

### Configuração do PWM para o Servo

A função `servo_pwm_setup` configura o PWM para controlar a posição do servo motor:

```c
uint servo_pwm_setup() {
    gpio_set_function(PIN_SERVO, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(PIN_SERVO);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 64.0f);
    pwm_set_wrap(slice_num, 39062);
    pwm_init(slice_num, &config, true);
    return slice_num;
}
```

### Controle do Servo

A função `set_servo_pulse` define a largura do pulso para controlar a posição do servo:

```c
void set_servo_pulse(uint slice_num, float pulse_width_us) {
    int level = (int)((pulse_width_us / 20000.0f) * 39062.0f);
    pwm_set_gpio_level(PIN_SERVO, level);
}
```

### Função Principal

A função `main` configura os PWM e controla o LED e o servo motor:

```c
int main() {
    uint led_slice = led_pwm_setup_irq();
    uint servo_slice = servo_pwm_setup();
    set_servo_pulse(servo_slice, 2400.0f);
    sleep_ms(5000);
    set_servo_pulse(servo_slice, 1470.0f);
    sleep_ms(5000);
    set_servo_pulse(servo_slice, 500.0f);
    sleep_ms(5000);
    while (true) {
        for (float pulse = 500.0f; pulse <= 2400.0f; pulse += 5.0f) {
            set_servo_pulse(servo_slice, pulse);
            sleep_ms(10);
        }
        for (float pulse = 2400.0f; pulse >= 500.0f; pulse -= 5.0f) {
            set_servo_pulse(servo_slice, pulse);
            sleep_ms(10);
        }
    }
}
```

## Licença

Este projeto está licenciado sob a licença [MIT](https://opensource.org/license/mit). Veja o arquivo LICENSE para mais detalhes.
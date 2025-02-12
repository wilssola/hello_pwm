#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/time.h"

#include "hardware/irq.h"
#include "hardware/pwm.h"

#define PIN_LED 12
#define PIN_SERVO 22

int level = 0; // Nível de iluminação do LED
bool rise = true; // Direção da variação de iluminação

void wrap() {
    pwm_clear_irq(pwm_gpio_to_slice_num(PIN_LED)); // Resetar o flag de interrupção para o slice

    level = rise ? level + 1 : level - 1; // Incrementa ou decrementa o nível de iluminação
    level = level > 255 ? 255 : level < 0 ? 0 : level; // Limita o nível de iluminação entre 0 e 255
    rise = level == 255 ? false : level == 0 ? true : rise; // Inverte a direção da variação de iluminação

    pwm_set_gpio_level(PIN_LED, level * level); // Define o nível de iluminação do LED (PWM) de forma quadrática (mais suave)
}

uint led_pwm_setup_irq() {
    gpio_set_function(PIN_LED, GPIO_FUNC_PWM); // Configurar o pino GPIO para a função de PWM
    
    uint slice_num = pwm_gpio_to_slice_num(PIN_LED); // Obter o slice do PWM associado ao pino GPIO (canal do PWM)

    pwm_clear_irq(slice_num); // Resetar o flag de interrupção para o slice
    pwm_set_irq_enabled(slice_num, true); // Habilitar a interrupção para o slice
    irq_set_exclusive_handler(PWM_IRQ_WRAP, wrap); // Definir um tipo de interrupção exclusiva para o wrap
    irq_set_enabled(PWM_IRQ_WRAP, true); // Habilitar a interrupção de wrap

    pwm_config config = pwm_get_default_config(); // Obter a configuração padrão do PWM
    pwm_config_set_clkdiv(&config, 2.0f); // Define o divisor de clock do PWM
    pwm_init(slice_num, &config, true); // Inicializa o slice do PWM com a configuração definida

    return slice_num;
}

uint servo_pwm_setup() {
    gpio_set_function(PIN_SERVO, GPIO_FUNC_PWM);
    
    uint slice_num = pwm_gpio_to_slice_num(PIN_SERVO);

    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 64.0f); // Ajuste o divisor de clock para obter a frequência desejada
    pwm_set_wrap(slice_num, 39062); // Define o valor de wrap para obter um período de 20ms (50Hz)
    pwm_init(slice_num, &config, true);

    return slice_num;
}

void set_servo_pulse(uint slice_num, float pulse_width_us) {
    int level = (int)((pulse_width_us / 20000.0f) * 39062.0f);
    pwm_set_gpio_level(PIN_SERVO, level);
}

int main() {
    uint led_slice = led_pwm_setup_irq();
    printf("LED PWM slice: %d\n", led_slice);

    uint servo_slice = servo_pwm_setup();
    printf("Servo PWM slice: %d\n", servo_slice);

    // Posição 180 graus
    set_servo_pulse(servo_slice, 2400.0f);
    printf("Servo position: 180 degrees\n");
    sleep_ms(5000);

    // Posição 0 graus
    set_servo_pulse(servo_slice, 1470.0f);
    printf("Servo position: 0 degrees\n");
    sleep_ms(5000);

    // Posição 90 graus
    set_servo_pulse(servo_slice, 500.0f);
    printf("Servo position: 90 degrees\n");
    sleep_ms(5000);

    // Movimentação periódica suave entre 0 e 180 graus
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
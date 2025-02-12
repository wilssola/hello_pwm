#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/time.h"

#include "hardware/irq.h"
#include "hardware/pwm.h"

#define PIN_LED 12

int level = 0; // Nível de iluminação do LED
bool rise = true; // Direção da variação de iluminação

void wrap(){
    pwm_clear_irq(pwm_gpio_to_slice_num(PIN_LED)); // Resetar o flag de interrupção para o slice

    level = rise ? level + 1 : level - 1; // Incrementa ou decrementa o nível de iluminação
    level = level > 255 ? 255 : level < 0 ? 0 : level; // Limita o nível de iluminação entre 0 e 255
    rise = level == 255 ? false : level == 0 ? true : rise; // Inverte a direção da variação de iluminação

    pwm_set_gpio_level(PIN_LED, level * level); // Define o nível de iluminação do LED (PWM) de forma quadrática (mais suave)
}

uint pwm_setup_irq(){
    gpio_set_function(PIN_LED, GPIO_FUNC_PWM); // Configurar o pino GPIO para a função de PWM
    
    uint slice_enum = pwm_gpio_to_slice_num(PIN_LED); // Obter o slice do PWM associado ao pino GPIO (canal do PWM)

    pwm_clear_irq(slice_enum); // Resetar o flag de interrupção para o slice
    pwm_set_irq_enabled(slice_enum, true); // Habilitar a interrupção para o slice
    irq_set_exclusive_handler(PWM_IRQ_WRAP, wrap); // Definir um tipo de interrupção exclusiva para o wrap
    irq_set_enabled(PWM_IRQ_WRAP, true); // Habilitar a interrupção de wrap

    pwm_config config = pwm_get_default_config(); // Obter a configuração padrão do PWM
    pwm_config_set_clkdiv(&config, 10.0f); // Define o divisor de clock do PWM
    pwm_init(slice_enum, &config, true); // Inicializa o slice do PWM com a configuração definida

    return slice_enum;
}

int main(){
    uint slice_enum = pwm_setup_irq();

    while(true) {
        pwm_set_irq_enabled(slice_enum, true);
        sleep_ms(5000);

        pwm_set_irq_enabled(slice_enum, false);
        sleep_ms(5000);
    }
}
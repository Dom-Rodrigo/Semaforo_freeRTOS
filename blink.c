#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "hardware/pwm.h"
#include "hardware/clocks.h"


#define led_pin_green 11
#define led_pin_red 13
#define BUZZER_A 21
#define BUZZER_B 10

void vBlinkTask()
{
    while (true)
    {
        gpio_put(led_pin_green, true);
        vTaskDelay(pdMS_TO_TICKS(2000));
        gpio_put(led_pin_green, false);
        vTaskDelay(pdMS_TO_TICKS(1000));
        printf("Blink\n");
    }
}

void vBlinkTask2()
{

    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        gpio_put(led_pin_red, true);
        vTaskDelay(pdMS_TO_TICKS(2000));
        gpio_put(led_pin_red, false);
        printf("Blink\n");
    }
}

void vBeepVerde(){
    while (true){
        pwm_set_gpio_level(BUZZER_A, 2048);
        vTaskDelay(pdMS_TO_TICKS(200));
        pwm_set_gpio_level(BUZZER_A, 0); 
        vTaskDelay(pdMS_TO_TICKS(2800));
    }
}

void vBeepAmarelo(){
    while (true){
        vTaskDelay(pdMS_TO_TICKS(1000));
        pwm_set_gpio_level(BUZZER_A, 2048);
        vTaskDelay(pdMS_TO_TICKS(200));
        pwm_set_gpio_level(BUZZER_A, 0); 
        vTaskDelay(pdMS_TO_TICKS(1800));
    }
}


// Trecho para modo BOOTSEL com botão B
#include "pico/bootrom.h"
#define botaoB 6
void gpio_irq_handler(uint gpio, uint32_t events)
{
    reset_usb_boot(0, 0);
}

// Definição de uma função para inicializar o PWM no pino do buzzer
void pwm_init_buzzer(uint pin, int frequency)
{
    // Configurar o pino como saída de PWM
    gpio_set_function(pin, GPIO_FUNC_PWM);

    // Obter o slice do PWM associado ao pino
    uint slice_num = pwm_gpio_to_slice_num(pin);

    // Configurar o PWM com frequência desejada
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, clock_get_hz(clk_sys) / (frequency * 4096)); // Divisor de clock
    pwm_init(slice_num, &config, true);

    // Iniciar o PWM no nível baixo
    pwm_set_gpio_level(pin, 0);
}

int main()
{
    // Para ser utilizado o modo BOOTSEL com botão B
    gpio_init(botaoB);
    gpio_set_dir(botaoB, GPIO_IN);
    gpio_pull_up(botaoB);
    gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL,
                                         true, &gpio_irq_handler);
    // Fim do trecho para modo BOOTSEL com botão B

    stdio_init_all();
    gpio_init(led_pin_red);
    gpio_init(led_pin_green);
    gpio_set_dir(led_pin_red, GPIO_OUT);
    gpio_set_dir(led_pin_green, GPIO_OUT);

    gpio_init(BUZZER_A);
    gpio_set_dir(BUZZER_A, GPIO_OUT);
    pwm_init_buzzer(BUZZER_A, 100);

    gpio_init(BUZZER_B);
    gpio_set_dir(BUZZER_B, GPIO_OUT);
    pwm_init_buzzer(BUZZER_B, 100);

    xTaskCreate(vBlinkTask, "Tarefa do LED VERDE", 
        configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vBlinkTask2, "Tarefa do LED VERMELHO", 
        configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vBeepVerde, "Tareda do Beep do farol verde", 
        configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vBeepAmarelo, "Tareda do Beep do farol amarelo", 
        configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    vTaskStartScheduler();
    panic_unsupported();
}

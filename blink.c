#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "pico/bootrom.h"


#define led_pin_green 11
#define led_pin_red 13
#define BUZZER_A 21
#define BUZZER_B 10
#define botaoA 5
#define botaoB 6

TaskHandle_t xBlinkTask = NULL;
TaskHandle_t xBlinkTask2 = NULL;
TaskHandle_t xBeepVerde = NULL;
TaskHandle_t xBeepAmarelo = NULL;
TaskHandle_t xBeepVermelho = NULL;
TaskHandle_t xModoNoturno = NULL;

void vBlinkTask()
{
    while (true)
    {
        gpio_put(led_pin_green, true);
        vTaskDelay(pdMS_TO_TICKS(20000));
        gpio_put(led_pin_green, false);
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

void vBlinkTask2()
{

    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(10000));
        gpio_put(led_pin_red, true);
        vTaskDelay(pdMS_TO_TICKS(20000));
        gpio_put(led_pin_red, false);
    }
}

void vBeepVerde(){
    uint slice_num_a = pwm_gpio_to_slice_num(BUZZER_A);
    while (true){
        vTaskDelay(pdMS_TO_TICKS(2000));
        pwm_set_enabled(slice_num_a, true);  
        pwm_set_gpio_level(BUZZER_A, 0); 
        pwm_set_gpio_level(BUZZER_A, 2048);
        vTaskDelay(pdMS_TO_TICKS(2000));
        pwm_set_gpio_level(BUZZER_A, 0); 
        vTaskDelay(pdMS_TO_TICKS(26000));
    }
}

void vBeepAmarelo(){
    uint slice_num_b = pwm_gpio_to_slice_num(BUZZER_B);
    while (true){
        pwm_set_enabled(slice_num_b, true);  
        pwm_set_gpio_level(BUZZER_B, 0); 
        vTaskDelay(pdMS_TO_TICKS(10000));
        pwm_set_gpio_level(BUZZER_B, 2048);
        vTaskDelay(pdMS_TO_TICKS(2000));
        pwm_set_gpio_level(BUZZER_B, 0); 
        pwm_set_gpio_level(BUZZER_B, 2048);
        vTaskDelay(pdMS_TO_TICKS(2000));
        pwm_set_gpio_level(BUZZER_B, 0); 
        vTaskDelay(pdMS_TO_TICKS(16000));
    }
}

void vBeepVermelho(){
    uint slice_num_a = pwm_gpio_to_slice_num(BUZZER_A);
    while (true){
        pwm_set_enabled(slice_num_a, true);  
        pwm_set_gpio_level(BUZZER_A, 0); 
        vTaskDelay(pdMS_TO_TICKS(20000));
        pwm_set_gpio_level(BUZZER_A, 2048);
        vTaskDelay(pdMS_TO_TICKS(2000));
        pwm_set_gpio_level(BUZZER_A, 0); 
        pwm_set_gpio_level(BUZZER_A, 2048);
        vTaskDelay(pdMS_TO_TICKS(2000));
        pwm_set_gpio_level(BUZZER_A, 0); 
        vTaskDelay(pdMS_TO_TICKS(6000));
    }
}

void vModoNoturno(){


    while (true){
        gpio_put(led_pin_green, true);
        gpio_put(led_pin_red, true);
        pwm_set_gpio_level(BUZZER_A, 0);  // Periféricos são independentes
        pwm_set_gpio_level(BUZZER_B, 0); 
        uint slice_num_a = pwm_gpio_to_slice_num(BUZZER_A);
        uint slice_num_b = pwm_gpio_to_slice_num(BUZZER_B);
        pwm_set_enabled(slice_num_a, false);
        pwm_set_enabled(slice_num_b, false);
        vTaskSuspend(xBlinkTask);
        vTaskSuspend(xBlinkTask2);
        vTaskSuspend(xBeepVerde);
        vTaskSuspend(xBeepAmarelo);
        vTaskSuspend(xBeepVermelho);

        // vTaskDelay(pdMS_TO_TICKS(2000));
        // gpio_put(led_pin_green, true);
        // gpio_put(led_pin_red, true);
        vTaskSuspend(NULL);

    }
}

// Trecho para modo BOOTSEL com botão B e alter flag do modo com botão A
volatile uint32_t last_time;
bool modo_noturno = false;
volatile bool a_pressionado = false;
void gpio_irq_handler(uint gpio, uint32_t events)
{
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    if (current_time - last_time > 500000){
        if (!gpio_get(botaoB)) {
            last_time = current_time;
            rom_reset_usb_boot(0, 0);
        }
        if (!gpio_get(botaoA)) {
            a_pressionado = true;
            last_time = current_time;
        }

    }   
}

// Interrupção como tarefa
void vInterrupcaoBotao(void *pvParameters) {
    while (true) {
        if (a_pressionado) {
            a_pressionado = false;
            modo_noturno = !modo_noturno;
            if (modo_noturno) {
                vTaskResume(xModoNoturno);
            } else {
                vTaskResume(xBlinkTask);
                vTaskResume(xBlinkTask2);
                vTaskResume(xBeepVerde);
                vTaskResume(xBeepAmarelo);
                vTaskResume(xBeepVermelho);
                uint slice_num_a = pwm_gpio_to_slice_num(BUZZER_A);
                uint slice_num_b = pwm_gpio_to_slice_num(BUZZER_B);
                // pwm_set_enabled(slice_num_a, true);
                // pwm_set_enabled(slice_num_b, true);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
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

    gpio_init(botaoA);
    gpio_set_dir(botaoA, GPIO_IN);
    gpio_pull_up(botaoA);
    gpio_set_irq_enabled_with_callback(botaoA, GPIO_IRQ_EDGE_FALL,
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
    pwm_init_buzzer(BUZZER_B, 150);

    xTaskCreate(vBlinkTask, "Tarefa do LED VERDE", 
        configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &xBlinkTask);
    xTaskCreate(vBlinkTask2, "Tarefa do LED VERMELHO", 
        configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &xBlinkTask2);
    xTaskCreate(vBeepVerde, "Tarefa do Beep do farol verde", 
        configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &xBeepVerde);
    xTaskCreate(vBeepAmarelo, "Tarefa do Beep do farol amarelo", 
        configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &xBeepAmarelo);
    xTaskCreate(vBeepVermelho, "Tarefa do Beep do farol vermelho", 
        configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &xBeepVermelho);
    xTaskCreate(vModoNoturno, "Tarefa do Modo Noturno",
        configMINIMAL_STACK_SIZE, NULL, 2, &xModoNoturno);
    xTaskCreate(vInterrupcaoBotao, "Tarefa do Modo Noturno",
        configMINIMAL_STACK_SIZE, NULL, 3, NULL);
    vTaskStartScheduler();
    panic_unsupported();
}

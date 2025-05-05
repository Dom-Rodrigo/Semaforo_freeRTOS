#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "pico/bootrom.h"
#include "ws2818b.pio.h"
#include "hardware/pio.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

#define led_pin_green 11
#define led_pin_red 13
#define BUZZER_A 21
#define BUZZER_B 10
#define botaoA 5
#define botaoB 6
#define LED_MATRIZ 7
#define LED_COUNT 25

int array_num_zero[12] = {1, 2, 3, 6, 13, 16, 23, 22, 21, 18, 11, 8};
int array_num_um[6] = {2, 7, 12, 17, 16, 22};
int array_num_dois[8] = {1, 2, 7, 12, 11, 18, 21, 22};
int array_num_tres[8] = {1, 2, 8, 11, 12, 18, 21, 22};
int array_num_quatro[9] = {23, 16, 13, 12, 11, 18, 21, 8, 1};
int array_num_cinco[8] = {21, 22, 17, 12, 11, 8, 1, 2};
int array_num_seis[12] = {1, 2, 3, 6, 13, 16, 23 ,22, 21, 12, 11, 8};
int array_num_sete[6] = {23, 22, 17, 12, 7, 2};
int array_num_oito[13] = {1, 2, 3, 6, 13, 16, 23, 22, 21, 18, 11, 8, 12};
int array_num_nove[10] = {1, 13, 16, 23, 22, 21, 18, 11, 8, 12};
// Definição de pixel GRB
struct pixel_t
{
    uint8_t G, R, B; // Três valores de 8-bits compõem um pixel.
};
typedef struct pixel_t pixel_t;
typedef pixel_t npLED_t; // Mudança de nome de "struct pixel_t" para "npLED_t" por clareza.

// Declaração do buffer de pixels que formam a matriz.
npLED_t leds[LED_COUNT];
// Variáveis para uso da máquina PIO.
PIO np_pio;
uint sm;

/**
 * Inicializa a máquina PIO para controle da matriz de LEDs.
 */
void npInit(uint pin)
{

    // Cria programa PIO.
    uint offset = pio_add_program(pio0, &ws2818b_program);
    np_pio = pio0;

    // Toma posse de uma máquina PIO.
    sm = pio_claim_unused_sm(np_pio, false);
    if (sm < 0)
    {
        np_pio = pio1;
        sm = pio_claim_unused_sm(np_pio, true); // Se nenhuma máquina estiver livre, panic!
    }

    // Inicia programa na máquina PIO obtida.
    ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);

    // Limpa buffer de pixels.
    for (uint i = 0; i < LED_COUNT; ++i)
    {
        leds[i].R = 0;
        leds[i].G = 0;
        leds[i].B = 0;
    }
}

/**
 * Atribui uma cor RGB a um LED.
 */
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b)
{
    leds[index].R = r;
    leds[index].G = g;
    leds[index].B = b;
}

/**
 * Limpa o buffer de pixels.
 */
void npClear()
{
    for (uint i = 0; i < LED_COUNT; ++i)
        npSetLED(i, 0, 0, 0);
}

/**
 * Escreve os dados do buffer nos LEDs.
 */
void npWrite()
{
    // Escreve cada dado de 8-bits dos pixels em sequência no buffer da máquina PIO.
    for (uint i = 0; i < LED_COUNT; ++i)
    {
        pio_sm_put_blocking(np_pio, sm, leds[i].G);
        pio_sm_put_blocking(np_pio, sm, leds[i].R);
        pio_sm_put_blocking(np_pio, sm, leds[i].B);
    }
    sleep_us(100); // Espera 100us, sinal de RESET do datasheet.
}

void desenha_numero(int array_pixels[], int size, int R, int G, int B){
    npClear();
    npWrite();
    for (int i=0;  i < size; i++){
        npSetLED(array_pixels[i], R, G, B);
        sleep_us(200);
    }
    npWrite();
}

void desenha_numeros(int numero, int R, int G, int B){
    switch (numero)
    {
        case 0:
            desenha_numero(array_num_zero, 12, R, G, B);
            break;
        case 1:
            desenha_numero(array_num_um, 6, R, G, B);
            break;
        case 2:
            desenha_numero(array_num_dois, 8, R, G, B);
            break;
        case 3:
            desenha_numero(array_num_tres, 8, R, G, B);
            break;
        case 4:
            desenha_numero(array_num_quatro, 9, R, G, B);
            break;
        case 5:
            desenha_numero(array_num_cinco, 8, R, G, B);
            break;
        case 6:
            desenha_numero(array_num_seis, 12, R, G, B);
            break;
        case 7:
            desenha_numero(array_num_sete, 6, R, G, B);
            break;
        case 8:
            desenha_numero(array_num_oito, 13, R, G, B);
            break;
        case 9:
            desenha_numero(array_num_nove, 10, R, G, B);
            break;
    }
}
TaskHandle_t xBlinkTask = NULL;
TaskHandle_t xBlinkTask2 = NULL;
TaskHandle_t xBeepVerde = NULL;
TaskHandle_t xBeepAmarelo = NULL;
TaskHandle_t xBeepVermelho = NULL;
TaskHandle_t xModoNoturno = NULL;

// volatile int numero = 0;
// void vTimerMatriz(){
//     int num = numero;
//     if (num > 9)
//         num = 0;
//     desenha_numeros(num, 50, 0, 0);
//     vTaskDelay(1000);
// }

void vTimerMatriz(){
    desenha_numeros(1, 50, 0, 0);
    vTaskDelay(pdMS_TO_TICKS(1000));

}
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

void vMatrizTask()
{
    int contador = 0;
    int color = 0;
    bool cor = true;
    while (true)
    {
                    
        if (contador > 9){
            contador = 0;
            color++;
        }
        if (color > 2){
            color=0;
        }
        if (color == 0){
            desenha_numeros(contador, 0, 50, 0);
        }
        if (color == 1){
            desenha_numeros(contador, 50, 50, 0);
        }
        if (color == 2){
            desenha_numeros(contador, 50, 0, 0);
        }
        contador++; // Incrementa o contador
        vTaskDelay(1000);
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
        vTaskDelay(pdMS_TO_TICKS(200));  // Debounce
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
                // gpio_put(led_pin_green, false);
                // gpio_put(led_pin_red, false);
                // uint slice_num_a = pwm_gpio_to_slice_num(BUZZER_A);
                // uint slice_num_b = pwm_gpio_to_slice_num(BUZZER_B);
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

    npInit(LED_MATRIZ);
    npClear();
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
        configMINIMAL_STACK_SIZE, NULL, 3, &xBlinkTask);
    xTaskCreate(vBlinkTask2, "Tarefa do LED VERMELHO", 
        configMINIMAL_STACK_SIZE, NULL, 3, &xBlinkTask2);
    xTaskCreate(vBeepVerde, "Tarefa do Beep do farol verde", 
        configMINIMAL_STACK_SIZE, NULL, 3, &xBeepVerde);
    xTaskCreate(vBeepAmarelo, "Tarefa do Beep do farol amarelo", 
        configMINIMAL_STACK_SIZE, NULL, 3, &xBeepAmarelo);
    xTaskCreate(vBeepVermelho, "Tarefa do Beep do farol vermelho", 
        configMINIMAL_STACK_SIZE, NULL, 3, &xBeepVermelho);
    xTaskCreate(vMatrizTask, "Tarefa do Beep do farol vermelho", 
        configMINIMAL_STACK_SIZE, NULL, 3, &xBeepVermelho);
    // xTaskCreate(vTimerMatriz, "Tarefa da Matriz", configMINIMAL_STACK_SIZE, NULL, 0, NULL);
    // xTaskCreate(vTimerMatriz, "Tarefa do Timer com a Matriz", 
    //     configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    // xTaskCreate(vModoNoturno, "Tarefa do Modo Noturno",
    //     configMINIMAL_STACK_SIZE, NULL, 2, &xModoNoturno);
    // xTaskCreate(vInterrupcaoBotao, "Tarefa do Modo Noturno",
    //     configMINIMAL_STACK_SIZE, NULL, 3, NULL);
    vTaskStartScheduler();
    panic_unsupported();
}

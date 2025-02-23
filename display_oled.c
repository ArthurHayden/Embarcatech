#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "hardware/clocks.h"
#include <stdbool.h> // Inclua a biblioteca para usar bool
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "inc/ssd1306.h" // Inclua a biblioteca SSD1306
#include "hardware/i2c.h"
#include "hardware/pwm.h" // Inclua a biblioteca PWM

#include <time.h>
//#include "ws2818b.pio.h"

// Biblioteca gerada pelo arquivo .pio durante compilação.
#define LED_COUNT 25
#define LED_PIN 7
// Define the LED pins
#define LED_RED 13     // Replace with your actual red LED pin
#define LED_GREEN 11   // Replace with your actual green LED pin
#define LED_BLUE 12    // Replace with your actual blue LED pin
#define BOTAO_A 5      // Pino do botão A
#define BOTAO_B 6
#define BUZZER_PIN 21
// Definições para o I2C 
#define I2C_SDA 14
#define I2C_SCL 15

#define JOYSTICK_CENTER 2000 // Ajuste este valor com base no seu joystick
#define JOYSTICK_DEADZONE 500
// Variável global para armazenar o slice do PWM do buzzer



uint buzzer_slice_num;
typedef struct {
    int frequency;
    uint duration_ms;
} Note;
//FUNÇÕES DO BUZZER
// Inicializa o PWM no pino do buzzer com uma frequência inicial (ex: 1kHz)
void pwm_init_buzzer(uint pin, int frequency) {
    // Configurar o pino como saída de PWM
    gpio_set_function(pin, GPIO_FUNC_PWM);

    // Obter o slice do PWM associado ao pino e armazenar na variável global
    buzzer_slice_num = pwm_gpio_to_slice_num(pin);

    // Configurar o PWM com a frequência inicial
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, clock_get_hz(clk_sys) / (frequency * 4096)); // Divisor de clock
    pwm_init(buzzer_slice_num, &config, true);

    // Iniciar o PWM no nível baixo
    pwm_set_gpio_level(pin, 0);
}

// Função para ajustar a frequência do buzzer
void set_buzzer_frequency(int frequency) {
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, clock_get_hz(clk_sys) / (frequency * 4096));
    pwm_init(buzzer_slice_num, &config, true);
    pwm_set_gpio_level(BUZZER_PIN, 2048); // <---- Adicione esta linha
}


// Definição de uma função para emitir um beep com duração e frequência especificadas
void beep(uint pin, int frequency, uint duration_ms) {

    set_buzzer_frequency(frequency); // Ajusta a frequência antes de tocar

    // Configurar o duty cycle para 50% (ativo)
    pwm_set_gpio_level(pin, 2048);

    // Temporização
    sleep_ms(duration_ms);

    // Desativar o sinal PWM (duty cycle 0)
    pwm_set_gpio_level(pin, 0);

    // Pausa entre os beeps
    sleep_ms(100); // Pausa de 100ms
}

// Função para tocar uma sequência de notas
void play_melody(uint pin, const Note melody[], int melody_len) {
    for (int i = 0; i < melody_len; i++) {
        set_buzzer_frequency(melody[i].frequency);
        pwm_set_gpio_level(pin, 2048); // Inicia o som
        sleep_ms(melody[i].duration_ms);
        pwm_set_gpio_level(pin, 0);    // Desliga o som
        sleep_ms(50); // Pausa entre as notas
    }
}
//FIM DAS FUNÇÕES DO BUZZER
// Frequências PWM desejadas


// Função para inicializar o PWM para um LED com a frequência especificada

// Definição dos estados
typedef enum {
    MENU,
    JOGO_1,
    JOGO_2,
    JOGO_3,
    JOGO_4
} Estado;
Estado estado_atual = MENU; // Estado inicial

int selected_game = 1;
uint8_t ssd[ssd1306_buffer_length];
struct render_area frame_area = {
    start_column: 0, end_column: ssd1306_width - 1,
    start_page: 0, end_page: ssd1306_n_pages - 1
};



/*void executar_menu() {
    gpio_put(LED_BLUE, 1);
    
    adc_select_input(0); // Seleciona a entrada ADC do joystick (eixo Y)
    sleep_ms(2000);
    gpio_put(LED_BLUE, 0);
    //estado_atual = JOGO_1;
    while (true) {
        memset(ssd, 0, ssd1306_buffer_length); // Limpa o buffer

        // Exibe o número do jogo selecionado
        char game_num_str[2];
        sprintf(game_num_str, "%d", selected_game);
        ssd1306_draw_string(ssd, 50, 25, game_num_str);
        render_on_display(ssd, &frame_area);

        uint16_t adc_y_raw = adc_read();

        // Ajusta o jogo selecionado com base na posição do joystick
        if (adc_y_raw < 1000 && selected_game > 1) { // Movimento para cima/esquerda
            selected_game--;
            sleep_ms(200); // Debounce
            sprintf(game_num_str, "%d", selected_game);
            ssd1306_draw_string(ssd, 50, 25, game_num_str);
            render_on_display(ssd, &frame_area);
        } else if (adc_y_raw > 60000 && selected_game < 4) { // Movimento para baixo/direita
            selected_game++;
            sprintf(game_num_str, "%d", selected_game);
            ssd1306_draw_string(ssd, 50, 25, game_num_str);
            render_on_display(ssd, &frame_area);
            sleep_ms(200); // Debounce
        }

        // Verifica se o botão A foi pressionado
        if (gpio_get(BOTAO_A) == 0) {
            sleep_ms(200); // Debounce
            switch (selected_game) {
                case 1: estado_atual = JOGO_1; break;
                case 2: estado_atual = JOGO_2; break;
                case 3: estado_atual = JOGO_3; break;
                case 4: estado_atual = JOGO_4; break;
            }
            break; // Sai do loop
        }
    }
    gpio_put(LED_BLUE, 0);
}*/

float gerar_numero_aleatorio() {
    int parte_inteira = rand() % 100 + 1; // Gera um número inteiro entre 1 e 10
    //int parte_decimal = rand() % 100; // Gera um número inteiro entre 0 e 99
    return parte_inteira ; // Combina as partes inteira e decimal
}
void executar_jogo_1() {
    memset(ssd, 0, ssd1306_buffer_length);
    ssd1306_draw_string(ssd, 0, 0, "voce esta no jogo 1");
    render_on_display(ssd, &frame_area);
    if (gpio_get(BOTAO_B) == 0) {
        estado_atual = MENU;
    }
}

void executar_jogo_2() {
    memset(ssd, 0, ssd1306_buffer_length);
    ssd1306_draw_string(ssd, 0, 0, "voce esta no jogo 2");
    render_on_display(ssd, &frame_area);
    if (gpio_get(BOTAO_B) == 0) {
        estado_atual = MENU;
    }
}
bool joystick_adjusted = false; // Declare a variável aqui

int main() {
    stdio_init_all();
    adc_init();
    srand(time(NULL)); // Inicializa o gerador de números aleatórios
    bool in_game = false;
    
    memset(ssd, 0, ssd1306_buffer_length); 
    // Inicializa o I2C para o display OLED
    i2c_init(i2c1, ssd1306_i2c_clock * 1000); // Use i2c1 como no seu exemplo
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Inicializa o display OLED (como no seu exemplo)
    ssd1306_init(); 

    // Define a área de renderização (como no seu exemplo)
    struct render_area frame_area = {
        start_column: 0, end_column: ssd1306_width - 1,
        start_page: 0, end_page: ssd1306_n_pages - 1
    };
    calculate_render_area_buffer_length(&frame_area);
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area); 

    ssd1306_draw_string(ssd, 0, 0, "Teste");
    render_on_display(ssd, &frame_area);
    // Initialize the ADC pins for the joystick
    adc_gpio_init(26);
    adc_gpio_init(27);

    // VARIAVEIS DO SOM
    stdio_init_all();
    pwm_init_buzzer(BUZZER_PIN, 1000); // Inicializa com 1kHz
    Note acerto_melody[] = {
        {8000, 100}, // Nota 1: 800Hz por 100ms
        {10000, 100}, // Nota 2: 1000Hz por 100ms
        {10000, 100}, // Nota 3: 1200Hz por 100ms
        
    };
    int acerto_len = sizeof(acerto_melody) / sizeof(acerto_melody[0]);

    Note erro_melody[] = {
        {2000, 100}, // Nota 1: 800Hz por 100ms
        {2000, 100}, // Nota 2: 1000Hz por 100ms
        
        

    };
    int erro_len = sizeof(erro_melody) / sizeof(erro_melody[0]);

    //FIM DAS VARIAVEIS DO SOM


    // Inicializa o botão A como entrada com pull-up
    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);

     // Inicializa o botão A como entrada com pull-up
     gpio_init(BOTAO_B);
     gpio_set_dir(BOTAO_B, GPIO_IN);
     gpio_pull_up(BOTAO_B);

    // Inicializa o PWM para os LEDs com as frequências especificadas
    gpio_init(LED_BLUE); gpio_set_dir(LED_BLUE, GPIO_OUT);
    gpio_init(LED_GREEN); gpio_set_dir(LED_GREEN, GPIO_OUT);
    gpio_init(LED_RED); gpio_set_dir(LED_RED, GPIO_OUT);
    
    char direcao[20] = "Centro"; // Inicializa com "Centro"
    
    // Read the joystick valueslear
    adc_select_input(0);
    uint16_t adc_y_raw = adc_read();
    adc_select_input(1);
    uint16_t adc_x_raw = adc_read();
        
        
    printf("antes do loop");
    gpio_put(LED_BLUE, 1);
    sleep_ms(2000);
    gpio_put(LED_BLUE, 0);
    // Mostra a informação no display usando render_on_display 
    while (true) {
        switch (estado_atual) {
            case MENU:
                memset(ssd, 0, ssd1306_buffer_length); // Limpa o buffer
                ssd1306_draw_string(ssd, 0, 0, "entrou no menu");
                render_on_display(ssd, &frame_area); 
                gpio_put(LED_GREEN, 1);
                sleep_ms(2000);
                gpio_put(LED_GREEN, 0);
                memset(ssd, 0, ssd1306_buffer_length); // Limpa o buffer
                ssd1306_draw_string(ssd, 0, 0, "Jogo 1");
                ssd1306_draw_string(ssd, 0, 15, "Jogo 2");
                ssd1306_draw_string(ssd, 0, 30, "Jogo 3");
                ssd1306_draw_string(ssd, 0, 45, "Jogo 4");
                
                render_on_display(ssd, &frame_area); 
                sleep_ms(3000);
                //executar_menu();
                while (true) {
                    //memset(ssd, 0, ssd1306_buffer_length); // Limpa o buffer

                    // Exibe o número do jogo selecionado
                    char game_num_str[2];
                    sprintf(game_num_str, "%d", selected_game);
                    ssd1306_draw_string(ssd, 70, 25, game_num_str);
                    render_on_display(ssd, &frame_area);

                    uint16_t adc_y_raw = adc_read();
                    uint16_t adc_x_raw = adc_read();
                    // Ajusta o jogo selecionado com base na posição do joystick
                  
                    

                    // Lógica de ajuste do jogo com joystick
                    if (adc_x_raw > (JOYSTICK_CENTER + JOYSTICK_DEADZONE) && !joystick_adjusted) {
                        if (selected_game < 4) {
                            selected_game++;
                            sprintf(game_num_str, "%d", selected_game);
                            ssd1306_draw_string(ssd, 70, 25, game_num_str);
                            render_on_display(ssd, &frame_area);
                            sleep_ms(100);
                        }
                        joystick_adjusted = true;
                    } else if (adc_x_raw < (JOYSTICK_CENTER - JOYSTICK_DEADZONE) && !joystick_adjusted) {
                        if (selected_game > 1) {
                            selected_game--;
                             sprintf(game_num_str, "%d", selected_game);
                            ssd1306_draw_string(ssd, 70, 25, game_num_str);
                            render_on_display(ssd, &frame_area);
                            sleep_ms(100);
                        }
                        joystick_adjusted = true;
                    } else if (adc_x_raw >= (JOYSTICK_CENTER - JOYSTICK_DEADZONE) &&
                               adc_x_raw <= (JOYSTICK_CENTER + JOYSTICK_DEADZONE)) {
                        joystick_adjusted = false;
                    }

                        // Verifica se o botão A foi pressionado
                    if (gpio_get(BOTAO_A) == 0) {
                        sleep_ms(200); // Debounce
                        switch (selected_game) {
                            case 1: estado_atual = JOGO_1; break;
                            case 2: estado_atual = JOGO_2; break;
                            case 3: estado_atual = JOGO_3; break;
                            case 4: estado_atual = JOGO_4; break;
                        }
                        break; // Sai do loop
                    }
                }
                gpio_put(LED_BLUE, 0);
                break;
            case JOGO_1:
                memset(ssd, 0, ssd1306_buffer_length); 
                ssd1306_draw_string(ssd, 0, 0, "jogo 1 ");
                ssd1306_draw_string(ssd, 0, 20, "selecionado");
                render_on_display(ssd, &frame_area); 
                gpio_put(LED_RED, 1);
                sleep_ms(2000);

                memset(ssd, 0, ssd1306_buffer_length); 
                ssd1306_draw_string(ssd, 0, 0, "Aperte botao B");
                ssd1306_draw_string(ssd, 0, 20, "para voltar ");
                ssd1306_draw_string(ssd, 0, 30, "ao menu");
                render_on_display(ssd, &frame_area); 
                sleep_ms(2000);
                memset(ssd, 0, ssd1306_buffer_length); 
                ssd1306_draw_string(ssd, 0, 0, "numero aleatorio");
                render_on_display(ssd, &frame_area); 
                sleep_ms(2000);
                memset(ssd, 0, ssd1306_buffer_length);
                render_on_display(ssd, &frame_area); 

                int num=0;
                
                while(gpio_get(BOTAO_B) != 0){
                    if(gpio_get(BOTAO_B) == 0){
                        sleep_ms(100); // Debounce
                        estado_atual = MENU;
                        break;
                    }
                    num = gerar_numero_aleatorio(); // Chama a função e armazena o valor em num
                    memset(ssd, 0, ssd1306_buffer_length);
                    char num_str[1];
                    memset(ssd, 0, ssd1306_buffer_length); 
                    ssd1306_draw_string(ssd, 0, 0, "numero sorteado");
                    render_on_display(ssd, &frame_area);
                    sprintf(num_str, "%d",num); // F
                    ssd1306_draw_string(ssd, 0, 20, num_str); // Exibe a string no display
                    render_on_display(ssd, &frame_area);
                    sleep_ms(3000);
                    for (int contador = 1; contador <= 100; contador++) {
                        // Exibe o contador no display
                        memset(ssd, 0, ssd1306_buffer_length);
                        char contador_str[1];
                        sprintf(contador_str, "%d", contador);
                        ssd1306_draw_string(ssd, 0, 30, contador_str); // Exibe na segunda linha
                        render_on_display(ssd, &frame_area);
                        sleep_ms(100);
                        // Verifica se o botão A foi pressionado
                        if (gpio_get(BOTAO_A) == 0) {
                            // Verifica se o jogador acertou o número
                            
                            if (contador == num) {
                                // Jogador ganhou

                                memset(ssd, 0, ssd1306_buffer_length);
                                ssd1306_draw_string(ssd, 0, 0, "Voce ganhou!");
                                
                                render_on_display(ssd, &frame_area);
                                play_melody(BUZZER_PIN, acerto_melody, acerto_len); // Toca a melodia de acerto
                                sleep_ms(3000);
                            } else {
                                // Jogador perdeu
                                memset(ssd, 0, ssd1306_buffer_length);
                                ssd1306_draw_string(ssd, 0, 0, "Voce perdeu!");
                                render_on_display(ssd, &frame_area);
                                play_melody(BUZZER_PIN, erro_melody, erro_len); // Toca a melodia de acerto
                                char num_str[1];
                                ssd1306_draw_string(ssd, 0, 10, "numero correto");
                                sprintf(num_str, "%d", num);
                                ssd1306_draw_string(ssd, 0, 20, num_str); //
                                render_on_display(ssd, &frame_area);
                                sleep_ms(3000);
                            }
                            while(true){
                                if (gpio_get(BOTAO_A) == 0) {
                                    estado_atual = JOGO_1;
                                    break;
                                }
                                else if (gpio_get(BOTAO_B) == 0){
                                    estado_atual = MENU;
                                    break;
                                }
                                memset(ssd, 0, ssd1306_buffer_length);
                                ssd1306_draw_string(ssd, 0, 10, "Aperte A");
                                ssd1306_draw_string(ssd, 0, 20, "para jogar ");
                                ssd1306_draw_string(ssd, 0, 30, "de novo");
                                render_on_display(ssd, &frame_area);
                                
                            }
                            break;

                        }

                    }
                }
                gpio_put(LED_RED, 0);
                gpio_put(LED_BLUE, 1);
    
                
                sleep_ms(2000);
                gpio_put(LED_BLUE, 0);
                memset(ssd, 0, ssd1306_buffer_length);
                estado_atual = MENU;
                //executar_jogo_1(BOTAO_B);
                break;
            case JOGO_2:
                gpio_put(LED_RED, 1);
                gpio_put(LED_BLUE, 1);
                memset(ssd, 0, ssd1306_buffer_length); 
                ssd1306_draw_string(ssd, 0, 0, "jogo 2 ");
                ssd1306_draw_string(ssd, 0, 20, "selecionado");
                render_on_display(ssd, &frame_area); 
                sleep_ms(2000);
                memset(ssd, 0, ssd1306_buffer_length); 
                ssd1306_draw_string(ssd, 0, 0, "Aperte botao B");
                ssd1306_draw_string(ssd, 0, 20, "para voltar ");
                ssd1306_draw_string(ssd, 0, 30, "ao menu");
                render_on_display(ssd, &frame_area); 
                sleep_ms(2000);
                while(true){
                    if(gpio_get(BOTAO_B) == 0){
                        sleep_ms(100); // Debounce
                        estado_atual = MENU;
                        break;
                    }
                    /*memset(ssd, 0, ssd1306_buffer_length); 
                    ssd1306_draw_string(ssd, 0, 0, "jogo das cores");
                    render_on_display(ssd, &frame_area); 
                    sleep_ms(2000);
                    memset(ssd, 0, ssd1306_buffer_length);
                    render_on_display(ssd, &frame_area); 

                    memset(ssd, 0, ssd1306_buffer_length); 
                    ssd1306_draw_string(ssd, 0, 0, "Grave a sequencia");
                    ssd1306_draw_string(ssd, 0, 10, "vermelho");
                    ssd1306_draw_string(ssd, 0, 20, "verde");
                    ssd1306_draw_string(ssd, 0, 30, "azul");
                    render_on_display(ssd, &frame_area); 
                    sleep_ms(4000);
                    memset(ssd, 0, ssd1306_buffer_length);
                    render_on_display(ssd, &frame_area);

                    memset(ssd, 0, ssd1306_buffer_length); 
                    ssd1306_draw_string(ssd, 0, 0, "sua vez");
                    render_on_display(ssd, &frame_area);
                    int cont;
                    while(cont>=3){
                        if(gpio_get(BOTAO_A) == 0 &&()){
                            sleep_ms(100); // Debounce
                            
                        }
                    } 

                }*/
                gpio_put(LED_RED, 0);
                gpio_put(LED_BLUE, 0);
                break;
            case JOGO_3:
             
                gpio_put(LED_BLUE, 1);
                memset(ssd, 0, ssd1306_buffer_length); 
                ssd1306_draw_string(ssd, 0, 0, "jogo 3 ");
                ssd1306_draw_string(ssd, 0, 20, "selecionado");
                render_on_display(ssd, &frame_area);  // ...
                sleep_ms(2000);
                memset(ssd, 0, ssd1306_buffer_length); 
                ssd1306_draw_string(ssd, 0, 0, "Aperte botao B");
                ssd1306_draw_string(ssd, 0, 20, "para voltar ");
                ssd1306_draw_string(ssd, 0, 30, "ao menu");
                render_on_display(ssd, &frame_area); 
                sleep_ms(2000);
                while(true){
                    if(gpio_get(BOTAO_B) == 0){
                        sleep_ms(100); // Debounce
                        estado_atual = MENU;
                        break;
                    }
                }
                gpio_put(LED_BLUE, 0);
                break;
            case JOGO_4:
                gpio_put(LED_BLUE, 1);
                gpio_put(LED_GREEN, 1);
                memset(ssd, 0, ssd1306_buffer_length); 
                ssd1306_draw_string(ssd, 0, 0, "jogo 4 ");
                ssd1306_draw_string(ssd, 0, 20, "selecionado");
                render_on_display(ssd, &frame_area);  // ...
                sleep_ms(2000);
                memset(ssd, 0, ssd1306_buffer_length); 
                ssd1306_draw_string(ssd, 0, 0, "Aperte botao B");
                ssd1306_draw_string(ssd, 0, 20, "para voltar ");
                ssd1306_draw_string(ssd, 0, 30, "ao menu");
                render_on_display(ssd, &frame_area); 
                sleep_ms(2000);
                while(true){
                    if(gpio_get(BOTAO_B) == 0){
                        sleep_ms(100); // Debounce
                        estado_atual = MENU;
                        break;
                    }
                }
                gpio_put(LED_BLUE, 0);
                gpio_put(LED_GREEN, 0);
                break;
        }
    sleep_ms(50);
        
    }
}
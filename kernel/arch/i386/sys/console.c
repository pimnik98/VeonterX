#include "kernel/drv/keyboard_buffer.h"
#include <kernel/kernel.h>
#include <kernel/mini_programs/mini_programs.h>
#include <stdint.h>
#include <stdio.h>  // Для printf и других стандартных функций
#include <string.h> // Для strcmp и других строковых функций
#include <stdlib.h> // Для strtol
#include <kernel/drv/ps2_mouse.h>

extern rgb_color_t fg_color;
extern rgb_color_t bg_color;
extern rgb_color_t main_color;

#define HISTORY_SIZE 10  // Максимальное количество команд в истории
#define SCREEN_WIDTH 80

// Структура для истории команд
typedef struct {
    char commands[HISTORY_SIZE][COMMAND_BUFFER_SIZE];
    int current_index;
    int total_commands;
    int history_index;  // Индекс для навигации по истории
} CommandHistory;

static CommandHistory history = { .current_index = 0, .total_commands = 0, .history_index = -1 };

static char command_buffer[COMMAND_BUFFER_SIZE];
static size_t command_length = 0;

void add_command_to_history(const char* command) {
    if (history.total_commands < HISTORY_SIZE) {
        strcpy(history.commands[history.total_commands++], command);
    } else {
        for (int i = 1; i < HISTORY_SIZE; i++) {
            strcpy(history.commands[i - 1], history.commands[i]);
        }
        strcpy(history.commands[HISTORY_SIZE - 1], command);
    }
    history.current_index = history.total_commands;
    history.history_index = -1;
}

const char* get_previous_command() {
    if (history.total_commands == 0 || history.history_index == 0) {
        return NULL;
    }
    if (history.history_index == -1) {
        history.history_index = history.total_commands - 1;
    } else {
        history.history_index--;
    }
    return history.commands[history.history_index];
}

const char* get_next_command() {
    if (history.history_index == -1 || history.history_index == history.total_commands - 1) {
        return NULL;
    }
    history.history_index++;
    return history.commands[history.history_index];
}

void console_initialize() {
    vbe_clear_screen(bg_color);
    printf(PROMPT_STRING);
}

const char* console_help_content_en[] = {
        "help - Displays help about all possible commands.",
        "clear - Clear the screen.",
        "mm_test - Runs a Memory test program.",
        "window - Displays a test window.",
        "calc - Calculator.",
        "logo - Displays logo.",
        "off - Shutdown PC.",
        "reboot - Rebooting PC.",
        "cpu - Shows CPU information.",
        "colors - Allows you to color your terminal in the colors you want.",
        "echo <text> - Displays the text you enter.",
        "beep <frequency> - Allows you to hear a beep with the specified frequency.",
        "vbe_test - Checking the operation of VBE graphics mode.",
        "disks - Show disk list available in system.",
        "dhv - Disk Hex View (first 1024 bytes).",
        "meminfo - Show memory info.",
        "mousetest - Show button flags and coordinates.",
        0
};
 
const char* console_help_content_ru[] = {
        "help - Показывает справку по всем доступным командам.",
        "clear - Очищает экран.",
        "mm_test - Тест памяти.",
        "window - Тестовое окно.",
        "calc - Калькулятор.",
        "logo - Показывает логотип.",
        "off - Выключает компьютер.",
        "reboot - Перезагрузка компьютера.",
        "cpu - Показывает информацию о процессоре.",
        "colors - Раскрашивает терминал в выбранный цвет.",
        "echo <текст> - Выводит введеный текст.",
        "beep <частота> - Включает встроенную пищалку на выбранной частоте.",
        "vbe_test - Проверка работы графического режима VBE.",
        "disks - Показывает список дисков доступных в системе.",
        "dhv - Показывает первые 1024 байт с диска.",
        "meminfo - Выводит информацию о памяти.",
        "mousetest - Показывает координаты и флаги кнопок мыши.",
        0
};

extern int LANGUAGE;

void show_help_menu() {
    int content_lines = 0;

    const char** real_content = LANGUAGE == 1 ? console_help_content_ru : console_help_content_en;
    char** backup_pointer = (char**)real_content;

    while(*backup_pointer) {
        content_lines++;
        backup_pointer++;
    }

    printf("\tHELP MENU\n\n");
    

    for (int i = 0; i < content_lines; ++i) {
        printf("\t\t%s\n", real_content[i]);
    }
}

void console_process_command(const char* command) {
    if (strncmp(command, "echo ", 5) == 0) {  // Проверяем команду echo
        printf("%s\n", command + 5);  // Выводим текст после echo
    } else if (strcmp(command, "clear") == 0) {
        vbe_clear_screen(bg_color);
    } else if (strncmp(command, "beep ", 5) == 0) {
        const char* freq_str = command + 5;
        char* endptr;
        unsigned int frequency = strtol(freq_str, &endptr, 10);
        if (*endptr == '\0') {
            beep(frequency, 100);  // Используем указанную частоту
        } else {
            printf("Invalid frequency: %s\n", freq_str);
        }
    } else if (strcmp(command, "help") == 0) {
        printf("\n");
        show_help_menu();
        printf("\n");
    } else if (strcmp(command, "mm_test") == 0) {
        printf("\n");
        mm_test();
        printf("\n");
    } else if (strcmp(command, "calc") == 0) {
        printf("\n");
        calculator();
        printf("\n");
    } else if (strcmp(command, "logo") == 0) {
        printf("\n");
        vbe_clear_screen(bg_color);
        printf("\n");
        logo();
        shell_text_color(fg_color);
        printf("\n");
    } else if (strcmp(command, "off") == 0) {
        printf("Shutting down...\n");
        sleep(1000);
        outw(0x604, 0x2000);
    } else if (strcmp(command, "window") == 0) {
        printf("Showing Window...\n");
        Window("My Program", 512, 10, 370, 250);
    } else if (strcmp(command, "reboot") == 0) {
        printf("Rebooting...\n");
        sleep(1000);
        outb(0x64, 0xFE);
    } else if (strcmp(command, "vbe_test") == 0) {
        vbe_clear_screen(bg_color);
        vbe_test();
        vbe_clear_screen(bg_color);
        // Сообщаем об окончании теста
        printf("VBE Test completed successfully!\n");

    } else if (strcmp(command, "colors") == 0) {
        colors_program();
    } else if (strcmp(command, "cpu") == 0) {
        main_color = fg_color;
        printf("\n");
        shell_text_color(RGB(150, 150, 150));
        detect_cpu();
        shell_text_color(main_color);
        printf("\n");
    } else if(strcmp(command, "disks") == 0) {
        disk_list();
    } else if(strcmp(command, "dhv") == 0) {
        dhv_program();
    } else if(strcmp(command, "meminfo") == 0) {
        meminfo_program();
    } else if(strcmp(command, "mousetest") == 0) {
        printf("\n  Press q to exit.\n");
        while(true) {
            uint16_t sym = keyboard_get_from_buffer();

            if(sym == 'q') {
                printf("\n\n");
                break;
            }
            printf("  Buttons: %x; X: %lu; Y: %lu; Wheel: %d   \r", mouse_get_buttons(), mouse_get_x(), mouse_get_y(), mouse_get_wheel());
        }
    } else {
        printf("Unknown command: ");
        printf("%s", command);
        shell_putchar('\n');
    }
    add_command_to_history(command); // Добавление команды в историю
    printf(PROMPT_STRING);
}

void console_input_loop() {
	uint16_t c;

	while (1) {
        enable_cursor(); // показываем курсор в текущем положении
        c = keyboard_get_char();
        disable_cursor(); // скрываем курсор перед изменением экрана

        if (c == '\n') {
            putchar('\n');
            if (command_length == 0) {
                printf(PROMPT_STRING);
            } else {
                command_buffer[command_length] = 0;
                console_process_command((char*)command_buffer);
                command_length = 0;
            }
        } else if (c == '\b') {
            if (command_length > 0) {
                if (vbe_getcolumn() > PROMPT_LENGTH) {
                    if(command_buffer[command_length - 1] & 0b10000000) {
                        command_length--;
                    }
                    command_length--;
                    putchar('\b'); // Удаление символа с экрана
                }
            }
        } else if (c == '\x1B') {
            // Обработка специального символа для стрелок
            c = keyboard_get_char();
            if (c == '[') {
                c = keyboard_get_char();
                if (c == 'A') { // Стрелка вверх
                    const char* previous_command = get_previous_command();
                    if (previous_command) {
                        // Очистка текущей строки
                        while (command_length > 0) {
                            shell_putchar('\b');
                            command_length--;
                        }
                        printf("%s", previous_command);
                        strcpy(command_buffer, previous_command);
                        command_length = strlen(previous_command);
                    }
                } else if (c == 'B') { // Стрелка вниз
                    const char* next_command = get_next_command();
                    if (next_command) {
                        while (command_length > 0) {
                            shell_putchar('\b');
                            command_length--;
                        }
                        printf("%s", next_command);
                        strcpy(command_buffer, next_command);
                        command_length = strlen(next_command);
                    } else {
                        while (command_length > 0) {
                            shell_putchar('\b');
                            command_length--;
                        }
                    }
                }
            }
        } else {
            if (command_length < COMMAND_BUFFER_SIZE - 1) {
                if(c <= 0xff) {
                    command_buffer[command_length++] = (char)c;
                } else {
                    command_buffer[command_length++] = (char)(c & 0xff);
                    command_buffer[command_length++] = (char)(c >> 8);
                }
                putchar(c); // Отображение символа на экране
            }
        }
        enable_cursor(); // показываем курсор после изменения экрана
    }
}

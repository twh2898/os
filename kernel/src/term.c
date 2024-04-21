#include "term.h"

#include "debug.h"
#include "drivers/keyboard.h"
#include "drivers/vga.h"
#include "libc/mem.h"
#include "libc/stdio.h"
#include "libc/string.h"

#define FATAL(MSG)                         \
    {                                      \
        vga_color(VGA_RED_ON_WHITE);       \
        printf(__FILE__ ":%u ", __LINE__); \
        vga_print(MSG);                    \
        term_last_ret = 1;                 \
        keybuff_i = 0;                     \
    }

typedef struct {
    const char * command;
    command_cb_t cb;
} command_t;

#define MAX_CHARS 4095
char keybuff[MAX_CHARS + 1] = {0};
size_t keybuff_i = 0;

#define MAX_COMMANDS 4096
command_t commands[MAX_COMMANDS] = {0};
size_t n_commands = 0;

int term_last_ret = 0;

static bool is_ws(char c);
static void exec_buff();
static char ** parse_args(const char * line, size_t * out_len);

static void key_cb(uint8_t code, char c, keyboard_event_t event, keyboard_mod_t mod) {
    if (event != KEY_EVENT_RELEASE && c) {
        if (code == KEY_ENTER) {
            vga_putc(c);
            exec_buff();
            vga_color(VGA_WHITE_ON_BLACK);
            vga_print("> ");
            return;
        }

        if (code == KEY_BACKSPACE) {
            if (keybuff_i > 0) {
                keybuff_i--;
                vga_putc(c);
            }
            return;
        }

        if (keybuff_i > MAX_CHARS) {
            vga_color(VGA_RED_ON_WHITE);
            vga_print("TERMINAL KEY BUFFER OVERFLOW!\n");
            return;
        }

        keybuff[keybuff_i++] = c;
        vga_putc(c);
    }
}

void term_init() {
    keyboard_set_cb(&key_cb);
    vga_print("> ");
}

void term_command_add(const char * command, command_cb_t cb) {
    if (n_commands > MAX_COMMANDS) {
        vga_color(VGA_RED_ON_WHITE);
        vga_print("TERMINAL COMMAND REGISTER OVERFLOW!\n");
        return;
    }

    commands[n_commands].command = command;
    commands[n_commands++].cb = cb;
}

static void exec_buff() {
    if (keybuff_i == 0) {
        term_last_ret = 0;
        if (debug)
            puts("Buffer empty\n");
        return;
    }

    if (keybuff_i > MAX_CHARS) {
        keybuff_i = MAX_CHARS + 1;
    }

    // terminate full command / terminal line
    keybuff[keybuff_i] = 0;

    // Skip any leading whitespace
    char * line = keybuff;
    size_t line_len = keybuff_i;
    while (line_len > 0 && is_ws(*line)) {
        line++;
        line_len--;
    }

    // Trim trailing whitespace
    while (line_len > 1 && is_ws(line[line_len - 1])) {
        line_len--;
    }

    if (debug)
        printf("Trimmed line length %u starting at +%u\n",
               line_len,
               (line - keybuff));

    // Terminate trimmed line
    line[line_len] = 0;

    // Clear key buffer
    keybuff_i = 0;

    // Find the length of the first non whitespace word
    int first_len = 0;
    while (first_len < line_len && !is_ws(line[first_len])) first_len++;

    // Check against all commands
    bool found = false;
    for (size_t i = 0; i < n_commands; i++) {
        size_t command_len = strlen(commands[i].command);

        // Check length of command vs first word
        if (first_len < command_len) {
            if (debug)
                printf("Command too short %u < %u\n", first_len, command_len);
            continue;
        }

        if (first_len > command_len) {
            if (debug)
                printf("Command too long %u > %u\n", first_len, command_len);
            continue;
        }

        // Check command string
        int match = memcmp(line, commands[i].command, command_len);
        if (match != 0) {
            if (debug)
                printf("Command does not match %s\n", commands[i].command);
            continue;
        }

        // Command is a match, parse arguments
        found = true;
        size_t argc;
        char ** argv = parse_args(line, &argc);
        if (!argv) {
            FATAL("SYNTAX ERROR!\n");
            term_last_ret = 1;
            return;
        }

        // Execute the command with parsed args
        term_last_ret = commands[i].cb(argc, argv);

        // Free parsed args
        for (size_t i = 0; i < argc; i++) {
            free(argv[i]);
        }
        free(argv);

        break;
    }

    // No match was found
    if (!found) {
        printf("Unknown command '%s'\n", line);
        term_last_ret = 1;
    }
}

static bool is_ws(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\b';
}

static int take_quote(const char * str) {
    if (*str != '"')
        return -1;

    size_t len = strlen(str);
    for (size_t i = 1; i < len; i++) {
        if (str[i] == '"' && str[i - 1] != '\\')
            return i;
    }

    return -1;
}

static int count_args(const char * line) {
    int n = 0;
    while (*line) {
        // Skip whitespace
        while (*line && is_ws(*line)) line++;

        // end of string
        if (*line == 0)
            break;

        // Handle quote
        if (*line == '"') {
            int next = take_quote(line);
            if (next < 1)
                return -1;
            n++;
            line += 2 + next;
            continue;
        }

        // handle word
        while (*line && !is_ws(*line)) line++;
        n++;
    }
    return n;
}

static char ** parse_args(const char * line, size_t * out_len) {
    int len = count_args(line);
    if (len < 1) {
        return 0;
    }

    *out_len = len;
    char ** args = malloc(sizeof(char *) * len);
    size_t arg_i = 0;

    while (*line) {
        if (arg_i > len) {
            FATAL("SYNTAX ERROR!\n");
            if (debug) {
                printf("expected %u args but have %u\n", len, arg_i);
                printf("current parse char is 0x%02x\n", *line);
            }
            return 0;
        }
        // Skip whitespace
        while (*line && is_ws(*line)) line++;

        // end of string
        if (*line == 0)
            break;

        // Handle quote
        if (*line == '"') {
            int next = take_quote(line);
            if (next < 1)
                return 0;

            line++;

            args[arg_i] = malloc(sizeof(char) * next);
            memcpy(args[arg_i], line, next - 1);
            args[arg_i][next - 1] = 0;
            arg_i++;

            line += next;
            continue;
        }

        const char * start = line;
        // handle word
        while (*line && !is_ws(*line)) line++;

        size_t word_len = line - start;
        args[arg_i] = malloc(sizeof(char) * word_len + 1);
        memcpy(args[arg_i], start, word_len);
        args[arg_i][word_len] = 0;
        arg_i++;
    }

    if (arg_i < len) {
        FATAL("SYNTAX ERROR!\n");
        if (debug) {
            printf("expected %u args but have %u\n", len, arg_i);
            printf("current parse char is 0x%02x\n", *line);
        }
        return 0;
    }

    return args;
}

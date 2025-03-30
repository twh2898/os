#include "drivers/keyboard.h"
#include "ebus.h"
#include "libc/datastruct/circular_buffer.h"
#include "libc/memory.h"
#include "libc/proc.h"
#include "libc/signal.h"
#include "libc/stdio.h"
#include "libc/string.h"

#define ERROR(MSG)                                \
    {                                             \
        printf(__FILE__ ":%u %s", __LINE__, MSG); \
    }

#define FATAL(MSG)         \
    {                      \
        ERROR(MSG)         \
        term_last_ret = 1; \
    }

typedef int (*command_cb_t)(size_t argc, char ** argv);

typedef struct {
    const char * command;
    command_cb_t cb;
} command_t;

void term_init();
void term_update();
void term_run();
bool term_command_add(const char * command, command_cb_t cb);
void set_command_lookup(command_cb_t command);

#define MAX_CHARS 4095
static cb_t            keybuff;
static char            command_buff[MAX_CHARS + 1];
static volatile size_t command_ready = 0;

#define MAX_COMMANDS 4096
static command_t commands[MAX_COMMANDS] = {0};
static size_t    n_commands             = 0;

int term_last_ret = 0;

void           term_run();
static int     help_cmd(size_t argc, char ** argv);
static size_t  buff_read(const cb_t * cb, uint8_t * data, size_t count);
static size_t  buff_remove(cb_t * cb, size_t count);
static bool    is_ws(char c);
static void    exec_buff();
static char ** parse_args(const char * line, size_t * out_len);

static command_cb_t command_lookup;

int __start(size_t argc, char ** argv) {
    command_lookup = 0;

    term_command_add("help", help_cmd);

    if (cb_create(&keybuff, MAX_CHARS, 1)) {
        return 1;
    }
    command_ready = false;

    term_run();

    return 0;
}

static void dump_buff() {
    for (size_t i = 0; i < cb_len(&keybuff); i++) {
        printf("%X ", cb_peek(&keybuff, i));
    }
}

static void key_cb(uint8_t code, char c, keyboard_event_t event, keyboard_mod_t mod) {
    if ((event == KEY_EVENT_PRESS || event == KEY_EVENT_REPEAT) && c) {
        if (cb_len(&keybuff) >= MAX_CHARS) {
            ERROR("key buffer overflow");
            printf("(%u out of %u)", cb_len(&keybuff), MAX_CHARS);
            PANIC("key buffer overflow");
            return;
        }

        if (code == KEY_BACKSPACE) {
            if (cb_len(&keybuff) > 0) {
                putc(c);
                cb_rpop(&keybuff, 0);
            }
            return;
        }

        if (cb_push(&keybuff, &c)) {
            ERROR("key buffer write error");
            return;
        }

        // kprintf("Circbuff char %x at len %d / %d\n", c, circbuff_len(&keybuff), circbuff_buff_size(&keybuff));
        // dump_buff();

        if (code == KEY_ENTER) {
            command_ready++;
        }

        putc(c);
    }
}

static void key_event_handler(const ebus_event_t * event) {
    key_cb(event->key.keycode, event->key.c, event->key.event, event->key.mods);
}

static int help_cmd(size_t argc, char ** argv) {
    for (size_t i = 0; i < n_commands; i++) {
        puts(commands[i].command);
        putc('\n');
    }
    return 0;
}

void term_update() {
    if (!command_ready) {
        return;
    }

    command_ready--;

    size_t cmd_len  = 0;
    bool   found_nl = false;
    // puts("Ready\n");
    // dump_buff();
    for (size_t i = 0; i < cb_len(&keybuff); i++) {
        char c = *(char *)cb_peek(&keybuff, i);
        if (c == '\n') {
            cmd_len  = i;
            found_nl = true;
            break;
        }
    }

    if (!found_nl) {
        ERROR("key buffer without newline");
        return;
    }

    if (cmd_len > 0) {
        size_t res;

        // +1 to include newline that is set to 0 later
        res = buff_read(&keybuff, command_buff, cmd_len);
        if (res != cmd_len) {
            ERROR("key buffer failed to read");
            return;
        }
        // change newline to 0
        command_buff[cmd_len] = 0;

        res = buff_remove(&keybuff, cmd_len);
        if (res != cmd_len) {
            ERROR("key buffer failed to remove");
            return;
        }

        exec_buff();
    }

    // pop newline
    cb_pop(&keybuff, NULL);

    puts("$ ");
}

void term_run() {
    puts("$ ");

    for (;;) {
        ebus_event_t event;
        int          ev = pull_event(EBUS_EVENT_KEY, &event);
        if (ev == EBUS_EVENT_KEY) {
            key_cb(event.key.keycode, event.key.c, event.key.event, event.key.mods);
        }
        term_update();
    }
}

bool term_command_add(const char * command, command_cb_t cb) {
    if (!command || !cb) {
        return false;
    }

    if (n_commands > MAX_COMMANDS) {
        ERROR("TERMINAL COMMAND REGISTER OVERFLOW!\n");
        return false;
    }

    commands[n_commands].command = command;
    commands[n_commands++].cb    = cb;
    return true;
}

void set_command_lookup(command_cb_t lookup) {
    command_lookup = lookup;
}

static size_t buff_read(const cb_t * cb, uint8_t * data, size_t count) {
    if (!cb || !data || !count) {
        return 0;
    }

    if (count > cb_len(cb)) {
        count = cb_len(cb);
    }

    for (size_t i = 0; i < count; i++) {
        char * c = cb_peek(cb, i);
        data[i]  = *c;
    }

    return count;
}

static size_t buff_remove(cb_t * cb, size_t count) {
    if (!cb || !count) {
        return 0;
    }

    if (count > cb_len(cb)) {
        count = cb_len(cb);
    }

    for (size_t i = 0; i < count; i++) {
        cb_pop(cb, 0);
    }

    return count;
}

static void exec_buff() {
    // Skip any leading whitespace
    char * line     = command_buff;
    size_t line_len = kstrlen(command_buff);
    while (line_len > 0 && is_ws(*line)) {
        line++;
        line_len--;
    }

    // Trim trailing whitespace
    while (line_len > 1 && is_ws(line[line_len - 1])) {
        line_len--;
    }

    // Terminate trimmed line
    line[line_len] = 0;

    // Find the length of the first non whitespace word
    int first_len = 0;
    while (first_len < line_len && !is_ws(line[first_len])) {
        first_len++;
    }

    // Prepare command and args
    size_t  argc;
    char ** argv = parse_args(line, &argc);
    if (!argc || !argv) {
        FATAL("SYNTAX ERROR!\n");
        term_last_ret = 1;
        return;
    }

    bool         found   = false;
    command_cb_t command = 0;

    // Check against all commands
    for (size_t i = 0; i < n_commands && !found; i++) {
        size_t command_len = kstrlen(commands[i].command);

        // Check length of command vs first word
        if (first_len < command_len) {
            continue;
        }

        if (first_len > command_len) {
            continue;
        }

        // Check command string
        int match = kmemcmp(argv[0], commands[i].command, command_len);
        if (match != 0) {
            continue;
        }

        // Command is a match, parse arguments
        found   = true;
        command = commands[i].cb;
    }

    if (found) {
        // Execute the command with parsed args
        term_last_ret = command(argc, argv);
    }

    // Try command lookup
    else if (command_lookup) {
        command_ready++;
        term_last_ret = command_lookup(argc, argv);
        command_ready--;
    }

    // No match was found
    else {
        printf("Unknown command '%s'\n", line);
        term_last_ret = 1;
    }

    // Free parsed args
    for (size_t i = 0; i < argc; i++) {
        pfree(argv[i]);
    }
    pfree(argv);
}

static bool is_ws(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\b';
}

static int take_quote(const char * str) {
    if (!str || *str != '"') {
        return -1;
    }

    size_t len = kstrlen(str);
    for (size_t i = 1; i < len; i++) {
        if (str[i] == '"' && str[i - 1] != '\\') {
            return i;
        }
    }

    return -1;
}

static int count_args(const char * line) {
    if (!line) {
        return -1;
    }

    int n = 0;
    while (*line) {
        // Skip whitespace
        while (*line && is_ws(*line)) {
            line++;
        }

        // end of string
        if (*line == 0) {
            break;
        }

        // Handle quote
        if (*line == '"') {
            int next = take_quote(line);
            if (next < 1) {
                return -1;
            }
            n++;
            line += 2 + next;
            continue;
        }

        // handle word
        while (*line && !is_ws(*line)) {
            line++;
        }
        n++;
    }
    return n;
}

static char ** parse_args(const char * line, size_t * out_len) {
    if (!line || !out_len) {
        return 0;
    }

    int len = count_args(line);
    if (len < 1) {
        return 0;
    }

    *out_len      = len;
    char ** args  = pmalloc(sizeof(char *) * len);
    size_t  arg_i = 0;

    while (*line) {
        if (arg_i > len) {
            FATAL("SYNTAX ERROR!\n");
            return 0;
        }
        // Skip whitespace
        while (*line && is_ws(*line)) {
            line++;
        }

        // end of string
        if (*line == 0) {
            break;
        }

        // Handle quote
        if (*line == '"') {
            int next = take_quote(line);
            if (next < 1) {
                return 0;
            }

            line++;

            args[arg_i] = pmalloc(sizeof(char) * next);
            kmemcpy(args[arg_i], line, next - 1);
            args[arg_i][next - 1] = 0;
            arg_i++;

            line += next;
            continue;
        }

        const char * start = line;
        // handle word
        while (*line && !is_ws(*line)) {
            line++;
        }

        size_t word_len = line - start;
        void * ptr      = pmalloc(sizeof(char) * word_len + 1);
        args[arg_i]     = ptr;
        kmemcpy(args[arg_i], start, word_len);
        args[arg_i][word_len] = 0;
        arg_i++;
    }

    if (arg_i < len) {
        FATAL("SYNTAX ERROR!\n");
        return 0;
    }

    return args;
}

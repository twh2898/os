#include "drivers/keyboard.h"

#include "cpu/isr.h"
#include "cpu/ports.h"
#include "libc/proc.h"
#include "libc/stdio.h"

static uint8_t last_code = 0;
static bool    lctrl     = false;
static bool    rctrl     = false;
static bool    lalt      = false;
static bool    ralt      = false;
static bool    lshift    = false;
static bool    rshift    = false;
static bool    lsuper    = false;
static bool    rsuper    = false;

static void keyboard_callback(registers_t * regs) {
    /* The PIC leaves us the scancode in port 0x60 */
    uint8_t          scancode  = port_byte_in(0x60);
    uint8_t          keycode   = scancode;
    keyboard_event_t key_event = KEY_EVENT_PRESS;
    bool             press     = keycode < 0x80;

    if (press) {
        if (keycode == last_code) {
            key_event = KEY_EVENT_REPEAT;
        }
        else {
            if (keycode == KEY_LSHIFT) {
                lshift = true;
            }
            if (keycode == KEY_RSHIFT) {
                lshift = true;
            }
        }
        last_code = keycode;
    }

    else {
        keycode -= 0x80;
        key_event = KEY_EVENT_RELEASE;
        last_code = 0;

        if (keycode == KEY_LSHIFT) {
            lshift = false;
        }
        if (keycode == KEY_RSHIFT) {
            lshift = false;
        }
    }

    char c = keyboard_char(keycode, lshift || rshift);

    keyboard_mod_t mods = 0;
    if (lctrl || rctrl) {
        mods |= KEY_MOD_CTRL;
    }
    if (lalt || ralt) {
        mods |= KEY_MOD_ALT;
    }
    if (lshift || rshift) {
        mods |= KEY_MOD_SHIFT;
    }
    if (lsuper || rsuper) {
        mods |= KEY_MOD_SUPER;
    }

    ebus_event_t event;
    event.event_id     = EBUS_EVENT_KEY;
    event.key.event    = key_event;
    event.key.mods     = mods;
    event.key.c        = c;
    event.key.keycode  = keycode;
    event.key.scancode = scancode;
    queue_event(&event);
}

void init_keyboard() {
    register_interrupt_handler(IRQ1, keyboard_callback);
}

char keyMap[0xFF]      = {0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '+', '\b', '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, 0, 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
char shiftKeyMap[0xFF] = {0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, 0, 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

char keyboard_char(uint8_t code, bool shift) {
    code = code & 0x7F;
    if (shift) {
        return shiftKeyMap[code];
    }
    return keyMap[code];
}

#include "drivers/keyboard.h"

#include "cpu/isr.h"
#include "cpu/ports.h"
#include "libc/stdio.h"

void print_letter(uint8_t scancode);

static keyboard_cb_t _cb = 0;
static uint8_t last_code = 0;
static bool lctrl = false;
static bool rctrl = false;
static bool lalt = false;
static bool ralt = false;
static bool lshift = false;
static bool rshift = false;
static bool lsuper = false;
static bool rsuper = false;

void keyboard_set_cb(keyboard_cb_t cb) {
    _cb = cb;
}

static void keyboard_callback(registers_t regs) {
    /* The PIC leaves us the scancode in port 0x60 */
    uint8_t scancode = port_byte_in(0x60);
    uint8_t keycode = scancode;
    if (_cb) {
        keyboard_event_t event = KEY_EVENT_PRESS;
        bool press = keycode < 0x80;

        if (press) {
            if (keycode == last_code)
                event = KEY_EVENT_REPEAT;
            else {
                if (keycode == KEY_LSHIFT)
                    lshift = true;
                if (keycode == KEY_RSHIFT)
                    lshift = true;
            }
            last_code = keycode;
        }

        else {
            keycode -= 0x80;
            event = KEY_EVENT_RELEASE;
            last_code = 0;

            if (keycode == KEY_LSHIFT)
                lshift = false;
            if (keycode == KEY_RSHIFT)
                lshift = false;
        }

        char c = keyboard_char(keycode, lshift || rshift);

        keyboard_mod_t mods = 0;
        if (lctrl || rctrl)
            mods |= KEY_MOD_CTRL;
        if (lalt || ralt)
            mods |= KEY_MOD_ALT;
        if (lshift || rshift)
            mods |= KEY_MOD_SHIFT;
        if (lsuper || rsuper)
            mods |= KEY_MOD_SUPER;

        _cb(keycode, c, event, mods);
    }
    else {
        kprintf("Keyboard scancode: %u, ", scancode);
        print_letter(scancode);
        kprintf("\n");
    }
}

void init_keyboard() {
    register_interrupt_handler(IRQ1, keyboard_callback);
}

char keyMap[0xFF] = {0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '+', '\b', '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, 0, 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
char shiftKeyMap[0xFF] = {0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, 0, 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

char keyboard_char(uint8_t code, bool shift) {
    code = code & 0x7F;
    if (shift) {
        return shiftKeyMap[code];
    }
    return keyMap[code];
}

void print_letter(uint8_t scancode) {
    switch (scancode) {
        default:
            /* 'keuyp' event corresponds to the 'keydown' + 0x80
             * it may still be a scancode we haven't implemented yet, or
             * maybe a control/escape sequence */
            if (scancode <= 0x7f) {
                kprintf("Unknown key down");
            }
            else if (scancode <= 0x39 + 0x80) {
                kprintf("key up ");
                print_letter(scancode - 0x80);
            }
            else
                kprintf("Unknown key up");
            break;
        case 0x0:
            break;
        case 0x1:
            kprintf("ESC");
            break;
        case 0x2:
            kprintf("1");
            break;
        case 0x3:
            kprintf("2");
            break;
        case 0x4:
            kprintf("3");
            break;
        case 0x5:
            kprintf("4");
            break;
        case 0x6:
            kprintf("5");
            break;
        case 0x7:
            kprintf("6");
            break;
        case 0x8:
            kprintf("7");
            break;
        case 0x9:
            kprintf("8");
            break;
        case 0x0A:
            kprintf("9");
            break;
        case 0x0B:
            kprintf("0");
            break;
        case 0x0C:
            kprintf("-");
            break;
        case 0x0D:
            kprintf("+");
            break;
        case 0x0E:
            kprintf("Backspace");
            break;
        case 0x0F:
            kprintf("Tab");
            break;
        case 0x10:
            kprintf("Q");
            break;
        case 0x11:
            kprintf("W");
            break;
        case 0x12:
            kprintf("E");
            break;
        case 0x13:
            kprintf("R");
            break;
        case 0x14:
            kprintf("T");
            break;
        case 0x15:
            kprintf("Y");
            break;
        case 0x16:
            kprintf("U");
            break;
        case 0x17:
            kprintf("I");
            break;
        case 0x18:
            kprintf("O");
            break;
        case 0x19:
            kprintf("P");
            break;
        case 0x1A:
            kprintf("[");
            break;
        case 0x1B:
            kprintf("]");
            break;
        case 0x1C:
            kprintf("ENTER");
            break;
        case 0x1D:
            kprintf("LCtrl");
            break;
        case 0x1E:
            kprintf("A");
            break;
        case 0x1F:
            kprintf("S");
            break;
        case 0x20:
            kprintf("D");
            break;
        case 0x21:
            kprintf("F");
            break;
        case 0x22:
            kprintf("G");
            break;
        case 0x23:
            kprintf("H");
            break;
        case 0x24:
            kprintf("J");
            break;
        case 0x25:
            kprintf("K");
            break;
        case 0x26:
            kprintf("L");
            break;
        case 0x27:
            kprintf(";");
            break;
        case 0x28:
            kprintf("'");
            break;
        case 0x29:
            kprintf("`");
            break;
        case 0x2A:
            kprintf("LShift");
            break;
        case 0x2B:
            kprintf("\\");
            break;
        case 0x2C:
            kprintf("Z");
            break;
        case 0x2D:
            kprintf("X");
            break;
        case 0x2E:
            kprintf("C");
            break;
        case 0x2F:
            kprintf("V");
            break;
        case 0x30:
            kprintf("B");
            break;
        case 0x31:
            kprintf("N");
            break;
        case 0x32:
            kprintf("M");
            break;
        case 0x33:
            kprintf(",");
            break;
        case 0x34:
            kprintf(".");
            break;
        case 0x35:
            kprintf("/");
            break;
        case 0x36:
            kprintf("Rshift");
            break;
        case 0x37:
            kprintf("Keypad *");
            break;
        case 0x38:
            kprintf("LAlt");
            break;
        case 0x39:
            kprintf("Spc");
            break;
    }
}

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
        printf("Keyboard scancode: %u, ", scancode);
        print_letter(scancode);
        printf("\n");
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
                printf("Unknown key down");
            }
            else if (scancode <= 0x39 + 0x80) {
                printf("key up ");
                print_letter(scancode - 0x80);
            }
            else
                printf("Unknown key up");
            break;
        case 0x0:
            break;
        case 0x1:
            printf("ESC");
            break;
        case 0x2:
            printf("1");
            break;
        case 0x3:
            printf("2");
            break;
        case 0x4:
            printf("3");
            break;
        case 0x5:
            printf("4");
            break;
        case 0x6:
            printf("5");
            break;
        case 0x7:
            printf("6");
            break;
        case 0x8:
            printf("7");
            break;
        case 0x9:
            printf("8");
            break;
        case 0x0A:
            printf("9");
            break;
        case 0x0B:
            printf("0");
            break;
        case 0x0C:
            printf("-");
            break;
        case 0x0D:
            printf("+");
            break;
        case 0x0E:
            printf("Backspace");
            break;
        case 0x0F:
            printf("Tab");
            break;
        case 0x10:
            printf("Q");
            break;
        case 0x11:
            printf("W");
            break;
        case 0x12:
            printf("E");
            break;
        case 0x13:
            printf("R");
            break;
        case 0x14:
            printf("T");
            break;
        case 0x15:
            printf("Y");
            break;
        case 0x16:
            printf("U");
            break;
        case 0x17:
            printf("I");
            break;
        case 0x18:
            printf("O");
            break;
        case 0x19:
            printf("P");
            break;
        case 0x1A:
            printf("[");
            break;
        case 0x1B:
            printf("]");
            break;
        case 0x1C:
            printf("ENTER");
            break;
        case 0x1D:
            printf("LCtrl");
            break;
        case 0x1E:
            printf("A");
            break;
        case 0x1F:
            printf("S");
            break;
        case 0x20:
            printf("D");
            break;
        case 0x21:
            printf("F");
            break;
        case 0x22:
            printf("G");
            break;
        case 0x23:
            printf("H");
            break;
        case 0x24:
            printf("J");
            break;
        case 0x25:
            printf("K");
            break;
        case 0x26:
            printf("L");
            break;
        case 0x27:
            printf(";");
            break;
        case 0x28:
            printf("'");
            break;
        case 0x29:
            printf("`");
            break;
        case 0x2A:
            printf("LShift");
            break;
        case 0x2B:
            printf("\\");
            break;
        case 0x2C:
            printf("Z");
            break;
        case 0x2D:
            printf("X");
            break;
        case 0x2E:
            printf("C");
            break;
        case 0x2F:
            printf("V");
            break;
        case 0x30:
            printf("B");
            break;
        case 0x31:
            printf("N");
            break;
        case 0x32:
            printf("M");
            break;
        case 0x33:
            printf(",");
            break;
        case 0x34:
            printf(".");
            break;
        case 0x35:
            printf("/");
            break;
        case 0x36:
            printf("Rshift");
            break;
        case 0x37:
            printf("Keypad *");
            break;
        case 0x38:
            printf("LAlt");
            break;
        case 0x39:
            printf("Spc");
            break;
    }
}

# WIP - System Calls

System calls are performed through interrupt 48 (`int 0x30`). The interrupt
takes a `uint16_t` id and some number of arguments.

## Interrupt

| ID   | Family          |
| ---- | --------------- |
| 0x01 | I/O             |
| 0x02 | Memory          |
| 0x03 | Process Control |
| 0x10 | Tmp Std I/O     |

An interrupt id is an 8 bit family + an 8 bit id.

| Family          | ID     | Name                                               |
| --------------- | ------ | -------------------------------------------------- |
| I/O             | 0x0100 | open                                               |
|                 | 0x0101 | close                                              |
|                 | 0x0102 | read                                               |
|                 | 0x0103 | write                                              |
| Memory          | 0x0200 | `void * malloc(size_t size)`                       |
|                 | 0x0201 | `void * realloc(void * ptr, size_t size)`          |
|                 | 0x0202 | `void free(void * ptr)`                            |
| Process Control | 0x0300 | `void exit(uint8_t code)`                          |
|                 | 0x0301 | `void exit(uint8_t code, const char * msg)`        |
| Tmp Std I/O     | 0x1000 | `size_t putc(char c)`                              |
|                 | 0x1001 | `size_t puts(const char * str)`                    |
|                 | 0x1002 | `size_t vprintf(const char * fmt, va_list params)` |

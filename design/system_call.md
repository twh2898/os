# WIP - System Calls

System calls are performed through interrupt 48 (`int 0x30`). The interrupt
takes a `uint16_t` id and some number of arguments.

## Interrupt

| ID   | Family |
| ---- | ------ |
| 0x01 | I/O    |
| 0x02 | Memory |

An interrupt id is an 8 bit family + an 8 bit id.

| Family | ID     | Name                                 | Description |
| ------ | ------ | ------------------------------------ | ----------- |
| I/O    | 0x0100 | open                                 |             |
|        | 0x0101 | close                                |             |
|        | 0x0102 | read                                 |             |
|        | 0x0103 | write                                |             |
| Memory | 0x0200 | `malloc(size_t size)`                |             |
|        | 0x0201 | `calloc(size_t size, uint8_t value)` |             |
|        | 0x0202 | `realloc(void * ptr, size_t size)`   |             |
|        | 0x0203 | `free(void * ptr)`                   |             |

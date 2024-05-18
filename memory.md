# Memory (malloc)

- Pages are 4 kb each
- Malloc area is the largest continuous area above 0x90000
- 62 page addresses per page block

## Page Address (PA)

|    bits | name              |
| ------: | ----------------- |
|  0 - 31 | page index        |
| 32 - 63 | size in pages     |
|      64 | 1 = free 0 = used |

## Page Block

| start | size | name                             |
| ----: | ---: | -------------------------------- |
|     0 |    4 | next address block (0 for end    |
|     4 |    4 | prev address block (0 for start) |
|     8 |    4 | page address (repeat 62 times)   |

## Process

### Malloc

1. Find next free area of at least size
   1. If multiple free areas add up to size combine them
2. If none found
   1. Add new area to the end
3. If area size - requested > page size
   1. split block
4. Return

### Realloc

### Free

1. Find block for pointer
2. Set flag to free

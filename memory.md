# Memory (malloc)

## Region Header

| start | size | name                                    |
| ----: | ---: | --------------------------------------- |
|     0 |    4 | ptr to next region header (0 for end)   |
|     4 |    4 | ptr to prev region header (0 for start) |
|     8 |    4 | size of region                          |

## Process

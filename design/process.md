# Process

## Creating a Process

Each process needs a page directory, heap and stack.

1. Create a page dir
2. Load proc cr3 into temp page
3. Clear dir
4. Map first page to kernel page
5. Setup Stack
   1. Set proc field for stack address
   2. Add page for stack
6. Setup Heap
   1. Set heap start
   2. Add page for heap if needed
7. Free from temp page

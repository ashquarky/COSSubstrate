extern void *(* MEMAllocFromExpHeapEx)(int heap, unsigned int size, int align);
extern int (* MEMCreateExpHeapEx)(void* address, unsigned int size, unsigned short flags);
extern void *(* MEMDestroyExpHeap)(int heap);
extern void (* MEMFreeToExpHeap)(int heap, void* ptr);

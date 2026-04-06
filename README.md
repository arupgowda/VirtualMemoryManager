# MemoryManager

This project implements a virtual memory manager in C++ that takes a large contiguous block of memory and manages allocations.
It supports allocation, freeing, defragmentation, and coalescing of free blocks. It uses the First Fit approach where it
tries to find the first block that is of requested size or larger. This makes allocation fast but with the draw back that it fragments
the front. 

---

## Files

| File | Description |
|---|---|
| `MemoryManager.h` | Class declaration and data structures |
| `MemoryManager.cpp` | Implementation of all memory management logic |
| `main.cpp` | Demo program exercising all features |
| `Readme.md` | Project readme file |

---

## Data Structures

### `MemoryBlock`
Tracks a region of the buffer internally.
```cpp
struct MemoryBlock {
    int offset;    // offset from start of buffer
    size_t size;   // size of the block in bytes
    bool isFree;   // whether the block is free
};
```

### `MemoryHandle`
Abstracts raw memory away from the caller. Users never receive raw pointers — only integer handle IDs.
```cpp
struct MemoryHandle {
    std::list<MemoryBlock>::iterator blockIt; // points to the block in the list
    bool isValid;                             // whether the handle is in use
};
```

---

## API

### `MemoryManager(int num_bytes)`
Allocates a contiguous buffer of `num_bytes` bytes and initializes it as a single free block.

### `int Alloc(int size)`
Walks the free block list and allocates the first block large enough to fit `size` bytes. Returns an integer handle ID, or `INVALID_HANDLE` (-1) if no space is available.

### `void Free(int h)`
Frees the memory associated with handle `h` and attempts to coalesce adjacent free blocks.

### `char* DereferenceHandle(int h)`
Returns a raw pointer to the start of the allocated memory for handle `h`. This is the only way to access the underlying buffer.

### `void Defragment()`
Compacts all allocated blocks to the left of the buffer, merges all free space into a single block on the right, and updates all handle iterators accordingly.

### `void CoalesceBlocks(iterator it)`
When a block is freed, checks if adjacent blocks are also free and merges them to reduce fragmentation.

### `void PrintState()`
Prints the current state of all memory blocks (offset, size, free/used).

---

## Building

```bash
g++ -std=c++20 main.cpp MemoryManager.cpp -o mem 
```

---

## Test Case
Example test case: 
   - initialize a buffer of 5 chars: ----- 
   - allocate 5 blocks of 1 char:    XXXXX
   - free the 2nd and 4th:           X-X-X 

---

## Benefits

- Simple memory management.
- Raw memory is never shared with caller allowing for de-fragmentation.
- Coalescing helps in de-fragmentation as an additional step.

---

## Limitations

- The time complexity on this first fir alocator is O(n), making it slower than other algorithms.
- Allocation algorithm causes high fragmentation of memory.
- Single-threaded only — so not thread safe. Will need locking to make thread safe.
- Buffer size is fixed at construction time and cannot be resized.
- We do not check if all memory allocated if freed by caller before calling destructor. 
  This can result in data loss and unexprected behaviour. 
- Error handling is rudimentary - we throw some exceptions but no handling.
- Defragmentaion needs to be called by user.
- No tracking of how much space is left, caller does not know if space is available but fragmented.

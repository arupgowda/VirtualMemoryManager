#pragma once

#include <iostream>
#include <cstdlib>
#include <stack>
#include <list>
#include <vector>

static const int INVALID_HANDLE = -1;

// internal defined entity to track memory blocks
struct MemoryBlock {
    int offset;
    size_t size;
    bool isFree;
};

/* internal entity to abstract away raw memory from caller,
 * user will only get a index to a vector of these handles.
 * The handles are then de-referenced to get access to underlying
 * allocated memory. Handles are re-used after being freed.
 */
struct MemoryHandle {
    std::list<MemoryBlock>::iterator blockIt;
    bool isValid = false;
};

class MemoryManager {
  public:
    MemoryManager(int num_bytes);
    int Alloc(int size);
    char* DereferenceHandle(int h);
    void Free(int h);
    void Defragment();
    void PrintState() const;
    ~MemoryManager();

  private:
    char* buffer; // underlying memory
    size_t max_size; // size of underlying memory
    std::vector<MemoryHandle> memory_handles; // a vector of all inuse and free memory handles
    std::list<MemoryBlock> memory_blocks; // a linked list of memory blocks tracking the underlying memory
    std::stack<int> free_handles; // tracks re-use of freed handles
    
    int GetHandle();
    void ValidateHandle(int h) const;
    void CoalesceBlocks(std::list<MemoryBlock>::iterator it);
};

#include "MemoryManager.h"
#include <cstring>
#include <iostream>
#include <stdexcept>

/* Constructer follows RAII philosophy of resource
 * allocation and release.
 */
MemoryManager::MemoryManager(int num_bytes) {
    buffer = (char*) std::malloc(num_bytes * sizeof(char));
    max_size = num_bytes;
    // create 1 entry encompassing the whole block in block linked list
    struct MemoryBlock free_block = {0, max_size, true};
    memory_blocks.push_back(free_block);
}

/*
 * Create and reuse handles to prevent
 * un-bounded growth in handles
 */
int MemoryManager::GetHandle() {
    if (!free_handles.empty()) {
        int h = free_handles.top();
        free_handles.pop();
        return h;
    }
    else {
        memory_handles.push_back({});
        return memory_handles.size() - 1;
    }
}

/*
 * Memory allocator uses First Fit O(N) runtime algorithm
 */
int MemoryManager::Alloc(int size) {
    // Walk through the blocks to see if we have size block available
    std::list<MemoryBlock>::iterator it;
    for (it = memory_blocks.begin(); it != memory_blocks.end(); it++) {
        if (it->isFree && (it->size >= size)) {
            std::cout << "Size requested is " << size << " available size is " << it->size << " - Allocated\n"; 
            // create and add an allocated block entry to Linked List before free block
            struct MemoryBlock alloc_block = {it->offset, (size_t) size, false};
            memory_blocks.insert(it, alloc_block);
            // update free block meta data to reflect the new carved out block
            it->offset = it->offset + size;
            it->size -= size;
            // get a handle to return to caller
            int h = GetHandle();
            // pointer to block on memory handle
            memory_handles[h].blockIt = std::prev(it);
            memory_handles[h].isValid = true;
            return h;
        }
    }

    std::cout << "Free space of size " << size << " is not available - Alloc failed\n";
    return INVALID_HANDLE;
}

void MemoryManager::ValidateHandle(int h) const {
    if (h >= memory_handles.size())
        throw std::out_of_range("Invalid Handle");
    if (!memory_handles[h].isValid)
        throw std::invalid_argument("Freed handle");
}

/*
 * Since we do not pass raw pointer to caller, we provide
 * this Dereference handler which retuns the raw pointer to
 * allocated memory
 */
char* MemoryManager::DereferenceHandle(int h) {
    ValidateHandle(h);
    return buffer + memory_handles[h].blockIt->offset;
}

/*
 * Since we do not pass raw pointers to caller we can de-fragment memory by
 * copying data to the left of the memory block and moving the free blocks to the right
 */
void MemoryManager::Defragment() {
    // Move all in use data to left side of overall memory block being maintained
    int free_offset = 0;
    for (auto it = memory_blocks.begin(); it != memory_blocks.end(); it++) {
        if (it->isFree)
            continue;

        if (free_offset != it->offset) {
            std::memmove(buffer + free_offset,
                    buffer + it->offset,
                    it->size);
            it->offset = free_offset;
        }
        free_offset += it->size;
    }

    // delete dangling free memory blocks which are possibly
    // now over written due to defragmentation
    auto it = memory_blocks.begin();
    while (it != memory_blocks.end()) {
        if (it->isFree)
            it = memory_blocks.erase(it);
        else
            it++;
    }

    // since all data is moved to left, the free_offset should mark
    // the point where everything after is free space. Enter a new
    // block entry indicating this free space.
    size_t free_size = max_size - free_offset;
    if (free_size > 0) {
        std::cout << "Was able to defragment " << free_size << " blocks of memory\n";
        memory_blocks.push_back({free_offset, free_size, true});
    }
}

// a print utility showing memory blocks usage
void MemoryManager::PrintState() const {
    std::cout << "-----------State-----------\n";
    for (auto it: memory_blocks) {
        std::cout << " [off = " << it.offset
                  << " size = " << it.size
                  << (it.isFree ? " Free]\n" : " Used]\n");
    }
    std::cout << "-----------------------\n";
}

// If we have two blocks freed next to each other we can combine them togeather as one
void MemoryManager::CoalesceBlocks(std::list<MemoryBlock>::iterator it) {
    auto next = std::next(it);
    auto prev = std::prev(it);

    if (next != memory_blocks.end() && next->isFree) {
        it->size += next->size;
        memory_blocks.erase(next);
        std::cout << "Coalesced freed block with next adjacent free block\n";
    }
    else if (it != memory_blocks.begin() && prev->isFree) {
        prev->size += it->size;
        memory_blocks.erase(it);
        std::cout << "Coalesced freed block with prev adjacent free block\n";
    }
}

// Free the allocated memory after use
void MemoryManager::Free(int h) {
    ValidateHandle(h);
    auto it = memory_handles[h].blockIt;
    // mark the block as being free
    it->isFree = true;
    // invalidate handle and add it to free handles stack
    memory_handles[h].isValid = false;
    free_handles.push(h);
    std::cout << "Freed memory of size " << it->size << "\n";
    // coalesce if possible
    CoalesceBlocks(it);
}

MemoryManager::~MemoryManager() {
    std::free(buffer);
}

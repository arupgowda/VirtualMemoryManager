#include "MemoryManager.h"
#include <cstdio>

int main() {
    MemoryManager* mm = new MemoryManager(5);

    int h1 = mm->Alloc(1);
    mm->PrintState();

    // showing usage of memory space
    char* mem_ptr = mm->DereferenceHandle(h1);
    *mem_ptr = 'a';
    std::cout << "Data in buffer is = " << *mem_ptr << "\n";

    int h2 = mm->Alloc(1);
    int h3 = mm->Alloc(1);
    int h4 = mm->Alloc(1);
    int h5 = mm->Alloc(1);
    int h6 = mm->Alloc(1);
    mm->PrintState();
    
    mm->Free(h1);
    // This free should cause the memory to fragment
    mm->Free(h3);
    mm->PrintState();

    // Allocation fails due to fragmentation
    h6 = mm->Alloc(2);
    if (h6 == INVALID_HANDLE)
       std::cout << "Allocation failed - invalid memory handle\n";
    
    // De fragmenting memory
    mm->Defragment();
    mm->PrintState();
    
    // Allocation should now succeed
    h6 = mm->Alloc(2); 
    mm->PrintState();

    // Freeing h2
    mm->Free(h2);
    mm->PrintState();
    // Freeing h4 - should kick off coalescing
    mm->Free(h4);
    mm->PrintState();
        
    delete mm;
    return 0;
}

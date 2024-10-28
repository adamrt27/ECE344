#include "wut.h"

#include <assert.h> // assert
#include <errno.h> // errno
#include <stddef.h> // NULL
#include <stdio.h> // perror
#include <stdlib.h> // reallocarray
#include <sys/mman.h> // mmap, munmap
#include <sys/signal.h> // SIGSTKSZ
#include <sys/queue.h> // TAILQ_*
#include <ucontext.h> // getcontext, makecontext, setcontext, swapcontext
#include <valgrind/valgrind.h> // VALGRIND_STACK_REGISTER
#include <stdbool.h>

// FIFO (implemented as a doubly LL with queue.h)
// This struct defines a single entry in our doubly linked list
struct list_entry{
    int id;                             // id of context (index in list)
    TAILQ_ENTRY(list_entry) pointers;   // adds two pointers for each node, one to next, one to previous
};
TAILQ_HEAD(list_head, list_entry);      // defines the head for our list
static struct list_head LIST_HEAD;      // initializes an instance of the head

void FIFO_print(void) {
    printf("List:");
    struct list_entry* e;
    TAILQ_FOREACH(e, &LIST_HEAD, pointers) {
        printf(" %d", e->id);
    }
    printf("\n");
}

void FIFO_append(int id) {
    struct list_entry *new_entry = malloc(sizeof(struct list_entry));
    if (new_entry == NULL) {
        perror("Failed to allocate memory for new entry");
        return;
    }

    // initialize id
    new_entry->id = id;
    TAILQ_INSERT_TAIL(&LIST_HEAD, new_entry, pointers);
}

struct list_entry* FIFO_get(void) {
    // Check if the list is empty
    if (TAILQ_EMPTY(&LIST_HEAD)) {
        return NULL;  // No elements in the queue
    }

    // Get the first element in the queue
    struct list_entry *entry = TAILQ_FIRST(&LIST_HEAD);

    // Remove the entry from the queue
    TAILQ_REMOVE(&LIST_HEAD, entry, pointers);

    return entry;  // Return the removed entry
}

bool FIFO_find(int id) {
    struct list_entry *entry;

    // Iterate through the list to find an entry with the specified ID
    TAILQ_FOREACH(entry, &LIST_HEAD, pointers) {
        if (entry->id == id) {
            return true;  // ID found in the list
        }
    }

    return false;  // ID not found in the list
}

// Thread control blocks array
struct TCB {
    int id;                // Unique thread ID
    ucontext_t *context;   // Pointer to thread context
    int status;            // Exit status (0-255)
    bool finished;         // Completion flag
};

struct TCB_arr {
    struct TCB** arr; // Array of pointers to ucontext_t
    int size;         // Current number of contexts
    int cap;          // Capacity of the array
} TCB_array = {NULL, 0, 0};


// Initialize the TCB array with an initial capacity
void TCB_init(struct TCB_arr *a, int initial_cap) {
    if (initial_cap > 0) {
        a->arr = malloc(initial_cap * sizeof(struct TCB*));
        if (a->arr == NULL) {
            perror("Failed to allocate memory for TCB array");
            exit(EXIT_FAILURE);
        }
        a->cap = initial_cap;
    } else {
        a->arr = NULL;
        a->cap = 0;
    }
    a->size = 0;
}

// Add a new context to the TCB array
int TCB_add(struct TCB_arr *a, ucontext_t* uct) {
    // Check if we need to expand the array
    if (a->size >= a->cap) {
        int new_cap = (a->cap > 0) ? a->cap * 2 : 1;
        struct TCB** new_arr = realloc(a->arr, new_cap * sizeof(struct TCB*));
        if (new_arr == NULL) {
            perror("Failed to reallocate memory for TCB array");
            exit(EXIT_FAILURE);
        }
        a->arr = new_arr;
        a->cap = new_cap;
    }

    // Create and initialize a new TCB
    struct TCB *new_tcb = malloc(sizeof(struct TCB));
    if (new_tcb == NULL) {
        perror("Failed to allocate memory for TCB");
        return -1;
    }
    new_tcb->id = a->size;       // Assign an ID based on the current size
    new_tcb->context = uct;      // Set the context pointer
    new_tcb->status = -1;        // Default exit status (indicates not yet exited)
    new_tcb->finished = false;   // Mark as not finished

    // Add the new TCB to the array and increment size
    a->arr[a->size] = new_tcb;
    a->size++;
    return new_tcb->id; // Return the new thread's ID
}

// keep track of main thread
int ct_id;

static void die(const char* message) {
    int err = errno;
    perror(message);
    exit(err);
}

// creates a stack for a thread
static char* new_stack(void) {
    char* stack = mmap(
        NULL,
        SIGSTKSZ,
        PROT_READ | PROT_WRITE | PROT_EXEC,
        MAP_ANONYMOUS | MAP_PRIVATE,
        -1,
        0
    );
    if (stack == MAP_FAILED) {
        die("mmap stack failed");
    }
    VALGRIND_STACK_REGISTER(stack, stack + SIGSTKSZ);
    return stack;
}

// deletes the stack for a thread
static void delete_stack(char* stack) {
    if (munmap(stack, SIGSTKSZ) == -1) {
        die("munmap stack failed");
    }
}

void wut_init() {
    // initialize the queue (FIFO) for threads
    TAILQ_INIT(&LIST_HEAD);

    // initialize TCB_Array
    TCB_init(&TCB_array, 0);

    // create main thread and add to TCB_array
    ucontext_t* t0_ucontext = malloc(sizeof(ucontext_t));
    TCB_add(&TCB_array, t0_ucontext);

    // initialize cur_thread tracker
    ct_id = 0;

    // getcontext to initialize the thread
    getcontext(t0_ucontext);
}

int wut_id() {
    return ct_id;
}

// define this function to implicitly exit threads

void (*global_func_ptr)(void);

void thread_wrapper(void) {
    global_func_ptr();
    wut_exit(0);
}

int wut_create(void (*run)(void)) {
    // create new thread and associated stack
    ucontext_t* tN_context = malloc(sizeof(ucontext_t));
    char *tN_stack = new_stack();

    // intialize
    getcontext(tN_context);
    tN_context->uc_stack.ss_sp = tN_stack;
    tN_context->uc_stack.ss_size = SIGSTKSZ;

    // call makecontext to set the context
    global_func_ptr = run;
    makecontext(
        tN_context,
        thread_wrapper,
        0
    );

    // add to TCB_array
    int id = TCB_add(&TCB_array, tN_context);

    // add to FIFO
    FIFO_append(id);

    return id;
}

int wut_cancel(int id) {
    return -1;
}

int wut_join(int id) {
    // save current thread and switch to next thread in the queue (don't add to queue)
    int jt_id = ct_id;

    // get next in FIFO
    struct list_entry* temp = FIFO_get();
    // if there is nothign else in queue return error
    if(temp == NULL) {
        return -1;
    }
    
    // switch to the next process

    // poll for TCB_array[id] to be finished
    return -1;
}

int wut_yield() {
    // get next in FIFO
    struct list_entry* temp = FIFO_get();
    // if there is nothign else in queue return error
    if(temp == NULL) {
        return -1;
    }

    // put current thread in end of the queue
    FIFO_append(ct_id);

    // save old ct value and put in new ct
    ucontext_t* old = TCB_array.arr[ct_id]->context;
    ct_id = temp->id;

    // swap the two
    if (swapcontext(old, TCB_array.arr[ct_id]->context) == -1) {
        perror("swapcontext failed");
        return -1;
    }

    return 0;
}

void wut_exit(int status) {
    // just switch to next thread and dont put the current back in queue
    // get next in FIFO
    struct list_entry* temp = FIFO_get();
    // if the queue is empty, exit the process
    if(temp == NULL){
        exit(0);
    }

    // set exit status of current
    TCB_array.arr[ct_id]->status = status & 0xFF;
    TCB_array.arr[ct_id]->finished = true;

    // save old ct value and put in new ct
    ct_id = temp->id;

    // go to next
    setcontext(TCB_array.arr[ct_id]->context);
}

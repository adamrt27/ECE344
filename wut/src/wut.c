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

// FIFO (implemented as a doubly LL with queue.h)
// This struct defines a single entry in our doubly linked list
struct list_entry{
    int id;                             // id of context (index in list)
    ucontext_t* context;                // actual context
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

void FIFO_append(int id, ucontext_t* context) {
    struct list_entry *new_entry = malloc(sizeof(struct list_entry));
    if (new_entry == NULL) {
        perror("Failed to allocate memory for new entry");
        return;
    }

    // initialize id
    new_entry->id = id;

    // initialize context
    new_entry->context = context;

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

// Thread control blocks array
struct TCB_arr {
    ucontext_t** arr; // Array of pointers to ucontext_t
    int size;         // Current number of contexts
    int cap;          // Capacity of the array
} TCB_array = {NULL, 0, 0};


// Initialize the TCB array with an initial capacity
void TCB_init(struct TCB_arr *a, int initial_cap) {
    if (initial_cap > 0) {
        a->arr = malloc(initial_cap * sizeof(ucontext_t*));
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
        ucontext_t** new_arr = realloc(a->arr, new_cap * sizeof(ucontext_t*));
        if (new_arr == NULL) {
            perror("Failed to reallocate memory for TCB array");
            exit(EXIT_FAILURE);
        }
        a->arr = new_arr;
        a->cap = new_cap;
    }

    // Add the new context to the array and increment size
    a->arr[a->size] = uct;
    a->size++;
    return a->size - 1; // returns id
}

// keep track of main thread
typedef struct cur_thread {
    int id;
    ucontext_t* uct;
} cur_thread;

cur_thread ct;

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
    /*
    This will always be called once before a user makes any other call to your library. 
    You need to set up the main thread executing wut_init as thread 0. You should 
    initialize or setup anything else you need here.

    Your library should keep track of the following: the currently running thread, a 
    FIFO queue of waiting (or ready) threads, and thread control blocks for all threads. 
    Your thread control blocks should be in a dynamically sized array, you'll find realloc
    array helpful.
    */

    // initialize the queue (FIFO) for threads
    TAILQ_INIT(&LIST_HEAD);

    // initialize TCB_Array
    TCB_init(&TCB_array, 0);

    // create main thread and add to TCB_array
    ucontext_t* t0_ucontext = malloc(sizeof(ucontext_t));
    TCB_add(&TCB_array, t0_ucontext);

    // initialize cur_thread tracker
    ct.id = 0;
    ct.uct = t0_ucontext;

    // getcontext to initialize the thread
    getcontext(t0_ucontext);
}

int wut_id() {
    return ct.id;
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
    FIFO_append(id, tN_context);

    return id;
}

int wut_cancel(int id) {
    return -1;
}

int wut_join(int id) {

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
    FIFO_append(ct.id, ct.uct);

    // save old ct value and put in new ct
    ucontext_t* old = ct.uct;
    ct.uct = temp->context;
    ct.id = temp->id;

    // swap the two
    if (swapcontext(old, ct.uct) == -1) {
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

    // save old ct value and put in new ct
    ct.uct = temp->context;
    ct.id = temp->id;

    // go to next
    setcontext(ct.uct);
}

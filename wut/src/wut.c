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

// Thread control blocks array
typedef struct TCB {
    int id;                // Unique thread ID
    ucontext_t *context;   // Pointer to thread context
    char *stack;

    int status;            // Exit status (0-255)
    int done;         
    int joined;
    int cancelled;
    int running;

    void (*run)(void);     // function to run

    TAILQ_ENTRY(TCB) entryz;
} TCB;

static TCB *cur_thread;

static TCB threads[20000];
static TAILQ_HEAD(, TCB) ready_queue;

struct reuse_node {
    int id;
    struct reuse_node* next;
};

static struct reuse_node* reuse_queue_head = NULL;

static int id_counter = 1;

void die(const char* message) {
    int err = errno;
    perror(message);
    exit(err);
}

void insert_reuse_id(int id) {
    struct reuse_node* new_node = malloc(sizeof(struct reuse_node));
    if (new_node == NULL) {
        die("malloc failed for reuse_node");
    }
    new_node->id = id;
    new_node->next = NULL;

    if (reuse_queue_head == NULL || id < reuse_queue_head->id) {
        // Insert at the head
        new_node->next = reuse_queue_head;
        reuse_queue_head = new_node;
    } else {
        // Insert in the middle or end
        struct reuse_node* current = reuse_queue_head;
        while (current->next != NULL && current->next->id < id) {
            current = current->next;
        }
        new_node->next = current->next;
        current->next = new_node;
    }
}

int get_reuse_id() {
    if (reuse_queue_head == NULL) {
        return -1; // No IDs to reuse
    }
    int id = reuse_queue_head->id;
    struct reuse_node* temp = reuse_queue_head;
    reuse_queue_head = reuse_queue_head->next;
    free(temp);
    return id;
}

int cur_thread_cnt = 0;


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
    TAILQ_INIT(&ready_queue);
    // TAILQ_INIT(&reuse_queue);
    reuse_queue_head = NULL;

    // initialize the main TCB
    threads[0].id = 0;
    threads[0].context = malloc(sizeof(ucontext_t));
    if (threads[0].context == NULL) {
        die("malloc failed for main TCB");
    }
    threads[0].stack = NULL;
    threads[0].status = 0;
    threads[0].done = 0;
    threads[0].joined = 0;
    threads[0].running = 1;
    threads[0].cancelled = 0;
    threads[0].run = NULL;

    if (getcontext(threads[0].context) == -1) {
        die("get_context failed");
    }
    threads[0].context->uc_link = NULL;
    cur_thread = &threads[0];
}

int wut_id() {
    return cur_thread->id;
}

// define this function to implicitly exit threads

void thread_wrapper() {
    if (cur_thread->cancelled) {
        wut_exit(128);
        exit(1);
    }
    cur_thread->run();
    // After the run function completes, call wut_exit
    wut_exit(0);
    exit(1);
}

int wut_create(void (*run)(void)) {

    // get id to use
    int id =  get_reuse_id();
    if (id == -1) {
        if (id_counter >= 20000) {
            return -1; // Exceeded maximum number of threads
        }
        id = id_counter++;
    }
    printf("ID %d\n", id);

    // create new thread and associated stack
    ucontext_t* tN_context = malloc(sizeof(ucontext_t));
    char *tN_stack = new_stack();
    if (tN_context == NULL || tN_stack == NULL) {
        perror("malloc failed for TCB or stack");
        free(tN_stack);
        free(tN_context);
        return -1;
    }

    // intialize
    getcontext(tN_context);
    tN_context->uc_stack.ss_sp = tN_stack;
    tN_context->uc_stack.ss_size = SIGSTKSZ;

    // Create and initialize a new TCB
    struct TCB *new_tcb = malloc(sizeof(struct TCB));
    if (new_tcb == NULL) {
        perror("malloc failed for TCB");
        free(tN_stack);
        free(tN_context);
        return -1;
    }
    new_tcb->id = id;
    new_tcb->context = tN_context;
    new_tcb->status = 0;
    new_tcb->done = 0;
    new_tcb->joined = 0;
    new_tcb->running = 1;
    new_tcb->cancelled = 0;
    new_tcb->run = run; // Store the thread's run function
    new_tcb->stack = tN_stack;
    
    threads[id] = *new_tcb;

    // Prepare to pass the TCB pointer to thread_wrapper via makecontext
    makecontext(
        tN_context,
        (void (*)(void))thread_wrapper,
        0
    );

    TAILQ_INSERT_TAIL(&ready_queue, &threads[id], entryz);

    return id;
}

int wut_cancel(int id) {
    if (id < 0) return -1;
    if (id > 20000) return -1;
    if (threads[id].done) return -1;
    if (threads[id].joined) return -1;
    // mark id as inactive

    threads[id].cancelled = 1; // mark as cancelled
    threads[id].done = 1; // mark as done
    threads[id].running = 0; // mark as not running
    threads[id].status = 128; // mark as cancelled

    // remove from q
    TCB *temp;
    TAILQ_FOREACH(temp, &ready_queue, entryz) {
        if (temp->id == id) {
            TAILQ_REMOVE(&ready_queue, temp, entryz);
            break;
        }
    }

    return 0;
}

int wut_join(int id) {

    if (id < 0) return -1;
    if (id > 20000) return -1;
    if (id == cur_thread->id) return -1;
    if (threads[id].joined) return -1;
    if (!threads[id].running && !threads[id].done) return -1;

    while(!threads[id].done) {
        if(wut_yield() == -1){
            return -1;
        }
    }
    
    threads[id].joined = 1;
    insert_reuse_id(id);

    // Free the thread's resources
    delete_stack(threads[id].stack);
    free(threads[id].context);

    return threads[id].status;
}

int wut_yield() {
    if (TAILQ_EMPTY(&ready_queue)) {
        return -1;
    }

    if (cur_thread->status == 0) {
        TAILQ_INSERT_TAIL(&ready_queue, cur_thread, entryz);
    }

    TCB *next_thread = TAILQ_FIRST(&ready_queue);
    TAILQ_REMOVE(&ready_queue, next_thread, entryz);

    TCB *prev_thread = cur_thread;
    cur_thread = next_thread;

    if (swapcontext(prev_thread->context, cur_thread->context) == -1) {
        die("swapcontext failed");
    }

    return 0;
}

void wut_exit(int status) {
    cur_thread->status = status & 0xFF;
    cur_thread->done = 1;
    cur_thread->running = 0;

    TAILQ_REMOVE(&ready_queue, cur_thread, entryz);

    if (!TAILQ_EMPTY(&ready_queue)) {
        
        TCB *next_thread = TAILQ_FIRST(&ready_queue);
        TAILQ_REMOVE(&ready_queue, next_thread, entryz);

        cur_thread = next_thread;

        if (setcontext(cur_thread->context) == -1) {
            die("setcontext failed");
        }
    } else {
        exit(0);
    }

    exit(1);  
}
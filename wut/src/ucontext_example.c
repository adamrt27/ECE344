// ucontext example

static ucontext_t t0_ucontext;
static ucontext_t t1_ucontext;
static ucontext_t t2_ucontext;

static char* t1_stack;
static char* t2_stack;

static void t2_run(void) {
    printf("T2 should be done, switch back to T0\n");
    delete_stack(t1_stack);
    setcontext(&t0_ucontext);
}

static void t1_run(int arg0) {
    printf("Hooray! got arg0: %d\n", arg0);
    setcontext(&t0_ucontext);
}

int main(void) {

    // initializes the context struct
    // saves all values (pc, register values, etc) to ucontext struct
    getcontext(&t0_ucontext);

    // to go back to last saved state
    // resumes execution back wherever you used getcontext() last
    // this code would result in an infinite loop
    // setcontext(&t0_ucontext);

    // lets create a context that will execute the run function
    t1_stack = new_stack();                     // create stack for user thread
    getcontext(&t1_ucontext);                   // initialize context with current thread
    t1_ucontext.uc_stack.ss_sp = t1_stack;      // sets stack pointer of context to t1_stack
    t1_ucontext.uc_stack.ss_size = SIGSTKSZ;    // just a defined stack size

    makecontext(
        &t1_ucontext,                           // The ucontext to use
        (void (*)(void)) t1_run,                // the function to run
        1,                                      // how many arguements we are passing into function
        42                                      // the arguements
    );
    // first does a getcontext for t0, but fudge it a bit so the program will resume after this line
    // then does a setcontext to t1, so the program will run the thread
    swapcontext(&t0_ucontext, &t1_ucontext); 
}
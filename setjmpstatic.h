typedef struct {
    void * __regs[14];
} jmp_buf[1];

static void longjmperror(void);

static __attribute__((__returns_twice__)) __attribute__((naked)) int _setjmp(jmp_buf buf) {
__asm(
    "ldr x8, .Lmagic\n"
    "mov c9, csp\n"
    "stp c8, c9, [c0], #32\n"
    "stp c19, c20, [c0], #32\n"
    "stp c21, c22, [c0], #32\n"
    "stp c23, c24, [c0], #32\n"
    "stp c25, c26, [c0], #32\n"
    "stp c27, c28, [c0], #32\n"
    "stp c29, c30, [c0], #32\n"
    "mov x0, #0\n"
    "ret\n"
".Lmagic:\n"
    ".quad 0xfb5d25837d7ff7ff"
     );
}

static __attribute__((__noreturn__)) __attribute__((naked)) void _longjmp(jmp_buf buf, int retval) {
__asm(
    "ldr c8, [c0], #16\n"
    "ldr x9, .Lmagic\n"
    "cmp x8, x9\n"
    "b.ne 0f\n"
    "ldr c8, [c0], #16\n"
    "mov csp, c8\n"
    "ldp c19, c20, [c0], #32\n"
    "ldp c21, c22, [c0], #32\n"
    "ldp c23, c24, [c0], #32\n"
    "ldp c25, c26, [c0], #32\n"
    "ldp c27, c28, [c0], #32\n"
    "ldp c29, c30, [c0], #32\n"
    "mov x0, x1\n"
    "ret\n"
 "0: bl longjmperror\n"
    "bl abort"
     );
}



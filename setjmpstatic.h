/*
 * Adapted from the AArch64 FreeBSD setjmp implementation:
 *
 * Copyright (c) 2014 Andrew Turner
 * Copyright (c) 2014 The FreeBSD Foundation
 *
 * Portions of this software were developed by Andrew Turner
 * under sponsorship from the FreeBSD Foundation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

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



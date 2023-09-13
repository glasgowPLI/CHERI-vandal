/*
 * Copyright (c) 2023 University of Glasgow
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not, see
 * <https://www.gnu.org/licenses/>.
 */

.global vandalise
.balign 16
.type vandalise, %function
vandalise:
    // ensure c16-c18 are clear
    chktgd c16
    b.cs .Lerr
    chktgd c17
    b.cs .Lerr
    chktgd c18
    b.cs .Lerr
    // previous accessible call frames in c12
    mov c18, csp
    gclim x16, c18
    sub x16, x16, x18
    scbnds c18, c18, x16
    // confirm 28 free stack slots
    sub csp, csp, #(28*16)
    mov w16, #28
 0: sub w16, w16, #1
    ldr c17, [csp, w16, uxtw #4]
    chktgd c17
    b.cc 1f
    sub csp, csp, #(28*16)
    add csp, csp, w16, uxtw #4
    mov w16, #28
 1: cbnz w16, 0b
    // we have confirmed that c10, c11, c12, and 28 stack slots are clear, now spill other regs
    stp c0,  c1,  [csp, #(26*16)]
    stp c2,  c3,  [csp, #(24*16)]
    stp c4,  c5,  [csp, #(22*16)]
    stp c6,  c7,  [csp, #(20*16)]
    stp c8,  c9,  [csp, #(18*16)]
    stp c10, c11, [csp, #(16*16)]
    stp c12, c13, [csp, #(14*16)]
    stp c14, c15, [csp, #(12*16)]
    stp c19, c20, [csp, #(10*16)]
    stp c21, c22, [csp, #(8*16)]
    stp c23, c24, [csp, #(6*16)]
    stp c25, c26, [csp, #(4*16)]
    stp c27, c28, [csp, #(2*16)]
    stp c29, c30, [csp, #(0*16)]
    mov c29, csp
   
    adr c17, #0
    clrperm c17, c17, x
    stp c17, c18, [csp, #-32]!
    scbnds c0, csp, #30, lsl #4
    mov w1, #30
    gcvalue x2, c17
    bl vandalise_arr
    mov csp, c29
    ldp c29, c30, [csp], #(28*16)
    ret

.Lerr: mov x0, #(-1)
    ret
.size vandalise, .-vandalise


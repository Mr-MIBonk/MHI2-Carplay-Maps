/* Wireless CarPlay Maps. Maintainer: omonob. Source date: 2026-08-29. */
/* ARM EABI. Preserve all incoming registers and original variadic stack.
 * Non-target calls tail-call the original libc symbol without entering C.
 * This library is exact-receiver-bound; no writable executable patches. */
.syntax unified
.arm
.text
.align 2
.global fprintf
.type fprintf,%function
fprintf:
    movw ip,#0xcbcc
    movt ip,#0x000e
    cmp lr,ip
    beq .Lobserve
    movw ip,#0xba38
    movt ip,#0x000e
    cmp lr,ip
    beq .Lobserve
    movw ip,#0xe6dc
    movt ip,#0x000e
    cmp lr,ip
    bne .Lfprintf_pass
.Lobserve:
    push {r0-r12,lr}
    mov r0,sp
    bl v75_observe_regs
    pop {r0-r12,lr}
.Lfprintf_pass:
    ldr ip,.Lfprintf_offset
.Lfprintf_pc:
    add ip,pc,ip
    ldr ip,[ip]
    cmp ip,#0
    bxne ip
    mvn r0,#0
    bx lr
.align 2
.Lfprintf_offset: .word v75_real_fprintf-(.Lfprintf_pc+8)
.size fprintf,.-fprintf

.global memcpy
.type memcpy,%function
memcpy:
    movw ip,#0xdd9c
    movt ip,#0x000e
    cmp lr,ip
    bne .Lmemcpy_pass
    push {r0-r12,lr}
    mov r3,sp
    bl v75_copy_link
    pop {r0-r12,lr}
    bx lr
.Lmemcpy_pass:
    ldr ip,.Lmemcpy_offset
.Lmemcpy_pc:
    add ip,pc,ip
    ldr ip,[ip]
    cmp ip,#0
    bxne ip
    /* dlsym may itself copy before constructor resolution has completed. */
    cmp r2,#0
    bxeq lr
    push {r0,r3}
.Lbytecopy:
    ldrb r3,[r1],#1
    strb r3,[r0],#1
    subs r2,r2,#1
    bne .Lbytecopy
    pop {r0,r3}
    bx lr
.align 2
.Lmemcpy_offset: .word v75_real_memcpy-(.Lmemcpy_pc+8)
.size memcpy,.-memcpy

.global free
.type free,%function
free:
    movw ip,#0x4824
    movt ip,#0x000e
    cmp lr,ip
    bne .Lfree_pass
    push {r0-r12,lr}
    mov r0,sp
    bl v75_add_hid_regs
    pop {r0-r12,lr}
.Lfree_pass:
    ldr ip,.Lfree_offset
.Lfree_pc:
    add ip,pc,ip
    ldr ip,[ip]
    cmp ip,#0
    bxne ip
    bx lr
.align 2
.Lfree_offset: .word v75_real_free-(.Lfree_pc+8)
.size free,.-free
.section .note.GNU-stack,"",%progbits

/* Unique imported alias, not a process-wide getenv interposer. Preserve all
 * registers and flags because one caller replaces an original load only.
 */
.syntax unified
.arm
.text
.align 2
.global v75gen
.type v75gen,%function
v75gen:
    push {r0-r12,lr}
    mrs r3,cpsr
    push {r2,r3}
    add r0,sp,#8
    bl v75_stream_getenv_dispatch
    str r0,[sp,#8]
    pop {r2,r3}
    msr CPSR_f,r3
    pop {r0-r12,pc}
.size v75gen,.-v75gen
.section .note.GNU-stack,"",%progbits

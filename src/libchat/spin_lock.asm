    .const
        Locked QWORD 1
    .code
    ; check calling convention depends on your OS
    
    ; extern void spin_lock(SPINLOCK_KEYHOLDER* keyholder);
    ; paramater: SPINLOCK_KEYHOLDER* in RCX
    ; return: void
    ;
    ; Description
    ;   test paramater and key value,
    ;   if is is the same, it means locked. Go back and loop
    ;   if it is different, it mean unlocked. Go next and lock.
    ;
    spin_lock proc
@@:
    mov                 RAX, [Locked]
    test                RAX, [RCX]      ; equal(not zero) - locked, not equal(zero) - unlocked
    jne                 @B
    
    lock xchg           RAX, [RCX]      
    test                RAX, RAX
    jnz                 @B

    ret
    spin_lock endp

    ;extern void spin_unlock(SPINLOCK_KEYHOLDER* keyholder);
    ; paramater: SPINLOCK_KEYHOLDER* in RCX
    ; return: void
    ;
    ; Description
    ;   set keyholder variable unlocked
    ;
    spin_unlock proc
    xor                 RAX, RAX
    lock xchg           RAX, [RCX]
    ret
    spin_unlock endp

    
end
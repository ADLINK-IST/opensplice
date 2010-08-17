/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
/*
 * Implements PA abstraction for arm9
 */

typedef struct {
//mutex: value 0 = locked
//             1 = free
  volatile int mutex;
} atomic_t;

atomic_t atomic={1};

os_uint32 pa_increment(os_uint32 *count)
{
    unsigned long tmp;
    os_uint32 value;
    volatile int *ptr=&(atomic.mutex);

       __asm__ __volatile__(
"1:  ldr %2, =0\n"        // value locked
"    swp %2, %2, [%4]\n"  // store value locked into mutex
"    teq %2, #0\n"
"    beq 1b\n"
"    add %1, %3, #1\n"    // increment
"    mov %0, %1\n"        // copy
"    ldr %2, =1\n"        // value unlocked
"    swp %2, %2, [%4]\n"  // store value unlocked into mutex
     : "=&r" (value), "=r" (*count), "=&r" (tmp)
     : "r" (*count),  "r" (ptr)
     : "memory");
       return value;
}

os_uint32 pa_decrement(os_uint32 *count)
{
    unsigned long tmp;
    os_uint32 value;
    volatile int *ptr=&(atomic.mutex);

       __asm__ __volatile__(
"1:  ldr %2, =0\n"        // value locked
"    swp %2, %2, [%4]\n"  // store value locked into mutex
"    teq %2, #0\n"
"    beq 1b\n"
"    sub %1, %3, #1\n"    // decrement
"    mov %0, %1\n"        // copy
"    ldr %2, =1\n"        // value unlocked
"    swp %2, %2, [%4]\n"  // store value unlocked into mutex
     : "=&r" (value), "=r" (*count), "=&r" (tmp)
     : "r" (*count),  "r" (ptr)
     : "memory");
       return value;
}

void p_mutex()
{
    unsigned long tmp;
    volatile int *ptr=&(atomic.mutex);

       __asm__ __volatile__(
"    ldr %0, =0\n"        // value locked
"    swp %0, %0, [%1]\n"  // store value locked into mutex
     :  "=&r" (tmp)
     : "r" (ptr)
     : "memory");
}

void v_mutex()
{
    unsigned long tmp;
    volatile int *ptr=&(atomic.mutex);

       __asm__ __volatile__(
"    ldr %0, =1\n"        // value unlocked
"    swp %0, %0, [%1]\n"  // store value unlocked into mutex
     :  "=&r" (tmp)
     : "r" (ptr)
     : "memory");
}



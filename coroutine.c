#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>


#define STACK_SIZE 4096


typedef struct context_t context_t;
typedef void (*coroutine_entry)(void *);
typedef struct coroutine_t coroutine_t;

enum GPR {
    RAX = 0,
    RBX,
    RCX,
    RDX,
    RSI,
    RDI,
    RBP,
    RSP,
    R8,
    R9,
    R10,
    R11,
    R12,
    R13,
    R14,
    R15,
    RFLAG,
};

struct context_t {
    uint64_t gpr[32];
};

struct coroutine_t {
    context_t context;
    char stack[STACK_SIZE];
    enum {
        READY,
        RUNNING,
        STOPING,
        DEADED,
    } state;
    coroutine_entry entry;
    void *arg;
};

coroutine_t coroutines[10];
int co_num = 0;
coroutine_t *current;

__attribute__((naked))
void switch_context(context_t *current, context_t *next) {
    __asm__ __volatile__(
        // save current context
        "movq %rax, 0(%rdi)   \n"
        "movq %rbx, 8(%rdi)   \n"
        "movq %rcx, 16(%rdi)  \n"
        "movq %rdx, 24(%rdi)  \n"
        "movq %rsi, 32(%rdi)  \n"
        "movq %rdi, 40(%rdi)  \n"
        "movq %rbp, 48(%rdi)  \n"
        "movq %rsp, 56(%rdi)  \n"
        "movq %r8,  64(%rdi)  \n"
        "movq %r9,  72(%rdi)  \n"
        "movq %r10, 80(%rdi)  \n"
        "movq %r11, 88(%rdi)  \n"
        "movq %r12, 96(%rdi)  \n"
        "movq %r13, 104(%rdi) \n"
        "movq %r14, 112(%rdi) \n"
        "movq %r15, 120(%rdi) \n"
        // restore next context
        "movq 0(%rsi),   %rax \n"
        "movq 8(%rsi),   %rbx \n"
        "movq 16(%rsi),  %rcx \n"
        "movq 24(%rsi),  %rdx \n"
        "movq 40(%rsi),  %rdi \n"
        "movq 48(%rsi),  %rbp \n"
        "movq 56(%rsi),  %rsp \n"
        "movq 64(%rsi),  %r8  \n"
        "movq 72(%rsi),  %r9  \n"
        "movq 80(%rsi),  %r10 \n"
        "movq 88(%rsi),  %r11 \n"
        "movq 96(%rsi),  %r12 \n"
        "movq 104(%rsi), %r13 \n"
        "movq 112(%rsi), %r14 \n"
        "movq 120(%rsi), %r15 \n"
        "movq 32(%rsi),  %rsi \n"
        // resume next
        "ret \n"
    );
}

void yield() {
    coroutine_t *next;
    for (int i = 0; i < 10; i++) {
        next = &coroutines[i];
        if (next->state == STOPING || next->state == READY) {
            if (current->state == RUNNING) {
                current->state = STOPING;
            }
            next->state = RUNNING;
            break;
        }
    }
    coroutine_t *prev = current;
    current = next;
    switch_context(&prev->context, &current->context);
}

void resume(coroutine_t *next) {
    switch_context(&current->context, &next->context);
}

void _coroutine_start(coroutine_t *co) {
    co->entry(co->arg);
    co->state = DEADED;
    yield();
}

coroutine_t *get_main_coroutine() {
    char dummy;

    if (co_num == 10) {
        exit(-1);
    }
    coroutine_t *co = &coroutines[co_num++];
    co->state = RUNNING;

    return co;
}

coroutine_t *create_coroutine(coroutine_entry entry, void *arg) {
    if (co_num == 10) {
        exit(-1);
    }

    coroutine_t *co = &coroutines[co_num++];

    co->state = READY;
    co->entry = entry;
    co->arg = arg;
    co->context.gpr[RSP] = (uint64_t)(co->stack + STACK_SIZE - sizeof(void *));
    co->context.gpr[RDI] = (uint64_t)co;
    *(uint64_t *)co->context.gpr[RSP] = (uint64_t)_coroutine_start;

    return co;
}

void print(void *arg) {
    printf("hello coroutine!\n");
}

int main() {
    coroutine_t *main = get_main_coroutine();
    coroutine_t *a = create_coroutine(print, NULL);
    current = main;
    yield();
    printf("return from print\n");
}

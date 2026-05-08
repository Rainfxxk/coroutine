#ifndef __COROUTINE_H__
#define __COROUTINE_H__


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

void switch_context(context_t *current, context_t *next);
void yield();
void resume(coroutine_t *next);
void _coroutine_start(coroutine_t *co);
coroutine_t *get_main_coroutine();
coroutine_t *create_coroutine(coroutine_entry entry, void *arg);

#endif // __COROUTINE_H__

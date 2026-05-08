#include "../../coroutine.h"

coroutine_t *a;
coroutine_t *b;

void print(void *arg) {
    char ch = (char) arg;
    for (int i = 0; i < 10; i++) {
        printf("%c: hello coroutine!\n", ch);
        if (ch == 'a')
            resume(b);
        else
            resume(a);
    }
}

int main() {
    a = create_coroutine(print, (void *)'a');
    b = create_coroutine(print, (void *)'b');
    resume(a);
    printf("return from print\n");
}

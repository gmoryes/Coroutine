#include <iostream>
#include <csetjmp>
#include <cstdio>
#include <algorithm>

using std::cout;
using std::endl; 

struct Context {
    Context(): low(nullptr), high(nullptr), stack(nullptr) {}
    char *low;
    char *high;
    char* stack;
    int size;
    std::jmp_buf env;
};

Context a_ctx, b_ctx, run_ctx;

void store(Context &ctx) {
    volatile char high;
    ctx.high = (char*)&high;
    if (ctx.stack != nullptr)
        delete[] ctx.stack;

    int size = std::abs(ctx.high - ctx.low);
    ctx.stack = new char[size];
    char *cur = ctx.low;
    int i = 0;
    while (cur != ctx.high) {
        ctx.stack[i] = *cur;
        cur--;
        i++;
    }
    ctx.size = i;
}

char* cur_static;
void change_coro(Context &next) {
    cur_static = next.low;
    for (int i = 0; i < next.size; i++) {
        *cur_static = next.stack[i];
        cur_static--;
    }

    longjmp(next.env, 1);
}

void b() {
    cout << "B(1)" << endl;
    if (setjmp(b_ctx.env) > 0) {
        cout << "B(2)" << endl;
        change_coro(a_ctx);
    } else {
        store(b_ctx);
        change_coro(a_ctx);
    }
}

void a() {
    cout << "A(1)" << endl;
    if (setjmp(a_ctx.env) > 0) {
        cout << "A(2)" << endl;
        if (setjmp(a_ctx.env) > 0) {
            change_coro(run_ctx);
        } else {
            store(a_ctx);
            change_coro(b_ctx);
        }
    } else {
        store(a_ctx);
        b();
    }
}



void run() {
    volatile char stack_start_here;

    a_ctx.low = (char*)&stack_start_here; 
    b_ctx.low = (char*)&stack_start_here;
    run_ctx.low = (char*)&stack_start_here;

    if (setjmp(run_ctx.env) > 0) {
        cout << "Done" << endl;
    } else {
        store(run_ctx);
        a();
    }
}

int main() {
    run();
    return 0;
}

// RUN: %clang_cc1 -fcxx-exceptions -fexceptions -load %llvmshlibdir/libChacshinNoexcept_Chacshin_Vladimir_FIIT3_ClangAST%pluginext -plugin chacshin_noexcept_plugin -fsyntax-only %s 2>&1 | FileCheck %s

class Base {
public:
    virtual ~Base() {}
};

class Derived : public Base {};

struct CtorThrow {
    CtorThrow() { throw 1; }
};

struct CtorNoThrow {
    CtorNoThrow() {}
};

void empty() {}

void simpleThrow() {
    throw 42;
}

void withNew() {
    int* p = new int;
    delete p;
}

void castToRef() {
    Derived d;
    Base& b = d;
    Derived& d2 = dynamic_cast<Derived&>(b);
}

void castToPtr() {
    Derived d;
    Base* b = &d;
    Derived* d2 = dynamic_cast<Derived*>(b);
}

void callNoThrow() {
    empty();
}

void callThrow() {
    simpleThrow();
}

int fact(int n) {
    if (n <= 1) return 1;
    return n * fact(n - 1);
}

int factThrow(int n) {
    if (n <= 1) throw 1;
    return n * factThrow(n - 1);
}

void lambdaThrow() {
    auto l = []() { throw 1; };
    l();
}

void lambdaNoThrow() {
    auto l = []() {};
    l();
}

void createCtorThrow() {
    CtorThrow obj;
}

void createCtorNoThrow() {
    CtorNoThrow obj;
}

void callCtorThrow() {
    CtorThrow obj;
}

void callCtorNoThrow() {
    CtorNoThrow obj;
}

void foo(int) {}

void pointerCall(void (*f)(int)) {
    f(42);
}

// CHECK: Function empty marked noexcept
// CHECK: Function simpleThrow remains potentially throwing
// CHECK: Function withNew remains potentially throwing
// CHECK: Function castToRef remains potentially throwing
// CHECK: Function castToPtr marked noexcept
// CHECK: Function callNoThrow marked noexcept
// CHECK: Function callThrow remains potentially throwing
// CHECK: Function fact marked noexcept
// CHECK: Function factThrow remains potentially throwing
// CHECK: Function lambdaThrow remains potentially throwing
// CHECK: Function lambdaNoThrow marked noexcept
// CHECK: Function createCtorThrow remains potentially throwing
// CHECK: Function createCtorNoThrow marked noexcept
// CHECK: Function callCtorThrow remains potentially throwing
// CHECK: Function callCtorNoThrow marked noexcept
// CHECK: Function pointerCall marked noexcept
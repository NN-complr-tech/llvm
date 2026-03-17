
// RUN: %clang_cc1 -load %llvmshlibdir/luzan_e_cstyle_casts_ClangAST%pluginext -plugin luzan_e_cstyle_casts -fsyntax-only %s 2>&1 | FileCheck %s

int foo(int a) {
    return 0;
}
int StaticTest0(float a) {
    return (int)a; 
    // CHECK-LABEL: int StaticTest0(float a)
    // CHECK-NEXT: return static_cast<int>(a)
}

int StaticTest1(float a) {
    return (int)a;
    // CHECK-LABEL: int StaticTest1(float a)
    // CHECK-NEXT: return static_cast<int>(a);
}

double StaticTest2(float a) {
    return (double)a;
    // CHECK-LABEL: double StaticTest2(float a)
    // CHECK-NEXT: return static_cast<double>(a);
}

int StaticExpressionTest(float a, float b) {
    return (int)(a + b);
    // CHECK-LABEL: int StaticExpressionTest(float a, float b)
    // CHECK-NEXT: return static_cast<int>((a + b));
}

int staticCastNegative(double a) {
    return (int)-a;
    // CHECK-LABEL: int staticCastNegative(double a)
    // CHECK-NEXT: return static_cast<int>(-a);
}

int staticCastTernary(float a, float b) {
    return (int)(a > 0 ? a : b);
    // CHECK-LABEL: int staticCastTernary(float a, float b)
    // CHECK-NEXT: return static_cast<int>((a > 0 ? a : b));
}

int NestedCasts(float a) {
    return (int)(double)a;
    // CHECK-LABEL: int NestedCasts(float a)
    // CHECK-NEXT: static_cast<int>(static_cast<double>(a))
}

int NestedCasdts(float a, double b) {
    return (int)((double)a + b);
    // CHECK-LABEL: int NestedCasdts(float a, double b)
    // CHECK-NEXT: static_cast<int>((static_cast<double>(a) + b))
}

void multipleCasts(float a, float b) {
    int x = (int)a;
    int y = (int)b;
    // CHECK-LABEL: void multipleCasts(float a, float b)
    // CHECK-NEXT: int x = static_cast<int>(a);
    // CHECK-NEXT: int y = static_cast<int>(b);
}

void reinterpretCase(void* p) {
    int* x = (int*)p;
    // CHECK-LABEL: void reinterpretCase(void* p)
    // CHECK-NEXT: int* x = reinterpret_cast<int *>(p)
}

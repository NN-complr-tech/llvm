
// RUN: %clang_cc1 -load %llvmshlibdir/luzan_e_cstyle_casts_ClangAST%pluginext -plugin luzan_e_cstyle_casts -fsyntax-only %s 2>&1 | FileCheck %s

int foo(int a) {
    return 0;
}
int staticTest0(float a) {
    return (int)a; 
    // CHECK-LABEL: int staticTest0(float a)
    // CHECK-NEXT: return static_cast<int>(a)
}

int staticTest1(float a) {
    return (int)a;
    // CHECK-LABEL: int staticTest1(float a)
    // CHECK-NEXT: return static_cast<int>(a);
}

double staticTest2(float a) {
    return (double)a;
    // CHECK-LABEL: double staticTest2(float a)
    // CHECK-NEXT: return static_cast<double>(a);
}

int staticExpressionTest(float a, float b) {
    return (int)(a + b);
    // CHECK-LABEL: int staticExpressionTest(float a, float b)
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

int nestedCasts(float a) {
    return (int)(double)a;
    // CHECK-LABEL: int nestedCasts(float a)
    // CHECK-NEXT: static_cast<int>(static_cast<double>(a))
}

int complexNestedCasts(float a, double b) {
    return (int)((double)a + b);
    // CHECK-LABEL: int complexNestedCasts(float a, double b)
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

int noCast(int a) {
    return a;
    // CHECK-LABEL: int noCast2(int a)
    // CHECK: return a;
}

int alreadyCpp(float a) {
    return static_cast<int>(a);
    // CHECK-LABEL: int alreadyCpp(float a)
    // CHECK: return static_cast<int>(a);
}

void sameLine(float a, float b) {
    int x = (int)a, y = (int)b;
    // CHECK-LABEL: void sameLine(float a, float b)
    // CHECK: int x = static_cast<int>(a), y = static_cast<int>(b);
}

void manyCasts(float a, float b) {
    int x = (int)a + (int)b;
    // CHECK-LABEL: void sameLine(float a, float b)
    // CHECK: int x = static_cast<int>(a) + static_cast<int>(b);
}

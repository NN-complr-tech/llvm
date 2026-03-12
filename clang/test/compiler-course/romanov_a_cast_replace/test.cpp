// RUN: split-file %s %t
// RUN: %clang_cc1 -load %llvmshlibdir/romanov_a_cast_replace_ClangAST%pluginext -plugin romanov_a_cast_replace_plugin -fsyntax-only %t/input.cpp 2>&1 | FileCheck %s

// CHECK-LABEL: int staticCastBase1(float a) {
// CHECK-NEXT:     return static_cast<int>(a);
// CHECK-NEXT: }

// CHECK-LABEL: int staticCastBase2(float a) {
// CHECK-NEXT:     return int(a);
// CHECK-NEXT: }

// CHECK-LABEL: int staticCastBase3(float a) {
// CHECK-NEXT:     return static_cast<int>((a));
// CHECK-NEXT: }

// CHECK-LABEL: int staticCastBase4(float a) {
// CHECK-NEXT:     return (static_cast<int>((a)));
// CHECK-NEXT: }

// CHECK-LABEL: long long staticCastBase5(float a) {
// CHECK-NEXT:     return static_cast<long long>(a);
// CHECK-NEXT: }

// CHECK-LABEL: long long staticCastBase6(float a) {
// CHECK-NEXT:     return static_cast<long long>((a));
// CHECK-NEXT: }

// CHECK-LABEL: int staticCastBase7(float a) {
// CHECK-NEXT:     return static_cast<int>(a);
// CHECK-NEXT: }

// CHECK-LABEL: long double staticCastSeq(float a) {
// CHECK-NEXT:     return static_cast<long double>(static_cast<long long>(a));
// CHECK-NEXT: }

// CHECK-LABEL: double staticCastAfterOp(long long a, long long b) {
// CHECK-NEXT:     return static_cast<double>((a + b));
// CHECK-NEXT: }

// CHECK-LABEL: short staticCastSeqWithOp(long long a, char b) {
// CHECK-NEXT:     return static_cast<short>((static_cast<char>(a) + b));
// CHECK-NEXT: }

// CHECK-LABEL: bool staticCastInCondition(float a) {
// CHECK-NEXT:     return static_cast<int>(a) > 0;
// CHECK-NEXT: }

// CHECK-LABEL: int staticCastNegative(double a) {
// CHECK-NEXT:     return static_cast<int>(-a);
// CHECK-NEXT: }

// CHECK-LABEL: int staticCastTernary(float a, float b) {
// CHECK-NEXT:     return static_cast<int>((a > 0 ? a : b));
// CHECK-NEXT: }

// CHECK-LABEL: int *constCastBase1(const int *a) {
// CHECK-NEXT:     return const_cast<int *>(a);
// CHECK-NEXT: }

// CHECK-LABEL: const int *constCastBase2(int *a) {
// CHECK-NEXT:     return const_cast<const int *>(a);
// CHECK-NEXT: }

// CHECK-LABEL: int *constCastBase3(const int *a) {
// CHECK-NEXT:     return (const_cast<int *>((a)));
// CHECK-NEXT: }

// CHECK-LABEL: volatile int *constCastVolatile1(int *a) {
// CHECK-NEXT:     return const_cast<volatile int *>(a);
// CHECK-NEXT: }

// CHECK-LABEL: int *constCastVolatile2(volatile int *a) {
// CHECK-NEXT:     return const_cast<int *>(a);
// CHECK-NEXT: }

// CHECK-LABEL: int *constCastConstVolatile(const volatile int *a) {
// CHECK-NEXT:     return const_cast<int *>(a);
// CHECK-NEXT: }

// CHECK-LABEL: int *reinterpretCastBase1(float *a) {
// CHECK-NEXT:     return reinterpret_cast<int *>(a);
// CHECK-NEXT: }

// CHECK-LABEL: long reinterpretCastPtrToInt(int *a) {
// CHECK-NEXT:     return reinterpret_cast<long>(a);
// CHECK-NEXT: }

// CHECK-LABEL: int *reinterpretCastIntToPtr(long a) {
// CHECK-NEXT:     return reinterpret_cast<int *>(a);
// CHECK-NEXT: }

// CHECK-LABEL: void *reinterpretCastToVoidPtr(int *a) {
// CHECK-NEXT:     return static_cast<void *>(a);
// CHECK-NEXT: }

// CHECK-LABEL: int *reinterpretCastFromVoidPtr(void *a) {
// CHECK-NEXT:     return static_cast<int *>(a);
// CHECK-NEXT: }

// CHECK-LABEL: char *reinterpretCastWithParens(float *a) {
// CHECK-NEXT:     return (reinterpret_cast<char *>((a)));
// CHECK-NEXT: }

//--- input.cpp
int staticCastBase1(float a) {
    return (int)a;
}

int staticCastBase2(float a) {
    return int(a); // Won't be replaced, because it is functional style cast, not C-style cast
}

int staticCastBase3(float a) {
    return (int)(a);
}

int staticCastBase4(float a) {
    return ((int)(a));
}

long long staticCastBase5(float a) {
    return (long long)a;
}

long long staticCastBase6(float a) {
    return (long long)(a);
}

int staticCastBase7(float a) {
    return static_cast<int>(a); // Won't be replaced, because it is already C++ style cast, not C-style cast
}

long double staticCastSeq(float a) {
    return (long double)(long long)a;
}

double staticCastAfterOp(long long a, long long b) {
    return (double)(a + b);
}

short staticCastSeqWithOp(long long a, char b) {
    return (short)((char)a + b);
}

bool staticCastInCondition(float a) {
    return (int)a > 0;
}

int staticCastNegative(double a) {
    return (int)-a;
}

int staticCastTernary(float a, float b) {
    return (int)(a > 0 ? a : b);
}

int *constCastBase1(const int *a) {
    return (int *)a;
}

const int *constCastBase2(int *a) {
    return (const int *)a;
}

int *constCastBase3(const int *a) {
    return ((int *)(a));
}

volatile int *constCastVolatile1(int *a) {
    return (volatile int *)a;
}

int *constCastVolatile2(volatile int *a) {
    return (int *)a;
}

int *constCastConstVolatile(const volatile int *a) {
    return (int *)a;
}

int *reinterpretCastBase1(float *a) {
    return (int *)a;
}

long reinterpretCastPtrToInt(int *a) {
    return (long)a;
}

int *reinterpretCastIntToPtr(long a) {
    return (int *)a;
}

void *reinterpretCastToVoidPtr(int *a) {
    return (void *)a;
}

int *reinterpretCastFromVoidPtr(void *a) {
    return (int *)a;
}

char *reinterpretCastWithParens(float *a) {
    return ((char *)(a));
}
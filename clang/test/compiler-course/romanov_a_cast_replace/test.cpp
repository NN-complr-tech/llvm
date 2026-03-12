// RUN: %clang_cc1 -load %llvmshlibdir/romanov_a_cast_replace_ClangAST%pluginext -plugin romanov_a_cast_replace_plugin -fsyntax-only %s 2>&1 | FileCheck %s


// CHECK-LABEL: int baseCast1(float a) {
// CHECK-NEXT:     return static_cast<int>(a);
// CHECK-NEXT: }
int baseCast1(float a) {
    return (int)a;
}

// CHECK-LABEL: int baseCast2(float a) {
// CHECK-NEXT:     return int(a);
// CHECK-NEXT: }
int baseCast2(float a) {
    return int(a); // No replacement because it is functional-cast (not C-style cast)
}

// CHECK-LABEL: int baseCast3(float a) {
// CHECK-NEXT:     return static_cast<int>((a));
// CHECK-NEXT: }
int baseCast3(float a) {
    return (int)(a);
}

// CHECK-LABEL: int baseCast4(float a) {
// CHECK-NEXT:     return (static_cast<int>((a)));
// CHECK-NEXT: }
int baseCast4(float a) {
    return ((int)(a));
}

// CHECK-LABEL: long long baseCast5(float a) {
// CHECK-NEXT:     return static_cast<long long>(a);
// CHECK-NEXT: }
long long baseCast5(float a) {
    return (long long)a;
}

// CHECK-LABEL: long long baseCast6(float a) {
// CHECK-NEXT:     return static_cast<long long>((a));
// CHECK-NEXT: }
long long baseCast6(float a) {
    return (long long)(a);
}

// CHECK-LABEL: int baseCast7(float a) {
// CHECK-NEXT:     static_cast<int>(a);
// CHECK-NEXT: }
int baseCast7(float a) {
    return static_cast<int>(a); // No replacement because it is C++-style cast (not C-style cast)
}

// CHECK-LABEL: double baseCastAfterOp1(long long a, long long b) {
// CHECK-NEXT:     return static_cast<double>(a + b + c);
// CHECK-NEXT: }
double baseCastAfterOp1(long long a, long long b) {
    return (double)(a + b);
}
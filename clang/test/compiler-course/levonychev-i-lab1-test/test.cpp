// RUN: %clang_cc1 -load %llvmshlibdir/levonychev-i-lab1_ClangAST%pluginext -plugin const_change_plugin -fsyntax-only %s 2>&1 | FileCheck %s



// CHECK: void test_func1(const float{{\*}} const x1) {

void test_func1(float* x1) {
    float var = *x1;
}


// CHECK: const int{{\*}} const x2_ptr = &x2;
void test_func2() {
    int x2 = 0;
    int* x2_ptr = &x2;
}



// CHECK: int{{\*}} const x3_ptr = &x3;
void test_func3() {
    int x3 = 0;
    int* x3_ptr = &x3;
    *x3_ptr = 1;
}

// CHECK-NOT: int{{\*}} const x4_ptr = &x4;
void test_func4() {
    int x4 = 0;
    int* x4_ptr = &x4;
    x4_ptr++;
    x4_ptr--;
    *x4_ptr = 1;
}

// CHECK: void test_func5(const double& x5{{\)}} {
void test_func5(double& x5) {
    double var = x5;
}


// CHECK-NOT: void test_func6(const double& x6{{\)}} {
void test_func6(double& x6) {
    x6 = 4.0;
}

// CHECK-NOT: const int& x7 {{\=}} var;
void test_func7() {
    int var = 10;
    int& x7 = var;
    x7 = 100;
}
// CHECK-NOT: const int& x8 {{\=}} var;
void test_func8() {
    int var = 10;
    int& x8 = var;
    ++x8;
}
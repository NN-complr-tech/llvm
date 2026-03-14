// RUN: %clang_cc1 -fcxx-exceptions -fexceptions -load %llvmshlibdir/nikitina_v_lab1_ClangAST%pluginext -add-plugin nikitina_v_noexcept_plugin -fsyntax-only %s 2>&1 | FileCheck %s
void SafeCalculation() {
    double a = 3.14;
    double b = 2.0;
    double c = a * b;
}

void RaisesException() { 
    throw "Error occurred!"; 
}

void ConditionalThrow(bool flag) {
    int value = 100;
    if (flag) {
        throw value;
    }
}

void WrapperForThrow() {
    RaisesException();
}

void WrapperForSafe() {
    SafeCalculation();
}

void HeapAllocation() {
    long* data = new long[100];
    delete[] data;
}

void DeepCallC() { throw 404; }
void DeepCallB() { DeepCallC(); }  
void DeepCallA() { DeepCallB(); }

void ExecuteTask() {
    RaisesException();
    RaisesException();
}

void FibonacciThrow(int step) {
    if (step > 0) {
        if (step == 42) throw step; 
        FibonacciThrow(step - 1);
    }
}

class DangerStruct {
public:
    DangerStruct() { throw 1; } 
};

void InstantiateStruct() {
    DangerStruct instance; 
}

// CHECK: Function SafeCalculation: exception spec: 5
// CHECK: Function RaisesException: exception spec: 0
// CHECK: Function ConditionalThrow: exception spec: 0
// CHECK: Function WrapperForThrow: exception spec: 0
// CHECK: Function WrapperForSafe: exception spec: 5
// CHECK: Function HeapAllocation: exception spec: 0
// CHECK: Function DeepCallC: exception spec: 0
// CHECK: Function DeepCallB: exception spec: 0
// CHECK: Function DeepCallA: exception spec: 0
// CHECK: Function ExecuteTask: exception spec: 0
// CHECK: Function FibonacciThrow: exception spec: 0
// CHECK: Function DangerStruct: exception spec: 0
// CHECK: Function InstantiateStruct: exception spec: 0
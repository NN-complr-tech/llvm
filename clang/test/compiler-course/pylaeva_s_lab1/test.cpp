// RUN: %clang_cc1 -load %llvmshlibdir/pylaeva_s_lab1_ClangAST%pluginext -plugin pylaeva_s_lab1_plugin -fsyntax-only %s 2>&1 | FileCheck %s

extern "C" {
    void* malloc(unsigned long size);
    void* calloc(unsigned long count, unsigned long size);
    void* realloc(void* ptr, unsigned long size);
    void free(void* ptr);
    void* fopen(const char* filename, const char* mode);
    int fclose(void* stream);
}

// Проверка обнаружения простых утечек памяти внутри функций

// CHECK: warning: potential memory leak detected at line [[#]]
void test_calloc_simple_leak() {
    int *p = (int*)calloc(10, sizeof(int));
}

// CHECK-DAG: warning: potential memory leak detected at line [[#]]
void test_new_simple_leak() {
    int* p = new int;
}

// CHECK-DAG: warning: potential file handle leak detected at line [[#]]
void test_fopen_simple_leak() {
    void* f = fopen("test.txt", "r");
}

// CHECK-DAG: warning: potential memory leak detected at line [[#]]
void test_malloc_simple_leak() {
    int* p = (int*)malloc(sizeof(int));
}


// Проверка обнаружения простых утечек памяти на глобальном уровне

// CHECK-DAG: warning: potential memory leak detected at line [[#]]
int* leak_malloc = (int*)malloc(100);

// CHECK-DAG: warning: potential memory leak detected at line [[#]]
int* leak_сalloc = (int*)calloc(10, sizeof(int));

// CHECK-DAG: warning: potential memory leak detected at line [[#]]
int* leak_realloc = (int*)realloc(nullptr, 200);

// CHECK-DAG: warning: potential file handle leak detected at line [[#]]
void* file = fopen("test.txt", "r");

// CHECK-DAG: warning: potential memory leak detected at line [[#]]
int* leak_new1 = new int(42);

// CHECK-DAG: warning: potential memory leak detected at line [[#]]
int* leak_new2 = new int[100];


// Разные области видимости с одинаковыми именами переменных

void scope_test1() {
    // CHECK-DAG: warning: potential memory leak detected at line [[#]]
    int* p = (int*)malloc(10);  
    {
        int* p = (int*)malloc(20);  
        free(p);  
    }  
}

void scope_test2() {
    int* p = (int*)malloc(10);
    free(p);  
    {
        // CHECK-DAG: warning: potential memory leak detected at line [[#]]
        int* p = (int*)malloc(20);
    }
}

// Тесты на перезапись указателей

void overwrite_test() {
    // CHECK-DAG: warning: potential memory leak detected at line [[#]]
    int* p = (int*)malloc(10);
    p = (int*)malloc(20); 
    free(p); 
}

void overwrite_correct() {
    int* p1 = (int*)malloc(10);
    int* p2 = (int*)malloc(20);
    p1 = p2; 
    free(p1);
    // CHECK-DAG: warning: potential memory leak detected at line [[#]]
}

// Выделение памяти в сложном выражении
void complex_expression_leak() {
    // CHECK-DAG: warning: potential memory leak detected at line [[#]]
    int* p = (int*)malloc(sizeof(int) * (10 + 20));
}

// Множественное выделение в одной строке
void multiple_allocation_same_line() {
    // CHECK-DAG: warning: potential memory leak detected at line [[#]]
    int* a = (int*)malloc(10), *b = (int*)malloc(20);
    free(a); 
}

// Утечка памяти в if
void if_leak(int x) {
    if (x > 0) {
        // CHECK-DAG: warning: potential memory leak detected at line [[#]]
        int* p = (int*)malloc(10);
    }    
}

// Выделение памяти в цикле, утечка в каждой итерации
void loop_leak() {
    for (int i = 0; i < 5; i++) {
        // CHECK-DAG: warning: potential memory leak detected at line [[#]]
        int* p = (int*)malloc(10);
    }
}

// тесты без утечек


// CHECK-NOT: warning: potential memory leak detected at line [[#]]
void correct_malloc() {
    int* p = (int*)malloc(100);
    free(p);
}

// CHECK-NOT: warning: potential memory leak detected at line [[#]]
void correct_calloc() {
    int* p = (int*)calloc(10, sizeof(int));
    free(p);
}

// CHECK-NOT: warning: potential file handle leak detected at line [[#]]
void correct_fopen() {
    void* f = fopen("test.txt", "r");
    fclose(f);
}

// CHECK-NOT: warning: potential memory leak detected at line [[#]]
void correct_new1() {
    int* p = new int(64);
    delete p;
}

// CHECK-NOT: warning: potential memory leak detected at line [[#]]
void correct_new2() {
    int* p = new int[10];
    delete[] p;
}

// CHECK-NOT: warning: potential memory leak detected at line [[#]]
void correct_multiple() {
    int* a = (int*)malloc(10);
    int* b = new int(5);
    void* f = fopen("test.txt", "r"); 
    free(a);
    delete b;
    fclose(f);
}

// CHECK-NOT: warning: potential memory leak detected at line [[#]]
void correct_if(int x) {
    int* p = (int*)malloc(10);
    if (x > 0) {
        free(p);
    } else {
        free(p);
    }
}

// CHECK-NOT: warning: potential memory leak detected at line [[#]]
void loop_correct() {
    for (int i = 0; i < 5; i++) {
        int* p = (int*)malloc(10);
        free(p);
    }
}

// CHECK-NOT: warning: potential memory leak detected at line [[#]]
int* correct_return(int x) {
    int* p = new int(x);
    return p;
}

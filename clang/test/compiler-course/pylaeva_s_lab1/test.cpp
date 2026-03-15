// RUN: %clang_cc1 -load %llvmshlibdir/pylaeva_s_lab1_ClangAST%pluginext -plugin pylaeva_s_lab1_plugin -fsyntax-only %s 2>&1 | FileCheck %s

extern "C" {
    void* malloc(unsigned long size);
    void* calloc(unsigned long count, unsigned long size);
    void* realloc(void* ptr, unsigned long size);
    void free(void* ptr);
    
    void* fopen(const char* filename, const char* mode);
    int fclose(void* stream);
}

// ==================== ТЕСТ 1: Базовые утечки памяти (malloc/calloc/realloc) ====================
// Проверка обнаружения простых утечек памяти

// CHECK: Потенциальная утечка memory: ресурс выделен в строке [[#@LINE+2]] не освобожден
void test_malloc_leak() {
    int *p = (int*)malloc(sizeof(int) * 10);
    *p = 42;
}

// CHECK: Потенциальная утечка memory: ресурс выделен в строке [[#@LINE+2]] не освобожден
void test_calloc_leak() {
    int *p = (int*)calloc(10, sizeof(int));
}

// CHECK: Потенциальная утечка memory: ресурс выделен в строке [[#@LINE+3]] не освобожден
void test_realloc_leak() {
    int *p = (int*)malloc(sizeof(int) * 5);
    p = (int*)realloc(p, sizeof(int) * 10);
}

// ==================== ТЕСТ 2: Базовые утечки файлов (fopen) ====================
// Проверка обнаружения простых утечек файлов

// CHECK: Потенциальная утечка file: ресурс выделен в строке [[#@LINE+2]] не освобожден
void test_fopen_leak() {
    void *f = fopen("test.txt", "r");
}

// ==================== ТЕСТ 3: Присваивания глобальным переменным ====================
// Проверка работы с глобальными переменными

int* global_ptr;

// CHECK: Потенциальная утечка memory: ресурс выделен в строке [[#@LINE+2]] не освобожден
void test_global_assign() {
    global_ptr = (int*)malloc(sizeof(int) * 10);
}

// ==================== ТЕСТ 4: Присваивания локальным переменным ====================
// Проверка присваивания уже объявленным локальным переменным

// CHECK: Потенциальная утечка memory: ресурс выделен в строке [[#@LINE+3]] не освобожден
void test_assign_to_local() {
    int *local;
    local = (int*)malloc(sizeof(int) * 10);
}

// ==================== ТЕСТ 5: Присваивания в условиях ====================
// Проверка присваивания внутри условных операторов

// CHECK: Потенциальная утечка memory: ресурс выделен в строке [[#@LINE+4]] не освобожден
void test_assign_in_condition(int cond) {
    int *p;
    if (cond > 0) {
        p = (int*)malloc(sizeof(int) * 10);
    }
}

// ==================== ТЕСТ 6: Утечки в ветвлениях ====================
// Проверка освобождения только в одной ветке условия

// CHECK: Потенциальная утечка memory: ресурс выделен в строке [[#@LINE+2]] не освобожден
void test_branch_leak(int cond) {
    int *p = (int*)malloc(sizeof(int) * 10);
    if (cond > 0) {
        free(p);
    }
}

// ==================== ТЕСТ 7: Ранний возврат из функции ====================
// Проверка утечек при ранних return без освобождения

// CHECK: Потенциальная утечка memory: ресурс выделен в строке [[#@LINE+2]] не освобожден
void test_early_return_leak(int cond) {
    int *p = (int*)malloc(sizeof(int) * 10);
    if (cond < 0) {
        return;
    }
    free(p);
}

// ==================== ТЕСТ 8: Контекст функций ====================
// Проверка, что контексты функций не смешиваются

// CHECK: Потенциальная утечка memory: ресурс выделен в строке [[#@LINE+2]] не освобожден
void helper_func() {
    int *p = (int*)malloc(sizeof(int) * 10);
}

void caller_func() {
    int *p = (int*)malloc(sizeof(int) * 20);
    free(p);
    helper_func();
}

// ==================== ТЕСТ 9: Выделение без присваивания ====================
// Проверка случаев, когда malloc/fopen используются без сохранения результата

// CHECK: Потенциальная утечка memory: ресурс выделен в строке [[#@LINE+2]] не освобожден
void test_malloc_no_assign() {
    malloc(sizeof(int) * 10);
}

// CHECK: Потенциальная утечка file: ресурс выделен в строке [[#@LINE+2]] не освобожден
void test_fopen_no_assign() {
    fopen("test.txt", "r");
}

// ==================== ТЕСТ 10: Переприсваивание переменной ====================
// Проверка обнаружения утечки при переприсваивании указателя

// CHECK: Потенциальная утечка memory: ресурс выделен в строке [[#@LINE+2]] не освобожден
void test_reassignment_leak() {
    int *p = (int*)malloc(sizeof(int) * 10);  // первая утечка
    p = (int*)malloc(sizeof(int) * 20);       // второе выделение
    free(p);  // освобождается только второе выделение
}

// ==================== ТЕСТ 11: Корректное освобождение ====================
// Проверка, что нет ложных срабатываний при правильном освобождении

void test_malloc_free() {
    int *p = (int*)malloc(sizeof(int) * 10);
    free(p);
}

void test_fopen_fclose() {
    void *f = fopen("test.txt", "r");
    fclose(f);
}

void test_multiple_alloc_free() {
    int *p1 = (int*)malloc(sizeof(int) * 10);
    int *p2 = (int*)malloc(sizeof(int) * 20);
    free(p1);
    free(p2);
}

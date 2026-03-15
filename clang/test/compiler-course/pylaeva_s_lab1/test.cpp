// RUN: %clang_cc1 -load %llvmshlibdir/pylaeva_s_lab1_ClangAST%pluginext -plugin pylaeva_s_lab1_plugin -fsyntax-only %s 2>&1 | FileCheck %s

extern "C" {
    void* malloc(unsigned long size);
    void* calloc(unsigned long count, unsigned long size);
    void* realloc(void* ptr, unsigned long size);
    void free(void* ptr);
    
    void* fopen(const char* filename, const char* mode);
    int fclose(void* stream);
}

// ==================== ТЕСТ 1: Базовые утечки ====================
// Проверка обнаружения простых утечек

// CHECK: Потенциальная утечка memory: ресурс выделен в строке [[#LINE:]] не освобожден
void test_malloc_leak() {
    int *p = (int*)malloc(sizeof(int) * 10);
    *p = 42;
}

// CHECK: Потенциальная утечка memory: ресурс выделен в строке [[#LINE:]] не освобожден
void test_calloc_leak() {
    int *p = (int*)calloc(10, sizeof(int));
}

// CHECK: Потенциальная утечка memory: ресурс выделен в строке [[#LINE:]] не освобожден
void test_realloc_leak() {
    int *p = (int*)malloc(sizeof(int) * 5);
    p = (int*)realloc(p, sizeof(int) * 10);
}

// CHECK: Потенциальная утечка file: ресурс выделен в строке [[#LINE:]] не освобожден
void test_fopen_leak() {
    void *f = fopen("test.txt", "r");
}


// ==================== ТЕСТ 3: Присваивания вне функций ====================
// Проверка работы с глобальными переменными и присваиваниями

int* global_ptr;

// CHECK: Потенциальная утечка memory: ресурс выделен в строке [[#LINE:]] не освобожден
void test_global_assign() {
    global_ptr = (int*)malloc(sizeof(int) * 10);
}

// CHECK: Потенциальная утечка memory: ресурс выделен в строке [[#LINE:]] не освобожден
void test_assign_to_local() {
    int *local;
    local = (int*)malloc(sizeof(int) * 10);  // присваивание уже объявленной переменной
}

// CHECK: Потенциальная утечка memory: ресурс выделен в строке [[#LINE:]] не освобожден
void test_assign_in_condition(int cond) {
    int *p;
    if (cond > 0) {
        p = (int*)malloc(sizeof(int) * 10);  // присваивание в ветке
    }
    // нет освобождения
}

// ==================== ТЕСТ 4: Переприсваивание переменных ====================
// Проверка обнаружения утечек при переиспользовании переменных

// CHECK: Потенциальная утечка memory: ресурс выделен в строке [[#LINE:]] не освобожден
// CHECK: Потенциальная утечка memory: ресурс выделен в строке [[#LINE:]] не освобожден
void test_reassignment_leak() {
    int *p = (int*)malloc(sizeof(int) * 10);  
    p = (int*)malloc(sizeof(int) * 20);       
    free(p);  // освобождается только второе выделение
}

// CHECK: Потенциальная утечка memory: ресурс выделен в строке [[#LINE:]] не освобожден
void test_reassignment_in_loop() {
    int *p = nullptr;
    for (int i = 0; i < 3; i++) {
        p = (int*)malloc(sizeof(int) * 10);
        // каждый раз теряем предыдущий указатель
    }
    free(p);  // освобождается только последний
}

// ==================== ТЕСТ 5: Ветвления и пути выполнения ====================
// Проверка анализа разных веток выполнения

// CHECK: Потенциальная утечка memory: ресурс выделен в строке [[#LINE:]] не освобожден
void test_branch_leak(int cond) {
    int *p = (int*)malloc(sizeof(int) * 10);
    if (cond > 0) {
        free(p);  // освобождение только в одной ветке
    }
    // в другой ветке - утечка
}

// CHECK: Потенциальная утечка memory: ресурс выделен в строке [[#LINE:]] не освобожден
void test_early_return_leak(int cond) {
    int *p = (int*)malloc(sizeof(int) * 10);
    if (cond < 0) {
        return;  // ранний возврат без освобождения
    }
    free(p);
}

// ==================== ТЕСТ 8: Проверка контекста функций ====================
// Проверка, что контексты функций не смешиваются

// CHECK: Потенциальная утечка memory: ресурс выделен в строке [[#LINE:]] не освобожден
void helper_func() {
    int *p = (int*)malloc(sizeof(int) * 10);  // утечка во вспомогательной функции
}

void caller_func() {
    int *p = (int*)malloc(sizeof(int) * 20);
    free(p);  // здесь все правильно
    helper_func();  // а здесь утечка
}

// ==================== ТЕСТ 9: Присваивание без объявления ====================
// Проверка случаев, когда malloc используется без присваивания переменной

// CHECK: Потенциальная утечка memory: ресурс выделен в строке [[#LINE:]] не освобожден
void test_malloc_no_assign() {
    malloc(sizeof(int) * 10);  // выделили и потеряли указатель сразу
}

// CHECK: Потенциальная утечка file: ресурс выделен в строке [[#LINE:]] не освобожден
void test_fopen_no_assign() {
    fopen("test.txt", "r");  // открыли и потеряли указатель
}

// ==================== ТЕСТ 10: Множественные утечки в одной функции ====================

// CHECK: Потенциальная утечка memory: ресурс выделен в строке [[#LINE:]] не освобожден
// CHECK: Потенциальная утечка memory: ресурс выделен в строке [[#LINE:]] не освобожден
void test_multiple_leaks() {
    int *p1 = (int*)malloc(sizeof(int) * 10);
    int *p2 = (int*)malloc(sizeof(int) * 20);
    void *f1 = fopen("test1.txt", "r");
    void *f2 = fopen("test2.txt", "r");
    
    free(p1);  // освободили только p1
    fclose(f1);  // закрыли только f1
    // p2 и f2 остались - должны быть предупреждения
}


// ==================== ТЕСТ : Корректное освобождение ====================
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

int* test_return_resource(int cond) {
    int *p = (int*)malloc(sizeof(int) * 10);
    if (cond > 0) {
        return p;  // возврат ресурса - ок
    }
    free(p);
    return nullptr;
}
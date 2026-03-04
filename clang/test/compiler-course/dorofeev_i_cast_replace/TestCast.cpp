// RUN: %clang_cc1 -load %llvmshlibdir/dorofeev_i_cast_replace_ClangAST%pluginext -plugin cast_replace_plugin -fsyntax-only %s 2>&1 | FileCheck %s

void test_casts() {
    int a = 5;
    
    // CHECK: double b = static_cast<double>(a);
    double b = (double)a;

    // CHECK: int *p = reinterpret_cast<int *>(a);
    int *p = (int *)a;

    const int c = 10;
    
    // CHECK: int *q = const_cast<int *>(&c);
    int *q = (int *)&c;
}
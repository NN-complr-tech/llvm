// RUN: %clang_cc1 -load %llvmshlibdir/VarStatPlugin_Kurpiakov_Aleksei_FIIT3_ClangAST.so%pluginext -plugin example_plugin -fsyntax-only %s 2>&1 | FileCheck %s

//CHECK: Global variables : *
//CHECK-NEXT: Static variables : *
//CHECK-NEXT: Local variables  : *
//CHECK-NEXT: Function params  : *

static int x = 0;
long y = 0L;

template<typename Y>
class Class{
public:
    int a;
    Y k;
};

char foo(int a, int b){
    int z = 42 + static_cast<char>(x) + static_cast<char>(y);
    return z;
}

int main(){
    int a;
    int b;
    Class<int> c_int;
    Class<float> c_float;
    char z = foo(a, b);
    static constexpr int var = 52;
    static int var2  = 52;
    return 0;
}
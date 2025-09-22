; RUN: split-file %s %t
; RUN: opt -load-pass-plugin %llvmshlibdir/add_replace_pass_Shishkarev_Andrey_FIIT2_LLVM_IR%pluginext -passes="add_replace_pass" -S %t/a.ll | FileCheck %t/a.ll
; RUN: opt -load-pass-plugin %llvmshlibdir/add_replace_pass_Shishkarev_Andrey_FIIT2_LLVM_IR%pluginext -passes="add_replace_pass" -S %t/a.ll | FileCheck %t/b.ll

;--- a.ll
; CHECK-LABEL: @add
define i32 @add(i32 %a, i32 %b) {
  %result = add i32 %a, %b
  ret i32 %result
}

; CHECK-LABEL: @foo
; CHECK-NOT: add i32 %x, %y
; CHECK: call i32 @add(i32 %x, i32 %y)
define i32 @foo(i32 %x, i32 %y) {
  %sum = add i32 %x, %y
  ret i32 %sum
}

;--- b.ll
; Тест с различными типами
define i64 @add(i64 %a, i64 %b) {
  %result = add i64 %a, %b
  ret i64 %result
}

define i64 @bar(i64 %x, i64 %y) {
  %sum = add i64 %x, %y
  ret i64 %sum
}

; Функция add с неподходящей сигнатурой (не должна заменяться)
define void @add_void(i32 %a, i32 %b) {
  ret void
}

define i32 @baz(i32 %x, i32 %y) {
  ; Эта операция add не должна заменяться, т.к. типы не совпадают с add_void
  %sum = add i32 %x, %y
  ret i32 %sum
}
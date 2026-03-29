; RUN: opt -load-pass-plugin %llvmshlibdir/nikitina_v_lab2_LLVM_IR%pluginext\
; RUN: -passes=mul-div-to-shift -S %s | FileCheck %s

define i32 @test_mul(i32 %0) {
; CHECK-LABEL: @test_mul
; CHECK: %shl_opt = shl i32 %0, 3
  %2 = mul i32 %0, 8
  ret i32 %2
}

define i32 @test_mul_inv(i32 %0) {
; CHECK-LABEL: @test_mul_inv
; CHECK: %shl_opt = shl i32 %0, 2
  %2 = mul i32 4, %0
  ret i32 %2
}

define i32 @test_udiv(i32 %0) {
; CHECK-LABEL: @test_udiv
; CHECK: %lshr_opt = lshr i32 %0, 4
  %2 = udiv i32 %0, 16
  ret i32 %2
}

define i32 @test_sdiv(i32 %0) {
; CHECK-LABEL: @test_sdiv
; CHECK: %ashr_opt = ashr i32 %0, 1
  %2 = sdiv i32 %0, 2
  ret i32 %2
}

define i32 @test_no_opt(i32 %0) {
; CHECK-LABEL: @test_no_opt
; CHECK: %2 = sdiv i32 %0, 7
  %2 = sdiv i32 %0, 7
  ret i32 %2
}
; RUN: opt -load-pass-plugin %llvmshlibdir/nikitina_v_lab2_LLVM_IR%pluginext -passes=mul-div-to-shift -S %s | FileCheck %s

define i32 @test_mul(i32 %a) {
; CHECK-LABEL: define i32 @test_mul(i32 %a)
; CHECK-NEXT:    %shl_opt = shl i32 %a, 3
; CHECK-NEXT:    ret i32 %shl_opt
  %res = mul i32 %a, 8
  ret i32 %res
}


define i32 @test_mul_inv(i32 %a) {
; CHECK-LABEL: define i32 @test_mul_inv(i32 %a)
; CHECK-NEXT:    %shl_opt = shl i32 %a, 2
; CHECK-NEXT:    ret i32 %shl_opt
  %res = mul i32 4, %a
  ret i32 %res
}


define i32 @test_udiv(i32 %a) {
; CHECK-LABEL: define i32 @test_udiv(i32 %a)
; CHECK-NEXT:    %lshr_opt = lshr i32 %a, 4
; CHECK-NEXT:    ret i32 %lshr_opt
  %res = udiv i32 %a, 16
  ret i32 %res
}

define i32 @test_sdiv(i32 %a) {
; CHECK-LABEL: define i32 @test_sdiv(i32 %a)
; CHECK-NEXT:    %ashr_opt = ashr i32 %a, 1
; CHECK-NEXT:    ret i32 %ashr_opt
  %res = sdiv i32 %a, 2
  ret i32 %res
}


define i32 @test_mul_negative(i32 %a) {
; CHECK-LABEL: define i32 @test_mul_negative(i32 %a)
; CHECK-NEXT:    %res = mul i32 %a, -8
; CHECK-NEXT:    ret i32 %res
  %res = mul i32 %a, -8
  ret i32 %res
}

define i32 @test_sdiv_negative(i32 %a) {
; CHECK-LABEL: define i32 @test_sdiv_negative(i32 %a)
; CHECK-NEXT:    %res = sdiv i32 %a, -4
; CHECK-NEXT:    ret i32 %res
  %res = sdiv i32 %a, -4
  ret i32 %res
}


define i32 @test_no_opt(i32 %a) {
; CHECK-LABEL: define i32 @test_no_opt(i32 %a)
; CHECK-NEXT:    %res = sdiv i32 %a, 7
; CHECK-NEXT:    ret i32 %res
  %res = sdiv i32 %a, 7
  ret i32 %res
}
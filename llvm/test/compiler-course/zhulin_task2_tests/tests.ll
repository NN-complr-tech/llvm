; RUN: opt -load-pass-plugin %llvmshlibdir/zhulin_task2_LLVM_IR%pluginext -passes=scalarize -S %s | FileCheck %s

; CHECK: extractelement <4 x i32>
; CHECK: insertelement <4 x i32>
; CHECK: ret <4 x i32>

define <4 x i32> @test_add(<4 x i32> %a, <4 x i32> %b) {
  %res = add nsw <4 x i32> %a, %b
  ret <4 x i32> %res
}

; CHECK: mul nuw i32
; CHECK: ret <4 x i32>

define <4 x i32> @test_mul(<4 x i32> %a, <4 x i32> %b) {
  %res = mul nuw <4 x i32> %a, %b
  ret <4 x i32> %res
}

; CHECK: extractelement <2 x i32>
; CHECK: insertelement <2 x i32>
; CHECK: ret <2 x i32>

define <2 x i32> @test_v2i32(<2 x i32> %a, <2 x i32> %b) {
  %res = add <2 x i32> %a, %b
  ret <2 x i32> %res
}

; CHECK: add <8 x i32>
; CHECK-NOT: extractelement <8 x i32>

define <8 x i32> @test_no_change(<8 x i32> %a, <8 x i32> %b) {
  %res = add <8 x i32> %a, %b
  ret <8 x i32> %res
}

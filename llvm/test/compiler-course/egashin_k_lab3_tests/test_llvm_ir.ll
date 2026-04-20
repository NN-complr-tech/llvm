; RUN: opt -load-pass-plugin %llvmshlibdir/egashin_k_lab3_LLVM_IR%pluginext \
; RUN:   -passes=limited-loop-unroll -S %s | FileCheck %s

define void @unroll_three_iterations(ptr %out) {
; CHECK-LABEL: define void @unroll_three_iterations(
; CHECK-NOT: phi i32
; CHECK-NOT: br i1 %done, label %exit, label %loop
; CHECK: store i32
; CHECK: store i32
; CHECK: store i32
; CHECK: ret void
entry:
  br label %loop

loop:
  %i = phi i32 [ 0, %entry ], [ %next, %loop ]
  store i32 %i, ptr %out, align 4
  %next = add nuw nsw i32 %i, 1
  %done = icmp eq i32 %next, 3
  br i1 %done, label %exit, label %loop

exit:
  ret void
}

define void @unroll_five_iterations(ptr %out) {
; CHECK-LABEL: define void @unroll_five_iterations(
; CHECK-NOT: phi i32
; CHECK-NOT: br i1 %done, label %exit, label %loop
; CHECK: store i32
; CHECK: store i32
; CHECK: store i32
; CHECK: store i32
; CHECK: store i32
; CHECK: ret void
entry:
  br label %loop

loop:
  %i = phi i32 [ 0, %entry ], [ %next, %loop ]
  store i32 %i, ptr %out, align 4
  %next = add nuw nsw i32 %i, 1
  %done = icmp eq i32 %next, 5
  br i1 %done, label %exit, label %loop

exit:
  ret void
}

define void @keep_six_iterations(ptr %out) {
; CHECK-LABEL: define void @keep_six_iterations(
; CHECK: %i = phi i32 [ 0, %entry ], [ %next, %loop ]
; CHECK: store i32 %i, ptr %out, align 4
; CHECK: br i1 %done, label %exit, label %loop
entry:
  br label %loop

loop:
  %i = phi i32 [ 0, %entry ], [ %next, %loop ]
  store i32 %i, ptr %out, align 4
  %next = add nuw nsw i32 %i, 1
  %done = icmp eq i32 %next, 6
  br i1 %done, label %exit, label %loop

exit:
  ret void
}

define void @keep_runtime_trip_count(i32 %n, ptr %out) {
; CHECK-LABEL: define void @keep_runtime_trip_count(
; CHECK: %i = phi i32 [ 0, %entry ], [ %next, %loop ]
; CHECK: store i32 %i, ptr %out, align 4
; CHECK: %done = icmp eq i32 %next, %n
; CHECK: br i1 %done, label %exit, label %loop
entry:
  br label %loop

loop:
  %i = phi i32 [ 0, %entry ], [ %next, %loop ]
  store i32 %i, ptr %out, align 4
  %next = add nuw nsw i32 %i, 1
  %done = icmp eq i32 %next, %n
  br i1 %done, label %exit, label %loop

exit:
  ret void
}

define void @mixed_nested_runtime_outer(i32 %n, ptr %out) {
; CHECK-LABEL: define void @mixed_nested_runtime_outer(
; CHECK: %i = phi i32 [ 0, %entry ], [ %i.next, %{{[^]]+}} ]
; CHECK-NOT: %j = phi i32
; CHECK-NOT: br i1 %inner.done, label %outer.latch, label %inner.header
; CHECK: store i32 1, ptr %out, align 4
; CHECK: store i32 1, ptr %out, align 4
; CHECK: %outer.done = icmp eq i32 %i.next, %n
; CHECK: br i1 %outer.done, label %exit, label %outer.header
entry:
  br label %outer.header

outer.header:
  %i = phi i32 [ 0, %entry ], [ %i.next, %outer.latch ]
  br label %inner.header

inner.header:
  %j = phi i32 [ 0, %outer.header ], [ %j.next, %inner.header ]
  store i32 1, ptr %out, align 4
  %j.next = add nuw nsw i32 %j, 1
  %inner.done = icmp eq i32 %j.next, 2
  br i1 %inner.done, label %outer.latch, label %inner.header

outer.latch:
  %i.next = add nuw nsw i32 %i, 1
  %outer.done = icmp eq i32 %i.next, %n
  br i1 %outer.done, label %exit, label %outer.header

exit:
  ret void
}

define void @unroll_nested_loops(ptr %out) {
; CHECK-LABEL: define void @unroll_nested_loops(
; CHECK-NOT: phi i32
; CHECK-NOT: br i1 %inner.done, label %outer.latch, label %inner.header
; CHECK-NOT: br i1 %outer.done, label %exit, label %outer.header
; CHECK: store i32 1, ptr %out, align 4
; CHECK: store i32 1, ptr %out, align 4
; CHECK: store i32 1, ptr %out, align 4
; CHECK: store i32 1, ptr %out, align 4
; CHECK: store i32 1, ptr %out, align 4
; CHECK: store i32 1, ptr %out, align 4
; CHECK: ret void
entry:
  br label %outer.header

outer.header:
  %i = phi i32 [ 0, %entry ], [ %i.next, %outer.latch ]
  br label %inner.header

inner.header:
  %j = phi i32 [ 0, %outer.header ], [ %j.next, %inner.header ]
  store i32 1, ptr %out, align 4
  %j.next = add nuw nsw i32 %j, 1
  %inner.done = icmp eq i32 %j.next, 3
  br i1 %inner.done, label %outer.latch, label %inner.header

outer.latch:
  %i.next = add nuw nsw i32 %i, 1
  %outer.done = icmp eq i32 %i.next, 2
  br i1 %outer.done, label %exit, label %outer.header

exit:
  ret void
}

add_subdirectory(llvm-ir)
add_subdirectory(backend)
set(LLVM_TEST_DEPENDS ${LLVM_TEST_DEPENDS} PARENT_SCOPE)

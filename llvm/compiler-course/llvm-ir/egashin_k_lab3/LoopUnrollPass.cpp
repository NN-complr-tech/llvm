#include "llvm/Transforms/Scalar/LoopUnrollPass.h"
#include "llvm/IR/Function.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

using namespace llvm;

namespace {

LoopUnrollOptions makeUnrollOptions() {
  LoopUnrollOptions Options;
  Options.setPartial(false)
      .setPeeling(false)
      .setRuntime(false)
      .setUpperBound(false)
      .setProfileBasedPeeling(false)
      .setFullUnrollMaxCount(5);
  return Options;
}

struct LimitedLoopUnrollPass : PassInfoMixin<LimitedLoopUnrollPass> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
    LoopUnrollPass Pass(makeUnrollOptions());
    return Pass.run(F, AM);
  }

  static bool isRequired() { return true; }
};

} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "LimitedLoopUnrollPass", "v1.0",
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "limited-loop-unroll") {
                    FPM.addPass(LimitedLoopUnrollPass());
                    return true;
                  }
                  return false;
                });
          }};
}

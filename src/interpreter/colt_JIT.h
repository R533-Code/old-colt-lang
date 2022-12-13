#ifndef HG_COLT_JIT
#define HG_COLT_JIT

#include <llvm/ADT/StringRef.h>
#include <llvm/ExecutionEngine/JITSymbol.h>
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/Core.h>
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/ExecutionEngine/Orc/ExecutorProcessControl.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/IRTransformLayer.h>
#include <llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h>
#include <llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Support/TargetSelect.h>
#include <memory>
#include <code_gen/llvm_ir_gen.h>

namespace colt::gen
{
  class ColtJIT
  {
    std::unique_ptr<llvm::orc::LLLazyJIT> JIT;

  public:
    ColtJIT() = delete;
    ColtJIT(std::unique_ptr<llvm::orc::LLLazyJIT> JIT) noexcept
      : JIT(std::move(JIT)) {}

    llvm::Error addModule(GeneratedIR&& IR) noexcept
    {      
      if (auto err = JIT->addLazyIRModule(llvm::orc::ThreadSafeModule{ std::move(IR.module), std::move(IR.context)}))
        return err;
      return llvm::Error::success();
    }

    llvm::Expected<llvm::orc::ExecutorAddr> lookup(llvm::StringRef str) noexcept
    {
      return JIT->lookup(str);
    }

    static llvm::Expected<std::unique_ptr<ColtJIT>> Create() noexcept
    {
      using namespace llvm;

      auto JIT = orc::LLLazyJITBuilder().create();
      if (!JIT)
        return JIT.takeError();
      const DataLayout& DL = (*JIT)->getDataLayout();
      auto DLSG = orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(DL.getGlobalPrefix());
      if (!DLSG)
        return DLSG.takeError();
      (*JIT)->getMainJITDylib().addGenerator(std::move(*DLSG));

      return std::make_unique<ColtJIT>(std::move(*JIT));
    }
  };
}

#endif //!HG_COLT_JIT
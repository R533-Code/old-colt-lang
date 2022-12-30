/** @file colt_JIT.h
* Contains the Colt LLVM JIT interpreter.
*/

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
  /// @brief An LLVM JIT interpreter
  class ColtJIT
  {
    /// @brief Pointer to the JIT
    std::unique_ptr<llvm::orc::LLLazyJIT> JIT;

  public:
    ColtJIT() = delete;
    /// @brief Constructor
    /// @param JIT The JIT to store
    ColtJIT(std::unique_ptr<llvm::orc::LLLazyJIT> JIT) noexcept
      : JIT(std::move(JIT)) {}

    /// @brief Adds generated IR to compile
    /// @param IR The IR to compile
    /// @return success if no error are encountered
    llvm::Error addModule(GeneratedIR&& IR) noexcept
    {      
      if (auto err = JIT->addLazyIRModule(llvm::orc::ThreadSafeModule{ std::move(IR.module), std::move(IR.context)}))
        return err;
      return llvm::Error::success();
    }

    /// @brief Lookups a symbol in the generated code
    /// @param str The name of the symbol
    /// @return The symbol if found or error
    llvm::Expected<llvm::orc::ExecutorAddr> lookup(llvm::StringRef str) noexcept
    {
      return JIT->lookup(str);
    }

    /// @brief Creates an instance of the JIT
    /// @return A JIT if no error was generated
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
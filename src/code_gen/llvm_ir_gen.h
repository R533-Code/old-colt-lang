#ifndef HG_LLVM_IR_GENERATOR
#define HG_LLVM_IR_GENERATOR

#ifndef COLT_NO_LLVM

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/ToolOutputFile.h>
#include <llvm/Target/TargetOptions.h>

#include <util/colt_pch.h>
#include <type/colt_type.h>
#include <ast/colt_ast.h>
#include <code_gen/mangle.h>

/// @brief Contains classes responsible of producing code from the Colt AST
namespace colt::gen
{
	/// @brief Converts a StringView to a StringRef
	/// @param view The StringView to convert
	/// @return The converted StringRef
	llvm::StringRef ToStringRef(colt::StringView view) noexcept;

	struct GeneratedIR
	{
		std::unique_ptr<llvm::LLVMContext> context = std::make_unique<llvm::LLVMContext>();
		std::unique_ptr<llvm::Module> module = std::make_unique<llvm::Module>("Colt", *context);
		PTR<llvm::TargetMachine> target_machine;

		void print_module(llvm::raw_ostream& os = llvm::errs()) const noexcept;

		Expected<bool, const char*> to_object_file(const char* path) noexcept;

		void optimize(colt::gen::OptimizationLevel level) noexcept;
	};	

	Expected<GeneratedIR, std::string> GenerateIR(const lang::AST& ast, const std::string& target_triple = LLVM_DEFAULT_TARGET_TRIPLE) noexcept;

	/// @brief Class responsible of generating LLVM IR
	class LLVMIRGenerator
	{
		/// @brief The LLVM context
		llvm::LLVMContext& context;
		/// @brief The LLVM Module
		llvm::Module& module;
		/// @brief The helper for generating IR
		llvm::IRBuilder<> builder;
		/// @brief Function map
		Map<PTR<const lang::FnDeclExpr>, PTR<llvm::Function>> function_map;
		/// @brief Contains all global variables
		Map<StringView, PTR<llvm::GlobalVariable>> global_vars{};
		/// @brief The IR to generate before the code in main
		Vector<PTR<llvm::Value>> call_before_main{};
		/// @brief Contains all local variables
		Vector<PTR<llvm::AllocaInst>> local_vars{};
		/// @brief Contains the result of visiting an expression
		PTR<llvm::Value> returned_value = nullptr;
		/// @brief Contains the current function whose IR is being generated
		PTR<llvm::Function> current_fn = nullptr;
		/// @brief Current loop begin block (used for continue)
		PTR<llvm::BasicBlock> loop_begin = nullptr;

	public:
		/// @brief No default constructor
		LLVMIRGenerator() = delete;
		/// @brief No default copy constructor
		LLVMIRGenerator(const LLVMIRGenerator&) = delete;
		/// @brief No default move constructor
		LLVMIRGenerator(LLVMIRGenerator&&) = delete;

		/// @brief Generate LLVM IR from expressions
		/// @param ast The AST to compile to IR
		/// @param level The optimization level
		LLVMIRGenerator(const lang::AST& ast, llvm::LLVMContext& ctx, llvm::Module& mod) noexcept;

	private:
		void gen_ir(PTR<const lang::Expr> ptr) noexcept;

		void gen_literal(PTR<const lang::LiteralExpr> ptr) noexcept;

		void gen_unary(PTR<const lang::UnaryExpr> ptr) noexcept;

		void gen_binary(PTR<const lang::BinaryExpr> ptr) noexcept;

		void gen_convert(PTR<const lang::ConvertExpr> ptr) noexcept;

		void gen_var_decl(PTR<const lang::VarDeclExpr> ptr) noexcept;
		
		void gen_var_read(PTR<const lang::VarReadExpr> ptr) noexcept;

		void gen_var_write(PTR<const lang::VarWriteExpr> ptr) noexcept;

		void gen_fn_def(PTR<const lang::FnDefExpr> ptr) noexcept;

		void gen_fn_ret(PTR<const lang::FnReturnExpr> ptr) noexcept;

		void gen_fn_call(PTR<const lang::FnCallExpr> ptr) noexcept;
		
		void gen_scope(PTR<const lang::ScopeExpr> ptr) noexcept;
		
		void gen_condition(PTR<const lang::ConditionExpr> ptr) noexcept;

		void gen_while_loop(PTR<const lang::WhileLoopExpr> ptr) noexcept;

		PTR<llvm::Type> type_to_llvm(PTR<const lang::Type> type) noexcept;
	};
}

#endif //!COLT_NO_LLVM

#endif //!HG_LLVM_IR_GENERATOR
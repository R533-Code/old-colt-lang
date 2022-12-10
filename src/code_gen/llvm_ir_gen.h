#ifndef HG_LLVM_IR_GENERATOR
#define HG_LLVM_IR_GENERATOR

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

#include <util/colt_pch.h>
#include <type/colt_type.h>
#include <ast/colt_ast.h>

/// @brief Contains classes responsible of producing code from the Colt AST
namespace colt::gen
{
	/// @brief Class responsible of generating LLVM IR
	class LLVMIRGenerator
	{
		/// @brief The LLVM context
		std::unique_ptr<llvm::LLVMContext> context = std::make_unique<llvm::LLVMContext>();
		/// @brief The helper for generating IR
		llvm::IRBuilder<> builder = llvm::IRBuilder<>(*context);
		/// @brief The LLVM module used in the current context
		std::unique_ptr<llvm::Module> module = std::make_unique<llvm::Module>("Colt", *context);
		/// @brief Contains all global variables
		Vector<llvm::GlobalVariable> global_vars;
		/// @brief Contains all local variables
		Vector<PTR<llvm::AllocaInst>> local_vars;
		/// @brief Contains the result of visiting an expression
		PTR<llvm::Value> returned_value = nullptr;
		/// @brief Contains the current function whose IR is being generated
		PTR<llvm::Function> current_fn = nullptr;

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
		LLVMIRGenerator(const lang::AST& ast, llvm::OptimizationLevel level = llvm::OptimizationLevel::O3) noexcept
		{
			module->setTargetTriple(LLVM_DEFAULT_TARGET_TRIPLE);

			for (size_t i = 0; i < ast.expressions.get_size(); i++)
				gen_ir(ast.expressions[i]);
			
			optimize(level);
		}
		
		void print_module() const noexcept;

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

		PTR<llvm::Type> type_to_llvm(PTR<const lang::Type> type) noexcept;

		/// @brief Optimizes the module's IR
		/// @param level The optimization level
		void optimize(llvm::OptimizationLevel level) noexcept;
	};
}

#endif //!HG_LLVM_IR_GENERATOR
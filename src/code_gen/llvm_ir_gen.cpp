#include <code_gen/llvm_ir_gen.h>
#include <ast/colt_ast.h>

/// @brief Contains code generators
namespace colt::gen
{
  using namespace llvm;

  void LLVMIRGenerator::gen_ir(PTR<const lang::Expr> ptr) noexcept
  {
    using namespace lang;

    switch (ptr->classof())
    {
    break; case Expr::EXPR_LITERAL:
      gen_literal(as<PTR<const LiteralExpr>>(ptr));
    break; case Expr::EXPR_UNARY:
      gen_unary(as<PTR<const UnaryExpr>>(ptr));
    break; case Expr::EXPR_BINARY:
      gen_binary(as<PTR<const BinaryExpr>>(ptr));
    break; case Expr::EXPR_CONVERT:
      gen_convert(as<PTR<const ConvertExpr>>(ptr));
    break; case Expr::EXPR_VAR_DECL:
      gen_var_decl(as<PTR<const VarDeclExpr>>(ptr));
    break; case Expr::EXPR_VAR_READ:
      gen_var_read(as<PTR<const VarReadExpr>>(ptr));
    break; case Expr::EXPR_VAR_WRITE:
      gen_var_write(as<PTR<const VarWriteExpr>>(ptr));
    break; case Expr::EXPR_FN_DEF:
      gen_fn_def(as<PTR<const FnDefExpr>>(ptr));
    break; case Expr::EXPR_FN_CALL:
      gen_fn_call(as<PTR<const FnCallExpr>>(ptr));
    break; case Expr::EXPR_FN_RETURN:
      gen_fn_ret(as<PTR<const FnReturnExpr>>(ptr));
    break; case Expr::EXPR_SCOPE:
      gen_scope(as<PTR<const ScopeExpr>>(ptr));
    break; case Expr::EXPR_CONDITION:
      gen_condition(as<PTR<const ConditionExpr>>(ptr));
    break; case Expr::EXPR_FOR_LOOP:
    break; case Expr::EXPR_WHILE_LOOP:
    break; case Expr::EXPR_BREAK_CONTINUE:    
    break; default:
      colt_unreachable("Generating invalid expression!");
    }
  }

  void LLVMIRGenerator::gen_literal(PTR<const lang::LiteralExpr> ptr) noexcept
  {
    using namespace colt::lang;

    switch (as<PTR<const lang::BuiltInType>>(ptr->get_type())->get_builtin_id())
    {
    break; case BuiltInType::U8:
      returned_value = ConstantInt::get(llvm::Type::getInt8Ty(*context), ptr->get_value().u8_v);
    break; case BuiltInType::U16:
      returned_value = ConstantInt::get(llvm::Type::getInt16Ty(*context), ptr->get_value().u16_v);
    break; case BuiltInType::U32:
      returned_value = ConstantInt::get(llvm::Type::getInt32Ty(*context), ptr->get_value().u32_v);
    break; case BuiltInType::U64:
      returned_value = ConstantInt::get(llvm::Type::getInt64Ty(*context), ptr->get_value().u64_v);
    break; case BuiltInType::U128:
      returned_value = ConstantInt::get(llvm::Type::getInt128Ty(*context), ptr->get_value().u64_v);
    break; case BuiltInType::I8:
      returned_value = ConstantInt::get(llvm::Type::getInt8Ty(*context), ptr->get_value().i8_v);
    break; case BuiltInType::I16:
      returned_value = ConstantInt::get(llvm::Type::getInt16Ty(*context), ptr->get_value().i16_v);
    break; case BuiltInType::I32:
      returned_value = ConstantInt::get(llvm::Type::getInt32Ty(*context), ptr->get_value().i32_v);
    break; case BuiltInType::I64:
      returned_value = ConstantInt::get(llvm::Type::getInt64Ty(*context), ptr->get_value().i64_v);
    break; case BuiltInType::I128:
      returned_value = ConstantInt::get(llvm::Type::getInt128Ty(*context), ptr->get_value().i64_v);
    break; case BuiltInType::F32:
      returned_value = ConstantFP::get(llvm::Type::getFloatTy(*context), ptr->get_value().float_v);
    break; case BuiltInType::F64:
      returned_value = ConstantFP::get(llvm::Type::getDoubleTy(*context), ptr->get_value().double_v);
    break; case BuiltInType::BOOL:
      returned_value = ConstantInt::get(llvm::Type::getInt1Ty(*context), ptr->get_value().bool_v);
    break; default:
      colt_unreachable("Invalid literal expr!");
    }
  }

  void LLVMIRGenerator::gen_unary(PTR<const lang::UnaryExpr> ptr) noexcept
  {
    gen_ir(ptr->get_child());
    Value* child = returned_value;

    using namespace colt::lang;

    switch (ptr->get_operation())
    {
    break; case UnaryOperator::OP_PRE_INCREMENT:     
    break; case UnaryOperator::OP_POST_INCREMENT:
    
    break; case UnaryOperator::OP_NEGATE:
      returned_value = builder.CreateNeg(child);
    break; case UnaryOperator::OP_BIT_NOT:
      returned_value = builder.CreateNot(child);
    break; case UnaryOperator::OP_BOOL_NOT:
      returned_value = builder.CreateIsNull(child);
    break; case UnaryOperator::OP_ADDRESSOF:
      
    break; default:
      colt_unreachable("Not implemented!");
    }
  }

  void LLVMIRGenerator::gen_binary(PTR<const lang::BinaryExpr> ptr) noexcept
  {
    gen_ir(ptr->get_LHS());
    Value* lhs = returned_value;
    gen_ir(ptr->get_RHS());
    Value* rhs = returned_value;

    assert_true(lhs && rhs, "Error generating binary expr!");

    using namespace colt::lang;

    auto expr_t = as<PTR<const BuiltInType>>(ptr->get_type());

    switch (ptr->get_operation())
    {
      /*********** ARITHMETIC ***********/

    break; case BinaryOperator::OP_SUM:
      if (expr_t->is_integral())
        returned_value = builder.CreateAdd(lhs, rhs);
      else if (expr_t->is_floating())
        returned_value = builder.CreateFAdd(lhs, rhs);
    break; case BinaryOperator::OP_SUB:
      if (expr_t->is_integral())
        returned_value = builder.CreateSub(lhs, rhs);
      else if (expr_t->is_floating())
        returned_value = builder.CreateFSub(lhs, rhs);
    break; case BinaryOperator::OP_MUL:
      if (expr_t->is_integral())
        returned_value = builder.CreateMul(lhs, rhs);
      else if (expr_t->is_floating())
        returned_value = builder.CreateFMul(lhs, rhs);
    break; case BinaryOperator::OP_DIV:
      if (expr_t->is_unsigned_int())
        returned_value = builder.CreateUDiv(lhs, rhs);
      else if (expr_t->is_signed_int())
        returned_value = builder.CreateSDiv(lhs, rhs);
      else if (expr_t->is_floating())
        returned_value = builder.CreateFDiv(lhs, rhs);
    break; case BinaryOperator::OP_MOD:
      if (expr_t->is_unsigned_int())
        returned_value = builder.CreateURem(lhs, rhs);
      if (expr_t->is_signed_int())
        returned_value = builder.CreateSRem(lhs, rhs);

      /*********** BITWISE ***********/

    break; case BinaryOperator::OP_BIT_AND:
      returned_value = builder.CreateAnd(lhs, rhs);
    break; case BinaryOperator::OP_BIT_OR:
      returned_value = builder.CreateOr(lhs, rhs);
    break; case BinaryOperator::OP_BIT_XOR:
      returned_value = builder.CreateXor(lhs, rhs);
    break; case BinaryOperator::OP_BIT_LSHIFT:
      returned_value = builder.CreateShl(lhs, rhs);
    break; case BinaryOperator::OP_BIT_RSHIFT:
      returned_value = builder.CreateLShr(lhs, rhs);

      /*********** BOOLEANS ***********/

    break; case BinaryOperator::OP_LESS:
      if (expr_t->is_unsigned_int())
        returned_value = builder.CreateICmpULE(lhs, rhs);
      else if (expr_t->is_signed_int())
        returned_value = builder.CreateICmpSLE(lhs, rhs);
      else if (expr_t->is_floating())
        returned_value = builder.CreateFCmpOLE(lhs, rhs);
    break; case BinaryOperator::OP_LESS_EQUAL:
      if (expr_t->is_unsigned_int())
        returned_value = builder.CreateICmpULE(lhs, rhs);
      else if (expr_t->is_signed_int())
        returned_value = builder.CreateICmpSLE(lhs, rhs);
      else if (expr_t->is_floating())
        returned_value = builder.CreateFCmpOLE(lhs, rhs);
    break; case BinaryOperator::OP_GREAT:
      if (expr_t->is_unsigned_int())
        returned_value = builder.CreateICmpULE(lhs, rhs);
      else if (expr_t->is_signed_int())
        returned_value = builder.CreateICmpSLE(lhs, rhs);
      else if (expr_t->is_floating())
        returned_value = builder.CreateFCmpOLE(lhs, rhs);
    break; case BinaryOperator::OP_GREAT_EQUAL:
      if (expr_t->is_unsigned_int())
        returned_value = builder.CreateICmpUGE(lhs, rhs);
      else if (expr_t->is_signed_int())
        returned_value = builder.CreateICmpSGE(lhs, rhs);
      else if (expr_t->is_floating())
        returned_value = builder.CreateFCmpOGE(lhs, rhs);
    break; case BinaryOperator::OP_EQUAL:
      if (expr_t->is_integral())
        returned_value = builder.CreateICmpEQ(lhs, rhs);
      else if (expr_t->is_floating())
        returned_value = builder.CreateFCmpOEQ(lhs, rhs);
    break; case BinaryOperator::OP_NOT_EQUAL:
      if (expr_t->is_integral())
        returned_value = builder.CreateICmpNE(lhs, rhs);
      else if (expr_t->is_floating())
        returned_value = builder.CreateFCmpONE(lhs, rhs);

    break; default:
      colt_unreachable("Invalid operation!");
    }
  }

  void LLVMIRGenerator::gen_convert(PTR<const lang::ConvertExpr> ptr) noexcept
  {
    using namespace colt::lang;

    gen_ir(ptr->get_child());

    assert_true(ptr->get_type()->is_builtin(), "Type must be built-in!");

    auto expr_t = as<PTR<const BuiltInType>>(ptr->get_type());
    auto child_t = as<PTR<const BuiltInType>>(ptr->get_child()->get_type());

    if (expr_t->get_builtin_id() == BuiltInType::BOOL)
      returned_value = builder.CreateIsNotNull(returned_value);
    if (child_t->is_floating() && expr_t->is_signed_int())
      returned_value = builder.CreateFPToSI(returned_value, type_to_llvm(expr_t));
    else if (child_t->is_floating() && expr_t->is_unsigned_int())
      returned_value = builder.CreateFPToUI(returned_value, type_to_llvm(expr_t));
    else if (child_t->is_unsigned_int() && expr_t->is_floating())
      returned_value = builder.CreateUIToFP(returned_value, type_to_llvm(expr_t));
    else if (child_t->is_signed_int() && expr_t->is_floating())
      returned_value = builder.CreateSIToFP(returned_value, type_to_llvm(expr_t));
    //Same types conversions
    else if (child_t->is_integral())
      returned_value = builder.CreateIntCast(returned_value, type_to_llvm(expr_t), expr_t->is_signed_int());
    else if (child_t->is_floating())
      returned_value = builder.CreateFPCast(returned_value, type_to_llvm(expr_t));
    else
      colt_unreachable("Invalid conversion!");
  }

  void LLVMIRGenerator::gen_var_decl(PTR<const lang::VarDeclExpr> ptr) noexcept
  {
    //Create an allocation on the stack and store it
    local_vars.push_back(
      builder.CreateAlloca(type_to_llvm(ptr->get_type()), nullptr,
        StringRef{ ptr->get_name().get_data(), ptr->get_name().get_size() })
    );
    
    //If initialized
    if (ptr->get_value())
    {
      gen_ir(ptr->get_value());
      PTR<Value> to_write = returned_value;
      builder.CreateStore(to_write, local_vars.get_back(), false);
    }
  }

  void LLVMIRGenerator::gen_var_read(PTR<const lang::VarReadExpr> ptr) noexcept
  {
    if (!ptr->is_global())
      returned_value = builder.CreateLoad(local_vars[ptr->get_local_ID()]->getAllocatedType(),
        local_vars[ptr->get_local_ID()], false);
    else
      colt_unreachable("Not implemented!");
  }

  void LLVMIRGenerator::gen_var_write(PTR<const lang::VarWriteExpr> ptr) noexcept
  {
    gen_ir(ptr->get_value());
    PTR<Value> to_write = returned_value;
    if (!ptr->is_global())
      returned_value = builder.CreateStore(to_write,
        local_vars[ptr->get_local_ID()], false);
    else
      colt_unreachable("Not implemented!");
  }

  void LLVMIRGenerator::gen_fn_def(PTR<const lang::FnDefExpr> ptr) noexcept
  {
    assert_true(ptr->get_body(), "Body should not be empty!");

    PTR<Function> fn = Function::Create(
      cast<FunctionType>(type_to_llvm(ptr->get_type())),
      GlobalValue::ExternalLinkage,
      StringRef{ ptr->get_name().get_data(), ptr->get_name().get_size() },
      module.get());
    
    current_fn = fn;
    //Reset current_fn to nullptr
    ON_EXIT{ current_fn = nullptr; };

    PTR<llvm::BasicBlock> BB = BasicBlock::Create(*context, "entry", fn);
    builder.SetInsertPoint(BB);

    size_t i = 0;
    for (auto& arg : fn->args())
    {
      //Create an allocation on the stack and store it
      local_vars.push_back(
        builder.CreateAlloca(arg.getType(), nullptr,
          StringRef{ ptr->get_params_name()[i].get_data(), ptr->get_params_name()[i].get_size() })
      );
      builder.CreateStore(&arg, local_vars.get_back());
      ++i;
    }

    gen_ir(ptr->get_body());
  }

  void LLVMIRGenerator::gen_fn_ret(PTR<const lang::FnReturnExpr> ptr) noexcept
  {
    if (ptr->get_value() != nullptr) //null means return void
    {
      gen_ir(ptr->get_value());
      returned_value = builder.CreateRet(returned_value);
    }
    else
      returned_value = builder.CreateRetVoid();
  }

  void LLVMIRGenerator::gen_fn_call(PTR<const lang::FnCallExpr> ptr) noexcept
  {
  }

  void LLVMIRGenerator::gen_scope(PTR<const lang::ScopeExpr> ptr) noexcept
  {
    //We store the variables count to be able to pop variables of the scope
    size_t current_scope_var_count = local_vars.get_size();

    for (auto body_expr : ptr->get_body_array())
      gen_ir(body_expr);

    //We pop variables allocated in the current scope
    local_vars.pop_back_n(local_vars.get_size() - current_scope_var_count);
  }

  void LLVMIRGenerator::gen_condition(PTR<const lang::ConditionExpr> ptr) noexcept
  {
    auto entry_inserter = builder.GetInsertBlock();
    //generate if condition
    gen_ir(ptr->get_if_condition());
    auto if_cond = returned_value;

    auto if_blk = BasicBlock::Create(*context, "br_true", current_fn);
    auto else_blk = BasicBlock::Create(*context, "br_false", current_fn);
    auto then_blk = BasicBlock::Create(*context, "then", current_fn);

    builder.SetInsertPoint(if_blk);
    gen_ir(ptr->get_if_statement());
    builder.CreateBr(then_blk);

    builder.SetInsertPoint(else_blk);
    if (ptr->get_else_statement() != nullptr)
      gen_ir(ptr->get_else_statement());
    builder.CreateBr(then_blk);

    builder.SetInsertPoint(entry_inserter);
    builder.CreateCondBr(if_cond, if_blk, else_blk);
    builder.SetInsertPoint(then_blk);
  }

  PTR<llvm::Type> LLVMIRGenerator::type_to_llvm(PTR<const lang::Type> type) noexcept
  {
    using namespace lang;

    switch (type->classof())
    {
    case lang::Type::TYPE_VOID:
      return llvm::Type::getVoidTy(*context);
    case lang::Type::TYPE_BUILTIN:
    {
      auto ptr = as<PTR<const BuiltInType>>(type);
      switch (ptr->get_builtin_id())
      {
      case BuiltInType::U8:
      case BuiltInType::I8:
        return llvm::Type::getInt8Ty(*context);
      case BuiltInType::U16:
      case BuiltInType::I16:
        return llvm::Type::getInt16Ty(*context);
      case BuiltInType::U32:
      case BuiltInType::I32:
        return llvm::Type::getInt32Ty(*context);
      case BuiltInType::U64:
      case BuiltInType::I64:
        return llvm::Type::getInt64Ty(*context);
      case BuiltInType::U128:
      case BuiltInType::I128:
        return llvm::Type::getInt128Ty(*context);
      case BuiltInType::F32:
        return llvm::Type::getFloatTy(*context);
      case BuiltInType::F64:
        return llvm::Type::getDoubleTy(*context);
      case BuiltInType::BOOL:
        return llvm::Type::getInt1Ty(*context);
      default:
        colt_unreachable("Invalid ID!");
      }
    }
    case lang::Type::TYPE_PTR:
    {
      auto ptr = as<PTR<const PtrType>>(type);
      return PointerType::get(type_to_llvm(ptr->get_type_to()), 0);
    }
    case lang::Type::TYPE_FN:
    {
      auto ptr = as<PTR<const FnType>>(type);
      llvm::SmallVector<PTR<llvm::Type>> arg_types;
      for (size_t i = 0; i < ptr->get_params_type().get_size(); i++)
        arg_types.push_back(type_to_llvm(ptr->get_params_type()[i]));
      return FunctionType::get(type_to_llvm(ptr->get_return_type()), arg_types, false);
    }      
    case lang::Type::TYPE_ARRAY:
    case lang::Type::TYPE_CLASS:      
    default:
      colt_unreachable("Unimplemented type!");
    }
  }

  void LLVMIRGenerator::print_module() const noexcept
  {
    module->print(llvm::errs(), nullptr);
  }

  void LLVMIRGenerator::optimize(llvm::OptimizationLevel level) noexcept
  {
    LoopAnalysisManager LAM;
    FunctionAnalysisManager FAM;
    CGSCCAnalysisManager CGAM;
    ModuleAnalysisManager MAM;

    PassBuilder PB;

    PB.registerModuleAnalyses(MAM);
    PB.registerCGSCCAnalyses(CGAM);
    PB.registerFunctionAnalyses(FAM);
    PB.registerLoopAnalyses(LAM);
    PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

    ModulePassManager MPM = PB.buildPerModuleDefaultPipeline(level);
    MPM.run(*module, MAM);
  }
}
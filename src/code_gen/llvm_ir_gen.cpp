/** @file llvm_ir_gen.cpp
* Contains definition of functions declared in 'llvm_ir_gen.h'.
*/

#include <code_gen/llvm_ir_gen.h>
#include <ast/colt_ast.h>

#ifndef COLT_NO_LLVM

/// @brief Contains code generators
namespace colt::gen
{
  using namespace llvm;

  StringRef ToStringRef(colt::StringView view) noexcept
  {
    return StringRef(view.get_data(), view.get_size());
  }

  Expected<GeneratedIR, std::string> GenerateIR(const lang::AST& ast, const std::string& target_triple) noexcept
  {
    GeneratedIR ir;
    std::string error;
    auto Target = llvm::TargetRegistry::lookupTarget(target_triple, error);
    if (!Target)
      return { Error, error };
    ir.target_machine = Target->createTargetMachine(target_triple, "generic", "", {}, {});
    ir.module->setTargetTriple(target_triple);
    ir.module->setDataLayout(ir.target_machine->createDataLayout());

    //Generate and store the IR in 'ir'
    LLVMIRGenerator ir_gen = { ast, *ir.context, *ir.module };
    //Verify module
    if (llvm::verifyModule(*ir.module, &llvm::errs()))
      return { Error, "Generated IR is invalid!" };
    return ir;
  }

  void GeneratedIR::print_module(llvm::raw_ostream& os) const noexcept
  {
    module->print(os, nullptr);
  }

  Expected<bool, const char*> GeneratedIR::to_object_file(const char* path) noexcept
  {
    std::error_code EC;
    raw_fd_ostream dest(path, EC);

    if (EC) {
      return "Could not open file!";
    }

    legacy::PassManager pass;
    if (target_machine->addPassesToEmitFile(pass, dest, nullptr, CGFT_ObjectFile))
      return "Target does not support emitting object file!";

    pass.run(*module);
    dest.flush();
    //No errors
    return true;
  }

  void GeneratedIR::optimize(colt::gen::OptimizationLevel level) noexcept
  {
    if (level == colt::gen::OptimizationLevel::O0)
      return;

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

    llvm::OptimizationLevel opt;
    switch (level)
    {
    break; case colt::gen::OptimizationLevel::O1:
      opt = llvm::OptimizationLevel::O1;
    break; case colt::gen::OptimizationLevel::O2:
      opt = llvm::OptimizationLevel::O2;
    break; case colt::gen::OptimizationLevel::O3:
      opt = llvm::OptimizationLevel::O3;
    break; case colt::gen::OptimizationLevel::Os:
      opt = llvm::OptimizationLevel::Os;
    break; case colt::gen::OptimizationLevel::Oz:
      opt = llvm::OptimizationLevel::Os;
    break; default:
      colt_unreachable("Invalid optimization level");
    }

    ModulePassManager MPM = PB.buildPerModuleDefaultPipeline(opt);
    MPM.run(*module, MAM);
  }

  LLVMIRGenerator::LLVMIRGenerator(const lang::AST& ast, llvm::LLVMContext& ctx, llvm::Module& mod) noexcept
    : context(ctx), module(mod), builder(ctx)
  {
    for (size_t i = 0; i < ast.expressions.get_size(); i++)
      gen_ir(ast.expressions[i]);
  }

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
    break; case Expr::EXPR_WHILE_LOOP:
      gen_while_loop(as<PTR<const WhileLoopExpr>>(ptr));
    break; case Expr::EXPR_NOP:
      returned_value = builder.CreateIntrinsic(llvm::Intrinsic::donothing, {}, {}, nullptr);
    break; case Expr::EXPR_PTR_LOAD:
      gen_ptr_load(as<PTR<const PtrLoadExpr>>(ptr));
    break; case Expr::EXPR_PTR_STORE:
      gen_ptr_store(as<PTR<const PtrStoreExpr>>(ptr));
    break; case Expr::EXPR_FOR_LOOP:
    break; case Expr::EXPR_BREAK_CONTINUE:    
    break; default:
      colt_unreachable("Generating invalid expression!");
    }
  }

  void LLVMIRGenerator::gen_literal(PTR<const lang::LiteralExpr> ptr) noexcept
  {
    using namespace colt::lang;

switch (ptr->get_type()->get_builtin_id())
{
break; case U8:
  returned_value = ConstantInt::get(llvm::Type::getInt8Ty(context), ptr->get_value().as<u8>());
break; case U16:
  returned_value = ConstantInt::get(llvm::Type::getInt16Ty(context), ptr->get_value().as<u16>());
break; case U32:
  returned_value = ConstantInt::get(llvm::Type::getInt32Ty(context), ptr->get_value().as<u32>());
break; case U64:
  returned_value = ConstantInt::get(llvm::Type::getInt64Ty(context), ptr->get_value().as<u64>());
break; case U128:
  returned_value = ConstantInt::get(llvm::Type::getInt128Ty(context), ptr->get_value().as<u64>());
break; case I8:
  returned_value = ConstantInt::get(llvm::Type::getInt8Ty(context), ptr->get_value().as<i8>());
break; case I16:
  returned_value = ConstantInt::get(llvm::Type::getInt16Ty(context), ptr->get_value().as<i16>());
break; case I32:
  returned_value = ConstantInt::get(llvm::Type::getInt32Ty(context), ptr->get_value().as<i32>());
break; case I64:
  returned_value = ConstantInt::get(llvm::Type::getInt64Ty(context), ptr->get_value().as<i64>());
break; case I128:
  returned_value = ConstantInt::get(llvm::Type::getInt128Ty(context), ptr->get_value().as<i64>());
break; case F32:
  returned_value = ConstantFP::get(llvm::Type::getFloatTy(context), ptr->get_value().as<f32>());
break; case F64:
  returned_value = ConstantFP::get(llvm::Type::getDoubleTy(context), ptr->get_value().as<f64>());
break; case BOOL:
  returned_value = ConstantInt::get(llvm::Type::getInt1Ty(context), ptr->get_value().as<bool>());
break; case CHAR:
  returned_value = ConstantInt::get(llvm::Type::getInt8Ty(context), ptr->get_value().as<char>());
break; case lang::lstring:
  returned_value = builder.CreateGlobalStringPtr(ToStringRef(*ptr->get_value().as<PTR<String>>()), "GlobStr", 0U, &module);
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
    case UnaryOperator::OP_INCREMENT:
    case UnaryOperator::OP_DECREMENT:
    {
      auto var_read = as<PTR<const VarReadExpr>>(ptr->get_child());
      PTR<Constant> val_one;
      if (as<PTR<const BuiltInType>>(var_read->get_type())->is_integral())
        val_one = ConstantInt::get(type_to_llvm(var_read->get_type()), 1);
      else //floating point
        val_one = ConstantFP::get(type_to_llvm(var_read->get_type()), 1.0);

      if (!var_read->is_global())
      {
        builder.CreateStore(val_one,
          local_vars[var_read->get_local_ID()], false);
        returned_value = builder.CreateLoad(
          local_vars[var_read->get_local_ID()]->getAllocatedType(), local_vars[var_read->get_local_ID()]
        );
      }
      else
      {
        auto gptr = global_vars.find(var_read->get_name())->second;
        builder.CreateStore(val_one, global_vars.find(var_read->get_name())->second);
        returned_value = builder.CreateLoad(gptr->getValueType(), gptr);
      }
    }
    break; case UnaryOperator::OP_ADDRESSOF:
    {
      auto var_read = as<PTR<const VarReadExpr>>(ptr->get_child());
      if (!var_read->is_global())
        returned_value = local_vars[var_read->get_local_ID()];
      else
        returned_value = global_vars.find(var_read->get_name())->second;
    }    
    break; case UnaryOperator::OP_NEGATE:
      returned_value = builder.CreateNeg(child);
      break;
    case UnaryOperator::OP_BIT_NOT:
    case UnaryOperator::OP_BOOL_NOT:
      returned_value = builder.CreateNot(child);

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
    auto type_t = as<PTR<const BuiltInType>>(ptr->get_LHS()->get_type());

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
      if (type_t->is_unsigned_int())
        returned_value = builder.CreateICmpULT(lhs, rhs, "ui_lt");
      else if (type_t->is_signed_int())
        returned_value = builder.CreateICmpSLT(lhs, rhs, "si_lt");
      else if (type_t->is_floating())
        returned_value = builder.CreateFCmpOLT(lhs, rhs, "fp_lt");
    break; case BinaryOperator::OP_LESS_EQUAL:
      if (type_t->is_unsigned_int())
        returned_value = builder.CreateICmpULE(lhs, rhs, "ui_leq");
      else if (type_t->is_signed_int())
        returned_value = builder.CreateICmpSLE(lhs, rhs, "si_leq");
      else if (type_t->is_floating())
        returned_value = builder.CreateFCmpOLE(lhs, rhs, "fp_leq");
    break; case BinaryOperator::OP_GREAT:
      if (type_t->is_unsigned_int())
        returned_value = builder.CreateICmpUGT(lhs, rhs, "ui_gt");
      else if (type_t->is_signed_int())
        returned_value = builder.CreateICmpSGT(lhs, rhs, "si_gt");
      else if (type_t->is_floating())
        returned_value = builder.CreateFCmpOGT(lhs, rhs, "fp_gt");
    break; case BinaryOperator::OP_GREAT_EQUAL:
      if (type_t->is_unsigned_int())
        returned_value = builder.CreateICmpUGE(lhs, rhs, "ui_geq");
      else if (type_t->is_signed_int())
        returned_value = builder.CreateICmpSGE(lhs, rhs, "si_geq");
      else if (type_t->is_floating())
        returned_value = builder.CreateFCmpOGE(lhs, rhs, "fp_geq");
    break; case BinaryOperator::OP_EQUAL:
      if (type_t->is_integral())
        returned_value = builder.CreateICmpEQ(lhs, rhs, "i_eq");
      else if (type_t->is_floating())
        returned_value = builder.CreateFCmpOEQ(lhs, rhs, "fp_eq");
    break; case BinaryOperator::OP_NOT_EQUAL:
      if (type_t->is_integral())
        returned_value = builder.CreateICmpNE(lhs, rhs, "i_neq");
      else if (type_t->is_floating())
        returned_value = builder.CreateFCmpONE(lhs, rhs, "fp_neq");

    break; default:
      colt_unreachable("Invalid operation!");
    }
  }

  void LLVMIRGenerator::gen_convert(PTR<const lang::ConvertExpr> ptr) noexcept
  {
    using namespace colt::lang;

    gen_ir(ptr->get_child());

    assert_true(ptr->get_type()->is_builtin(), "Type must be built-in!");

    auto expr_t = ptr->get_type();
    auto child_t = as<PTR<const BuiltInType>>(ptr->get_child()->get_type());

    if (ptr->get_conversion_type() == ConvertExpr::CNV_AS)
    {
      if (expr_t->get_builtin_id() == BOOL)
        returned_value = builder.CreateIsNotNull(returned_value, "to_bool");
      if (child_t->is_floating() && expr_t->is_signed_int())
        returned_value = builder.CreateFPToSI(returned_value, type_to_llvm(expr_t), "fp_to_si");
      else if (child_t->is_floating() && expr_t->is_unsigned_int())
        returned_value = builder.CreateFPToUI(returned_value, type_to_llvm(expr_t), "fp_to_ui");
      else if (child_t->is_unsigned_int() && expr_t->is_floating())
        returned_value = builder.CreateUIToFP(returned_value, type_to_llvm(expr_t), "ui_to_fp");
      else if (child_t->is_signed_int() && expr_t->is_floating())
        returned_value = builder.CreateSIToFP(returned_value, type_to_llvm(expr_t), "si_to_fp");
      //Same types conversions
      else if (child_t->is_integral())
        returned_value = builder.CreateIntCast(returned_value, type_to_llvm(expr_t), child_t->is_bool() ? false : expr_t->is_signed_int(), "i_to_i");
      else if (child_t->is_floating())
        returned_value = builder.CreateFPCast(returned_value, type_to_llvm(expr_t), "fp_to_fp");
      else
        colt_unreachable("Invalid conversion!");
    }
    else // bit_as
    {
      auto alloc = builder.CreateAlloca(returned_value->getType());
      builder.CreateStore(returned_value, alloc);
      returned_value = builder.CreateBitCast(alloc,
        llvm::PointerType::get(type_to_llvm(expr_t), 0));
      returned_value = builder.CreateLoad(type_to_llvm(expr_t), returned_value);
    }
  }

  void LLVMIRGenerator::gen_var_decl(PTR<const lang::VarDeclExpr> ptr) noexcept
  {
    if (current_fn != nullptr) //LOCAL VARIABLE
    {
      //Create an allocation on the stack and store it
      local_vars.push_back(
        builder.CreateAlloca(type_to_llvm(ptr->get_type()), nullptr,
          ToStringRef(ptr->get_name()))
      );
    
      //If initialized
      if (ptr->get_value())
      {
        gen_ir(ptr->get_value());
        PTR<Value> to_write = returned_value;
        builder.CreateStore(to_write, local_vars.get_back(), false);
      }
    }
    else //GLOBAL VARIABLE
    {
      assert(ptr->is_global());

      module.getOrInsertGlobal(ToStringRef(ptr->get_name()), type_to_llvm(ptr->get_type()));

      PTR<GlobalVariable> gvar = module.getNamedGlobal(ToStringRef(ptr->get_name()));
      //Insert variable
      global_vars.insert(ptr->get_name(), gvar);
      if (ptr->is_initialized())
      {
        gen_ir(ptr->get_value());
        if (auto p = llvm::dyn_cast<Constant>(returned_value))
          gvar->setInitializer(p);
        else
          call_before_main.push_back(returned_value);
      }
    }

  }

  void LLVMIRGenerator::gen_var_read(PTR<const lang::VarReadExpr> ptr) noexcept
  {
    if (!ptr->is_global())
      returned_value = builder.CreateLoad(type_to_llvm(ptr->get_type()),
        local_vars[ptr->get_local_ID()], false);
    else
    {
      auto gptr = global_vars.find(ptr->get_name());
      assert(gptr);
      returned_value = builder.CreateLoad(gptr->second->getValueType(), gptr->second);
    }
  }

  void LLVMIRGenerator::gen_var_write(PTR<const lang::VarWriteExpr> ptr) noexcept
  {
    gen_ir(ptr->get_value());
    PTR<Value> to_write = returned_value;
    if (!ptr->is_global())
    {
      builder.CreateStore(to_write,
        local_vars[ptr->get_local_ID()], false);
      returned_value = builder.CreateLoad(
        local_vars[ptr->get_local_ID()]->getAllocatedType(), local_vars[ptr->get_local_ID()]
      );
    }
    else
    {
      auto gptr = global_vars.find(ptr->get_name())->second;
      builder.CreateStore(to_write, global_vars.find(ptr->get_name())->second);
      returned_value = builder.CreateLoad(gptr->getValueType(), gptr);
    }
  }

  void LLVMIRGenerator::gen_fn_def(PTR<const lang::FnDefExpr> ptr) noexcept
  {
    PTR<Function> fn = Function::Create(
      cast<FunctionType>(type_to_llvm(ptr->get_type())),
      GlobalValue::ExternalLinkage,
      ToStringRef(colt::gen::mangle(ptr->get_fn_decl())),
      module);
    //Save to global table
    function_map.insert(ptr->get_fn_decl(), fn);
    
    //noexcept
    fn->addFnAttr(llvm::Attribute::NoUnwind);
    
    //Extern functions do not have bodies
    if (ptr->get_fn_decl()->is_extern())
      return;
    
    assert_true(ptr->get_body(), "Body should not be empty!");       
    
    current_fn = fn;
    //Reset current_fn to nullptr
    ON_EXIT{ current_fn = nullptr; };

    PTR<llvm::BasicBlock> BB = BasicBlock::Create(context, "entry", fn);
    
    if (ptr->get_name() == "main" && !call_before_main.is_empty())
    {
      PTR<llvm::BasicBlock> init = BasicBlock::Create(context, "init", fn, BB);
      builder.SetInsertPoint(init);
      for (auto i : call_before_main)
        builder.Insert(i);
      builder.CreateBr(BB);
    }    
    builder.SetInsertPoint(BB);

    size_t i = 0;

    //We store the variables count to be able to pop variables of the scope
    size_t current_scope_var_count = local_vars.get_size();

    for (auto& arg : fn->args())
    {
      arg.setName(ToStringRef(ptr->get_params_name()[i]));
      //Create an allocation on the stack and store it
      local_vars.push_back(
        builder.CreateAlloca(arg.getType(), nullptr,
          ToStringRef(ptr->get_params_name()[i]) + "_ArgCopy")
      );
      builder.CreateStore(&arg, local_vars.get_back());
      ++i;
    }
    gen_ir(ptr->get_body());

    //We pop variables allocated in the current scope
    local_vars.pop_back_n(local_vars.get_size() - current_scope_var_count);
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
    auto fn = function_map.find(ptr->get_fn_decl());
    assert(fn);

    llvm::SmallVector<PTR<Value>> args;
    auto call_args = ptr->get_arguments();
    for (size_t i = 0; i < call_args.get_size(); i++)
    {
      gen_ir(call_args[i]);
      args.push_back(returned_value);
    }
    returned_value = builder.CreateCall(fn->second, args,
      ptr->get_type()->is_void() ? "" : "call_ret");
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
    gen_ir(ptr->get_if_condition());
    Value* cond = returned_value;

    Function* function = builder.GetInsertBlock()->getParent();

    // Create blocks for the then and else cases.  Insert the 'then' block at the
    // end of the function.
    BasicBlock* if_st = BasicBlock::Create(context, "br_true", function);
    BasicBlock* else_st = BasicBlock::Create(context, "br_false");
    BasicBlock* after_st = BasicBlock::Create(context, "after_br");

    //If both if and else branches are terminated,
    //then no 'after_st' branches should be emitted
    bool are_both_branches_term;

    builder.CreateCondBr(cond, if_st, else_st);

    // Emit then value.
    builder.SetInsertPoint(if_st);
    
    gen_ir(ptr->get_if_statement());
    if (!(are_both_branches_term = lang::isTerminatedExpr(ptr->get_if_statement())))
      builder.CreateBr(after_st);
    
    // Emit else block.
    function->getBasicBlockList().push_back(else_st);
    builder.SetInsertPoint(else_st);

    if (ptr->get_else_statement())
    {
      gen_ir(ptr->get_else_statement());
      if (!(are_both_branches_term &= lang::isTerminatedExpr(ptr->get_else_statement())))
        builder.CreateBr(after_st);
    }
    else
      builder.CreateBr(after_st);

    if (!are_both_branches_term)
    {
      function->getBasicBlockList().push_back(after_st);
      builder.SetInsertPoint(after_st);
    }
  }

  void LLVMIRGenerator::gen_while_loop(PTR<const lang::WhileLoopExpr> ptr) noexcept
  {
    BasicBlock* while_cond = BasicBlock::Create(context, "while_cond", current_fn);
    BasicBlock* body = BasicBlock::Create(context, "loop_body", current_fn);
    BasicBlock* end = BasicBlock::Create(context, "after_loop", current_fn);
    loop_begin = while_cond;
    //Jump from current block to while condition
    builder.CreateBr(while_cond);
    
    builder.SetInsertPoint(while_cond);
    gen_ir(ptr->get_condition());
    builder.CreateCondBr(returned_value, body, end);

    builder.SetInsertPoint(body);
    gen_ir(ptr->get_body());
    //Jump back to reevaluate condition
    builder.CreateBr(while_cond);
    
    //Set insertion to after loop body
    builder.SetInsertPoint(end);
  }

  void LLVMIRGenerator::gen_ptr_load(PTR<const lang::PtrLoadExpr> ptr) noexcept
  {
    gen_ir(ptr->get_where());
    returned_value = builder.CreateLoad(type_to_llvm(ptr->get_type()),
      returned_value);
  }
  
  void LLVMIRGenerator::gen_ptr_store(PTR<const lang::PtrStoreExpr> ptr) noexcept
  {
    gen_ir(ptr->get_value());
    auto value = returned_value;
    gen_ir(ptr->get_where());
    auto store = builder.CreateStore(value, returned_value);
    returned_value = builder.CreateLoad(returned_value->getType(),
      store->getPointerOperand());
  }

  PTR<llvm::Type> LLVMIRGenerator::type_to_llvm(PTR<const lang::Type> type) noexcept
  {
    using namespace lang;

    switch (type->classof())
    {
    case lang::Type::TYPE_VOID:
      return llvm::Type::getVoidTy(context);
    case lang::Type::TYPE_BUILTIN:
    {
      auto ptr = as<PTR<const BuiltInType>>(type);
      switch (ptr->get_builtin_id())
      {
      case U8:
      case I8:
        return llvm::Type::getInt8Ty(context);
      case U16:
      case I16:
        return llvm::Type::getInt16Ty(context);
      case U32:
      case I32:
        return llvm::Type::getInt32Ty(context);
      case U64:
      case I64:
        return llvm::Type::getInt64Ty(context);
      case U128:
      case I128:
        return llvm::Type::getInt128Ty(context);
      case F32:
        return llvm::Type::getFloatTy(context);
      case F64:
        return llvm::Type::getDoubleTy(context);
      case BOOL:
        return llvm::Type::getInt1Ty(context);
      case CHAR:
        return llvm::Type::getInt8Ty(context);
      case lang::lstring:
        return PointerType::get(llvm::Type::getInt8Ty(context), 0);
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
      return FunctionType::get(type_to_llvm(ptr->get_return_type()), arg_types, ptr->is_varargs());
    }      
    case lang::Type::TYPE_ARRAY:
    case lang::Type::TYPE_CLASS:      
    default:
      colt_unreachable("Unimplemented type!");
    }
  }
}

#endif //!COLT_NO_LLVM
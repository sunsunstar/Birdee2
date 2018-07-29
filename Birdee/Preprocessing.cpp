#include "AST.h"
#include "CompileError.h"
#include "SourcePos.h"
#include <cassert>
#include "CastAST.h"
#include <sstream>
using std::unordered_map;
using std::string;
using std::reference_wrapper;

using namespace Birdee;

template <typename T>
T* FindImportByName(const unordered_map<reference_wrapper<const string>, T*>& M,
	const string& name)
{
	auto itr = M.find(name);
	if (itr == M.end())
		return nullptr;
	return (itr->second);
}

template <typename T>
T* FindImportByName(const unordered_map<string, std::unique_ptr<T>>& M,
	const string& name)
{
	auto itr = M.find(name);
	if (itr == M.end())
		return nullptr;
	return (itr->second.get());
}

static FunctionAST* cur_func=nullptr;
class ScopeManager
{
public:
	typedef unordered_map<reference_wrapper<const string>, VariableSingleDefAST*> BasicBlock;
	vector<ClassAST*> class_stack;
	vector <BasicBlock> basic_blocks;
	struct TemplateEnv
	{
		unordered_map<string, ResolvedType> typemap;
		unordered_map<string, ExprAST*> exprmap;
		TemplateEnv() {}
		TemplateEnv(TemplateEnv&& v)
		{
			typemap = std::move(v.typemap);
			exprmap = std::move(v.exprmap);
		}
	}template_env;

	inline bool IsCurrentClass(ClassAST* cls)
	{
		return class_stack.size() > 0 && class_stack.back() == cls;
	}

	FieldDef* FindFieldInClass(const string& name)
	{
		if (class_stack.size() > 0)
		{
			ClassAST* cls = class_stack.back();
			auto cls_field = cls->fieldmap.find(name);
			if (cls_field != cls->fieldmap.end())
			{
				return &(cls->fields[cls_field->second]);
			}
		}
		return nullptr;
	}

	MemberFunctionDef* FindFuncInClass(const string& name)
	{
		if (class_stack.size() > 0)
		{
			ClassAST* cls = class_stack.back();
			auto func_field = cls ->funcmap.find(name);
			if (func_field != cls->funcmap.end())
			{
				return &(cls->funcs[func_field->second]);
			}
		}
		return nullptr;
	}

	VariableSingleDefAST* FindLocalVar(const string& name)
	{
		if (basic_blocks.size() > 0)
		{
			for (auto itr = basic_blocks.rbegin(); itr != basic_blocks.rend(); itr++)
			{
				auto var = itr->find(name);
				if (var != itr->end())
				{
					return var->second;
				}
			}
		}
		return nullptr;
	}

	TemplateEnv SetTemplateEnv(const vector<TemplateArgument>& template_args)
	{
		........................;
		return std::move(template_env);
	}

	void RestoreTemplateEnv(TemplateEnv&& env)
	{
		template_env = std::move(env);
	}

	void PushBasicBlock()
	{
		basic_blocks.push_back(BasicBlock());
	}
	void PopBasicBlock()
	{
		basic_blocks.pop_back();
	}
	void PushClass(ClassAST* cls)
	{
		class_stack.push_back(cls);
	}
	void PopClass()
	{
		class_stack.pop_back();
	}

	

	unique_ptr<ResolvedIdentifierExprAST> ResolveName(const string& name,SourcePos pos,ImportTree*& out_import)
	{
		auto bb = FindLocalVar(name);
		if (bb)
		{
			return make_unique<LocalVarExprAST>(bb,pos);
		}
		auto cls_field = FindFieldInClass(name);
		if (cls_field)
		{
			return make_unique<MemberExprAST>(make_unique<ThisExprAST>(class_stack.back(),pos), cls_field,pos);
		}
		auto func_field = FindFuncInClass(name);
		if (func_field)
		{
			return make_unique<MemberExprAST>(make_unique<ThisExprAST>(class_stack.back(),pos), func_field,pos);
		}
		/*if (basic_blocks.size() != 1) // top-level variable finding disabled
		{
			auto global_dim = cu.dimmap.find(name);
			if (global_dim != cu.dimmap.end())
			{
				return make_unique<LocalVarExprAST>(&(global_dim->second.get()));
			}
		}*/
		auto func = cu.funcmap.find(name);
		if (func != cu.funcmap.end())
		{
			return make_unique<ResolvedFuncExprAST>(&(func->second.get()),pos);
		}

		auto ret1 = FindImportByName(cu.imported_dimmap, name);
		if(ret1)
			return make_unique<LocalVarExprAST>(ret1, pos);
		auto ret2 = FindImportByName(cu.imported_funcmap, name);
		if (ret2)
			return make_unique<ResolvedFuncExprAST>(ret2, pos);
		
		auto package = cu.imported_packages.FindName(name);
		if (package)
		{
			out_import = package;
			return nullptr;
		}
		throw CompileError(pos.line, pos.pos, "Cannot resolve name: " + name);
		return nullptr;
	}

}scope_mgr;

template <typename T,typename T2>
T GetItemByName(const unordered_map<T2, T>& M,
	const string& name, SourcePos pos)
{
	auto itr = M.find(name);
	if (itr == M.end())
		throw CompileError(pos.line, pos.pos, "Cannot find the name: " + name);
	return itr->second;
}


#define CompileAssert(a, p ,msg)\
do\
{\
	if (!(a))\
	{\
		throw CompileError(p.line, p.pos, msg);\
	}\
}while(0)\


void ThrowCastError(ResolvedType& target, ResolvedType& fromtype, SourcePos pos)
{
	string msg = "Cannot cast from type ";
	msg += fromtype.GetString();
	msg += " to type ";
	msg += target.GetString();
	throw CompileError(pos.line, pos.pos, msg);
}

template <Token typeto>
unique_ptr<ExprAST> FixTypeForAssignment2(ResolvedType& target, unique_ptr<ExprAST>&& val, SourcePos pos)
{
	switch (val->resolved_type.type)
	{
	case tok_int:
		return make_unique<CastNumberExpr<tok_int, typeto>>(std::move(val),pos);
	case tok_long:
		return make_unique<CastNumberExpr<tok_long, typeto>>(std::move(val),pos);
	case tok_ulong:
		return make_unique<CastNumberExpr<tok_ulong, typeto>>(std::move(val),pos);
	case tok_uint:
		return make_unique<CastNumberExpr<tok_uint, typeto>>(std::move(val),pos);
	case tok_float:
		return make_unique<CastNumberExpr<tok_float, typeto>>(std::move(val),pos);
	case tok_double:
		return make_unique<CastNumberExpr<tok_double, typeto>>(std::move(val),pos);
	case tok_byte:
		return make_unique<CastNumberExpr<tok_byte, typeto>>(std::move(val), pos);
	}
	ThrowCastError(target, val->resolved_type, pos);
	return nullptr;
}

void FixNull(ExprAST* v, ResolvedType& target)
{
	NullExprAST* expr = dynamic_cast<NullExprAST*>(v);
	assert(expr);
	expr->resolved_type = target;
}

unique_ptr<ExprAST> FixTypeForAssignment(ResolvedType& target, unique_ptr<ExprAST>&& val,SourcePos pos)
{
	if (target == val->resolved_type)
	{
		return std::move(val);
	}
	if ( target.isReference() && val->resolved_type.isNull())
	{
		FixNull(val.get(), target);
		return std::move(val);
	}
#define fix_type(typeto) FixTypeForAssignment2<typeto>(target,std::move(val),pos)
	else if (target.isNumber() && val->resolved_type.index_level==0)
	{
		switch (target.type)
		{
		case tok_int:
			return fix_type(tok_int);
		case tok_long:
			return fix_type(tok_long);
		case tok_ulong:
			return fix_type(tok_ulong);
		case tok_uint:
			return fix_type(tok_uint);
		case tok_float:
			return fix_type(tok_float);
		case tok_double:
			return fix_type(tok_double);
		case tok_byte:
			return fix_type(tok_byte);
		}
	}
#undef fix_type
	ThrowCastError(target, val->resolved_type, pos);
	return nullptr;
}


Token PromoteNumberExpression(unique_ptr<ExprAST>& v1, unique_ptr<ExprAST>& v2,bool isBool, SourcePos pos)
{
	static unordered_map<Token, int> promotion_map = {
	{ tok_byte,-1 },
	{ tok_int,0 },
	{tok_uint,1},
	{tok_long,2},
	{tok_ulong,3},
	{tok_float,4},
	{tok_double,5},
	};
	int p1 = promotion_map[v1->resolved_type.type];
	int p2 = promotion_map[v2->resolved_type.type];
	if (p1 == p2)
		return isBool ? tok_boolean: v1->resolved_type.type;
	else if (p1 > p2)
	{
		v2 = FixTypeForAssignment(v1->resolved_type, std::move(v2), pos);
		return isBool ? tok_boolean : v1->resolved_type.type;
	}
	else
	{
		v1 = FixTypeForAssignment(v2->resolved_type, std::move(v1), pos);
		return isBool ? tok_boolean : v2->resolved_type.type;
	}

}

extern string GetModuleNameByArray(const vector<string>& package);
namespace Birdee
{
	const string& GetClassASTName(ClassAST* cls)
	{
		return cls->name;
	}

	
	void ResolvedType::ResolveType(Type& type, SourcePos pos)
	{
		if (type.type == tok_identifier)
		{
			IdentifierType* ty = dynamic_cast<IdentifierType*>(&type);
			assert(ty && "Type should be a IdentifierType");
			this->type = tok_class;
			auto itr = cu.classmap.find(ty->name);
			if (itr == cu.classmap.end())
				this->class_ast = GetItemByName(cu.imported_classmap, ty->name, pos);
			else
				this->class_ast= &(itr->second.get());
			//fix-me: should find function proto
		}
		else if (type.type == tok_package)
		{
			QualifiedIdentifierType* ty=dynamic_cast<QualifiedIdentifierType*>(&type);
			assert(ty && "Type should be a QualifiedIdentifierType");
			string clsname = ty->name.back();
			ty->name.pop_back();
			auto node = cu.imported_packages.Contains(ty->name);
			if (!node || (node && !node->mod))
			{
				throw CompileError(pos.line,pos.pos,"The module " + GetModuleNameByArray(ty->name) + " has not been imported");
			}
			auto itr = node->mod->classmap.find(clsname);
			if(itr== node->mod->classmap.end())
				throw CompileError(pos.line, pos.pos, "Cannot find class " + clsname+" in module "+ GetModuleNameByArray(ty->name));
			this->type = tok_class;
			this->class_ast = itr->second.get();
		}

	}

	void CompileUnit::Phase0()
	{
		for (auto& node : funcmap)
		{
			node.second.get().Phase0();
		}
		for (auto& node : classmap)
		{
			node.second.get().Phase0();
		}
	}

	void CompileUnit::Phase1()
	{
		scope_mgr.PushBasicBlock();
		for (auto& stmt : toplevel)
		{
			stmt->Phase1();
		}
		scope_mgr.PopBasicBlock();
	}
	string ResolvedType::GetString()
	{
		if (type == tok_class)
			return GetClassASTName(class_ast);
		if (type == tok_null)
			return "null_t";
		if (type == tok_func)
			return "func";
		if (type == tok_void)
			return "void";
		if (isTypeToken(type))
		{
			std::stringstream buf;
			buf << GetTokenString(type);
			for(int i=0;i<index_level;i++)
				buf<<"[]";
			return buf.str();
		}
		else
			return "(Error type)";
	}

	bool operator==(const PrototypeAST& ths, const PrototypeAST& other) 
	{
		assert(ths.resolved_type.isResolved() && other.resolved_type.isResolved());
		if (!(ths.resolved_type == other.resolved_type))
			return false;
		if (ths.resolved_args.size() != other.resolved_args.size())
			return false;
		auto itr1 = ths.resolved_args.begin();
		auto itr2 = other.resolved_args.begin();
		for (; itr1 != ths.resolved_args.end(); itr1++, itr2++)
		{
			ResolvedType& t1 = (*itr1)->resolved_type;
			ResolvedType& t2 = (*itr2)->resolved_type;
			assert(t1.isResolved() && t2.isResolved());
			if (!(t1 == t2))
				return false;
		}
		return true;

	}
	bool ResolvedType::operator<(const ResolvedType & that) const
	{
		if (type != that.type)
			return type < that.type;
		if (index_level != that.index_level)
			return index_level < that.index_level;
		if (type == tok_class)
			return class_ast < that.class_ast;
		if (type == tok_package)
			return import_node < that.import_node;
		if (type == tok_func)
			return proto_ast < that.proto_ast;
		return false;		
	}
	bool Birdee::Type::operator<(const Type & that)
	{
		if (type != that.type)
			return type < that.type;
		if (index_level != that.index_level)
			return index_level < that.index_level;
		if (type == tok_package)
		{
			QualifiedIdentifierType* vthis = (QualifiedIdentifierType*)this;
			QualifiedIdentifierType* vthat = (QualifiedIdentifierType*)&that;
			return vthis->name < vthat->name;
		}
		else if (type == tok_identifier)
		{
			IdentifierType* vthis = (IdentifierType*)this;
			IdentifierType* vthat = (IdentifierType*)&that;
			return vthis->name < vthat->name;
		}
		else
			return false; //equals
	}

	bool Birdee::NumberLiteral::operator<(const NumberLiteral & v)
	{
		if (type != v.type)
			return type < type;
		return v_ulong<v.v_ulong;
	}

	bool Birdee::TemplateArgument::operator<(const TemplateArgument & that)
	{
		if (kind != that.kind)
			return kind < that.kind;
		if (kind == TEMPLATE_ARG_TYPE)
			return resolved_type < that.resolved_type;
		//kind==TEMPLATE_ARG_EXPR
		//let NumberExprAST<StringLiteralAST
		NumberExprAST* n1 = dynamic_cast<NumberExprAST*>(expr.get()), *n2=dynamic_cast<NumberExprAST*>(that.expr.get());
		StringLiteralAST* s1 = dynamic_cast<StringLiteralAST*>(expr.get()), *s2 = dynamic_cast<StringLiteralAST*>(that.expr.get());
		if (n1)
		{
			if (s2)
				return true;
			if (n2)
				return n1 < n2;
			assert(0 && "The expression is neither NumberExprAST nor StringLiteralAST");
		}
		else if(s1)
		{
			if (s2)
				return s1->Val < s2->Val;
			if (n2)
				return false; 
			assert(0 && "The expression is neither NumberExprAST nor StringLiteralAST");
		}
		assert(0 && "The expression is neither NumberExprAST nor StringLiteralAST");
		return false;
	}

	void TemplateArgument::Phase1(SourcePos Pos)
	{
		if (kind == TemplateArgument::TEMPLATE_ARG_EXPR)
			expr->Phase1();
		else
		{
			resolved_type = ResolvedType(*type, Pos);
			type = nullptr;
		}
	}

	string int2str(const int v)
	{
		std::stringstream stream;
		stream << v;
		return stream.str();   
	}

	void TemplateParameters::ValidateArguments(const vector<TemplateArgument>& args, SourcePos Pos)
	{
		CompileAssert(params.size() == args.size(), Pos, 
			string("The template requires ") + int2str(params.size()) + " Arguments, but " + int2str(args.size()) + "are given");
		
		for (int i = 0; i < args.size(); i++)
		{
			if (params[i].isTypeParameter())
			{
				CompileAssert(args[i].kind == TemplateArgument::TEMPLATE_ARG_TYPE, Pos, string("Expected a type at the template parameter number ") + int2str(i + 1));
			}
			else
			{
				CompileAssert(args[i].kind == TemplateArgument::TEMPLATE_ARG_EXPR, Pos, string("Expected an expression at the template parameter number ") + int2str(i + 1));
				CompileAssert(instance_of<StringLiteralAST>(args[i].expr.get()) || instance_of<NumberExprAST>(args[i].expr.get()), Pos,
					string("Expected an constant expression at the template parameter number ") + int2str(i + 1));
				args[i].expr->Phase1();
				CompileAssert(ResolvedType(*params[i].type, Pos) == args[i].expr->resolved_type, Pos,
					string("The expression type does not match at the template parameter number ") + int2str(i + 1));
			}
		}
		
	}

	void FunctionTemplateInstanceExprAST::Phase1()
	{
		FunctionAST* func = nullptr;
		expr->Phase1();
		IdentifierExprAST*  iden = dynamic_cast<IdentifierExprAST*>(expr.get());
		if (iden)
		{
			auto resolvedfunc = dynamic_cast<ResolvedFuncExprAST*>(iden->impl.get());
			CompileAssert(resolvedfunc, Pos, "Expected a function name for template");
			func = resolvedfunc->def;
		}
		else
		{
			MemberExprAST*  mem = dynamic_cast<MemberExprAST*>(expr.get());
			CompileAssert(mem, Pos, "Expected a function name or a member function for template");
			if (mem->kind == MemberExprAST::member_function)
			{
				func = mem->func->decl.get();
			}
			else if (mem->kind == MemberExprAST::member_imported_function)
			{
				func = mem->func->decl.get();
			}
			else
			{
				throw CompileError(Pos.line, Pos.pos, "Expected a function name or a member function for template");
			}
		}
		CompileAssert(func->isTemplate(), Pos, "The function is not a template");
		for (auto& arg : template_args)
			arg.Phase1(Pos);
		func->template_param->ValidateArguments(template_args, Pos);
		instance = func->template_param->GetOrCreate(template_args, func, Pos);
		resolved_type = instance->resolved_type;
	}

	FunctionAST * Birdee::TemplateParameters::GetOrCreate(const vector<TemplateArgument>& v, FunctionAST* source_template, SourcePos pos)
	{
		auto ins = instances.find(v);
		if (ins != instances.end())
			return ins->second.get();
		unique_ptr<FunctionAST> replica_func = source_template->CopyNoTemplate();
		instances.insert(std::make_pair(v, std::move(replica_func)));
		FunctionAST* ret = replica_func.get();
		auto env=scope_mgr.SetTemplateEnv(v);
		replica_func->Phase0();
		replica_func->Phase1();
		scope_mgr.RestoreTemplateEnv(std::move(env));
		return ret;
	}

	void AddressOfExprAST::Phase1()
	{
		
		expr->Phase1();
		if (is_address_of)
			CompileAssert(expr->GetLValue(true), Pos, "The expression in addressof should be a LValue");			
		else
			CompileAssert(expr->resolved_type.isReference(), Pos, "The expression in pointerof should be a reference type");
		resolved_type.type = tok_pointer;
	}

	void IndexExprAST::Phase1()
	{
		Expr->Phase1();
		CompileAssert(Expr->resolved_type.index_level > 0, Pos, "The indexed expression should be indexable");
		Index->Phase1();
		CompileAssert(Index->resolved_type.isInteger(), Pos, "The index should be an integer");
		resolved_type = Expr->resolved_type;
		resolved_type.index_level--;
	}

	void IdentifierExprAST::Phase1()
	{
		ImportTree* import_node;
		impl = scope_mgr.ResolveName(Name, Pos, import_node);
		if (!impl)
		{
			resolved_type.type = tok_package;
			resolved_type.import_node = import_node;
		}
		else
			resolved_type = impl->resolved_type;
	}

	void ASTBasicBlock::Phase1()
	{
		scope_mgr.PushBasicBlock();
		for (auto&& node : body)
		{
			node->Phase1();
		}
		scope_mgr.PopBasicBlock();
	}

	void ASTBasicBlock::Phase1(PrototypeAST* proto)
	{
		scope_mgr.PushBasicBlock();
		proto->Phase1();
		for (auto&& node : body)
		{
			node->Phase1();
		}
		scope_mgr.PopBasicBlock();
	}

	void VariableSingleDefAST::Phase1()
	{
		Phase0();
		if (resolved_type.type == tok_auto)
		{
			CompileAssert(val.get(), Pos, "dim with no type must have an initializer");
			val->Phase1();
			resolved_type = val->resolved_type;
		}
		else
		{
			if (val.get())
			{
				val->Phase1();
				val = FixTypeForAssignment(resolved_type, std::move(val), Pos);
			}
		}
		scope_mgr.basic_blocks.back()[name] = this;
	}

	void FunctionAST::Phase1()
	{
		if (isTemplate())
			return;
		auto prv_func = cur_func;
		cur_func = this;
		Phase0();
		Body.Phase1(Proto.get());
		cur_func = prv_func;
	}

	void VariableSingleDefAST::Phase1InClass()
	{
		Phase0();
		if (resolved_type.type == tok_auto)
		{
			throw CompileError(Pos.line, Pos.pos, "Member field of class must be defined with a type");
		}
		else
		{
			if (val.get())
			{
				throw CompileError(Pos.line, Pos.pos, "Member field of class cannot have initializer");
				//val->Phase1();
				//val = FixTypeForAssignment(resolved_type, std::move(val), Pos);
			}
		}
	}

	ClassAST* GetArrayClass()
	{
		string name("genericarray");
		if (cu.is_corelib)
			return &(cu.classmap.find(name)->second.get());
		else
			return cu.imported_classmap.find(name)->second;
	}

	void CheckFunctionCallParameters(PrototypeAST* proto, std::vector<std::unique_ptr<ExprAST>>& Args, SourcePos Pos)
	{
		if (proto->resolved_args.size() != Args.size())
		{
			std::stringstream buf;
			buf << "The function requires " << proto->resolved_args.size() << " Arguments, but " << Args.size() << "are given";
			CompileAssert(false, Pos, buf.str());
		}

		int i = 0;
		for (auto& arg : Args)
		{
			arg->Phase1();
			SourcePos pos = arg->Pos;
			arg = FixTypeForAssignment(proto->resolved_args[i]->resolved_type, std::move(arg), pos);
			i++;
		}
	}

	void NewExprAST::Phase1()
	{
		resolved_type = ResolvedType(*ty, Pos);
		for (auto& expr : args)
		{
			expr->Phase1();
			if (resolved_type.index_level > 0)
			{
				CompileAssert(expr->resolved_type.isInteger(), expr->Pos, "Expected an integer for array size");
			}
		}
		if (resolved_type.index_level == 0)
		{
			if (!method.empty())
			{
				CompileAssert(resolved_type.type==tok_class, Pos, "new expression only supports class types");
				ClassAST* cls = resolved_type.class_ast;
				auto itr = cls->funcmap.find(method);
				CompileAssert(itr != cls->funcmap.end(), Pos, "Cannot resolve name "+ method);
				func = &cls->funcs[itr->second];
				CompileAssert(func->access==access_public, Pos, "Accessing a private method");
				CheckFunctionCallParameters(func->decl->Proto.get(), args, Pos);
			}
		}
	}

	void ClassAST::Phase1()
	{
		Phase0();
		scope_mgr.PushClass(this);
		for (auto& fielddef : fields)
		{
			fielddef.decl->Phase1InClass();
		}
		for (auto& funcdef : funcs)
		{
			funcdef.decl->Phase1();
		}
		scope_mgr.PopClass();
	}
	void MemberExprAST::Phase1()
	{
		if (resolved_type.isResolved())
			return;
		Obj->Phase1();
		if (Obj->resolved_type.type == tok_package)
		{
			ImportTree* node = Obj->resolved_type.import_node;
			if (node->map.size() == 0)
			{
				auto ret1 = FindImportByName(node->mod->dimmap, member);
				if (ret1)
				{
					kind = member_imported_dim;
					import_dim = ret1;
					Obj = nullptr;
					resolved_type = ret1->resolved_type;
					return;
				}
				auto ret2 = FindImportByName(node->mod->funcmap, member);
				if (ret2)
				{
					kind = member_imported_function;
					import_func = ret2;
					Obj = nullptr;
					resolved_type = ret2->resolved_type;
					return;
				}
				throw CompileError(Pos.line,Pos.pos,"Cannot resolve name "+ member);
			}
			else
			{
				kind = member_package;
				resolved_type.type = tok_package;
				resolved_type.import_node = node->FindName(member);
				if(!resolved_type.import_node)
					throw CompileError(Pos.line, Pos.pos, "Cannot resolve name " + member);
				Obj = nullptr;
				return;
			}
			return;
		}
		if (Obj->resolved_type.index_level>0)
		{
			static ClassAST* array_cls = nullptr;
			if (!array_cls)
				array_cls = GetArrayClass();
			auto func = array_cls->funcmap.find(member);
			if (func != array_cls->funcmap.end())
			{
				kind = member_function;
				this->func = &(array_cls->funcs[func->second]);
				if (this->func->access == access_private && !scope_mgr.IsCurrentClass(array_cls)) // if is private and we are not in the class
					throw CompileError(Pos.line, Pos.pos, "Accessing a private member outside of a class");
				resolved_type = this->func->decl->resolved_type;
				return;
			}
			throw CompileError(Pos.line, Pos.pos, "Cannot find member " + member);
			return;
		}
		CompileAssert(Obj->resolved_type.type == tok_class, Pos, "The expression before the member should be an object");
		ClassAST* cls = Obj->resolved_type.class_ast;
		auto field = cls->fieldmap.find(member);
		if (field != cls->fieldmap.end())
		{
			if (cls->fields[field->second].access == access_private && !scope_mgr.IsCurrentClass(cls)) // if is private and we are not in the class
				throw CompileError(Pos.line, Pos.pos, "Accessing a private member outside of a class");
			kind = member_field;
			this->field = &(cls->fields[field->second]);
			resolved_type = this->field->decl->resolved_type;
			return;
		}
		auto func = cls->funcmap.find(member);
		if (func != cls->funcmap.end())
		{
			kind = member_function;
			this->func = &(cls->funcs[func->second]);
			if (this->func->access == access_private && !scope_mgr.IsCurrentClass(cls)) // if is private and we are not in the class
				throw CompileError(Pos.line, Pos.pos, "Accessing a private member outside of a class");
			resolved_type = this->func->decl->resolved_type;
			return;
		}
		throw CompileError(Pos.line, Pos.pos, "Cannot find member "+member);
	}

	void CallExprAST::Phase1()
	{
		Callee->Phase1();
		CompileAssert(Callee->resolved_type.type == tok_func, Pos, "The expression should be callable");
		auto proto = Callee->resolved_type.proto_ast;
		CheckFunctionCallParameters(proto, Args, Pos);
		resolved_type = proto->resolved_type;
	}

	void ThisExprAST::Phase1()
	{
		CompileAssert(scope_mgr.class_stack.size() > 0, Pos, "Cannot reference \"this\" outside of a class");
		resolved_type.type = tok_class;
		resolved_type.class_ast = scope_mgr.class_stack.back();
	}

	void ResolvedFuncExprAST::Phase1()
	{
		def->Phase0(); //fix-me: maybe don't need to call phase0?
		resolved_type = def->resolved_type;
	}

	void LocalVarExprAST::Phase1()
	{
		def->Phase0();
		resolved_type = def->resolved_type;
	}

	void NumberExprAST::Phase1()
	{
		resolved_type.type = Val.type;
	}

	void ReturnAST::Phase1()
	{
		Val->Phase1();
		assert(cur_func->Proto->resolved_type.type != tok_error && "The prototype should be resolved first");
		Val = FixTypeForAssignment(cur_func->Proto->resolved_type, std::move(Val),Pos);
	}

	ClassAST* GetStringClass()
	{
		string name("string");
		if (cu.is_corelib)
			return &(cu.classmap.find(name)->second.get());
		else
			return cu.imported_classmap.find(name)->second;
	}



	void StringLiteralAST::Phase1()
	{
		//fix-me: use the system package name of string
		
		static ClassAST& string_cls = *GetStringClass();
		resolved_type.type = tok_class;
		resolved_type.class_ast = &string_cls;	
	}

	void IfBlockAST::Phase1()
	{
		cond->Phase1();
		CompileAssert(cond->resolved_type.type == tok_boolean && cond->resolved_type.index_level == 0,
			Pos, "The condition of \"if\" expects a boolean expression");
		iftrue.Phase1();
		iffalse.Phase1();
	}

	void BinaryExprAST::Phase1()
	{
		LHS->Phase1();
		RHS->Phase1();
		if (Op == tok_assign)
		{
			if (IdentifierExprAST* idexpr = dynamic_cast<IdentifierExprAST*>(LHS.get()))
				CompileAssert(idexpr->impl->isMutable(), Pos, "Cannot assign to an immutable value");
			else if (MemberExprAST* memexpr = dynamic_cast<MemberExprAST*>(LHS.get()))
				CompileAssert(memexpr->isMutable(), Pos, "Cannot assign to an immutable value");
			else if (instance_of<IndexExprAST>(LHS.get()))
			{}
			else
				throw CompileError(Pos.line, Pos.pos, "The left vaule of the assignment is not an variable");
			RHS = FixTypeForAssignment(LHS->resolved_type, std::move(RHS), Pos);
			resolved_type.type = tok_void;
			return;
		}
		if (Op == tok_equal || Op == tok_ne)
		{
			if (LHS->resolved_type == RHS->resolved_type)
				resolved_type.type = tok_boolean;
			else if (LHS->resolved_type.isNull() && RHS->resolved_type.isReference())
			{
				FixNull(LHS.get(), RHS->resolved_type);
				resolved_type.type = tok_boolean;
			}
			else if (RHS->resolved_type.isNull() && LHS->resolved_type.isReference())
			{
				FixNull(RHS.get(), LHS->resolved_type);
				resolved_type.type = tok_boolean;
			}
			else
				resolved_type.type=PromoteNumberExpression(LHS, RHS, true, Pos);
		}
		else
		{
			if (LHS->resolved_type.type == tok_class && LHS->resolved_type.index_level == 0) //if is class object, check for operator overload
			{
				static unordered_map<Token, string> operator_map = {
				{tok_add,"__add__"},
				{tok_minus,"__sub__"},
				{tok_mul,"__mul__"},
				{tok_div,"__div__"},
				{tok_mod,"__mod__"},
				{tok_cmp_equal,"__eq__"},
				{ tok_ge,"__ge__" },
				{ tok_le,"__le__" },
				{ tok_logic_and,"__logic_and__" },
				{ tok_logic_or,"__logic_or__" },
				{ tok_gt,"__gt__" },
				{ tok_lt,"__lt__" },
				{ tok_and,"__and__" },
				{ tok_or,"__or__" },
				{ tok_not,"__not__" },
				{ tok_xor,"__xor__" },
				};
				string& name = operator_map[Op];
				auto itr=LHS->resolved_type.class_ast->funcmap.find(name);
				CompileAssert(itr!= LHS->resolved_type.class_ast->funcmap.end(), Pos, string("Cannot find function") + name);
				func = LHS->resolved_type.class_ast->funcs[itr->second].decl.get();
				auto proto = func->resolved_type.proto_ast;
				vector<unique_ptr<ExprAST>> args;args.push_back(std::move(RHS) );
				CheckFunctionCallParameters(proto,args , Pos);
				RHS = std::move(args[0]);
				resolved_type = proto->resolved_type;
				return;
			}
			CompileAssert(LHS->resolved_type.isNumber() && RHS->resolved_type.isNumber(), Pos, "Currently only binary expressions of Numbers are supported");
			resolved_type.type = PromoteNumberExpression(LHS, RHS, isBooleanToken(Op), Pos);
		}

	}

	void ForBlockAST::Phase1()
	{
		scope_mgr.PushBasicBlock();
		init->Phase1();
		if (!isdim)
		{
			auto bin = dynamic_cast<BinaryExprAST*>(init.get());
			if (bin)
			{
				CompileAssert(bin->Op == tok_assign, Pos, "Expected an assignment expression after for");
				loop_var = bin->LHS.get();
			}
			else
			{
				CompileAssert(((ExprAST*)init.get())->GetLValue(true), Pos, "The loop variable in for block should be a LValue");
				loop_var = (ExprAST*)init.get();
			}
			CompileAssert(loop_var->resolved_type.isInteger(), Pos, "The loop variable should be an integer");
		}
		else
		{
			CompileAssert(((VariableSingleDefAST*)init.get())->resolved_type.isInteger(), Pos, "The loop variable should be an integer");
			loop_var = nullptr;
		}
		till->Phase1();
		for (auto&& node : block.body)
		{
			node->Phase1();
		}
		scope_mgr.PopBasicBlock();
	}
}
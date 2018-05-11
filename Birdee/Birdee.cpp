// Birdee.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <vector>
#include "Tokenizer.h"
#include <iostream>

using namespace Birdee;

//===----------------------------------------------------------------------===//
// Abstract Syntax Tree (aka Parse Tree)
//===----------------------------------------------------------------------===//

namespace Birdee{



	/// ExprAST - Base class for all expression nodes.
	class ExprAST {
	public:
		virtual ~ExprAST() = default;
	};
	class StatementAST : public ExprAST {
	public:
		virtual ~StatementAST() = default;
	};
	/// NumberExprAST - Expression class for numeric literals like "1.0".
	class NumberExprAST : public ExprAST {
		double Val;

	public:
		NumberExprAST(double Val) : Val(Val) {}
	};

	/// VariableExprAST - Expression class for referencing a variable, like "a".
	class VariableExprAST : public ExprAST {
		std::string Name;

	public:
		VariableExprAST(const std::string &Name) : Name(Name) {}
	};

	/// BinaryExprAST - Expression class for a binary operator.
	class BinaryExprAST : public ExprAST {
		char Op;
		std::unique_ptr<ExprAST> LHS, RHS;

	public:
		BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS,
			std::unique_ptr<ExprAST> RHS)
			: Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
	};

	/// CallExprAST - Expression class for function calls.
	class CallExprAST : public ExprAST {
		std::string Callee;
		std::vector<std::unique_ptr<ExprAST>> Args;

	public:
		CallExprAST(const std::string &Callee,
			std::vector<std::unique_ptr<ExprAST>> Args)
			: Callee(Callee), Args(std::move(Args)) {}
	};

	/// PrototypeAST - This class represents the "prototype" for a function,
	/// which captures its name, and its argument names (thus implicitly the number
	/// of arguments the function takes).
	class PrototypeAST {
		std::string Name;
		std::vector<std::string> Args;

	public:
		PrototypeAST(const std::string &Name, std::vector<std::string> Args)
			: Name(Name), Args(std::move(Args)) {}

		const std::string &getName() const { return Name; }
	};

	/// FunctionAST - This class represents a function definition itself.
	class FunctionAST {
		std::unique_ptr<PrototypeAST> Proto;
		std::unique_ptr<ExprAST> Body;

	public:
		FunctionAST(std::unique_ptr<PrototypeAST> Proto,
			std::unique_ptr<ExprAST> Body)
			: Proto(std::move(Proto)), Body(std::move(Body)) {}
	};

	class Type {
		Token type;	
	public:
		virtual ~Type() = default;
		Type(Token _type):type(_type){}
	};

	class IdentifierType :public Type {
		std::string name;
	public:
		IdentifierType(const std::string&_name):Type(tok_identifier), name(_name){}
	};

	class VariableDefAST : public StatementAST {
		std::string name;
		std::unique_ptr<Type> type;
	public:
		VariableDefAST(const std::string& _name, std::unique_ptr<Type> _type) :name(_name),type(std::move(_type)){}
	};


	class CompileError {
		int linenumber;
		int pos;
		std::string msg;
	public:
		CompileError(int _linenumber, int _pos, const std::string& _msg): linenumber(_linenumber),pos(_pos),msg(_msg){}
		void print()
		{
			printf("Compile Error at line %d, postion %d : %s", linenumber, pos, msg.c_str());
		}
	};
}



Tokenizer tokenizer(fopen("test.txt", "r"));
inline void CompileExpect(Token expected_tok, const std::string& msg)
{
	Token tok=tokenizer.gettok();
	if (tok != expected_tok)
	{
		throw CompileError(tokenizer.GetLine(), tokenizer.GetPos(), msg);
	}
}

inline void CompileExpect(std::initializer_list<Token> expected_tok, const std::string& msg)
{
	Token tok = tokenizer.gettok();
	for(Token t :expected_tok)
	{
		if (t == tok)
			return;
	}
	throw CompileError(tokenizer.GetLine(), tokenizer.GetPos(), msg);
}

std::unique_ptr<VariableDefAST> ParseDim()
{
	CompileExpect(tok_identifier, "Expected an identifier to define a variable");
	std::string identifier = tokenizer.IdentifierStr;
	CompileExpect(tok_as, "Expected \'as\'");
	Token tok = tokenizer.gettok();
	std::unique_ptr<Type> type;
	switch (tok)
	{
	case tok_identifier:
		type = std::make_unique<IdentifierType>(identifier);
		break;
	case tok_int:
		type = std::make_unique<Type>(tok_int);
		break;
	default:
		throw CompileError(tokenizer.GetLine(), tokenizer.GetPos(), "Expected an identifier or basic type name");
	}
	CompileExpect({ tok_newline,tok_eof }, "Expected a new line after variable definition");
	return std::make_unique<VariableDefAST>(identifier, std::move(type));
}

int ParseTopLevel(std::vector<std::unique_ptr<StatementAST>>& out)
{

	Token tok = tok_add; //juat a dummy value
	try {
		while (tok != tok_eof && tok != tok_error)
		{
			tok = tokenizer.gettok();
			switch (tok)
			{
			case tok_dim:
				out.push_back(std::move(ParseDim()));
				break;
			case tok_eof:
				return 0;
				break;
			default:
				throw CompileError(tokenizer.GetLine(), tokenizer.GetPos(), "Unknown Token");
			}
		}
	}
	catch (CompileError e)
	{
		e.print();
		return 1;
	}
	return 0;
}

int main()
{
	std::vector<std::unique_ptr<StatementAST>> s;
	ParseTopLevel(s);

    return 0;
}


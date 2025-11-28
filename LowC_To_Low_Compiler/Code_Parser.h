#ifndef CODE_PARSER
#define CODE_PARSER

// NOTE values are pushed to the stack like so

/*

push BC;

--------

SP + $02	=	whatever was on the stack before
SP + $01	=	B
SP + $00	=	C

*/

#include<vector>

#include"LowC_Tokeniser.h"

#include<map>

// Will use parse tree structure for statements, expressions, and functions

extern std::string Local_Function_Scope_Name; // = "";

extern size_t Return_Type_Of_Current_Parsed_Function;// = S_VOID;

class Parse_Node
{
public:
	// This will have a map for the child nodes, containing a vector for key.
	size_t Syntax_ID;

	std::string Value;	// We can afford to make copies during compilation

	std::map<const char*, std::vector<Parse_Node>> Child_Nodes;

	Parse_Node() {}

	Parse_Node(size_t Syntax_IDp, std::string Valuep)
	{
		Syntax_ID = Syntax_IDp;

		Value = Valuep;
	}

	const std::vector<Parse_Node>& operator [](const char* Name) const
	{
		return Child_Nodes.at(Name);
	}
};

class Grammar_Checker;

class Checker_Function
{
public:
	size_t(*Function)(const Token*, std::vector<Parse_Node>*, const std::vector<Grammar_Checker>&, size_t);

	size_t Parameter = T_INVALID;

	const std::vector<Grammar_Checker>* Grammars = nullptr;
	
	size_t Check(const Token* Tokens, std::vector<Parse_Node>* Node) const
	{
		return Function(Tokens, Node, *Grammars, Parameter);
	}

	Checker_Function(size_t(*Checkp)(const Token*, std::vector<Parse_Node>*, const std::vector<Grammar_Checker>&, size_t), const std::vector<Grammar_Checker>& Grammarsp)
	{
		Function = Checkp;
		Grammars = &Grammarsp;
	}

	Checker_Function(size_t(*Checkp)(const Token*, std::vector<Parse_Node>*, const std::vector<Grammar_Checker>&, size_t), size_t Parameterp)
	{
		Parameter = Parameterp;
		Function = Checkp;
	}
};

extern std::vector<std::vector<Parse_Node>> Declared_Functions;

class Grammar_Checker
{
public:
	std::vector<Checker_Function> Checks;

	void (*Init_Function)(const Token*, std::vector<Parse_Node>&, Parse_Node*, size_t& Tokens_Passed);

	Grammar_Checker() {}

	Grammar_Checker(std::vector<Checker_Function> Checksp, void(*Init_Functionp)(const Token*, std::vector<Parse_Node>&, Parse_Node*, size_t&))
	{
		Checks = Checksp;
		Init_Function = Init_Functionp;
	}
};

enum Syntax_IDs
{
	SYNTAX_DEFAULT,

	S_STACK_DECLARATION_STATEMENT,	// creates local variable on stack
		/*
			(type)
			(name)
			(size)[S_INT_LITERAL]
		*/

	S_ROM_DECLARATION_STATEMENT,	// allocates data in ROM	-	'Value' field is the value of this rom dec
		/*
			(type)
			(name)
			(size)[S_INT_LITERAL]
			(data)[S_INT_LITERALS]
		*/

	S_ID_ASSIGN,				// assigns value to some ID
		/*
			(id)
			(value)
		*/
	S_FUNCTION_DEFINE,			// defines function
		/*
			(return_type)
			(name)
			(parameters)
			(statements)
		*/

	S_FUNCTION_DEC,				// simply declares the function
		/*
			(return_type)
			(name)
			(parameters)
		*/

	S_DEST_ASSIGN,				// assigns value to some location 'destination' in memory
		/*
			(destination)
			(value)
		*/
	S_RETURN,					// returns from function (whether with a value or not)
		/*
			(value)
		*/
	S_WHILE_LOOP,				// a loop
		/*
			(condition)
			(statements)
		*/
	S_INT_LITERALS,				// many int literals of some sort:	Value field is the int literal val
		/*
			(int_literals)
		*/

	S_BYTE,						// a byte type
	S_BYTE_POINTER,				// a pointer to a byte type
	S_BYTE_ARRAY,				// a local byte array pointer
	S_WORD,
	S_VOID,						// 'void' return type
	S_ID8,						// an 8-bit ID
	S_ID16,						// a 16-bit ID
	S_INT_LITERAL,				// an int literal (of any kind)
	S_EXPRESSION8,				// an expression with an 8-bit value
	S_EXPRESSION16,				// an expression with a 16-bit value
	
	S_DEREF8,					// an 8-bit value, collected from some 16-bit address

	S_PLUS8,					// addition between two 8-bit values
		/*
			(left)
			(right)
		*/
	S_PLUS16_8,					// addition between a 16-bit value and an 8-bit value


	S_FUNCTION_CALL,			// a call to some function, given some parameters
};

// void Parse_Tokens(std::vector<Syntax_Wrap>* Syntax, std::vector<Token>& Tokens);

#define Node_Value(New_Value)\
	New_Node->Value = New_Value

#define Node_Copy(Sub_ID, Generated_Node)\
	New_Node->Child_Nodes[Sub_ID].push_back(Generated_Node)

#define Node_Copy_Syntax(Sub_ID, S_ID)\
	New_Node->Child_Nodes[Sub_ID].back().Syntax_ID = S_ID

#define Node_Set(Generated_Node)\
	*New_Node = Generated_Node\

#define Node_Set_Syntax(S_ID)\
	New_Node->Syntax_ID = S_ID

#define Node_Add(Sub_ID, Syntax_ID, Representation)\
	New_Node->Child_Nodes[Sub_ID].push_back(Parse_Node(Syntax_ID, Representation))


#define Node_Init\
	[](const Token* Tokens, std::vector<Parse_Node>& Recursively_Generated_Nodes, Parse_Node* New_Node, size_t& Tokens_Passed)


size_t Parse_Recursive_Check(const Token* Tokens, std::vector<Parse_Node>* Node, const std::vector<Grammar_Checker>& Grammars, size_t Syntax_ID);

size_t Is_Token(const Token* Tokens, std::vector<Parse_Node>* Node, const std::vector<Grammar_Checker>& Grammars, size_t T);

void Parse_Function_Call_Parameters(const Token* Tokens, std::vector<Parse_Node>& Recursively_Generated_Nodes, Parse_Node* New_Node, size_t& Tokens_Passed, const Parse_Node& Parameter_Node);
std::vector<Parse_Node>& Get_Function_Call(std::string Name);

#endif
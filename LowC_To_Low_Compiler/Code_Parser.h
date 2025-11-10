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
};

class Grammar_Checker
{
public:
	std::vector<size_t(*)(const Token*, std::vector<Parse_Node>*)> Checks;

	void (*Init_Function)(const Token*, std::vector<Parse_Node>&, Parse_Node*);

	Grammar_Checker() {}

	Grammar_Checker(std::vector<size_t(*)(const Token*, std::vector<Parse_Node>*)> Checksp, void(*Init_Functionp)(const Token*, std::vector<Parse_Node>&, Parse_Node*))
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

	S_BYTE,						// a byte type
	S_BYTE_POINTER,				// a pointer to a byte type
	S_WORD,
	S_VOID,						// 'void' return type
	S_ID8,						// an 8-bit ID
	S_ID16,						// a 16-bit ID
	S_INT_LITERAL,				// an int literal (of any kind)
	S_EXPRESSION8,				// an expression with an 8-bit value
	S_EXPRESSION16,				// an expression with a 16-bit value
	S_DEREF_ADDRESS,			// an 8-bit value, collected from some 16-bit address
	S_FUNCTION_CALL,			// a call to some function, given some parameters
};

// void Parse_Tokens(std::vector<Syntax_Wrap>* Syntax, std::vector<Token>& Tokens);

#define Node_Copy(Sub_ID, Generated_Node)\
	New_Node->Child_Nodes[Sub_ID].push_back(Generated_Node)

#define Node_Set(Generated_Node)\
	*New_Node = Generated_Node;\

#define Node_Add(Sub_ID, Syntax_ID, Representation)\
	New_Node->Child_Nodes[Sub_ID].push_back(Parse_Node(Syntax_ID, Representation))


#define Node_Init\
	[](const Token* Tokens, std::vector<Parse_Node>& Recursively_Generated_Nodes, Parse_Node* New_Node)


template<const std::vector<Grammar_Checker>& Grammars, size_t Syntax_ID = SYNTAX_DEFAULT>
size_t Parse_Recursive_Check(const Token* Tokens, std::vector<Parse_Node>* Node);

template<size_t T>
size_t Is_Token(const Token* Tokens, std::vector<Parse_Node>* Node);

#endif
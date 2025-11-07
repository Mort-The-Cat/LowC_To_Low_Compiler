#ifndef CODE_PARSER
#define CODE_PARSER

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
};

enum Syntax_IDs
{
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


	S_ID8,						// an 8-bit ID
	S_ID16,						// a 16-bit ID
	S_INT_LITERAL,				// an int literal (of any kind)
	S_EXPRESSION8,				// an expression with an 8-bit value
	S_EXPRESSION16,				// an expression with a 16-bit value
	S_DEREF_ADDRESS,			// an 8-bit value, collected from some 16-bit address
	S_FUNCTION_CALL,			// a call to some function, given some parameters
};

// void Parse_Tokens(std::vector<Syntax_Wrap>* Syntax, std::vector<Token>& Tokens);

#endif
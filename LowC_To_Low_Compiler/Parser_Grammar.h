#ifndef PARSER_GRAMMAR_DECLARATIONS
#define PARSER_GRAMMAR_DECLARATIONS

#include "Code_Parser.h"

#include<utility>

extern std::vector<Token> Parser_Identifiers;	// name and what kind of ID it is

size_t Get_Syntax_ID_Of_Identifier(const char* Name);

template<size_t Syntax_ID>
size_t Is_ID(const Token* Tokens, std::vector<Parse_Node>* Node);

void Add_To_Parser_Identifiers(Token T);

#define Define_Grammar(Name)\
	extern const std::vector<Grammar_Checker> Name;

// This sucks but oh well

Define_Grammar(Identifier_Grammars);
Define_Grammar(Stack_Definition_Grammars);
Define_Grammar(Parameter_Grammars);
Define_Grammar(Parameters_Grammars);
Define_Grammar(Statement_Grammars);
Define_Grammar(Statements_Grammars);
Define_Grammar(Type_Grammars);
Define_Grammar(Return_Type_Grammars);
Define_Grammar(Function_Grammars);
Define_Grammar(Global_Declaration_Grammars);
Define_Grammar(Global_Declarations_Grammars);

Define_Grammar(ID_Assign_Grammars);

Define_Grammar(Store_High_Grammars);	// This defines the "store_high(" usage 

Define_Grammar(Expression8_Grammars);
Define_Grammar(Expression16_Grammars);

Define_Grammar(If_Grammars)
Define_Grammar(Function_Call_Grammars);	// Note that this is specifically a call to a constant function, not any kind of dynamic function pointer

void Generate_Parse_Tree(const Token* Tokens, std::vector<Parse_Node>* Node);

#endif
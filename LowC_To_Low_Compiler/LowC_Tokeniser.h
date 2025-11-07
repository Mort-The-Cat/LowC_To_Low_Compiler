#ifndef LOWC_TOKENISER
#define LOWC_TOKENISER

#include<vector>
#include<string>
#include<iostream>
#include<fstream>
#include<algorithm>

// This creates the sequence of tokens etc from the LowC file

#define CHARACTER_LETTER 0
#define CHARACTER_NUMBER 1
#define CHARACTER_NONALPHANUMERIC 2

size_t Is_Alphanumeric(char Character);

enum
{
	T_INVALID,

#define START_ALPHANUMERIC_TOKENS T_VOID

	T_VOID,
	T_CONST,
	T_BYTE,
	T_WORD,

	T_RETURN,
	T_IF,
	T_MEM,
	T_DO,
	T_WHILE,

#define END_ALPHANUMERIC_TOKENS T_WHILE + 1

	T_IDENTIFIER,

	T_INT_LITERAL,
	T_STRING_LITERAL,

#define START_NON_ALPHANUMERIC_TOKENS T_OPEN_BR
	T_OPEN_BR,
	T_CLOSE_BR,

	T_OPEN_SC,
	T_CLOSE_SC,

	T_OPEN_SQ,
	T_CLOSE_SQ,

	//

	T_SEMI,

	//

	T_EQUALS,
	T_PLUS_EQUALS,
	T_PLUS_PLUS,
	T_MINUS_EQUALS,
	T_MINUS_MINUS,
	T_AND_EQUALS,
	T_OR_EQUALS,
	T_XOR_EQUALS,
	T_POINTER,					// pointer symbol * or pointer dereference
	T_AMPERSAND,				// pointer reference &		ORRRR		& operator


#define END_NON_ALPHANUMERIC_TOKENS (T_AMPERSAND + 1)

	// will expand on this later


	NUMBER_OF_TOKEN_IDS
};

struct Token
{
	size_t Token;
	std::string Name;
};

bool Sort_Token_Compare(const Token& A, const Token& B);

size_t String_Matches_Token(const char* Text, size_t Length, const char* Token_Name);

Token Compiler_Tokens[];

void Sort_Compiler_Tokens();

// Annoying I have to do this, but out-of-order initialisation is "nonstandard" in C++, throwing an error on basically all compilers

void Tokenise(std::vector<Token>& Tokens, const char* File_Directory);

#endif
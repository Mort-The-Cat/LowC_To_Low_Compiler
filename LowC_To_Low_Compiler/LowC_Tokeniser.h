#ifndef LOWC_TOKENISER
#define LOWC_TOKENISER

#include<vector>
#include<string>
#include<iostream>
#include<fstream>
#include<algorithm>

// This creates the sequence of tokens etc from the LowC file

// I'm not using sscanf for unsafe operations, just getting integers, so I don't get

long Get_Value_From_String(const char* String);

#define CHARACTER_LETTER 0
#define CHARACTER_NUMBER 1
#define CHARACTER_NONALPHANUMERIC 2

size_t Is_Alphanumeric(char Character);

// The compiler has forced me to use an X-macro here
// evil, evil, evil

enum
{
	T_INVALID,

#define START_ALPHANUMERIC_TOKENS T_VOID

	T_VOID,
	T_CONST,
	T_BYTE,
	T_WORD,

	T_HIGH,		// This gets the high-byte of a 2-byte expression
	T_BIT,		// This gets the true/false of a bit as a conditional

	T_STORE_HIGH,	// This stores a byte in the upper-byte of a word

	T_SIZEOF,

	T_RETURN,
	T_IF,
	T_MEM,
	T_DO,
	T_WHILE,

#define END_ALPHANUMERIC_TOKENS T_WHILE + 1

	T_IDENTIFIER,

	T_INT_LITERAL,
	T_STRING_LITERAL,

	T_ADDRESSOF,

#define START_NON_ALPHANUMERIC_TOKENS T_OPEN_BR

	T_OPEN_BR,
	T_CLOSE_BR,

	T_OPEN_SC,
	T_CLOSE_SC,

	T_OPEN_SQ,
	T_CLOSE_SQ,

	T_COMMA,

	T_SEMI,

	T_EQUALS,
	T_PLUS_EQUALS,
	T_PLUS_PLUS,
	T_PLUS,
	T_MINUS_EQUALS,
	T_MINUS_MINUS,
	T_MINUS,
	T_AND_EQUALS,
	T_AND,
	T_OR_EQUALS,
	T_OR,
	T_XOR_EQUALS,
	T_XOR,

	T_NOT,

	T_LESS_THAN,
	T_GREATER_THAN,

	T_POINTER,
	T_AMPERSAND,

#define END_NON_ALPHANUMERIC_TOKENS (T_AMPERSAND + 1)

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
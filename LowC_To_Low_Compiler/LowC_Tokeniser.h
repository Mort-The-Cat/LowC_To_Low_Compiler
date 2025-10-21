#ifndef LOWC_TOKENISER
#define LOWC_TOKENISER

#include<vector>
#include<string>

// This creates the sequence of tokens etc from the LowC file

enum Token_ID
{
	T_INVALID,

	T_VOID,
	T_BYTE,
	T_POINTER,
	T_WORD,

	T_IDENTIFIER,

	T_OPEN_BR,
	T_CLOSE_BR,

	T_OPEN_SC,
	T_CLOSE_SC,

	T_OPEN_SQ,
	T_CLOSE_SQ,

	T_INT_LITERAL,
	T_STRING_LITERAL,

	//

	T_EQUALS,
	T_PLUS_EQUALS,
	T_MINUS_EQUALS,
	T_AND_EQUALS,
	T_OR_EQUALS,
	T_XOR_EQUALS

	// will expand on this later
};

struct Token
{
	size_t Token;
	std::string Name;
};

void Tokenise(std::vector<Token>& Tokens, const char* File_Directory)
{

}


#endif
#include <stdio.h>

#include "LowC_Tokeniser.h"

int main()
{
	Sort_Compiler_Tokens(); // required before doing any tokenising

	std::vector<Token> Tokens;

	Tokenise(Tokens, "Test_LowC_Program.lowc");

	for (size_t W = 0; W < Tokens.size(); W++)
	{
		if (Tokens[W].Token == T_IDENTIFIER)
			printf("@");
		printf("%s", Tokens[W].Name.c_str());
		
		if (Tokens[W].Token == T_IDENTIFIER)
			printf("@");

		printf(" ");
	}

	getc(stdin);

	return 0;
}
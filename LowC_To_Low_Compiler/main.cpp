#include <stdio.h>

#include "LowC_Tokeniser.h"

#include "Code_Parser.h"
#include "Parser_Grammar.h"
#include "Code_Trace_Handler.h"

int main()
{
	Sort_Compiler_Tokens(); // required before doing any tokenising

	std::vector<Token> Tokens;

	Tokenise(Tokens, "Test_LowC_2_Program.lowc");

	for (size_t W = 0; W < Tokens.size(); W++)
	{
		if (Tokens[W].Token == T_IDENTIFIER)
			printf("@");
		printf("%s", Tokens[W].Name.c_str());
		
		if (Tokens[W].Token == T_IDENTIFIER)
			printf("@");

		printf(" ");
	}

	//

	Parse_Node Program_Node;

	Generate_Parse_Tree(Tokens.data(), &Program_Node.Child_Nodes["statements"]);

	std::string Program_Output = "";

	Tracer_Data Tracer;

	Analyse_Parsed_LowC(Program_Output, Tracer, Program_Node);
	
	printf("Low code:\n================================\n\n%s", Program_Output.c_str());

	// Then, we want to parse the tokens into the parse tree!

	getc(stdin);

	return 0;
}
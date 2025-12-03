#include <stdio.h>

#include "LowC_Tokeniser.h"

#include "Code_Parser.h"
#include "Parser_Grammar.h"
#include "Code_Trace_Handler.h"

void Output_File(const char* File_Directory, const char* Data)
{
	std::ofstream File(File_Directory, std::ios::out);

	File.write(Data, strlen(Data));	// I'm pretty sure this writes its own null-terminator

	File.close();
}

int main()
{
	Sort_Compiler_Tokens(); // required before doing any tokenising

	std::vector<Token> Tokens;

	std::string User_Input;

	std::string Output_Directory;

	std::cin >> User_Input >> Output_Directory;

	Tokenise(Tokens, User_Input.c_str());

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

	Generate_Parse_Tree(Tokens.data(), &Program_Node.Child_Nodes["global_declarations"]);

	std::string Program_Output = "";

	Tracer_Data Tracer;

	Analyse_Parsed_LowC(Program_Output, Tracer, Program_Node);
	
	printf("\nLow code:\n================================\n\n%s\n\n", Program_Output.c_str());

	User_Input = "";

	Output_File(Output_Directory.c_str(), Program_Output.c_str());

	printf("\nCode compiled! Remember to check for errors\n");

	// Then, we want to parse the tokens into the parse tree!

	return 0;
}
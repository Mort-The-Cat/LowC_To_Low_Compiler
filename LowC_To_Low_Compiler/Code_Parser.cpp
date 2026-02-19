#include "Code_Parser.h"
#include "Parser_Grammar.h"

#include <set>

std::vector<Token> Parser_Identifiers;	// name and what kind of ID it is

std::string Local_Function_Scope_Name = "";

size_t Is_Token(const Token* Tokens, std::vector<Parse_Node>* Node, const std::vector<Grammar_Checker>& Grammars, size_t T)
{
	return Tokens[0].Token == T;
}
// std::set<const std::vector<Checker_Function>*> Forbidden_Grammars;

const char* Recently_Printed_Name = "nullptr";

size_t Parse_Recursive_Check(const Token* Tokens, std::vector<Parse_Node>* Node, const std::vector<Grammar_Checker>& Grammars, size_t Syntax_ID)
{
	//if (strcmp(Tokens[0].Name.c_str(), Recently_Printed_Name))
	//{
	//	printf("\t >> %s %s\n", Tokens[0].Name.c_str(), Tokens[1].Name.c_str());
	//	Recently_Printed_Name = Tokens[0].Name.c_str();
	//}

	for (size_t Index = 0; Index < Grammars.size(); Index++)
	{
		Parse_Node Test_Node;	// This will be the node we check the values with in order to recursively test and generate the parse tree
								// if the check fails, it's automatically deallocated (thank you, C++)

		size_t Tokens_Passed = 0;	// Number of tokens we've read

		std::vector<Parse_Node> Generated_Nodes;

		//

		for (size_t Check = 0; Check < Grammars[Index].Checks.size(); Check++)
		{
			size_t Delta;

			// Generated_Nodes.clear();

			if (Delta = Grammars[Index].Checks[Check].Check(Tokens + Tokens_Passed, &Generated_Nodes))
			{
				if ((&Grammars == &Function_Grammars || &Grammars == &Function_Dec_Grammars) && Check == 1)
				{
					Local_Function_Scope_Name = "__" + Tokens[Tokens_Passed].Name + "__";

					Add_To_Parser_Identifiers({ S_ID16, Tokens[Tokens_Passed].Name });
				}

				if ((&Grammars == &Function_Grammars || &Grammars == &Function_Dec_Grammars) && Check == 0)
				{
					// This'll set the function return type
					// Important for choosing the correct kind of expression for the return type

					Return_Type_Of_Current_Parsed_Function = Generated_Nodes[0].Syntax_ID;
				}

				if ((&Grammars == &Function_Grammars || &Grammars == &Function_Dec_Grammars) && Grammars[Index].Checks[Check].Parameter == T_CLOSE_BR)
				{
					Declared_Functions.push_back(Generated_Nodes);
				}

				Tokens_Passed += Delta;	// counts up how many tokens we've read
			}
			else
			{

				if ((&Grammars == &Function_Grammars || &Grammars == &Function_Dec_Grammars))
					Local_Function_Scope_Name = "";

				Tokens_Passed = 0;		// don't count the tokens we've read... a check has failed so we shouldn't use this

				break;
			}
		}

		if (Tokens_Passed)	// If all recursive checks passed for this representation of the grammar, we're 100% okay to use it!
		{
			// write it to 'node' and increment counter

			Grammars[Index].Init_Function(Tokens, Generated_Nodes, &Test_Node, Tokens_Passed);

			// Test_Node.Syntax_ID = Syntax_ID;

			Node->push_back(std::move(Test_Node));

			if ((&Grammars == &Function_Grammars || &Grammars == &Function_Dec_Grammars))
			{
				Local_Function_Scope_Name = "";
			}

			// also need to set the ID of this node, but I'll handle that later

			return Tokens_Passed;
		}

		// Otherwise? keep checking...
	}

	return 0;

	// Checks if this is a stack declaration

	// Go through a series of checks
}

/*bool Is_Expression(const Token* Tokens, size_t* Delta, size_t* Grammar_ID, size_t* Type_Specifier)
{
	// This'll determine if we have an expression

	// and then it'll specify what type of expression ofc
}*/

//

//

size_t Parse_Special_Recursive_Check(const Token* Tokens, std::vector<Parse_Node>* Node, const std::vector<Grammar_Checker>& Grammars, size_t Syntax_ID)
{
	for (long Grammar = Grammars.size() - 1; Grammar > -1; Grammar--)
	{

		Parse_Node Test_Node;

		size_t Tokens_Passed = 0;

		std::vector<Parse_Node> Generated_Nodes;

		for (long Check = 0; Check < Grammars[Grammar].Checks.size(); Check++)
		{
			size_t Delta = 0;

			if (Delta = Grammars[Grammar].Checks[Check].Check(Tokens + Tokens_Passed, &Generated_Nodes))
			{
				Tokens_Passed += Delta;
			}
			else
			{
				Tokens_Passed = 0;
				break;
			}
		}

		if (Tokens_Passed)
		{
			Grammars[Grammar].Init_Function(Tokens, Generated_Nodes, &Test_Node, Tokens_Passed);

			Node->push_back(std::move(Test_Node));

			// also need to set the ID of this node, but I'll handle that later

			return Tokens_Passed;
		}
	}

	return 0;
}
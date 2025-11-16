#include "Code_Parser.h"
#include "Parser_Grammar.h"

std::vector<Token> Parser_Identifiers;	// name and what kind of ID it is

size_t Is_Token(const Token* Tokens, std::vector<Parse_Node>* Node, const std::vector<Grammar_Checker>& Grammars, size_t T)
{
	return Tokens[0].Token == T;
}

size_t Parse_Recursive_Check(const Token* Tokens, std::vector<Parse_Node>* Node, const std::vector<Grammar_Checker>& Grammars, size_t Syntax_ID)
{
	for (size_t Index = 0; Index < Grammars.size(); Index++)
	{
		Parse_Node Test_Node;	// This will be the node we check the values with in order to recursively test and generate the parse tree
								// if the check fails, it's automatically deallocated (thank you, C++)

		size_t Tokens_Passed = 0;	// Number of tokens we've read

		std::vector<Parse_Node> Generated_Nodes;

		for (size_t Check = 0; Check < Grammars[Index].Checks.size(); Check++)
		{
			size_t Delta;
			// Generated_Nodes.clear();

			if (Delta = Grammars[Index].Checks[Check].Check(Tokens + Tokens_Passed, &Generated_Nodes))
			{
				Tokens_Passed += Delta;	// counts up how many tokens we've read
			}
			else
			{
				Tokens_Passed = 0;		// don't count the tokens we've read... a check has failed so we shouldn't use this
				break;
			}
		}

		if (Tokens_Passed)	// If all recursive checks passed for this representation of the grammar, we're 100% okay to use it!
		{
			// write it to 'node' and increment counter

			Grammars[Index].Init_Function(Tokens, Generated_Nodes, &Test_Node);

			// Test_Node.Syntax_ID = Syntax_ID;

			Node->push_back(Test_Node);

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
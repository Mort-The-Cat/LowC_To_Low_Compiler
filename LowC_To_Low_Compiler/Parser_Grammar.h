#ifndef PARSER_GRAMMAR_DECLARATIONS
#define PARSER_GRAMMAR_DECLARATIONS

#include "Code_Parser.h"

const std::vector<Grammar_Checker> Stack_Definition_Grammars =
{
	Grammar_Checker(
		{	// byte Id;
			Is_Token<T_BYTE>, Is_Token<T_IDENTIFIER>, Is_Token<T_SEMI>
		},

		Node_Init
		{
			Node_Add("type", S_BYTE, Tokens[0].Name);
			Node_Add("id", S_ID8, Tokens[1].Name);
			Node_Add("size", S_INT_LITERAL, "1");
		}
	),

	Grammar_Checker(
		{	// byte Id[0123];
			Is_Token<T_BYTE>, Is_Token<T_IDENTIFIER>, Is_Token<T_OPEN_SQ>, Is_Token<T_INT_LITERAL>, Is_Token<T_CLOSE_SQ>, Is_Token<T_SEMI>
		},

		Node_Init
		{
			Node_Add("type", S_BYTE_POINTER, "byte*");	// an array of this nature is analogous to a pointer in all but size
			Node_Add("id", S_ID16, Tokens[1].Name);
			Node_Add("size", S_INT_LITERAL, Tokens[3].Name);
		}
	),

	Grammar_Checker(
		{	// byte* Id;
			Is_Token<T_BYTE>, Is_Token<T_POINTER>, Is_Token<T_IDENTIFIER>, Is_Token<T_SEMI>
		},

		Node_Init
		{
			Node_Add("type", S_BYTE_POINTER, "byte*");
			Node_Add("id", S_ID16, Tokens[2].Name);
			Node_Add("size", S_INT_LITERAL, "2");
		}
	)
};

//

const std::vector<Grammar_Checker> Parameter_Grammars =
{
	Grammar_Checker(
		{
			Is_Token<T_BYTE>, Is_Token<T_IDENTIFIER>
		},
		Node_Init
		{
			Node_Add("type", S_BYTE, "byte");
			Node_Add("id", S_ID8, Tokens[1].Name);
			Node_Add("size", S_INT_LITERAL, "1");
		}
	),

	Grammar_Checker(
		{
			Is_Token<T_BYTE>, Is_Token<T_POINTER>, Is_Token<T_IDENTIFIER>
		},
		Node_Init
		{
			Node_Add("type", S_BYTE_POINTER, "byte*");
			Node_Add("id", S_ID16, Tokens[2].Name);
			Node_Add("size", S_INT_LITERAL, "2");
		}
	),

	Grammar_Checker(
		{
			Is_Token<T_CONST>, Is_Token<T_BYTE>, Is_Token<T_POINTER>, Is_Token<T_IDENTIFIER>
		},
		Node_Init
		{
			Node_Add("type", S_BYTE_POINTER, "byte*");
			Node_Add("id", S_ID16, Tokens[2].Name);
			Node_Add("size", S_INT_LITERAL, "2");
		}
	),

	Grammar_Checker(
		{
			Is_Token<T_WORD>, Is_Token<T_IDENTIFIER>
		},
		Node_Init
		{
			Node_Add("type", S_WORD, "word");
			Node_Add("id", S_ID16, Tokens[1].Name);
			Node_Add("size", S_INT_LITERAL, "2");
		}
	)
};

const std::vector<Grammar_Checker> Parameters_Grammars =
{
	Grammar_Checker(
		{
			Parse_Recursive_Check<Parameter_Grammars>, Parse_Recursive_Check<Parameters_Grammars>
		},
		Node_Init
		{
			Node_Copy("parameter", Recursively_Generated_Nodes[0]);

			Node_Copy("parameters", Recursively_Generated_Nodes[1]);
		}
	),
	Grammar_Checker(
		{
			Parse_Recursive_Check<Parameter_Grammars>
		},
		Node_Init
		{	
			Node_Copy("parameter", Recursively_Generated_Nodes[0]);
		}
	)
};

const std::vector<Grammar_Checker> Statement_Grammars
{
	Grammar_Checker(
		{
			Parse_Recursive_Check<Stack_Definition_Grammars, S_STACK_DECLARATION_STATEMENT>	// declares a stack variable
		},
		Node_Init
		{
			Node_Set(Recursively_Generated_Nodes[0]);
		}
	)
};

const std::vector<Grammar_Checker> Statements_Grammars
{
	Grammar_Checker(
		{
			Parse_Recursive_Check<Statement_Grammars>, Parse_Recursive_Check<Statements_Grammars>
		},
		Node_Init
		{
			Node_Set(Recursively_Generated_Nodes[0]);
			Node_Copy("statements", Recursively_Generated_Nodes[1]);
		}
	),

	Grammar_Checker(
		{
			Parse_Recursive_Check<Statement_Grammars>
		},
		Node_Init
		{
			Node_Copy("statement", Recursively_Generated_Nodes[0]);
		}
	)
};

const std::vector<Grammar_Checker> Type_Grammars
{

};

const std::vector<Grammar_Checker> Return_Type_Grammars
{
	Grammar_Checker(
		{
			Is_Token<T_VOID>
		},
		Node_Init
		{
			Node_Set(Parse_Node(S_VOID, "void"));
		}
	),

	Grammar_Checker(
		{
			Parse_Recursive_Check<Type_Grammars>
		},
		Node_Init
		{
			Node_Set(Recursively_Generated_Nodes[0]);
		}
	)
}

const std::vector<Grammar_Checker> Function_Grammars =
{
	Grammar_Checker(
		{
			Parse_Recursive_Check<>, Is_Token<T_IDENTIFIER>,
			Is_Token<T_OPEN_BR>,
				Parse_Recursive_Check<Parameters_Grammars>,
			Is_Token<T_CLOSE_BR>,
			Is_Token<T_OPEN_SC>,
				Parse_Recursive_Check<Statements_Grammars>,
			Is_Token<T_CLOSE_SC>
		},

		Node_Init
		{
			Node_Copy("return_type", Recursively_Generated_Nodes[0]);
			Node_Add("id", S_ID16, Tokens[1].Name);
			Node_Copy("parameters", Recursively_Generated_Nodes[1]);
			Node_Copy("statements", Recursively_Generated_Nodes[2]);
		}
	)
};

#endif
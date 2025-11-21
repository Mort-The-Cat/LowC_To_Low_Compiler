#include "Parser_Grammar.h"
#include "Code_Parser.h"

void Generate_Parse_Tree(const Token* Tokens, std::vector<Parse_Node>* Node)
{
	Parse_Recursive_Check(Tokens, Node, Global_Declarations_Grammars, T_INVALID);
}

size_t Get_Depth_Of_Node(Parse_Node Node, const char* Sub_ID)
{
	size_t Val = 1;
	if (Node.Child_Nodes.contains(Sub_ID))
		Val += Get_Depth_Of_Node(Node.Child_Nodes[Sub_ID][0], Sub_ID);
	return Val;
}

size_t Get_Syntax_ID_Of_Identifier(const char* Name)
{
	for (long Index = Parser_Identifiers.size() - 1; Index != -1; Index--)
		if (0 == strcmp(Name, Parser_Identifiers[Index].Name.c_str()))
			return Parser_Identifiers[Index].Token;


	return -1;	// Bad value!
}

size_t Is_ID(const Token* Tokens, std::vector<Parse_Node>* Node, const std::vector<Grammar_Checker>& Grammars, size_t Syntax_ID)
{
	if (Tokens[0].Token == T_IDENTIFIER && Get_Syntax_ID_Of_Identifier(Tokens[0].Name.c_str()) == Syntax_ID)
		return 1;

	return 0;
}

void Add_To_Parser_Identifiers(Token T)
{
	Parser_Identifiers.push_back(T);
}

const std::vector<Grammar_Checker> Identifier_Grammars =
{
	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_IDENTIFIER)
		},

		Node_Init
		{
			Node_Set(Parse_Node(S_ID8, Tokens[0].Name));
		}
	)
};

const std::vector<Grammar_Checker> Int_Literals_Grammars =
{
	Grammar_Checker(	// int literal, int literals
		{
			Checker_Function(Is_Token, T_INT_LITERAL), Checker_Function(Is_Token, T_COMMA),
			Checker_Function(Parse_Recursive_Check, Int_Literals_Grammars)
		},
		Node_Init
		{
			Node_Set(Parse_Node(S_INT_LITERAL, Tokens[0].Name));

			Node_Copy("int_literals", Recursively_Generated_Nodes[0]);
		}
	),

	Grammar_Checker(	// int literal
		{
			Checker_Function(Is_Token, T_INT_LITERAL)
		},
		Node_Init
		{
			Node_Set(Parse_Node(S_INT_LITERAL, Tokens[0].Name));
		}
	)
};

const std::vector<Grammar_Checker> Expression16_Grammars =
{
	Grammar_Checker(
		{
			Checker_Function(Parse_Recursive_Check, Expression16_Grammars),
			Checker_Function(Is_Token, T_PLUS),
			Checker_Function(Parse_Recursive_Check, Expression8_Grammars)
		},
		Node_Init
		{
			Node_Copy("left", Recursively_Generated_Nodes[0]);
			Node_Copy("right", Recursively_Generated_Nodes[1]);
			Node_Set_Syntax(S_PLUS16_8);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Is_ID, S_ID16)
		},
		Node_Init
		{
			Node_Set(Parse_Node(S_ID16, Tokens[0].Name));
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_INT_LITERAL)
		},
		Node_Init
		{
			Node_Set(Parse_Node(S_INT_LITERAL, Tokens[0].Name));
		}
	)
};

const std::vector<Grammar_Checker> Expression8_Grammars =
{
	Grammar_Checker(
		{
			Checker_Function(Parse_Recursive_Check, Expression8_Grammars),
			Checker_Function(Is_Token, T_PLUS),
			Checker_Function(Parse_Recursive_Check, Expression8_Grammars)
		},
		Node_Init
		{
			Node_Copy("left", Recursively_Generated_Nodes[0]);
			Node_Copy("right", Recursively_Generated_Nodes[1]);
			Node_Set_Syntax(S_PLUS8);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_INT_LITERAL)
		},
		Node_Init
		{
			Node_Set(Parse_Node(S_INT_LITERAL, Tokens[0].Name));
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Is_ID, S_ID8)
		},
		Node_Init
		{
			Node_Set(Parse_Node(S_ID8, Tokens[0].Name));
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_POINTER), Checker_Function(Parse_Recursive_Check, Expression16_Grammars)
		},
		Node_Init
		{
			Node_Copy("address", Recursively_Generated_Nodes[0]);
			Node_Set_Syntax(S_DEREF8);
		}
	)
};

const std::vector<Grammar_Checker> ID_Assign_Grammars =
{
	Grammar_Checker(
		{
			Checker_Function(Is_ID, S_ID8), Checker_Function(Is_Token, T_EQUALS), Checker_Function(Parse_Recursive_Check, Expression8_Grammars)
		},
		Node_Init
		{
			Node_Add("id", S_ID8, Tokens[0].Name);
			Node_Copy("value", Recursively_Generated_Nodes[0]);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Is_ID, S_ID16), Checker_Function(Is_Token, T_EQUALS), Checker_Function(Parse_Recursive_Check, Expression16_Grammars)
		},
		Node_Init
		{
			Node_Add("id", S_ID16, Tokens[0].Name);
			Node_Copy("value", Recursively_Generated_Nodes[0]);
		}
	)
};

// NOTE that an identifier is *automatically* identified by the tracer as *constant* if it's not found on the local stack or in existing global variables
// At which point, it'll retry (given the current local function scope name)

const std::vector<Grammar_Checker> ROM_Declaration_Grammars =
{
	Grammar_Checker(	// const byte Id = 01234;
		{
			Checker_Function(Is_Token, T_CONST),
			Checker_Function(Is_Token, T_BYTE), Checker_Function(Is_Token, T_IDENTIFIER),
			Checker_Function(Is_Token, T_EQUALS),
			Checker_Function(Is_Token, T_INT_LITERAL),
			Checker_Function(Is_Token, T_SEMI)
		},
		Node_Init
		{
			Node_Add("type", S_BYTE, Tokens[1].Name);

			std::string Name = Local_Function_Scope_Name + Tokens[2].Name;

			Node_Add("id", S_ID8, Name);

			Add_To_Parser_Identifiers({ S_ID8, Name });

			Node_Add("size", S_INT_LITERAL, "1");

			Node_Add("data", S_INT_LITERAL, Tokens[4].Name);
		}
	),

	Grammar_Checker(	// const byte Id[] = "insert string literal here";
		{
			Checker_Function(Is_Token, T_CONST),
			Checker_Function(Is_Token, T_BYTE), Checker_Function(Is_Token, T_IDENTIFIER),
			Checker_Function(Is_Token, T_OPEN_SQ), Checker_Function(Is_Token, T_CLOSE_SQ),
			Checker_Function(Is_Token, T_EQUALS),
			Checker_Function(Is_Token, T_STRING_LITERAL),
			Checker_Function(Is_Token, T_SEMI)
		},
		Node_Init
		{
			Node_Add("type", S_BYTE_ARRAY, "byte[]");

			std::string Name = Local_Function_Scope_Name + Tokens[2].Name;

			Node_Add("id", S_ID16, Name);

			Add_To_Parser_Identifiers({ S_ID16, Name });

			Node_Add("size", S_INT_LITERAL, std::to_string(Tokens[6].Name.length() + 1));

			Node_Add("data", S_INT_LITERAL, "\"" + Tokens[6].Name + "\"");
		}
	),

	Grammar_Checker(	// const byte Id[] = { 0, 1, 2, 3 };
		{
			Checker_Function(Is_Token, T_CONST),
			Checker_Function(Is_Token, T_BYTE), Checker_Function(Is_Token, T_IDENTIFIER),
			Checker_Function(Is_Token, T_OPEN_SQ), Checker_Function(Is_Token, T_CLOSE_SQ),
			Checker_Function(Is_Token, T_EQUALS),
			Checker_Function(Is_Token, T_OPEN_SC),
			Checker_Function(Parse_Recursive_Check, Int_Literals_Grammars),
			Checker_Function(Is_Token, T_CLOSE_SC),
			Checker_Function(Is_Token, T_SEMI)
		},
		Node_Init
		{
			Node_Add("type", S_BYTE_ARRAY, "byte[]");

			std::string Name = Local_Function_Scope_Name + Tokens[2].Name;

			Node_Add("id", S_ID16, Name);

			Add_To_Parser_Identifiers({ S_ID16, Name });

			Node_Add("size", S_INT_LITERAL, std::to_string(Get_Depth_Of_Node(Recursively_Generated_Nodes[0], "int_literals")));

			Node_Copy("data", Recursively_Generated_Nodes[0]);
		}
	)
};

const std::vector<Grammar_Checker> Stack_Definition_Grammars =
{
	Grammar_Checker(
		{	// byte Id;
			Checker_Function(Is_Token, T_BYTE), Checker_Function(Is_Token, T_IDENTIFIER), Checker_Function(Is_Token, T_SEMI)
		},

		Node_Init
		{
			Node_Add("type", S_BYTE, Tokens[0].Name);
			Node_Add("id", S_ID8, Tokens[1].Name);

			Add_To_Parser_Identifiers({ S_ID8, Tokens[1].Name });

			Node_Add("size", S_INT_LITERAL, "1");
		}
	),

	Grammar_Checker(
		{	// byte Id[0123];
			Checker_Function(Is_Token, T_BYTE), Checker_Function(Is_Token, T_IDENTIFIER), Checker_Function(Is_Token, T_OPEN_SQ), Checker_Function(Is_Token, T_INT_LITERAL), Checker_Function(Is_Token, T_CLOSE_SQ), Checker_Function(Is_Token, T_SEMI)
		},

		Node_Init
		{
			Node_Add("type", S_BYTE_ARRAY, "byte[]");	// an array of this nature is NOT analogous to a pointer because of how it's stored
			Node_Add("id", S_ID16, Tokens[1].Name);

			Add_To_Parser_Identifiers({ S_ID16, Tokens[1].Name });	// However it's still a 16-bit ID because the identifier itself refers to the address of the array

			Node_Add("size", S_INT_LITERAL, Tokens[3].Name);
		}
	),

	Grammar_Checker(
		{	// byte* Id;
			Checker_Function(Is_Token, T_BYTE), Checker_Function(Is_Token, T_POINTER), Checker_Function(Is_Token, T_IDENTIFIER), Checker_Function(Is_Token, T_SEMI)
		},

		Node_Init
		{
			Node_Add("type", S_BYTE_POINTER, "byte*");
			Node_Add("id", S_ID16, Tokens[2].Name);

			Add_To_Parser_Identifiers({ S_ID16, Tokens[2].Name });

			Node_Add("size", S_INT_LITERAL, "2");
		}
	)
};

//

const std::vector<Grammar_Checker> Parameter_Grammars =
{
	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_BYTE), Checker_Function(Is_Token, T_IDENTIFIER)
		},
		Node_Init
		{
			Node_Add("type", S_BYTE, "byte");
			Node_Add("id", S_ID8, Tokens[1].Name);

			Add_To_Parser_Identifiers({ S_ID8, Tokens[1].Name });

			Node_Add("size", S_INT_LITERAL, "1");
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_BYTE), Checker_Function(Is_Token, T_POINTER), Checker_Function(Is_Token, T_IDENTIFIER)
		},
		Node_Init
		{
			Node_Add("type", S_BYTE_POINTER, "byte*");
			Node_Add("id", S_ID16, Tokens[2].Name);

			Add_To_Parser_Identifiers({ S_ID16, Tokens[2].Name });

			Node_Add("size", S_INT_LITERAL, "2");
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_CONST), Checker_Function(Is_Token, T_BYTE), Checker_Function(Is_Token, T_POINTER), Checker_Function(Is_Token, T_IDENTIFIER)
		},
		Node_Init
		{
			Node_Add("type", S_BYTE_POINTER, "byte*");
			Node_Add("id", S_ID16, Tokens[2].Name);

			Add_To_Parser_Identifiers({ S_ID16, Tokens[2].Name });

			Node_Add("size", S_INT_LITERAL, "2");
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_WORD), Checker_Function(Is_Token, T_IDENTIFIER)
		},
		Node_Init
		{
			Node_Add("type", S_WORD, "word");
			Node_Add("id", S_ID16, Tokens[1].Name);

			Add_To_Parser_Identifiers({ S_ID16, Tokens[1].Name });

			Node_Add("size", S_INT_LITERAL, "2");
		}
	)
};

const std::vector<Grammar_Checker> Parameters_Grammars =
{
	Grammar_Checker(
		{
			Checker_Function(Parse_Recursive_Check, Parameter_Grammars), Checker_Function(Parse_Recursive_Check, Parameters_Grammars)
		},
		Node_Init
		{
			Node_Set(Recursively_Generated_Nodes[0]);

			Node_Copy("parameters", Recursively_Generated_Nodes[1]);
		}
	),
	Grammar_Checker(
		{
			Checker_Function(Parse_Recursive_Check, Parameter_Grammars)
		},
		Node_Init
		{
			Node_Set(Recursively_Generated_Nodes[0]);
		}
	)
};

const std::vector<Grammar_Checker> Return_Grammars =
{
	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_RETURN), Checker_Function(Is_Token, T_SEMI)
		},
		Node_Init
		{
			Node_Set(Parse_Node(S_RETURN, "return"));
		}
	)
};

const std::vector<Grammar_Checker> Statement_Grammars =
{
	Grammar_Checker(
		{
			Checker_Function(Parse_Recursive_Check, Stack_Definition_Grammars)	// declares a stack variable
		},
		Node_Init
		{
			Node_Set(Recursively_Generated_Nodes[0]);
			Node_Set_Syntax(S_STACK_DECLARATION_STATEMENT);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Parse_Recursive_Check, ROM_Declaration_Grammars)	// declares/sets a const ROM variable
		},
		Node_Init
		{
			Node_Set(Recursively_Generated_Nodes[0]);
			Node_Set_Syntax(S_ROM_DECLARATION_STATEMENT);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Parse_Recursive_Check, Return_Grammars)
		},
		Node_Init
		{
			Node_Set(Recursively_Generated_Nodes[0]);
			Node_Set_Syntax(S_RETURN);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Parse_Recursive_Check, ID_Assign_Grammars)
		},
		Node_Init
		{
			Node_Set(Recursively_Generated_Nodes[0]);
			Node_Set_Syntax(S_ID_ASSIGN);
		}
	)
};

const std::vector<Grammar_Checker> Statements_Grammars =
{
	Grammar_Checker(
		{
			Checker_Function(Parse_Recursive_Check, Statement_Grammars), Checker_Function(Parse_Recursive_Check, Statements_Grammars)
		},
		Node_Init
		{
			Node_Set(Recursively_Generated_Nodes[0]);
			Node_Copy("statements", Recursively_Generated_Nodes[1]);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Parse_Recursive_Check, Statement_Grammars)
		},
		Node_Init
		{
			Node_Set(Recursively_Generated_Nodes[0]);
		}
	)
};

const std::vector<Grammar_Checker> Type_Grammars =
{
	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_BYTE)
		},
		Node_Init
		{
			Node_Set(Parse_Node(S_BYTE, "byte"));
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_WORD)
		},
		Node_Init
		{
			Node_Set(Parse_Node(S_WORD, "word"));
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_BYTE), Checker_Function(Is_Token, T_POINTER)
		},
		Node_Init
		{
			Node_Set(Parse_Node(S_BYTE_POINTER, "byte*"));
		}
	)
};

const std::vector<Grammar_Checker> Return_Type_Grammars =
{
	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_VOID)
		},
		Node_Init
		{
			Node_Set(Parse_Node(S_VOID, "void"));
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Parse_Recursive_Check, Type_Grammars)
		},
		Node_Init
		{
			Node_Set(Recursively_Generated_Nodes[0]);
		}
	)
};

const std::vector<Grammar_Checker> Function_Grammars =
{
	Grammar_Checker(
		{
			Checker_Function(Parse_Recursive_Check, Return_Type_Grammars), Checker_Function(Parse_Recursive_Check, Identifier_Grammars),
			Checker_Function(Is_Token, T_OPEN_BR),
				Checker_Function(Parse_Recursive_Check, Parameters_Grammars),
			Checker_Function(Is_Token, T_CLOSE_BR),
			Checker_Function(Is_Token, T_OPEN_SC),
				Checker_Function(Parse_Recursive_Check, Statements_Grammars),
			Checker_Function(Is_Token, T_CLOSE_SC)
		},

		Node_Init
		{
			Node_Copy("return_type", Recursively_Generated_Nodes[0]);
			Node_Copy("id", Recursively_Generated_Nodes[1]);
			Node_Copy_Syntax("id", S_ID16);						// we want to specify that this is an ID16
			Node_Copy("parameters", Recursively_Generated_Nodes[2]);
			Node_Copy("statements", Recursively_Generated_Nodes[3]);
		}
	),

	Grammar_Checker(	// Some functions might have no parameters!
		{
			Checker_Function(Parse_Recursive_Check, Return_Type_Grammars), Checker_Function(Parse_Recursive_Check, Identifier_Grammars),
			Checker_Function(Is_Token, T_OPEN_BR),
			// No parameters term!
			Checker_Function(Is_Token, T_CLOSE_BR),
			Checker_Function(Is_Token, T_OPEN_SC),
				Checker_Function(Parse_Recursive_Check, Statements_Grammars),
			Checker_Function(Is_Token, T_CLOSE_SC)
		},

		Node_Init
		{
			Node_Copy("return_type", Recursively_Generated_Nodes[0]);
			Node_Copy("id", Recursively_Generated_Nodes[1]);
			Node_Copy_Syntax("id", S_ID16);						// we want to specify that this is an ID16
			Node_Copy("statements", Recursively_Generated_Nodes[2]);
		}
	)
};

const std::vector<Grammar_Checker> Global_Declaration_Grammars =
{
	Grammar_Checker(
		{
			Checker_Function(Parse_Recursive_Check, Function_Grammars)
		},
		Node_Init
		{
			Node_Set(Recursively_Generated_Nodes[0]);
			Node_Set_Syntax(S_FUNCTION_DEFINE);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Parse_Recursive_Check, ROM_Declaration_Grammars)
		},
		Node_Init
		{
			Node_Set(Recursively_Generated_Nodes[0]);
			Node_Set_Syntax(S_ROM_DECLARATION_STATEMENT);
		}
	)
};

const std::vector<Grammar_Checker> Global_Declarations_Grammars =
{
	Grammar_Checker(
		{
			Checker_Function(Parse_Recursive_Check, Global_Declaration_Grammars),
			Checker_Function(Parse_Recursive_Check, Global_Declarations_Grammars)
		},
		Node_Init
		{
			Node_Set(Recursively_Generated_Nodes[0]);
			Node_Copy("global_declarations", Recursively_Generated_Nodes[1]);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Parse_Recursive_Check, Global_Declaration_Grammars)
		},
		Node_Init
		{
			Node_Set(Recursively_Generated_Nodes[0]);
		}
	)
};
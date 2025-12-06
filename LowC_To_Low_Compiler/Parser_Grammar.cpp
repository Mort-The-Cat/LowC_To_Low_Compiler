#include "Parser_Grammar.h"
#include "Code_Parser.h"

size_t Return_Type_Of_Current_Parsed_Function;
std::vector<std::vector<Parse_Node>> Declared_Functions;	// This is used to get the return-type and parameter of a function when calling it using stock C syntax in the parser

void Generate_Parse_Tree(const Token* Tokens, std::vector<Parse_Node>* Node)
{
	Parse_Recursive_Check(Tokens, Node, Global_Declarations_Grammars, T_INVALID);
}

size_t Line_Length(std::string& Value)
{
	size_t Counter = 0;

	for (size_t Index = 0; Index < Value.size(); Index++)
	{
		if (Value[Index] == '\n')
			Counter = 0;
		else
			Counter++;
	}

	return Counter;
}

size_t Get_Depth_Of_Node(std::string& Value, const Parse_Node& Node, const char* Sub_ID)
{
	const Parse_Node* Current_Node = &Node;
	size_t Counter = 1;

	Value += Node.Value; // +" ";
	if (Line_Length(Value) > 40)
		Value += "\n\t";
	else
		Value += " ";

	while (Current_Node->Child_Nodes.contains(Sub_ID))
	{
		Counter++;

		Value += Current_Node->Value; // +" ";
		if (Line_Length(Value) > 40)
			Value += "\n\t";
		else
			Value += " ";

		Current_Node = &Current_Node->Child_Nodes.at(Sub_ID)[0];
	}

	return Counter;
}

/*size_t Get_Depth_Of_Node_Old(std::string& Value, const Parse_Node& Node, const char* Sub_ID)
{
	size_t Val = 1;
	Value += Node.Value; // +" ";
	if (Line_Length(Value) > 40)
		Value += "\n\t";
	else
		Value += " ";
	if (Node.Child_Nodes.contains(Sub_ID))
		Val += Get_Depth_Of_Node(Value, Node.Child_Nodes.at(Sub_ID)[0], Sub_ID);
	return Val;
}*/

size_t Get_Syntax_ID_Of_Identifier(const char* Name)
{
	for (long Index = Parser_Identifiers.size() - 1; Index != -1; Index--)
		if (0 == strcmp(Name, Parser_Identifiers[Index].Name.c_str()))
			return Parser_Identifiers[Index].Token;

	for (long Index = Parser_Identifiers.size() - 1; Index != -1; Index--)
		if (0 == strcmp((Local_Function_Scope_Name + Name).c_str(), Parser_Identifiers[Index].Name.c_str()))
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

size_t Parse_Int_Literals_Check(const Token* Tokens, std::vector<Parse_Node>* Node, const std::vector<Grammar_Checker>& Grammars, size_t Syntax_ID)
{
	// 'Node' is the recursively generated nodes

	// We'll only output 1

	if (Tokens[0].Token == T_INT_LITERAL || Tokens[0].Token == T_IDENTIFIER || Tokens[0].Token == T_HIGH)
	{
		Parse_Node Root_Node;

		Root_Node.Value = Tokens[0].Name;
		Root_Node.Syntax_ID = S_INT_LITERAL;

		size_t Index = 1;

		if (Tokens[0].Token == T_HIGH) // if this is a 'high' expression
		{
			Root_Node.Value += Tokens[Index + 2].Name; // (
			Root_Node.Value += Tokens[Index + 3].Name; // value
			Root_Node.Value += Tokens[Index + 4].Name; // )
			Index += 3;
		}

		while (Tokens[Index].Token == T_COMMA)
		{
			Root_Node.Value += " ";

			if (Line_Length(Root_Node.Value) > 40)
				Root_Node.Value += "\n\t";

			Root_Node.Value += Tokens[Index + 1].Name;

			if (Tokens[Index + 1].Token == T_HIGH) // if this is a 'high' expression
			{
				Root_Node.Value += Tokens[Index + 2].Name; // (
				Root_Node.Value += Tokens[Index + 3].Name; // value
				Root_Node.Value += Tokens[Index + 4].Name; // )
				Index += 3;
			}

			Index += 2;
		}

		Node->push_back(std::move(Root_Node));

		return Index;
	}
	else
		return 0;
}

const std::vector<Grammar_Checker> Int_Literals_Grammars =
{
	/*Grammar_Checker(	// int literal, int literals
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
	)*/

	Grammar_Checker(
		{
			Checker_Function(Parse_Int_Literals_Check, 0)
		},
		Node_Init
		{
			Node_Set(Recursively_Generated_Nodes[0]);
		}
	)
};

const std::vector<Grammar_Checker> Expression16_Grammars =
{
	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_AND),	// & token (this is used for the and operator AS WELL AS the reference operator)
			Checker_Function(Is_Token, T_IDENTIFIER)	// Some kind of local identifier (can be 8-bit or 16-bit
		},
		Node_Init
		{
			Node_Add("id", S_ID8, Tokens[1].Name);
			Node_Set_Syntax(S_REFERENCE);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_SHIFT_RIGHT),
			Checker_Function(Is_Token, T_OPEN_BR),
			Checker_Function(Parse_Recursive_Check, Expression16_Grammars),
			Checker_Function(Is_Token, T_CLOSE_BR)
		},
		Node_Init
		{
			Node_Copy("value", Recursively_Generated_Nodes[0]);
			Node_Set_Syntax(S_RIGHT_SHIFT16);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_SHIFT_LEFT),
			Checker_Function(Is_Token, T_OPEN_BR),
			Checker_Function(Parse_Recursive_Check, Expression16_Grammars),
			Checker_Function(Is_Token, T_CLOSE_BR)
		},
		Node_Init
		{
			Node_Copy("value", Recursively_Generated_Nodes[0]);
			Node_Set_Syntax(S_LEFT_SHIFT16);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_OPEN_BR),
			Checker_Function(Parse_Recursive_Check, Expression16_Grammars),
			Checker_Function(Is_Token, T_CLOSE_BR)
		},
		Node_Init
		{
			Node_Set(Recursively_Generated_Nodes[0]);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_SIZEOF),
			Checker_Function(Is_Token, T_OPEN_BR),
			Checker_Function(Is_Token, T_IDENTIFIER),
			Checker_Function(Is_Token, T_CLOSE_BR)
		},

		Node_Init
		{
			Node_Add("id", S_ID16, Tokens[2].Name);
			Node_Set_Syntax(S_SIZEOF);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_OPEN_BR),
			Checker_Function(Parse_Recursive_Check, Expression16_Grammars),
			Checker_Function(Is_Token, T_PLUS),
			Checker_Function(Parse_Recursive_Check, Expression16_Grammars),
			Checker_Function(Is_Token, T_CLOSE_BR)
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
			Checker_Function(Is_Token, T_OPEN_BR),
			Checker_Function(Parse_Recursive_Check, Expression16_Grammars),
			Checker_Function(Is_Token, T_PLUS),
			Checker_Function(Parse_Recursive_Check, Expression8_Grammars),
			Checker_Function(Is_Token, T_CLOSE_BR)
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
			Checker_Function(Is_ID, S_ID16),
			Checker_Function(Is_Token, T_PLUS),
			Checker_Function(Parse_Recursive_Check, Expression8_Grammars)
		},
		Node_Init
		{
			Node_Copy("left", Parse_Node(S_ID16, Tokens[0].Name));
			Node_Copy("right", Recursively_Generated_Nodes[0]);
			Node_Set_Syntax(S_PLUS16_8);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_INT_LITERAL),
			Checker_Function(Is_Token, T_PLUS),
			Checker_Function(Parse_Recursive_Check, Expression8_Grammars)
		},
		Node_Init
		{
			Node_Copy("left", Parse_Node(S_INT_LITERAL, Tokens[0].Name));
			Node_Copy("right", Recursively_Generated_Nodes[0]);
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

const std::vector<Grammar_Checker> Operator8_Grammars =
{
	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_PLUS)
		},
		Node_Init
		{
			Node_Set_Syntax(S_PLUS8);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_MINUS)
		},
		Node_Init
		{
			Node_Set_Syntax(S_MINUS8);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_AND)
		},
		Node_Init
		{
			Node_Set_Syntax(S_AND8);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_OR)
		},
		Node_Init
		{
			Node_Set_Syntax(S_OR8);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_XOR)
		},
		Node_Init
		{
			Node_Set_Syntax(S_XOR8);
		}
	)
};

const std::vector<Grammar_Checker> Expression8_Grammars =
{
	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_SHIFT_RIGHT),
			Checker_Function(Is_Token, T_OPEN_BR),
			Checker_Function(Parse_Recursive_Check, Expression8_Grammars),
			Checker_Function(Is_Token, T_CLOSE_BR)
		},
		Node_Init
		{
			Node_Copy("value", Recursively_Generated_Nodes[0]);
			Node_Set_Syntax(S_RIGHT_SHIFT8);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_SHIFT_LEFT),
			Checker_Function(Is_Token, T_OPEN_BR),
			Checker_Function(Parse_Recursive_Check, Expression8_Grammars),
			Checker_Function(Is_Token, T_CLOSE_BR)
		},
		Node_Init
		{
			Node_Copy("value", Recursively_Generated_Nodes[0]);
			Node_Set_Syntax(S_LEFT_SHIFT8);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_OPEN_BR),
			Checker_Function(Is_Token, T_BYTE),
			Checker_Function(Is_Token, T_CLOSE_BR),
			Checker_Function(Parse_Recursive_Check, Expression16_Grammars)
		},
		Node_Init
		{
			Node_Set(Recursively_Generated_Nodes[0]);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_HIGH),
			Checker_Function(Is_Token, T_OPEN_BR),
			Checker_Function(Parse_Recursive_Check, Expression16_Grammars),
			Checker_Function(Is_Token, T_CLOSE_BR)
		},
		Node_Init
		{
			Node_Set_Syntax(S_HIGH);
			Node_Copy("value", Recursively_Generated_Nodes[0]);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_OPEN_BR),
			Checker_Function(Parse_Recursive_Check, Expression8_Grammars),
			Checker_Function(Is_Token, T_CLOSE_BR)
		},
		Node_Init
		{
			Node_Set(Recursively_Generated_Nodes[0]);
			//Node_Set_Syntax(S_EXPRESSION8)
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_INT_LITERAL),
			Checker_Function(Parse_Recursive_Check, Operator8_Grammars),
			Checker_Function(Parse_Recursive_Check, Expression8_Grammars)
		},
		Node_Init
		{
			Node_Copy("left", Parse_Node(S_INT_LITERAL, Tokens[0].Name));
			Node_Copy("right", Recursively_Generated_Nodes[1]);
			Node_Set_Syntax(Recursively_Generated_Nodes[0].Syntax_ID);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_IDENTIFIER),
			Checker_Function(Parse_Recursive_Check, Operator8_Grammars),
			Checker_Function(Parse_Recursive_Check, Expression8_Grammars)
		},
		Node_Init
		{
			Node_Copy("left", Parse_Node(S_ID8, Tokens[0].Name));
			Node_Copy("right", Recursively_Generated_Nodes[1]);
			Node_Set_Syntax(Recursively_Generated_Nodes[0].Syntax_ID);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_POINTER),
			Checker_Function(Parse_Recursive_Check, Expression16_Grammars)
		},
		Node_Init
		{
			Node_Copy("address", Recursively_Generated_Nodes[0]);
			Node_Set_Syntax(S_DEREF8);
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
	)
};

const std::vector<Grammar_Checker> Dest_Assign_Grammars =
{
	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_POINTER),
			Checker_Function(Parse_Recursive_Check, Expression16_Grammars),
			Checker_Function(Is_Token, T_EQUALS),
			Checker_Function(Parse_Recursive_Check, Expression8_Grammars),
			Checker_Function(Is_Token, T_SEMI)
		},
		Node_Init
		{
			Node_Copy("destination", Recursively_Generated_Nodes[0]);
			Node_Copy("value", Recursively_Generated_Nodes[1]);
		}
	)
};

const std::vector<Grammar_Checker> Inc_Dec_Operator_Grammars =
{
	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_PLUS_PLUS)
		},
		Node_Init
		{
			Node_Set_Syntax(S_PLUS8);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_MINUS_MINUS)
		},
		Node_Init
		{
			Node_Set_Syntax(S_MINUS8);
		}
	)
};

const std::vector<Grammar_Checker> ID_Inc_Dec_Grammars =
{
	Grammar_Checker(
		{
			Checker_Function(Is_ID, S_ID8), Checker_Function(Parse_Recursive_Check, Inc_Dec_Operator_Grammars),
			Checker_Function(Is_Token, T_SEMI)
		},
		Node_Init
		{
			Node_Add("id", S_ID8, Tokens[0].Name);
			Node_Copy("operator", Recursively_Generated_Nodes[0]);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Is_ID, S_ID16), Checker_Function(Parse_Recursive_Check, Inc_Dec_Operator_Grammars),
			Checker_Function(Is_Token, T_SEMI)
		},
		Node_Init
		{
			Node_Add("id", S_ID16, Tokens[0].Name);
			Node_Copy("operator", Recursively_Generated_Nodes[0]);
			Node_Set_Syntax(S_ID_INC_DEC);
		}
	)
};

const std::vector<Grammar_Checker> Dest_Inc_Dec_Grammars =
{
	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_POINTER), Checker_Function(Parse_Recursive_Check, Expression16_Grammars),
			Checker_Function(Parse_Recursive_Check, Inc_Dec_Operator_Grammars),
			Checker_Function(Is_Token, T_SEMI)
		},
		Node_Init
		{
			Node_Copy("destination", Recursively_Generated_Nodes[0]);
			Node_Copy("operator", Recursively_Generated_Nodes[1]);
			Node_Set_Syntax(S_DEST_INC_DEC);
		}
	)
};

const std::vector<Grammar_Checker> ID_Assign_Grammars =
{
	Grammar_Checker(
		{
			Checker_Function(Is_ID, S_ID8), Checker_Function(Is_Token, T_EQUALS), Checker_Function(Parse_Recursive_Check, Expression8_Grammars), 
			Checker_Function(Is_Token, T_SEMI)
		},
		Node_Init
		{
			Node_Add("id", S_ID8, Tokens[0].Name);
			Node_Copy("value", Recursively_Generated_Nodes[0]);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Is_ID, S_ID8), Checker_Function(Is_Token, T_EQUALS), Checker_Function(Parse_Recursive_Check, Expression16_Grammars),
			Checker_Function(Is_Token, T_SEMI)
		},
		Node_Init
		{
			Node_Add("id", S_ID8, Tokens[0].Name);
			Node_Copy("value", Recursively_Generated_Nodes[0]);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Is_ID, S_ID16), Checker_Function(Is_Token, T_EQUALS), Checker_Function(Parse_Recursive_Check, Expression16_Grammars),
			Checker_Function(Is_Token, T_SEMI)
		},
		Node_Init
		{
			Node_Add("id", S_ID16, Tokens[0].Name);
			Node_Copy("value", Recursively_Generated_Nodes[0]);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Is_ID, S_ID16), Checker_Function(Is_Token, T_EQUALS), Checker_Function(Parse_Recursive_Check, Expression8_Grammars),
			Checker_Function(Is_Token, T_SEMI)
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

			Node_Add("size", S_INT_LITERAL, std::to_string(1 + std::count(Recursively_Generated_Nodes[0].Value.begin(), Recursively_Generated_Nodes[0].Value.end(), ' ')));

			//Node_Add("size", S_INT_LITERAL, std::to_string(Get_Depth_Of_Node(Value, Recursively_Generated_Nodes[0], "int_literals")));

			Node_Add("data", S_INT_LITERAL, Recursively_Generated_Nodes[0].Value);

			//Node_Copy("data", Recursively_Generated_Nodes[0]);
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
	),

	Grammar_Checker(
		{	// byte* Id;
			Checker_Function(Is_Token, T_CONST), Checker_Function(Is_Token, T_BYTE), Checker_Function(Is_Token, T_POINTER), Checker_Function(Is_Token, T_IDENTIFIER), Checker_Function(Is_Token, T_SEMI)
		},

		Node_Init
		{
			Node_Add("type", S_BYTE_POINTER, "byte*");
			Node_Add("id", S_ID16, Tokens[3].Name);

			Add_To_Parser_Identifiers({ S_ID16, Tokens[3].Name });

			Node_Add("size", S_INT_LITERAL, "2");
		}
	),

	Grammar_Checker(
		{	// word Id;
			Checker_Function(Is_Token, T_WORD), Checker_Function(Is_Token, T_IDENTIFIER), Checker_Function(Is_Token, T_SEMI)
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

//

const std::vector<Grammar_Checker> Parameter_Grammars =
{
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
			Node_Add("id", S_ID16, Tokens[3].Name);

			Add_To_Parser_Identifiers({ S_ID16, Tokens[3].Name });

			Node_Add("size", S_INT_LITERAL, "2");
		}
	),

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
			Checker_Function(Parse_Recursive_Check, Parameter_Grammars), 
			Checker_Function(Is_Token, T_COMMA),
			Checker_Function(Parse_Recursive_Check, Parameters_Grammars)
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
			Checker_Function(Is_Token, T_RETURN)//, Checker_Function(Is_Token, T_SEMI)
		},
		Node_Init
		{
			Node_Set(Parse_Node(S_RETURN, "return"));

			if (Return_Type_Of_Current_Parsed_Function == S_VOID)
			{
				// just increment it once past the semicolon

				Tokens_Passed++;
			}
			else
			{
				std::vector<Parse_Node> Generated_Nodes;

				if (Return_Type_Of_Current_Parsed_Function == S_BYTE)
				{
					Tokens_Passed += Parse_Recursive_Check(Tokens + Tokens_Passed, &Generated_Nodes, Expression8_Grammars, 0) + 1;
				}
				else
					Tokens_Passed += Parse_Recursive_Check(Tokens + Tokens_Passed, &Generated_Nodes, Expression16_Grammars, 0) + 1;

				// Put it on the "value" node

				Node_Copy("value", Generated_Nodes[0]);
			}
		}
	)
};

const std::vector<Grammar_Checker> Statement_Grammars =
{
	Grammar_Checker(
		{
			Checker_Function(Parse_Recursive_Check, Function_Call_Grammars),
			Checker_Function(Is_Token, T_SEMI)
		},
		Node_Init
		{
			Node_Set(Recursively_Generated_Nodes[0]);
			Node_Set_Syntax(S_FUNCTION_CALL);
		}

	),

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
			Checker_Function(Parse_Recursive_Check, ID_Inc_Dec_Grammars)
		},
		Node_Init
		{
			Node_Set(Recursively_Generated_Nodes[0]);
			Node_Set_Syntax(S_ID_INC_DEC);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Parse_Recursive_Check, Dest_Inc_Dec_Grammars)
		},
		Node_Init
		{
			Node_Set(Recursively_Generated_Nodes[0]);
			Node_Set_Syntax(S_DEST_INC_DEC);
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
	),

	Grammar_Checker(
		{
			Checker_Function(Parse_Recursive_Check, Dest_Assign_Grammars)
		},
		Node_Init
		{
			Node_Set(Recursively_Generated_Nodes[0]);
			Node_Set_Syntax(S_DEST_ASSIGN);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Parse_Recursive_Check, If_Grammars)
		},
		Node_Init
		{
			Node_Set(Recursively_Generated_Nodes[0]);
			Node_Set_Syntax(S_IF_STATEMENT);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Parse_Recursive_Check, While_Grammars)
		},
		Node_Init
		{
			Node_Set(Recursively_Generated_Nodes[0]);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Parse_Recursive_Check, Do_While_Grammars)
		},
		Node_Init
		{
			Node_Set(Recursively_Generated_Nodes[0]);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_STORE_HIGH), Checker_Function(Is_Token, T_OPEN_BR),
			Checker_Function(Is_ID, S_ID16), Checker_Function(Is_Token, T_COMMA),
			Checker_Function(Parse_Recursive_Check, Expression8_Grammars),
			Checker_Function(Is_Token, T_CLOSE_BR), Checker_Function(Is_Token, T_SEMI)
		},
		Node_Init
		{
			Node_Add("id", S_ID16, Tokens[2].Name);
			Node_Copy("value", Recursively_Generated_Nodes[0]);
			Node_Set_Syntax(S_STORE_HIGH);
		}
	),

	Grammar_Checker(
	{
		Checker_Function(Is_Token, T_STORE_LOW), Checker_Function(Is_Token, T_OPEN_BR),
		Checker_Function(Is_ID, S_ID16), Checker_Function(Is_Token, T_COMMA),
		Checker_Function(Parse_Recursive_Check, Expression8_Grammars),
		Checker_Function(Is_Token, T_CLOSE_BR), Checker_Function(Is_Token, T_SEMI)
		},
		Node_Init
		{
			Node_Add("id", S_ID16, Tokens[2].Name);
			Node_Copy("value", Recursively_Generated_Nodes[0]);
			Node_Set_Syntax(S_STORE_LOW);
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

const std::vector<Grammar_Checker> Condition_Grammars =
{
	Grammar_Checker(
		{
		Checker_Function(Is_Token, T_NOT),
		Checker_Function(Parse_Recursive_Check, Condition_Grammars)
		},
		Node_Init
		{
			Node_Copy("value", Recursively_Generated_Nodes[0]);
			Node_Set_Syntax(S_NOT);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_BIT), Checker_Function(Is_Token, T_OPEN_BR),
			Checker_Function(Parse_Recursive_Check, Expression8_Grammars),
			Checker_Function(Is_Token, T_COMMA),
			Checker_Function(Parse_Recursive_Check, Int_Literals_Grammars),	// Note we only really care about 1 (one) int literal
			Checker_Function(Is_Token, T_CLOSE_BR)
		},
		Node_Init
		{
			Node_Copy("value", Recursively_Generated_Nodes[0]);
			Node_Copy("bit", Recursively_Generated_Nodes[1]);
			Node_Set_Syntax(S_BIT);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Parse_Recursive_Check, Expression8_Grammars),
			Checker_Function(Is_Token, T_LESS_THAN),
			Checker_Function(Parse_Recursive_Check, Expression8_Grammars)
		},
		Node_Init
		{
			Node_Copy("left", Recursively_Generated_Nodes[0]),
			Node_Copy("right", Recursively_Generated_Nodes[1]),
			Node_Set_Syntax(S_LESS_THAN8);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Parse_Recursive_Check, Expression8_Grammars),
			Checker_Function(Is_Token, T_GREATER_THAN),
			Checker_Function(Parse_Recursive_Check, Expression8_Grammars)
		},
		Node_Init
		{
			Node_Copy("left", Recursively_Generated_Nodes[1]),		// We'll use the same operator, just swap the operands around
			Node_Copy("right", Recursively_Generated_Nodes[0]),
			Node_Set_Syntax(S_LESS_THAN8);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Parse_Recursive_Check, Expression8_Grammars)
		},
		Node_Init
		{
			Node_Copy("value", Recursively_Generated_Nodes[0]);
			Node_Set_Syntax(S_NOT_ZERO);
		}
	)
};

const std::vector<Grammar_Checker> Do_While_Grammars =
{
	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_DO), Checker_Function(Is_Token, T_OPEN_SC),
			Checker_Function(Parse_Recursive_Check, Statements_Grammars),
			Checker_Function(Is_Token, T_CLOSE_SC),
			Checker_Function(Is_Token, T_WHILE),
			Checker_Function(Is_Token, T_OPEN_BR),
			Checker_Function(Parse_Recursive_Check, Condition_Grammars),
			Checker_Function(Is_Token, T_CLOSE_BR),
			Checker_Function(Is_Token, T_SEMI)
		},
		Node_Init
		{
			Node_Copy("local_statements", Recursively_Generated_Nodes[0]);
			Node_Copy("condition", Recursively_Generated_Nodes[1]);
			Node_Set_Syntax(S_DO_WHILE_LOOP);
		}
	)
};

const std::vector<Grammar_Checker> While_Grammars =
{
	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_WHILE), Checker_Function(Is_Token, T_OPEN_BR),
			Checker_Function(Parse_Recursive_Check, Condition_Grammars),
			Checker_Function(Is_Token, T_CLOSE_BR),
			Checker_Function(Is_Token, T_OPEN_SC),
			Checker_Function(Parse_Recursive_Check, Statements_Grammars),
			Checker_Function(Is_Token, T_CLOSE_SC)
		},
		Node_Init
		{
			/*
			
			while(condition)
			{
				statements
			}

			is equivalent to

			if(condition)
			{
				do
				{
					statements
				} while(condition);
			}
			
			So I'll just store it as such
			
			*/

			Parse_Node Do_While_Loop;
			Do_While_Loop.Child_Nodes["local_statements"].push_back(Recursively_Generated_Nodes[1]);
			Do_While_Loop.Child_Nodes["condition"].push_back(Recursively_Generated_Nodes[0]);
			Do_While_Loop.Syntax_ID = S_DO_WHILE_LOOP;
			Node_Copy("local_statements", Do_While_Loop);
			Node_Copy("condition", Recursively_Generated_Nodes[0]);
			Node_Set_Syntax(S_IF_STATEMENT);
		}
	)
};

const std::vector<Grammar_Checker> If_Grammars =
{
	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_IF), Checker_Function(Is_Token, T_OPEN_BR),
			Checker_Function(Parse_Recursive_Check, Condition_Grammars),
			Checker_Function(Is_Token, T_CLOSE_BR),
			Checker_Function(Is_Token, T_OPEN_SC),
			Checker_Function(Parse_Recursive_Check, Statements_Grammars),
			Checker_Function(Is_Token, T_CLOSE_SC)
		},
		Node_Init
		{
			Node_Copy("condition", Recursively_Generated_Nodes[0]);
			Node_Copy("local_statements", Recursively_Generated_Nodes[1]);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_IF), Checker_Function(Is_Token, T_OPEN_BR),
			Checker_Function(Parse_Recursive_Check, Condition_Grammars),
			Checker_Function(Is_Token, T_CLOSE_BR),
			Checker_Function(Parse_Recursive_Check, Statement_Grammars)
		},
		Node_Init
		{
			Node_Copy("condition", Recursively_Generated_Nodes[0]);
			Node_Copy("local_statements", Recursively_Generated_Nodes[1]);
		}
	)
};

const std::vector<Grammar_Checker> Type_Grammars =
{
	Grammar_Checker(
		{
			Checker_Function(Is_Token, T_BYTE), Checker_Function(Is_Token, T_POINTER)
		},
		Node_Init
		{
			Node_Set(Parse_Node(S_BYTE_POINTER, "byte*"));
		}
	),

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

const std::vector<Grammar_Checker> Function_Dec_Grammars =
{
	Grammar_Checker(
		{
			Checker_Function(Parse_Recursive_Check, Return_Type_Grammars), Checker_Function(Parse_Recursive_Check, Identifier_Grammars),
			Checker_Function(Is_Token, T_OPEN_BR),
			Checker_Function(Parse_Recursive_Check, Parameters_Grammars),
			Checker_Function(Is_Token, T_CLOSE_BR), Checker_Function(Is_Token, T_SEMI)
		},

		Node_Init
		{
			Node_Copy("return_type", Recursively_Generated_Nodes[0]);
			Node_Copy("id", Recursively_Generated_Nodes[1]);
			Node_Copy_Syntax("id", S_ID16);						// we want to specify that this is an ID16

			Add_To_Parser_Identifiers({ S_ID16, Recursively_Generated_Nodes[1].Value });

			Node_Copy("parameters", Recursively_Generated_Nodes[2]);
		}
	),

	Grammar_Checker(
		{
			Checker_Function(Parse_Recursive_Check, Return_Type_Grammars), Checker_Function(Parse_Recursive_Check, Identifier_Grammars),
			Checker_Function(Is_Token, T_OPEN_BR),
			Checker_Function(Is_Token, T_CLOSE_BR), Checker_Function(Is_Token, T_SEMI)
		},

		Node_Init
		{
			Node_Copy("return_type", Recursively_Generated_Nodes[0]);
			Node_Copy("id", Recursively_Generated_Nodes[1]);
			Node_Copy_Syntax("id", S_ID16);						// we want to specify that this is an ID16

			Add_To_Parser_Identifiers({ S_ID16, Recursively_Generated_Nodes[1].Value });
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

			Add_To_Parser_Identifiers({ S_ID16, Recursively_Generated_Nodes[1].Value });

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

			Add_To_Parser_Identifiers({ S_ID16, Recursively_Generated_Nodes[1].Value });

			Node_Copy("statements", Recursively_Generated_Nodes[2]);
		}
	)
};

std::vector<Parse_Node>& Get_Function_Call(std::string Name)
{
	for (size_t W = 0; W < Declared_Functions.size(); W++)
		if (Declared_Functions[W][1].Value == Name)
			return Declared_Functions[W];
}

void Parse_Function_Call_Parameters(const Token* Tokens, std::vector<Parse_Node>& Recursively_Generated_Nodes, Parse_Node* New_Node, size_t& Tokens_Passed, const Parse_Node& Parameter_Node)
{
	if (Parameter_Node["type"][0].Syntax_ID == S_BYTE)
		Tokens_Passed += Parse_Recursive_Check(Tokens + Tokens_Passed, &Recursively_Generated_Nodes, Expression8_Grammars, 0);
	else
		Tokens_Passed += Parse_Recursive_Check(Tokens + Tokens_Passed, &Recursively_Generated_Nodes, Expression16_Grammars, 0);

	Node_Copy("parameters", Recursively_Generated_Nodes.back());

	if (Parameter_Node.Child_Nodes.count("parameters"))
	{
		Tokens_Passed++;	// skips the comma

		//New_Node->Child_Nodes["parameters"][0].Child_Nodes["parameters"].resize(1);

		Parse_Function_Call_Parameters(Tokens, Recursively_Generated_Nodes, &New_Node->Child_Nodes["parameters"][0], Tokens_Passed, Parameter_Node["parameters"][0]);
	}
}

const std::vector<Grammar_Checker> Function_Call_Grammars =
{
	Grammar_Checker(
		{
			Checker_Function(Is_ID, S_ID16),
			Checker_Function(Is_Token, T_OPEN_BR)
		},
		Node_Init
		{
			// Here, we want to let this be a function call, and then we recursively gather the parameter expressions based on data types
			Node_Set_Syntax(S_FUNCTION_CALL);

			Node_Add("id", S_ID16, Tokens[0].Name);

			std::vector<Parse_Node>& Function_Dec = Get_Function_Call(Tokens[0].Name);

			Node_Copy("return_type", Function_Dec[0]);

			if (Function_Dec.size() == 3)
			{

				Parse_Function_Call_Parameters(Tokens, Recursively_Generated_Nodes, New_Node, Tokens_Passed, Function_Dec[2]);
			}

			Tokens_Passed++; // close bracket
		}
	)
};

const std::vector<Grammar_Checker> Global_Declaration_Grammars =
{
	Grammar_Checker(
		{
			Checker_Function(Parse_Recursive_Check, ROM_Declaration_Grammars)
		},
		Node_Init
		{
			Node_Set(Recursively_Generated_Nodes[0]);
			Node_Set_Syntax(S_ROM_DECLARATION_STATEMENT);
		}
	),
	
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
			Checker_Function(Parse_Recursive_Check, Function_Dec_Grammars)
		},
		Node_Init
		{
			Node_Set(Recursively_Generated_Nodes[0]);
			Node_Set_Syntax(S_FUNCTION_DEC);
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
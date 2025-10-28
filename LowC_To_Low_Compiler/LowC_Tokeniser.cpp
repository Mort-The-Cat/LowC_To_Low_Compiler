#include "LowC_Tokeniser.h"

//

Token Compiler_Tokens[NUMBER_OF_TOKEN_IDS] =
{
	// can't use out-of-ordered initialiser, will just sort it explicitly afterwards accordingly to Token value
	{	T_VOID,			"void"		},
	{	T_CONST,		"const"		},		// Very important!! Lets us know if we can store something in ROM or not
	{	T_BYTE,			"byte"		},
	{	T_WORD,			"word"		},

	{	T_IF,			"if"		},
	{	T_WHILE,		"while"		},
	{	T_DO,			"do"		},
	{	T_RETURN,		"return"	},

	{	T_IDENTIFIER,	""			},

	{	T_OPEN_BR,		"("			},
	{	T_CLOSE_BR,		")"			},
	{	T_OPEN_SC,		"{"			},
	{	T_CLOSE_SC,		"}"			},
	{	T_OPEN_SQ,		"["			},
	{	T_CLOSE_SQ,		"]"			},
	{	T_SEMI,			";"			},
	{	T_EQUALS,		"="			},
	{	T_PLUS_EQUALS,	"+="		},
	{	T_MINUS_EQUALS, "-="		},
	{	T_PLUS_PLUS,	"++"		},
	{	T_MINUS_MINUS,	"--"		},
	{	T_AND_EQUALS,	"&="		},
	{	T_OR_EQUALS,	"|="		},
	{	T_XOR_EQUALS,	"^="		},
	{	T_POINTER,		"*"			},
	{	T_AMPERSAND,	"&"			}
};

//

size_t Is_Alphanumeric(char Character)
{
	if (Character >= '0' && Character <= '9')
		return CHARACTER_NUMBER;

	if (
		(Character >= 'a' && Character <= 'z') ||
		(Character >= 'A' && Character <= 'Z') ||
		(Character == '_')
		)
		return CHARACTER_LETTER;

	return CHARACTER_NONALPHANUMERIC;
}

bool Sort_Token_Compare(const Token& A, const Token& B)
{
	return A.Token > B.Token;
}

size_t String_Matches_Token(const char* Text, size_t Length, const char* Token_Name)
{
	// iterate while length-- and token_name matches text

	size_t Index = 0;

	while (Length-- && Text[Index] == Token_Name[Index])
		Index++;

	return Index * (0 == Token_Name[Index]);
}

void Sort_Compiler_Tokens()	// needs to be called to sort these elements
{
	Token New_Tokens[NUMBER_OF_TOKEN_IDS];

	for (size_t W = 0; W < NUMBER_OF_TOKEN_IDS; W++)
		if (Compiler_Tokens[W].Token != T_INVALID)
			New_Tokens[Compiler_Tokens[W].Token] = Compiler_Tokens[W];

	memcpy(Compiler_Tokens, New_Tokens, sizeof(Compiler_Tokens));
}

// We'll make a kind of parse tree for this

struct Defined_Macro
{
	std::string Name, Representation;
};

void Get_C_Project_File_Contents(std::string* String, const char* File_Directory, std::vector<std::string>* Included_Files, std::vector<Defined_Macro>* Macros)
{
	// This'll load line-by-line recursively 

	if (!strcmp(File_Directory, "stdlow.h"))	// We don't actually want the compiler to compile the stdlow file because it's not designed for the SM83 architecture
		return;

	std::ifstream File(File_Directory);

	Included_Files->push_back(File_Directory);

	if (!File.is_open())
	{
		printf("ERROR READING FROM FILE! %s\n", File_Directory);
		return;
	}

	std::string Buffer;

	while (std::getline(File, Buffer))
	{
		if (String_Matches_Token(Buffer.c_str(), Buffer.size(), "#include \""))
		{
			std::string New_File;
			while ((Buffer.c_str() + 10)[New_File.size()] != '\"')
				New_File.push_back((Buffer.c_str() + 10)[New_File.size()]);

			Get_C_Project_File_Contents(String, New_File.c_str(), Included_Files, Macros); // loads the new files accordingly
		}
		else if (String_Matches_Token(Buffer.c_str(), Buffer.size(), "#define "))
		{
			Defined_Macro Macro;
			
			while ((Buffer.c_str() + 8)[Macro.Name.size()] != ' ')
				Macro.Name.push_back((Buffer.c_str() + 8)[Macro.Name.size()]);

			while ((Buffer.c_str() + 9)[Macro.Name.size() + Macro.Representation.size()] != '\0')
				Macro.Representation.push_back((Buffer.c_str() + 9)[Macro.Name.size() + Macro.Representation.size()]);

			Macros->push_back(Macro);
		}
		else
		{
			for (size_t W = 0; W < Buffer.size(); W++)
				String->push_back(Buffer[W]);			// loads character-by-character, is a little slow but I don't mind

			String->push_back('\n');
		}
	}

	File.close();
}

void Get_File_Contents(std::string* String, const char* File_Directory)
{
	std::vector<std::string> Included_Files;

	std::vector<Defined_Macro> Macros;

	Get_C_Project_File_Contents(String, File_Directory, &Included_Files, &Macros);

	//

	// Removes comments etc from the macros

	for (size_t W = 0; W < Macros.size(); W++)
	{
		size_t Index;

		Index = Macros[W].Representation.find("//");
		if (Index != std::string::npos)
			Macros[W].Representation.resize(Index);

		// Replaces macros with the representations
		
		while((Index = String->find(Macros[W].Name)) != std::string::npos)
			String->replace(Index, Macros[W].Name.length(), Macros[W].Representation);
	}
}

size_t Count_Valid_Identifier(const char* File)
{
	// counts how many characters from the start of the 'File' pointer are alphanumeric (whilst disallowing numerals at the start of the string)

	size_t Count = 1;

	if (Is_Alphanumeric(File[0]) != CHARACTER_LETTER)
		return 0;

	while (Is_Alphanumeric(File[Count]) == CHARACTER_LETTER || Is_Alphanumeric(File[Count]) == CHARACTER_NUMBER)
		Count++;

	return Count;
}

size_t Count_Valid_Non_Alphanumeric(const char* File)
{
	size_t Count = 0;

	while (Is_Alphanumeric(File[Count]) == CHARACTER_NONALPHANUMERIC && File[Count] != ' ' && File[Count] != '\t' && File[Count] != '\n' && File[Count])
		Count++;

	return Count;
}

size_t Token_Check_String_Literal(const char* File, std::vector<Token>& Target_Tokens)
{
	if (File[0] == '\"')
	{
		// We have a string literal!

		Token New_Token;
		New_Token.Token = T_STRING_LITERAL;

		while ((++File)[0] != '\"')
		{
			New_Token.Name.push_back(File[0]);
		}

		Target_Tokens.push_back(New_Token);
		return New_Token.Name.length() + 2;		// account for 2 quote marks
	}

	return 0;
}

size_t Token_Check_Int_Literal(const char* File, std::vector<Token>& Target_Tokens)
{
	// searches for hex literals or int literals

	// also searches for single character literals

	if (File[0] == '\'')
	{
		// beautifully easy! just read this token and skip past

		Token New_Token;							// NOTE: This does NOT account for special characters such as \n, \0, \t, etc

		New_Token.Name = std::to_string(File[1]);	// Characters can be accurately interpreted as int literals
		New_Token.Token = T_INT_LITERAL;

		Target_Tokens.push_back(New_Token);

		return 3;									// pass over the character
	}

	if (String_Matches_Token(File, 3, "0x"))	// if we have the prefix 0x for hexadecimal numbers
	{
		size_t Count = Count_Valid_Identifier(File + 1) + 1;	// technically x70127313 etc etc is a valid identifier because it starts with a letter

		Token New_Token;

		New_Token.Name.resize(Count);

		for (size_t Index = 0; Index < Count; Index++)
		{
			New_Token.Name[Index] = File[Index];
		}

		New_Token.Token = T_INT_LITERAL;

		Target_Tokens.push_back(New_Token);

		return Count + 2;		// I think this is right? 
	}
	else
	{
		// we'll just count the numbers and add them as a token

		Token New_Token;
		New_Token.Token = T_INT_LITERAL;

		while (Is_Alphanumeric(File[New_Token.Name.size()]) == CHARACTER_NUMBER)
			New_Token.Name.push_back(File[New_Token.Name.size()]);

		if (New_Token.Name.size())
		{
			Target_Tokens.push_back(New_Token);

			return New_Token.Name.size();
		}
	}

	return 0;
}

size_t Token_Check_Alphanumeric_Tokens(const char* File, std::vector<Token>& Target_Tokens)
{
	size_t Count = Count_Valid_Identifier(File);

	if (!Count)
		return 0;

	size_t Index = START_ALPHANUMERIC_TOKENS;

	size_t Counted;

	while (Index < END_ALPHANUMERIC_TOKENS)
	{
		if (Counted = String_Matches_Token(File, Count + 1, Compiler_Tokens[Index].Name.data()))
		{
			Target_Tokens.push_back(Compiler_Tokens[Index]);
			return Counted;
		}

		Index++;
	}

	// If it's looking like an identifier, but it's not any keyword, 
	// make an identifier!!

	Token Identifier_Token;

	Identifier_Token.Token = T_IDENTIFIER;
	Identifier_Token.Name = std::string(File);
	Identifier_Token.Name.resize(Count);

	Target_Tokens.push_back(Identifier_Token);

	return Count;
}

size_t Token_Check_Non_Alphanumeric_Tokens(const char* File, std::vector<Token>& Target_Tokens)
{
	size_t Count = Count_Valid_Non_Alphanumeric(File);

	if (!Count)
		return 0;

	size_t Token = START_NON_ALPHANUMERIC_TOKENS;

	size_t Counted;

	while (Token < END_NON_ALPHANUMERIC_TOKENS)
	{
		if (Counted = String_Matches_Token(File, Count + 1, Compiler_Tokens[Token].Name.c_str()))
		{
			Target_Tokens.push_back(Compiler_Tokens[Token]);
			return Counted;
		}

		Token++;
	}

	return 0;
}

size_t Skip_Includes(const char* File)
{
	const char Include_Token[] = "#include";
	size_t Count = 0;
	if (String_Matches_Token(File, sizeof(Include_Token), Include_Token))
		while (File[Count] != '\n')
			Count++;
	else
		return 0;

	return Count + 1;
}

size_t Skip_Comments(const char* File)
{
	if (File[0] == '/' && File[1] == '/')	// a comment of some kind 
	{
		size_t Count = 2;
		while (File[Count] != '\n')
			Count++;

		return Count + 1;
	}

	return 0;
}

void Tokenise(std::vector<Token>& Tokens, const char* File_Directory)
{
	// Gets file contents
	// and generates useful tokens therefrom

	size_t Index = 0;
	size_t Delta = 0;

	std::string File_Contents;

	Get_File_Contents(&File_Contents, File_Directory);

	// go through, character-by-character, checking for specific tokens.

	while (File_Contents[Index])
	{
		if (Delta = Skip_Comments(File_Contents.c_str() + Index))
		{
			Index += Delta;
			continue;
		}

		/*if (Delta = Skip_Includes(File_Contents.c_str() + Index))
		{
			Index += Delta;
			continue;
		}*/

		if (Delta = Token_Check_String_Literal(File_Contents.c_str() + Index, Tokens))
		{
			Index += Delta;
			continue;
		}

		if (Delta = Token_Check_Int_Literal(File_Contents.c_str() + Index, Tokens))
		{
			Index += Delta;
			continue;
		}

		if (Delta = Token_Check_Alphanumeric_Tokens(File_Contents.c_str() + Index, Tokens))
		{
			Index += Delta;
			continue;
		}

		if (Delta = Token_Check_Non_Alphanumeric_Tokens(File_Contents.c_str() + Index, Tokens))
		{
			Index += Delta;
			continue;
		}

		Index++;
	}
}
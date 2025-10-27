#include "LowC_Tokeniser.h"

//

Token Compiler_Tokens[NUMBER_OF_TOKEN_IDS] =
{
	// can't use out-of-ordered initialiser, will just sort it explicitly afterwards accordingly to Token value
	{	T_VOID,			"void"		},
	{	T_BYTE,			"byte"		},
	{	T_POINTER,		"pointer"	},
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
	{	T_DEREF,		"*"			},
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

void Get_File_Contents(std::vector<char>* String, const char* File_Directory)
{
	std::ifstream File(File_Directory);

	if (!File.is_open())
	{
		printf("ERROR READING FROM FILE! %s\n", File_Directory);
		return;
	}

	File.seekg(0, std::ios::end);
	String->resize(File.tellg());
	File.seekg(0, std::ios::beg);

	File.read(String->data(), String->size());

	File.close();
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

void Tokenise(std::vector<Token>& Tokens, const char* File_Directory)
{
	// Gets file contents
	// and generates useful tokens therefrom

	size_t Index = 0;
	size_t Delta = 0;

	std::vector<char> File_Contents;

	Get_File_Contents(&File_Contents, File_Directory);

	// go through, character-by-character, checking for specific tokens.

	while (File_Contents[Index])
	{
		if (Delta = Skip_Includes(File_Contents.data() + Index))
		{
			Index += Delta;
			continue;
		}

		if (Delta = Token_Check_Alphanumeric_Tokens(File_Contents.data() + Index, Tokens))
		{
			Index += Delta;
			continue;
		}

		if (Delta = Token_Check_Non_Alphanumeric_Tokens(File_Contents.data() + Index, Tokens))
		{
			Index += Delta;
			continue;
		}

		Index++;
	}
}
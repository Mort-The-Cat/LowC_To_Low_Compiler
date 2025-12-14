#ifndef C_TOKENS
#define C_TOKENS

#include "C_Vector.h"

typedef struct
{
	unsigned long Token;
	char* Representation;
} Token;

typedef struct
{
	char* Macro_Identifier;
	char* Expansion;
} Macro;

enum Token_Enum
{
	T_WORD,
	T_BYTE,
	T_VOID,

	T_MACRO,

	T_ID,
	T_LOCAL_ID,

	T_OPEN_SCOPE,
	T_CLOSE_SCOPE,
	T_OPEN_SQ,
	T_CLOSE_SQ,
	T_DOT,				/* A.0 = 1; A.1 = 1; A.2 = 0; A.7 = 0; */
	T_COMMA,
	T_COLON,

	T_IF,			// if
	T_SUBROUTINE,	// subroutine
	T_FOR,			// for
	T_HIGH,			// This says that we only want the high byte of a 16-bit word
	T_WHILE,		// while
	T_RETURNI,		// returni
	T_RETURN,		// return
	T_DATA,
	T_LABEL,		// label
	T_CALL,
	T_PUSH,
	T_POP,
	T_JUMP,			// jump

	T_NUMBER,
	T_HEX_LITERAL,
	T_BINARY_LITERAL,
	T_STRING_LITERAL,

	T_SEMI,

	T_UNKNOWN,

	T_BCD,
	T_SWAP,
	T_ENABLEI,
	T_DISABLEI,

	T_AMPERSAND_EQUALS,	// &=
	T_PIPE_EQUALS,		// |=
	T_XOR_EQUALS,		// ^=
	T_PLUS_EQUALS,		// +=
	T_CARRY_PLUS_EQUALS,// |+=
	T_MINUS_EQUALS,		// -=
	T_CARRY_MINUS_EQUALS,// |-=
	T_PLUS_PLUS,		// ++
	T_MINUS_MINUS,		// --
	T_EQUALS,			// =
	T_EQUALS_EQUALS,	// ==
	T_FLIP_BITS,		// A ~= A;
	T_COMPARE,			// A < B;		Carry flag is set if A < B, zero flag is set if they're equal
	T_ARITHMETIC_RIGHT_SHIFT,	// A >>= 1;
	T_LOGICAL_RIGHT_SHIFT,		// A >>>= 1;
	T_LEFT_SHIFT,		// A <<= 1;
	T_CARRY_RIGHT_ROTATE,		// A |>><= 1;
	T_CARRY_LEFT_ROTATE,		// A <<>|= 1;
	T_ROTATE_RIGHT,				// A >><= 1;
	T_ROTATE_LEFT,				// A <<>= 1;

	T_CARRY,			// carry
	T_NOT_CARRY,		// not_carry
	T_ZERO,				// zero
	T_NOT_ZERO,			// not_zero

	// Registers:

	T_REGISTER_A,
	T_REGISTER_B,
	T_REGISTER_C,
	T_REGISTER_D,
	T_REGISTER_E,

	T_REGISTER_H,
	T_REGISTER_L,

	// Register pairs:

	T_REGISTER_PAIR_BC,
	T_REGISTER_PAIR_DE,
	T_REGISTER_PAIR_AF,
	T_SP,					// This is the stack-pointer, a 16-bit register of sorts
	// Honestly only usable for memory handling
	T_REGISTER_PAIR_HL,
};

const Token Compiler_Tokens[] =
{
#define TOKEN_TYPE_BEGIN 0u

	[T_WORD] = { T_WORD,		"word"		},		/*		16-bit value		*/
	[T_BYTE] = { T_BYTE,		"byte"		},		/*		8-bit value			*/
	[T_ID] = { T_ID, ""},

	[T_MACRO] = { T_MACRO, "macro"},

#define TOKEN_TYPE_END (T_BYTE + 1)
#define TOKEN_BRACKETS_BEGIN T_OPEN_SCOPE

	[T_OPEN_SCOPE] = { T_OPEN_SCOPE,	"{"			},
	[T_CLOSE_SCOPE] = { T_CLOSE_SCOPE,"}"			},
	[T_OPEN_SQ] = { T_OPEN_SQ,	"["			},
	[T_CLOSE_SQ] = { T_CLOSE_SQ,	"]"			},
	[T_COMMA] = { T_COMMA, ","},							/* This is categorised as a bracket for sake of convenience*/
	[T_DOT] = { T_DOT, "." },
	[T_COLON] = { T_COLON, ":"},

#define TOKEN_BRACKETS_END (T_COLON + 1)

	[T_SEMI] = { T_SEMI, ";" },
	[T_UNKNOWN] = { T_UNKNOWN, "???" },

#define TOKEN_CONTROL_FLOW_BEGIN T_IF

	[T_IF] = { T_IF, "if"},
	[T_PUSH] = { T_PUSH, "push"},
	[T_POP] = { T_POP, "pop"},
	[T_SUBROUTINE] = { T_SUBROUTINE, "subroutine"},
	[T_HIGH] = { T_HIGH, "high"},
	[T_DATA] = { T_DATA, "data"},
	[T_FOR] = { T_FOR, "for"},
	[T_WHILE] = {T_WHILE, "while"},
	[T_RETURN] = { T_RETURN, "return"},
	[T_RETURNI] = { T_RETURNI, "returni"},
	[T_LABEL] = { T_LABEL, "label"},
	[T_CALL] = { T_CALL, "call"},
	[T_JUMP] = { T_JUMP, "jump"},


#define TOKEN_CONTROL_FLOW_END (T_JUMP + 1)

#define TOKEN_OPERATORS_BEGIN T_AMPERSAND_EQUALS

	[T_AMPERSAND_EQUALS] = { T_AMPERSAND_EQUALS, "&="},
	[T_PIPE_EQUALS] = { T_PIPE_EQUALS, "|="},
	[T_XOR_EQUALS] = { T_XOR_EQUALS, "^="},
	[T_PLUS_PLUS] = { T_PLUS_PLUS, "++" },
	[T_MINUS_MINUS] = { T_MINUS_MINUS, "--" },
	[T_PLUS_EQUALS] = { T_PLUS_EQUALS, "+=" },
	[T_CARRY_PLUS_EQUALS] = { T_CARRY_PLUS_EQUALS, "|+="},
	[T_CARRY_MINUS_EQUALS] = { T_CARRY_MINUS_EQUALS, "|-="},
	[T_MINUS_EQUALS] = { T_MINUS_EQUALS, "-=" },
	[T_EQUALS] = { T_EQUALS, "=" },
	[T_EQUALS_EQUALS] = { T_EQUALS_EQUALS, "==" },
	[T_FLIP_BITS] = { T_FLIP_BITS, "~=" },
	[T_COMPARE] = { T_COMPARE, "<"},
	[T_ARITHMETIC_RIGHT_SHIFT] = { T_ARITHMETIC_RIGHT_SHIFT, ">>="},
	[T_LOGICAL_RIGHT_SHIFT] = { T_LOGICAL_RIGHT_SHIFT, ">>>="},
	[T_LEFT_SHIFT] = { T_LEFT_SHIFT, "<<="},
	[T_CARRY_RIGHT_ROTATE] = { T_CARRY_RIGHT_ROTATE, ">><="},
	[T_CARRY_LEFT_ROTATE] = { T_CARRY_LEFT_ROTATE, "<<>="},
	[T_ROTATE_RIGHT] = { T_ROTATE_RIGHT, "|>><="},
	[T_ROTATE_LEFT] = { T_ROTATE_LEFT, "<<>|="},

#define TOKEN_OPERATORS_END (T_ROTATE_LEFT + 1)

#define TOKEN_BCD_OR_SWAP_BEGIN T_BCD
	[T_BCD] = { T_BCD, "BCD"},
	[T_SWAP] = { T_SWAP, "swap"},
	[T_ENABLEI] = { T_ENABLEI, "enablei"},
	[T_DISABLEI] = { T_DISABLEI, "disablei"},
#define TOKEN_BCD_OR_SWAP_END (T_DISABLEI + 1u)

#define TOKEN_CONDITIONS_BEGIN T_CARRY
	[T_CARRY] = { T_CARRY, "carry"},
	[T_NOT_CARRY] = { T_NOT_CARRY, "not_carry"},
	[T_ZERO] = { T_ZERO, "zero"},
	[T_NOT_ZERO] = { T_NOT_ZERO, "not_zero"},

#define TOKEN_CONDITIONS_END (T_NOT_ZERO + 1u)

#define TOKEN_REGISTERS_BEGIN T_REGISTER_A

	[T_REGISTER_A] = { T_REGISTER_A, "A"},
	[T_REGISTER_B] = { T_REGISTER_B, "B"},
	[T_REGISTER_C] = { T_REGISTER_C, "C"},
	[T_REGISTER_D] = { T_REGISTER_D, "D"},
	[T_REGISTER_E] = { T_REGISTER_E, "E"},

	[T_REGISTER_H] = { T_REGISTER_H, "H"},
	[T_REGISTER_L] = { T_REGISTER_L, "L"},
#define TOKEN_REGISTERS_END (T_REGISTER_L + 1u)
#define TOKEN_REGISTER_PAIRS_BEGIN T_REGISTER_PAIR_BC

	[T_REGISTER_PAIR_BC] = { T_REGISTER_PAIR_BC, "BC"},
	[T_REGISTER_PAIR_DE] = { T_REGISTER_PAIR_DE, "DE"},
	[T_REGISTER_PAIR_AF] = { T_REGISTER_PAIR_AF, "AF"},
	[T_SP] = { T_SP, "SP" },
	[T_REGISTER_PAIR_HL] = { T_REGISTER_PAIR_HL, "HL"}

#define TOKEN_REGISTER_PAIRS_END (T_REGISTER_PAIR_HL + 1u)
};

//

unsigned char Load_File_Contents(Vector* File_Contents, const char* File_Directory)
{
	unsigned char Buffer[32];

	FILE* File = fopen(File_Directory, "r");

	if (!File)
		return 1;

	while (fgets(Buffer, 32, File))
	{
		Vector_Pop_Byte(File_Contents);				// Gets rid of any null-terminator
		Vector_From_String(File_Contents, Buffer);	// Pushes buffer to vector (up to and including the null-terminator)
	}

	fclose(File);

	return 0;
}

unsigned char Handle_File_Includes(FILE* File, Vector* File_Contents)
{
	// check for 'file' keyword

	const unsigned char Macro[] = "file ";

	unsigned char Character;

	Vector New_File_Include = { 0, 0, 0 };

	if (File_Contents->Size < sizeof(Macro)) // there can't possibly be a 'file' token
		return 0;							// return early

	if (!strcmp(File_Contents->Data + File_Contents->Size - sizeof(Macro), Macro))
	{
		// There's a file token! Start loading this file into the vector...

		Vector_Pop(File_Contents, sizeof(Macro));	// Removes 'file ' keyword
		Vector_Push_Byte(File_Contents, '\0');		// adds null-terminator back

		// now, we want to get the file directory

		do
		{
			Character = fgetc(File);
			Vector_Push_Byte(&New_File_Include, Character);
		} while (Character != ';');
		New_File_Include.Data[New_File_Include.Size - 1] = '\0';

		Character = Load_File_Contents(File_Contents, New_File_Include.Data);

		Vector_Clear(&New_File_Include);

		return Character;
	}

	return 0;
}

unsigned char Unpack_File_Contents(Vector* File_Contents, const char* File_Directory)
{
	int Character;
	FILE* File = fopen(File_Directory, "r");

	if (!File)
		return 1; // Rubbish val

	while ((Character = fgetc(File)) != EOF)
	{
		// Adds byte to file
		Vector_Pop_Byte(File_Contents);
		Vector_Push_Byte(File_Contents, Character);
		Vector_Push_Byte(File_Contents, '\0');

		if (Handle_File_Includes(File, File_Contents))
			return 1;	// Rubbish value
	}

	fclose(File);

	return 0;
}

//


unsigned char Token_Check_For_Brackets(Vector* Target_Tokens, const Vector* File_Contents, unsigned long Index)
{
	unsigned char Brackets_Index = TOKEN_BRACKETS_BEGIN;

	while (Brackets_Index < TOKEN_BRACKETS_END)
	{
		if (Compiler_Tokens[Brackets_Index].Representation[0] == File_Contents->Data[Index])
		{
			Vector_Push_Memory(Target_Tokens, &Compiler_Tokens[Brackets_Index], sizeof(Token));

			return 1; // Successfully identified!
		}

		Brackets_Index++;
	}

	return 0;
}

unsigned char Token_Check_For_Generic(Vector* Target_Tokens, const Vector* File_Contents, unsigned long Index, unsigned int Begin, unsigned int End)
{
	unsigned char Delta;
	unsigned char Types_Index = Begin;

	while (Types_Index < End)
	{
		if (Delta = Compare_Strings(File_Contents->Data + Index, Compiler_Tokens[Types_Index].Representation))
		{
			Vector_Push_Memory(Target_Tokens, &Compiler_Tokens[Types_Index], sizeof(Token));

			return Delta;
		}

		Types_Index++;
	}

	return 0;
}

unsigned int Token_Check_Identifier_For_Macro(const Vector* File_Contents, Vector* Compiler_Macros, unsigned long Index, unsigned char X)
{
	unsigned char Delta;
	unsigned int Macro_Index = 0;

	while (Macro_Index < Compiler_Macros->Size)
	{
		if (Delta = Strict_Compare_Strings(File_Contents->Data + Index, *(char**)(Compiler_Macros->Data + Macro_Index)))
		{
			return Macro_Index + 1;
		}

		Macro_Index += sizeof(Macro);
	}

	return 0;
}

unsigned char Token_Check_For_Identifier(Vector* Target_Tokens, Vector* Compiler_Macros, const Vector* File_Contents, unsigned long Index)
{
	Token Identifier;
	Vector Allocated_String = { 0, 0, 0 };

	unsigned int X = 1;
	unsigned int Is_Macro;
	if (Is_Alpha_Numeric(File_Contents->Data[Index]) == ALPHANUMERIC_LETTER) // We have an identifier!!
	{
		while (Is_Alpha_Numeric(File_Contents->Data[Index + X]))
		{
			X++;
		}

		// We'll do a check here to see if this is a macro or not

		if (Is_Macro = Token_Check_Identifier_For_Macro(File_Contents, Compiler_Macros, Index, X))
		{
			Is_Macro--;
			if (**(char**)(Compiler_Macros->Data + Is_Macro + sizeof(char*)) == '$')
			{
				Identifier.Token = T_HEX_LITERAL;

				//	Vector_From_String(&Allocated_String, *(char**)(Compiler_Macros->Data + Is_Macro + sizeof(char*)) + 1u);
			}
			else
			{
				Identifier.Token = T_NUMBER;
			}

			Vector_From_String(&Allocated_String, *(char**)(Compiler_Macros->Data + Is_Macro + sizeof(char*)));

			//}

			Identifier.Representation = Allocated_String.Data;

			Vector_Push_Memory(Target_Tokens, &Identifier, sizeof(Token));

			return X;
		}

		// If it is, we'll just load the macro's contents into identifier and push it there

		Vector_Push_Memory(&Allocated_String, File_Contents->Data + Index, X);
		Vector_Push_Byte(&Allocated_String, '\0'); // NULL-terminates the string

		Identifier.Token = T_ID;
		Identifier.Representation = Allocated_String.Data;

		Vector_Push_Memory(Target_Tokens, &Identifier, sizeof(Token));

		return X;
	}

	return 0;
}

unsigned char Token_Check_Integer_Literal(Vector* Target_Tokens, const Vector* File_Contents, unsigned long Index)
{
	Token Literal;
	Vector Allocated_String = { 0, 0, 0 };

	unsigned int X = 0;
	while (Is_Alpha_Numeric(File_Contents->Data[Index + X]) == ALPHANUMERIC_NUMBER)
		X++;

	if (X)
	{
		Vector_Push_Memory(&Allocated_String, File_Contents->Data + Index, X);

		Vector_Push_Byte(&Allocated_String, '\0'); // NULL-terminates the string, representing the int literal

		Literal.Token = T_NUMBER;
		Literal.Representation = Allocated_String.Data;

		Vector_Push_Memory(Target_Tokens, &Literal, sizeof(Token));

		return X;
	}

	return 0;
}

unsigned char Get_Operator_Length(const Vector* File_Contents, unsigned long Index, unsigned int Operator_Index)
{
	unsigned char Count = 0;
	while (File_Contents->Data[Index + Count] == Compiler_Tokens[Operator_Index].Representation[Count])
		Count++;

	return Count;
}

void Get_Token_Best_Operator(const Vector* File_Contents, unsigned long Index, unsigned char* Max_Length, unsigned char* Operator_Index,
	unsigned char Begin, unsigned char End)
{
	unsigned char Length;
	for (unsigned char W = Begin; W < End; W++)
	{
		Length = Get_Operator_Length(File_Contents, Index, W);

		if (*Max_Length < Length)
		{
			*Operator_Index = W;
			*Max_Length = Length;
		}
	}
}

unsigned char Token_Check_Operator(Vector* Target_Tokens, const Vector* File_Contents, unsigned long Index)
{
	// unsigned char Operator_Lengths[TOKEN_OPERATORS_END - TOKEN_OPERATORS_BEGIN];
	unsigned char Max_Length = 0;
	unsigned char Operator_Index = 0;

	Get_Token_Best_Operator(File_Contents, Index, &Max_Length, &Operator_Index, TOKEN_OPERATORS_BEGIN, TOKEN_OPERATORS_END);

	if (Max_Length)
	{
		Vector_Push_Memory(Target_Tokens, &Compiler_Tokens[Operator_Index], sizeof(Token));

		return Max_Length;
	}

	return 0;
}

char* Get_Hex_String_From_Word(unsigned int Value) // At most 16 bits
{
	const unsigned char Hex_Digits[] = "0123456789ABCDEF";
	char* Hex_String = (char*)malloc(6u); // $ character, 4 hex characters, and null-terminator
	Hex_String[0] = '$';
	Hex_String[5] = 0u;

	Hex_String[1] = Hex_Digits[Value >> 12u];
	Hex_String[2] = Hex_Digits[(Value >> 8u) & 0xF];
	Hex_String[3] = Hex_Digits[(Value >> 4u) & 0xF];
	Hex_String[4] = Hex_Digits[Value & 0xF];

	return Hex_String;
}

unsigned char Token_Check_Binary_Literal(Vector* Target_Tokens, const Vector* File_Contents, unsigned long Index)
{
	Token Literal;
	unsigned char Length = 1u;
	unsigned int Value = 0u;

	if (File_Contents->Data[Index] == '%') // This is the prefix for a binary literal
	{
		while (Is_Alpha_Numeric(File_Contents->Data[Index + Length]) == ALPHANUMERIC_NUMBER)
		{
			Value <<= 1u;
			Value += File_Contents->Data[Index + Length] - '0';
			Length++;
		}

		Literal.Token = T_HEX_LITERAL;
		Literal.Representation = Get_Hex_String_From_Word(Value);

		Vector_Push_Memory(Target_Tokens, &Literal, sizeof(Token));

		return Length;
	}

	return 0u;
}

unsigned char Token_Check_Hex_Literal(Vector* Target_Tokens, const Vector* File_Contents, unsigned long Index)
{
	Token Literal;
	Vector Allocated_String = { 0, 0, 0 };
	unsigned char Length = 1u;

	if (File_Contents->Data[Index] == '$')
	{
		while (Is_Alpha_Numeric(File_Contents->Data[Index + Length]))
			Length++;

		Vector_Push_Memory(&Allocated_String, File_Contents->Data + Index, Length);

		Vector_Push_Byte(&Allocated_String, '\0'); // NULL-terminates the string, representing the hex literal

		Literal.Token = T_HEX_LITERAL;
		Literal.Representation = Allocated_String.Data;

		Vector_Push_Memory(Target_Tokens, &Literal, sizeof(Token));

		return Length;
	}

	return 0;
}

unsigned char Ascii_To_Code(unsigned char Character)
{
	if ('A' <= Character && Character <= 'Z')
		return Character + 0x0Au - 'A';
	else if ('0' <= Character && Character <= '9')
		return Character - '0';


	switch (Character)
	{
	case ' ':
		return 0x7Du;

	case '.':
		return 0x24u;

	case ',':
		return 0x25u;

	case '!':
		return 0x26u;

	case '?':
		return 0x27u;

	case '\'':
		return 0x28u;

	case '-':
		return 0x2Bu;

	case '+':
		return 0x29u;

	case '%':
		return 0x2Au;
	}

	return 0x2Du;
}

unsigned char* Hex_Literal_From_Character(unsigned char Character)
{
	Vector Hex_Literal_Byte = { 0, 0, 0 };

	unsigned char Hex_Digits[0x10] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

	unsigned char Character_Code = 128u + Ascii_To_Code(Character);

	Vector_Resize(&Hex_Literal_Byte, 4u);

	Hex_Literal_Byte.Data[0] = '$';
	Hex_Literal_Byte.Data[3] = 0u;

	Hex_Literal_Byte.Data[1] = Hex_Digits[Character_Code >> 4u];
	Hex_Literal_Byte.Data[2] = Hex_Digits[Character_Code & 0xFu];

	return Hex_Literal_Byte.Data;
}

unsigned char Token_Check_String_Literal(Vector* Target_Tokens, const Vector* File_Contents, unsigned long Index)
{
	// This will detect a string literal. If there is one, it replaces each character with a hex literal byte.
	// Moves the index to the end of the string
	// (String can't be more than 253 characters long)

	Token T;

	unsigned char Delta = 0;

	T.Token = T_HEX_LITERAL;

	if (File_Contents->Data[Index] == '\"')
	{
		do
		{
			Index++;
			Delta++;

			T.Representation = Hex_Literal_From_Character(File_Contents->Data[Index]);

			Vector_Push_Memory(Target_Tokens, &T, sizeof(Token));
		} while (File_Contents->Data[Index + 1] != '\"');

		return Delta + 2;
	}

	return 0u;
}

unsigned char Token_Check_Registers(Vector* Target_Tokens, const Vector* File_Contents, unsigned long Index)
{
	unsigned char Max_Length = 0;
	unsigned char Register_Index = 0;

	Get_Token_Best_Operator(File_Contents, Index, &Max_Length, &Register_Index, TOKEN_REGISTERS_BEGIN, TOKEN_REGISTER_PAIRS_END);

	if (Max_Length)
	{
		if (Is_Alpha_Numeric(File_Contents->Data[Index + Max_Length])) // If the next thing is a number or letter,
			return 0;													// this is just an identifier lol (not a register)
		Vector_Push_Memory(Target_Tokens, &Compiler_Tokens[Register_Index], sizeof(Token));

		return Max_Length;
	}

	return 0;
}

unsigned char Token_Check_Macro(Vector* Macros, Vector* File_Contents, unsigned long Index)
{
	unsigned char Delta = 0;
	unsigned char Identifier_Length = 0;
	unsigned char Expansion_Length = 0;
	Vector Macro_Identifier = { 0, 0, 0 };
	Vector Expansion = { 0, 0, 0 };

	Macro Macro;

	if (Delta = Compare_Strings(&File_Contents->Data[Index], Compiler_Tokens[T_MACRO].Representation))
	{
		Identifier_Length = 0;
		while (File_Contents->Data[Index + Delta + Identifier_Length + 1] != ' ')
			Identifier_Length++;

		Vector_Push_Memory(&Macro_Identifier, &File_Contents->Data[Index + Delta + 1], Identifier_Length);
		Vector_Push_Byte(&Macro_Identifier, '\0'); // NULL-terminates the string nicely
		//Vector_String_Fit(&Macro_Identifier); // Tidy up the string

		// Loads the macro identifier into this new vector

		while (File_Contents->Data[Index + Delta + Identifier_Length + Expansion_Length + 2] != ';')
			Expansion_Length++;

		Vector_Push_Memory(&Expansion, &File_Contents->Data[Index + Delta + Identifier_Length + 2], Expansion_Length);
		Vector_Push_Byte(&Expansion, '\0'); // NULL-terminates the string
		//Vector_String_Fit(&Expansion);	// Tidy up the string

		Macro.Macro_Identifier = Macro_Identifier.Data;
		Macro.Expansion = Expansion.Data;

		Vector_Push_Memory(Macros, &Macro, sizeof(Macro));

		return Delta + Identifier_Length + Expansion_Length + 3; // This includes two spaces, and a semi-colon
	}
}

unsigned char Tokenise(Vector* Target_Tokens, const char* File_Directory)
{
	Vector File_Contents = { 0, 0, 0 };

	Vector Found_Macros = { 0, 0, 0 };

	unsigned long Index = 0;
	unsigned long Delta = 0;

	unsigned char Was_Previous_A_Type = 0;

	if (Unpack_File_Contents(&File_Contents, File_Directory))
		return 1;

	if (File_Contents.Data == 0) // NULL
		return 1;	// File is empty??

	// Now, we've finished loading the file

	while (File_Contents.Data[Index])
	{
		Was_Previous_A_Type <<= 1;

		if (Delta = Token_Check_Macro(&Found_Macros, &File_Contents, Index))
		{
			Index += Delta;
			continue;
		}

		if (File_Contents.Data[Index] == '#')
		{
			do
			{
				Index++;
			} while (File_Contents.Data[Index] != '\n' && File_Contents.Data[Index] != 0);

			continue;
		}

		if (File_Contents.Data[Index] == ' ' || File_Contents.Data[Index] == '\t' || File_Contents.Data[Index] == '\n')	// Whitespace, we don't care!
		{
			Index++;
			continue;
		}

		if (Delta = Token_Check_String_Literal(Target_Tokens, &File_Contents, Index))
		{
			Index += Delta;
			continue;
		}

		if (Token_Check_For_Brackets(Target_Tokens, &File_Contents, Index))
		{
			Index++;
			continue;
		}

		if (Delta = Token_Check_For_Generic(Target_Tokens, &File_Contents, Index,
			TOKEN_BCD_OR_SWAP_BEGIN, TOKEN_BCD_OR_SWAP_END))
		{
			Index += Delta;
			continue;
		}

		if (Delta = Token_Check_For_Generic(Target_Tokens, &File_Contents, Index,
			TOKEN_TYPE_BEGIN, TOKEN_TYPE_END))
		{
			Index += Delta;
			Was_Previous_A_Type = 1;
			continue;
		}

		if (Delta = Token_Check_For_Generic(Target_Tokens, &File_Contents, Index,
			TOKEN_CONTROL_FLOW_BEGIN, TOKEN_CONTROL_FLOW_END))
		{
			Index += Delta;
			continue;
		}

		if (Delta = Token_Check_For_Generic(Target_Tokens, &File_Contents, Index,
			TOKEN_CONDITIONS_BEGIN, TOKEN_CONDITIONS_END))
		{
			Index += Delta;
			continue;
		}

		if (File_Contents.Data[Index] == ';')
		{
			Vector_Push_Memory(Target_Tokens, &Compiler_Tokens[T_SEMI], sizeof(Token));
			Index++;
			continue;
		}

		if (Delta = Token_Check_Operator(Target_Tokens, &File_Contents, Index))
		{
			Index += Delta;

			continue;
		}

		if (Delta = Token_Check_Registers(Target_Tokens, &File_Contents, Index))
		{
			Index += Delta;
			continue;
		}

		if (Delta = Token_Check_For_Identifier(Target_Tokens, &Found_Macros, &File_Contents, Index))
		{
			Index += Delta;
			continue;
		}

		if (Delta = Token_Check_Integer_Literal(Target_Tokens, &File_Contents, Index))
		{
			Index += Delta;
			continue;
		}

		if (Delta = Token_Check_Hex_Literal(Target_Tokens, &File_Contents, Index))
		{
			Index += Delta;
			continue;
		}

		if (Delta = Token_Check_Binary_Literal(Target_Tokens, &File_Contents, Index))
		{
			Index += Delta;
			continue;
		}

		Vector_Push_Memory(Target_Tokens, &Compiler_Tokens[T_UNKNOWN], sizeof(Token));
		Delta = Characters_Until_Non_Alphanumeric(File_Contents.Data + Index); // Rubbish...
		Delta += Delta == 0;
		Index += Delta;
	}

	// Before we return from this function,

	Vector_Clear(&File_Contents);
	Vector_Clear(&Found_Macros);

	// Free the memory

	return 0;
}

#endif
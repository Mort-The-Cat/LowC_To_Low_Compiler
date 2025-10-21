#ifndef BYTE_CODE_GENERATION
#define BYTE_CODE_GENERATION

#include "Tokens.h"

typedef struct
{
	unsigned int ROM_Index;		// Location in ROM
	const unsigned char* Name;	// This is the name of the ID we want to replace
	unsigned char Byte_Count;	// How many bytes we want from this ID
	unsigned char Is_High;		// This lets the resolver know if we want the High-byte of a specific 16-bit word (because that's important)
} Unresolved_ID;

typedef struct
{
	const unsigned char* Representation;
	const unsigned char* Name;
} Identifier;

// This will be applied to the WHOLE ROM
// Just need to apply a simple pass to the Tokens in order to identify LOCAL ID's (which may provide important optimisations)

void Get_Global_IDs(Vector* Tokens, Vector* Global_IDs) // Global_IDs is just a list of char-pointers
{
	unsigned long long Index = 0;

	Token T;

	while (Index < Tokens->Size)
	{
		// finds subroutines and data and macros (identifying them as global IDs)

		T = *(Token*)(Tokens->Data + (Index += sizeof(Token)));

		if (T.Token == T_SUBROUTINE || T.Token == T_DATA)
		{
			T = *(Token*)(Tokens->Data + (Index += sizeof(Token)));

			Vector_Push_Memory(Global_IDs, &T.Representation, sizeof(const char*));
		}
	}
}

const char* Vector_Contains_String(const Vector* Strings, const char* String)
{
	unsigned long Index = 0;

	const char* Element;

	while (Index < Strings->Size)
	{
		Element = *(const char**)(Strings->Data + Index);
		if (!strcmp(Element, String))
			return Element;

		Index += sizeof(const char*);
	}

	return NULL;
}

typedef struct
{
	const char* Name;
	const char* Value;
	int Token;
} Symbol;

void Handle_Identifiers(Vector* Target_Tokens, unsigned int Index, const Vector* Identifiers)
{
	Symbol S;
	unsigned int Identifier_Index = 0;

	for (; Identifier_Index < Identifiers->Size; Identifier_Index += sizeof(Symbol))
	{
		S = *(Symbol*)(Identifiers->Data + Identifier_Index);
		if (!strcmp(((Token*)(Target_Tokens->Data + Index))->Representation, S.Name))
		{
			//free(((Token*)(Target_Tokens->Data + Index))->Representation);
			((Token*)(Target_Tokens->Data + Index))->Representation = S.Value;
			((Token*)(Target_Tokens->Data + Index))->Token = S.Token;
			return;
		}
	}
}

void Place_Identifiers(Vector* Target_Tokens)
{
	Vector Identifiers = { 0, 0, 0 };

	Symbol New_Symbol;

	unsigned int Local_Identifier_Count = 0;

	unsigned int Scope = 0;

	unsigned int Index = 0;

	Token* T;

	while (Index < Target_Tokens->Size)
	{
		T = (Token*)(Target_Tokens->Data + Index);

		switch (T->Token)
		{
		case T_OPEN_SCOPE:
			Scope = 1;
			break;

		case T_CLOSE_SCOPE:
			Scope = 0;
			Vector_Pop(&Identifiers, sizeof(Symbol) * Local_Identifier_Count);
			Local_Identifier_Count = 0;
			break;

		case T_DATA:
		case T_SUBROUTINE:
			if (T[2].Token == T_HEX_LITERAL || T[2].Token == T_NUMBER)
			{
				New_Symbol.Name = T[1].Representation;
				New_Symbol.Value = T[2].Representation;
				New_Symbol.Token = T[2].Token;

				Vector_Push_Memory(&Identifiers, (unsigned char*)&New_Symbol, sizeof(Symbol));
			}
			break;

		case T_ID:
			Handle_Identifiers(Target_Tokens, Index, &Identifiers);
			break;

		case T_BYTE:
		case T_WORD:
			New_Symbol.Name = T[1].Representation;
			New_Symbol.Value = T[3].Representation;
			New_Symbol.Token = T[3].Token;

			Local_Identifier_Count++;

			Vector_Push_Memory(&Identifiers, (unsigned char*)&New_Symbol, sizeof(Symbol));

			break;

		default:
			break;
		};

		Index += sizeof(Token);
	}

	// Count for local identifiers
	// Vector for all identifiers

	// If an identifier appears, check if we have it,
	// Otherwise, keep it there for the compiler to handle later

	// If we do have it, use the value and move on.
}

void Clean_Token_IDs(Vector* Tokens)
{
	// Finds jump statements that refer to LOCAL IDs and sets them accordingly

	Vector Global_Identifiers = { 0, 0, 0 };

	Get_Global_IDs(Tokens, &Global_Identifiers);

	unsigned long Index = 0;

	const char* String;

	Token* T;

	while (Index < Tokens->Size)
	{
		T = (Token*)(Tokens->Data + Index);
		Index += sizeof(Token);
		if (T->Token == T_JUMP)
		{
			T++;
			if (T->Token != T_ID && T->Token != T_HEX_LITERAL && T->Token != T_REGISTER_PAIR_HL)
				T++;

			String = Vector_Contains_String(&Global_Identifiers, (T)->Representation);
			if (!String && T->Token != T_REGISTER_PAIR_HL && T->Token != T_HEX_LITERAL)
				(T)->Token = T_LOCAL_ID;

		}
	}

	Place_Identifiers(Tokens);
}

void Generate_ROM_Header_Checksum(unsigned char* ROM)
{
	unsigned char X = 0;

	unsigned int Index = 0x0134u;

	while (Index <= 0x014Cu)
	{
		X = X - ROM[Index] - 1;
		Index++;
	}

	ROM[0x014Du] = X;
}

// Then we'll generate the other code

#endif































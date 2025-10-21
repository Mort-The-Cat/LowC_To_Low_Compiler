#ifndef LOW_SYNTAX_SCANNER
#define LOW_SYNTAX_SCANNER

#include "C_Vector.h"
#include "Byte_Code_Generation.h"

unsigned int Get_Value_From_String(const unsigned char* String)
{
	// $ indicates hex values
	// everything else is decimal

	unsigned int Value = 0;
	unsigned int Base;

	if (String[0] == '$') // Hex
	{
		String++;

		Base = 16;
	}
	else				// decimal
	{
		Base = 10;
	}

	while (*String)
	{
		Value *= Base;
		if (String[0] >= 'A')
		{
			Value += 10 + String[0] - 'A';
		}
		else
			Value += String[0] - '0';

		String++;
	}

	return Value;
}

typedef struct
{
	const unsigned char* Tokens;
	unsigned char Token_Count;
	unsigned int Opcode;
	void(*Create_Function)(unsigned char* ROM, unsigned int* Token_Index, unsigned int* ROM_Index, Vector* Identifiers, const Vector* Target_Tokens, unsigned int Opcode);
} Token_Pattern;

void Place_Opcode(unsigned char* ROM, unsigned int* Token_Index, unsigned int* ROM_Index, Vector* Identifiers, const Vector* Target_Tokens, unsigned int Opcode)
{
	ROM[(*ROM_Index)++] = Opcode;
}

void Place_CB_Opcode(unsigned char* ROM, unsigned int* Token_Index, unsigned int* ROM_Index, Vector* Identifiers, const Vector* Target_Tokens, unsigned int Opcode)
{
	ROM[(*ROM_Index)++] = 0xCB;
	ROM[(*ROM_Index)++] = Opcode;
}

void Place_CB_Bit_Opcode(unsigned char* ROM, unsigned int* Token_Index, unsigned int* ROM_Index, Vector* Identifiers, const Vector* Target_Tokens, unsigned int Opcode)
{
	unsigned int Index = *Token_Index;

	while (((Token*)(Target_Tokens->Data + Index))->Token != T_HEX_LITERAL && ((Token*)(Target_Tokens->Data + Index))->Token != T_NUMBER)
		Index -= sizeof(Token);

	ROM[(*ROM_Index)++] = 0xCB;
	ROM[(*ROM_Index)++] = Opcode + 8 * Get_Value_From_String(((Token*)(Target_Tokens->Data + Index))->Representation);
}

void Place_Direct_Word_Opcode(unsigned char* ROM, unsigned int* Token_Index, unsigned int* ROM_Index, Vector* Identifiers, const Vector* Target_Tokens, unsigned int Opcode)
{
	unsigned Index = *Token_Index;
	unsigned int Value;
	ROM[(*ROM_Index)++] = Opcode;

	// increments Index until hex literal or number is found

	while (((Token*)(Target_Tokens->Data + Index))->Token != T_HEX_LITERAL && ((Token*)(Target_Tokens->Data + Index))->Token != T_NUMBER)
		Index -= sizeof(Token);

	// Words are placed in high->low order in ROM

	Value = Get_Value_From_String(((Token*)(Target_Tokens->Data + Index))->Representation);
	ROM[(*ROM_Index)++] = Value & 0xFFu;
	ROM[(*ROM_Index)++] = Value >> 8u;
}

void Place_Direct_Byte_Opcode(unsigned char* ROM, unsigned int* Token_Index, unsigned int* ROM_Index, Vector* Identifiers, const Vector* Target_Tokens, unsigned int Opcode)
{
	unsigned Index = *Token_Index;
	unsigned int Value;
	ROM[(*ROM_Index)++] = Opcode;

	while (((Token*)(Target_Tokens->Data + Index))->Token != T_HEX_LITERAL && ((Token*)(Target_Tokens->Data + Index))->Token != T_NUMBER)
		Index -= sizeof(Token);

	Value = Get_Value_From_String(((Token*)(Target_Tokens->Data + Index))->Representation);
	ROM[(*ROM_Index)++] = Value & 0xFFu;
}

void Place_Identifier(Vector* Target_Tokens, Vector* Identifiers, unsigned int* Token_Index, unsigned int* ROM_Index, unsigned char Is_Word, unsigned char Is_High)
{
	Unresolved_ID Unresolved;
	unsigned int Index = *Token_Index;

	Unresolved.Byte_Count = Is_Word + 1;
	Unresolved.Is_High = Is_High;
	Unresolved.ROM_Index = *ROM_Index;

	// we'll include checks for Is_High outside of this

	while (((Token*)(Target_Tokens->Data + Index))->Token != T_ID && ((Token*)(Target_Tokens->Data + Index))->Token != T_LOCAL_ID)
		Index -= sizeof(Token);

	Unresolved.Name = ((Token*)(Target_Tokens->Data + Index))->Representation;

	Vector_Push_Memory(Identifiers, &Unresolved, sizeof(Unresolved_ID)); // Pushes unresolved ID to the list of unresolved identifiers

	(*ROM_Index) += (1 + Is_Word);
}

void Place_Identifier_Byte_Opcode(unsigned char* ROM, unsigned int* Token_Index, unsigned int* ROM_Index, Vector* Identifiers, const Vector* Target_Tokens, unsigned int Opcode)
{
	// We'll place the opcode and increment the ROM_Index, but we'll also create an unresolved ID for this ROM address so we can replace it with the according value later
	Unresolved_ID Unresolved;
	unsigned int Index = *Token_Index;
	
	ROM[(*ROM_Index)++] = Opcode;

	Place_Identifier(Target_Tokens, Identifiers, Token_Index, ROM_Index, 0, 0);
}

void Place_CB_Direct_Byte_Opcode(unsigned char* ROM, unsigned int* Token_Index, unsigned int* ROM_Index, Vector* Identifiers, const Vector* Target_Tokens, unsigned int Opcode)
{
	ROM[(*ROM_Index)++] = 0xCB;
	Place_Direct_Byte_Opcode(ROM, Token_Index, ROM_Index, Identifiers, Target_Tokens, Opcode);
}

void Place_CB_Identifier_Byte_Opcode(unsigned char* ROM, unsigned int* Token_Index, unsigned int* ROM_Index, Vector* Identifiers, const Vector* Target_Tokens, unsigned int Opcode)
{
	ROM[(*ROM_Index)++] = 0xCB;
	Place_Identifier_Byte_Opcode(ROM, Token_Index, ROM_Index, Identifiers, Target_Tokens, Opcode);
}

void Place_Relative_Identifier_Byte_Opcode(unsigned char* ROM, unsigned int* Token_Index, unsigned int* ROM_Index, Vector* Identifiers, const Vector* Target_Tokens, unsigned int Opcode)
{
	Place_Identifier_Byte_Opcode(ROM, Token_Index, ROM_Index, Identifiers + 1, Target_Tokens, Opcode);

	// This just places it in the 'relative unresolved IDs' vector instead
}

void Place_Identifier_Word_Opcode(unsigned char* ROM, unsigned int* Token_Index, unsigned int* ROM_Index, Vector* Identifiers, const Vector* Target_Tokens, unsigned int Opcode)
{
	Unresolved_ID Unresolved;
	unsigned int Index = *Token_Index;

	ROM[(*ROM_Index)++] = Opcode;

	Place_Identifier(Target_Tokens, Identifiers, Token_Index, ROM_Index, 1, 0);
}

#define LD_REG_REG(Left, Right, Opcode, Offset)\
	{ (const unsigned char[]){ Left, T_EQUALS, Right, T_SEMI }, 4, Opcode + Offset, Place_Opcode }

#define LD_DEFINE_REG(Left, Opcode)\
	LD_REG_REG(Left, T_REGISTER_B, Opcode, 0),\
	LD_REG_REG(Left, T_REGISTER_C, Opcode, 1),\
	LD_REG_REG(Left, T_REGISTER_D, Opcode, 2),\
	LD_REG_REG(Left, T_REGISTER_E, Opcode, 3),\
	LD_REG_REG(Left, T_REGISTER_H, Opcode, 4),\
	LD_REG_REG(Left, T_REGISTER_L, Opcode, 5),\
	{ (const unsigned char[]) { Left, T_EQUALS, T_OPEN_SQ, T_REGISTER_PAIR_HL, T_CLOSE_SQ, T_SEMI }, 6, Opcode + 6, Place_Opcode },\
	LD_REG_REG(Left, T_REGISTER_A, Opcode, 7)

#define LD_HL_POINTER_REG(Right, Offset)\
	{ (const unsigned char[]){ T_OPEN_SQ, T_REGISTER_PAIR_HL, T_CLOSE_SQ, T_EQUALS, Right, T_SEMI }, 6, 0x70u + Offset, Place_Opcode }



#define ALU_DEFINE(Operator, Opcode)\
	{ (const unsigned char[]){ T_REGISTER_A, Operator, T_REGISTER_B, T_SEMI }, 4, Opcode, Place_Opcode },\
	{ (const unsigned char[]){ T_REGISTER_A, Operator, T_REGISTER_C, T_SEMI }, 4, Opcode + 1, Place_Opcode },\
	{ (const unsigned char[]){ T_REGISTER_A, Operator, T_REGISTER_D, T_SEMI }, 4, Opcode + 2, Place_Opcode },\
	{ (const unsigned char[]){ T_REGISTER_A, Operator, T_REGISTER_E, T_SEMI }, 4, Opcode + 3, Place_Opcode },\
	{ (const unsigned char[]){ T_REGISTER_A, Operator, T_REGISTER_H, T_SEMI }, 4, Opcode + 4, Place_Opcode },\
	{ (const unsigned char[]){ T_REGISTER_A, Operator, T_REGISTER_L, T_SEMI }, 4, Opcode + 5, Place_Opcode },\
	{ (const unsigned char[]){ T_REGISTER_A, Operator, T_OPEN_SQ, T_REGISTER_PAIR_HL, T_CLOSE_SQ, T_SEMI }, 6, Opcode + 6, Place_Opcode },\
	{ (const unsigned char[]){ T_REGISTER_A, Operator, T_REGISTER_A, T_SEMI }, 4, Opcode + 7, Place_Opcode },\
	{ (const unsigned char[]){ T_REGISTER_A, Operator, T_HEX_LITERAL, T_SEMI }, 4, Opcode + 0x46, Place_Direct_Byte_Opcode },\
	{ (const unsigned char[]){ T_REGISTER_A, Operator, T_NUMBER, T_SEMI }, 4, Opcode + 0x46, Place_Direct_Byte_Opcode },\
	{ (const unsigned char[]){ T_REGISTER_A, Operator, T_ID, T_SEMI }, 4, Opcode + 0x46, Place_Identifier_Byte_Opcode }

#define INC_DEC_DEFINE(Register, Operator, Offset)\
	{ (const unsigned char[]){ Register, Operator, T_SEMI }, 3, Offset, Place_Opcode }

#define INC_DEC(Operator, Opcode)\
	INC_DEC_DEFINE(T_REGISTER_B, Operator, Opcode),\
	INC_DEC_DEFINE(T_REGISTER_C, Operator, Opcode + 0x08u),\
	INC_DEC_DEFINE(T_REGISTER_D, Operator, Opcode + 0x10u),\
	INC_DEC_DEFINE(T_REGISTER_E, Operator, Opcode + 0x18u),\
	INC_DEC_DEFINE(T_REGISTER_H, Operator, Opcode + 0x20u),\
	INC_DEC_DEFINE(T_REGISTER_L, Operator, Opcode + 0x28u),\
	{ (const unsigned char[]){ T_OPEN_SQ, T_REGISTER_PAIR_HL, T_CLOSE_SQ, Operator, T_SEMI }, 5, Opcode + 0x30u, Place_Opcode },\
	INC_DEC_DEFINE(T_REGISTER_A, Operator, Opcode + 0x38u)

#define Conditional_Branch_Define(Operator, Type, Opcode, Function)\
	{ (const unsigned char[]){ Operator, T_NOT_ZERO, Type, T_SEMI }, 4, Opcode, Function },\
	{ (const unsigned char[]){ Operator, T_NOT_CARRY, Type, T_SEMI }, 4, Opcode + 0x10u, Function },\
	{ (const unsigned char[]){ Operator, T_ZERO, Type, T_SEMI }, 4, Opcode + 0x08u, Function },\
	{ (const unsigned char[]){ Operator, T_CARRY, Type, T_SEMI }, 4, Opcode + 0x18u, Function }

#define Assign_RP(Register_Pair, Opcode)\
	{ (const unsigned char[]){ Register_Pair, T_EQUALS, T_ID, T_SEMI }, 4, Opcode, Place_Identifier_Word_Opcode },\
	{ (const unsigned char[]){ Register_Pair, T_EQUALS, T_HEX_LITERAL, T_SEMI }, 4, Opcode, Place_Direct_Word_Opcode },\
	{ (const unsigned char[]){ Register_Pair, T_EQUALS, T_NUMBER, T_SEMI }, 4, Opcode, Place_Direct_Word_Opcode }

#define Add_RP(Register_Pair, Opcode)\
	{ (const unsigned char[]){ T_REGISTER_PAIR_HL, T_PLUS_EQUALS, Register_Pair, T_SEMI }, 4, Opcode, Place_Opcode }

#define Inc_Dec_RP(Register_Pair, Operator, Opcode)\
	{ (const unsigned char[]){ Register_Pair, Operator, T_SEMI }, 3, Opcode, Place_Opcode }


#define Assign_RP_Pointer(Register_Pair, Opcode, Number)\
	{ (const unsigned char[]){ T_OPEN_SQ, Register_Pair, T_CLOSE_SQ, T_EQUALS, T_REGISTER_A, T_SEMI }, Number + 5, Opcode, Place_Opcode },\
	{ (const unsigned char[]){ T_REGISTER_A, T_EQUALS, T_OPEN_SQ, Register_Pair, T_CLOSE_SQ, T_SEMI }, Number + 5, Opcode + 0x08u, Place_Opcode }

#define Assign_HL_INC_DEC(Opcode, Inc_Dec)\
	{ (const unsigned char[]){ T_OPEN_SQ, T_REGISTER_PAIR_HL, Inc_Dec, T_CLOSE_SQ, T_EQUALS, T_REGISTER_A, T_SEMI }, 7, Opcode, Place_Opcode },\
	{ (const unsigned char[]){ T_REGISTER_A, T_EQUALS, T_OPEN_SQ, T_REGISTER_PAIR_HL, Inc_Dec, T_CLOSE_SQ, T_SEMI }, 7, Opcode + 0x08u, Place_Opcode }

#define LD_REG_VALUE(Register, Opcode)\
	{ (const unsigned char[]){ Register, T_EQUALS, T_NUMBER, T_SEMI }, 4, Opcode, Place_Direct_Byte_Opcode },\
	{ (const unsigned char[]){ Register, T_EQUALS, T_HEX_LITERAL, T_SEMI }, 4, Opcode, Place_Direct_Byte_Opcode },\
	{ (const unsigned char[]){ Register, T_EQUALS, T_ID, T_SEMI }, 4, Opcode, Place_Identifier_Byte_Opcode }

#define PUSH_POP(Register_Pair, Opcode)\
	{ (const unsigned char[]){ T_POP, Register_Pair, T_SEMI }, 3, Opcode, Place_Opcode },\
	{ (const unsigned char[]){ T_PUSH, Register_Pair, T_SEMI }, 3, Opcode + 4, Place_Opcode }

#define DEFINE_CB(Register, Operation, Opcode)\
	{ (const unsigned char[]){ Register, Operation, T_NUMBER, T_SEMI }, 4, Opcode, Place_CB_Opcode },\
	{ (const unsigned char[]){ Register, Operation, T_HEX_LITERAL, T_SEMI }, 4, Opcode, Place_CB_Opcode },\
	{ (const unsigned char[]){ Register, Operation, T_ID, T_SEMI }, 4, Opcode, Place_CB_Opcode }

#define DEFINE_HL_CB(Operation, Opcode)\
	{ (const unsigned char[]){ T_OPEN_SQ, T_REGISTER_PAIR_HL, T_CLOSE_SQ, T_EQUALS, T_NUMBER, T_SEMI }, 6, Opcode, Place_CB_Direct_Byte_Opcode },\
	{ (const unsigned char[]){ T_OPEN_SQ, T_REGISTER_PAIR_HL, T_CLOSE_SQ, T_EQUALS, T_HEX_LITERAL, T_SEMI }, 6, Opcode, Place_CB_Direct_Byte_Opcode },\
	{ (const unsigned char[]){ T_OPEN_SQ, T_REGISTER_PAIR_HL, T_CLOSE_SQ, T_EQUALS, T_ID, T_SEMI }, 6, Opcode, Place_CB_Identifier_Byte_Opcode }

#define DEFINE_CB_ROW(Operation, Opcode)\
	DEFINE_CB(T_REGISTER_B, Operation, Opcode),\
	DEFINE_CB(T_REGISTER_C, Operation, Opcode + 1),\
	DEFINE_CB(T_REGISTER_D, Operation, Opcode + 2),\
	DEFINE_CB(T_REGISTER_E, Operation, Opcode + 3),\
	DEFINE_CB(T_REGISTER_H, Operation, Opcode + 4),\
	DEFINE_CB(T_REGISTER_L, Operation, Opcode + 5),\
	DEFINE_HL_CB(Operation, Opcode + 6),\
	DEFINE_CB(T_REGISTER_A, Operation, Opcode + 7)\
	

#define DEFINE_SWAP(Register, Offset)\
	{ (const unsigned char[]){ T_SWAP, Register, T_SEMI }, 3, Offset + 0x30, Place_CB_Opcode }


#define DEFINE_CB_BIT(Register, Offset)\
	{ (const unsigned char[]){ Register, T_DOT, T_NUMBER, T_SEMI }, 4, Offset + 0x40, Place_CB_Bit_Opcode },\
	{ (const unsigned char[]) { Register, T_DOT, T_HEX_LITERAL, T_SEMI }, 4, Offset + 0x40, Place_CB_Bit_Opcode }


Token_Pattern Patterns[] =
{
	{ (const unsigned char[]) { T_BCD, T_REGISTER_A, T_SEMI }, 3, 0x27u, Place_Opcode },	// Simple BCD expression!

	{ (const unsigned char[]) { T_REGISTER_A, T_FLIP_BITS, T_REGISTER_A, T_SEMI }, 4, 0x2F, Place_Opcode },

	DEFINE_SWAP(T_REGISTER_B, 0),
	DEFINE_SWAP(T_REGISTER_C, 1),
	DEFINE_SWAP(T_REGISTER_D, 2),
	DEFINE_SWAP(T_REGISTER_E, 3),
	DEFINE_SWAP(T_REGISTER_H, 4),
	DEFINE_SWAP(T_REGISTER_L, 5),
	{ (const unsigned char[]) { T_SWAP, T_OPEN_SQ, T_REGISTER_PAIR_HL, T_CLOSE_SQ, T_SEMI }, 5, 0x36, Place_CB_Opcode },
	DEFINE_SWAP(T_REGISTER_A, 7),

	LD_DEFINE_REG(T_REGISTER_B, 0x40u),
	LD_DEFINE_REG(T_REGISTER_C, 0x48u),
	LD_DEFINE_REG(T_REGISTER_D, 0x50u),
	LD_DEFINE_REG(T_REGISTER_E, 0x58u),
	LD_DEFINE_REG(T_REGISTER_H, 0x60u),
	LD_DEFINE_REG(T_REGISTER_L, 0x68u),

	LD_HL_POINTER_REG(T_REGISTER_B, 0),
	LD_HL_POINTER_REG(T_REGISTER_C, 1),
	LD_HL_POINTER_REG(T_REGISTER_D, 2),
	LD_HL_POINTER_REG(T_REGISTER_E, 3),
	LD_HL_POINTER_REG(T_REGISTER_H, 4),
	LD_HL_POINTER_REG(T_REGISTER_L, 5),
	// [HL] = [HL]; doesn't exist -> becomes halt instruction
	LD_HL_POINTER_REG(T_REGISTER_A, 7),
	LD_DEFINE_REG(T_REGISTER_A, 0x78u),

	ALU_DEFINE(T_PLUS_EQUALS, 0x80u),
	ALU_DEFINE(T_CARRY_PLUS_EQUALS, 0x88u),
	ALU_DEFINE(T_MINUS_EQUALS, 0x90u),
	ALU_DEFINE(T_CARRY_MINUS_EQUALS, 0x98u),
	ALU_DEFINE(T_AMPERSAND_EQUALS, 0xA0u),
	ALU_DEFINE(T_XOR_EQUALS, 0xA8u),
	ALU_DEFINE(T_PIPE_EQUALS, 0xB0u),
	ALU_DEFINE(T_COMPARE, 0xB8u),

	INC_DEC(T_PLUS_PLUS, 0x04u),
	INC_DEC(T_MINUS_MINUS, 0x05u),

	Conditional_Branch_Define(T_JUMP, T_LOCAL_ID, 0x20u, Place_Relative_Identifier_Byte_Opcode),
	{ (const unsigned char[]) { T_JUMP, T_LOCAL_ID, T_SEMI }, 3, 0x18u, Place_Relative_Identifier_Byte_Opcode},

	Conditional_Branch_Define(T_JUMP, T_ID, 0xC2, Place_Identifier_Word_Opcode),
	Conditional_Branch_Define(T_JUMP, T_HEX_LITERAL, 0xC2, Place_Direct_Word_Opcode),
	{ (const unsigned char[]) { T_JUMP, T_ID, T_SEMI }, 3, 0xC3, Place_Identifier_Word_Opcode },
	{ (const unsigned char[]) { T_JUMP, T_HEX_LITERAL, T_SEMI }, 3, 0xC3, Place_Direct_Word_Opcode },

	{ (const unsigned char[]) { T_JUMP, T_REGISTER_PAIR_HL, T_SEMI }, 3, 0xE9, Place_Opcode },

	Conditional_Branch_Define(T_CALL, T_ID, 0xC4, Place_Identifier_Word_Opcode),
	Conditional_Branch_Define(T_CALL, T_HEX_LITERAL, 0xC4, Place_Direct_Word_Opcode),
	{ (const unsigned char[]) { T_CALL, T_ID, T_SEMI }, 3, 0xCD, Place_Identifier_Word_Opcode },
	{ (const unsigned char[]) { T_CALL, T_HEX_LITERAL, T_SEMI }, 3, 0xCD, Place_Direct_Word_Opcode },
	{ (const unsigned char[]) { T_CALL, T_NUMBER, T_SEMI }, 3, 0xCD, Place_Direct_Word_Opcode },

	{ (const unsigned char[]) { T_RETURN, T_NOT_CARRY, T_SEMI }, 3, 0xD0, Place_Opcode },
	{ (const unsigned char[]) { T_RETURN, T_NOT_ZERO, T_SEMI }, 3, 0xC0, Place_Opcode },
	{ (const unsigned char[]) { T_RETURN, T_CARRY, T_SEMI }, 3, 0xD8, Place_Opcode },
	{ (const unsigned char[]) { T_RETURN, T_ZERO, T_SEMI }, 3, 0xC8, Place_Opcode },
	{ (const unsigned char[]) { T_RETURN, T_SEMI }, 2, 0xC9, Place_Opcode },
	{ (const unsigned char[]) { T_RETURNI, T_SEMI }, 2, 0xD9, Place_Opcode },

	Assign_RP(T_REGISTER_PAIR_BC, 0x01),
	Assign_RP(T_REGISTER_PAIR_DE, 0x11),
	Assign_RP(T_REGISTER_PAIR_HL, 0x21),
	Assign_RP(T_SP, 0x31),

	Add_RP(T_REGISTER_PAIR_BC, 0x09),
	Add_RP(T_REGISTER_PAIR_DE, 0x19),
	Add_RP(T_REGISTER_PAIR_HL, 0x29),
	Add_RP(T_SP, 0x39),

	Inc_Dec_RP(T_REGISTER_PAIR_BC, T_PLUS_PLUS, 0x03u),
	Inc_Dec_RP(T_REGISTER_PAIR_DE, T_PLUS_PLUS, 0x13u),
	Inc_Dec_RP(T_REGISTER_PAIR_HL, T_PLUS_PLUS, 0x23u),
	Inc_Dec_RP(T_SP, T_PLUS_PLUS, 0x33u),

	Inc_Dec_RP(T_REGISTER_PAIR_BC, T_MINUS_MINUS, 0x0Bu),
	Inc_Dec_RP(T_REGISTER_PAIR_DE, T_MINUS_MINUS, 0x1Bu),
	Inc_Dec_RP(T_REGISTER_PAIR_HL, T_MINUS_MINUS, 0x2Bu),
	Inc_Dec_RP(T_SP, T_MINUS_MINUS, 0x3Bu),

	Assign_RP_Pointer(T_REGISTER_PAIR_BC, 0x02, 1),
	Assign_RP_Pointer(T_REGISTER_PAIR_DE, 0x12, 1),
	Assign_HL_INC_DEC(0x22, T_PLUS_PLUS),
	Assign_HL_INC_DEC(0x32, T_MINUS_MINUS),

	LD_REG_VALUE(T_REGISTER_B, 0x06),
	LD_REG_VALUE(T_REGISTER_C, 0x0E),
	LD_REG_VALUE(T_REGISTER_D, 0x16),
	LD_REG_VALUE(T_REGISTER_E, 0x1E),
	LD_REG_VALUE(T_REGISTER_H, 0x26),
	LD_REG_VALUE(T_REGISTER_L, 0x2E),
	{ (const unsigned char[]) { T_OPEN_SQ, T_REGISTER_PAIR_HL, T_CLOSE_SQ, T_EQUALS, T_NUMBER, T_SEMI }, 6, 0x36, Place_Direct_Byte_Opcode },
	{ (const unsigned char[]) { T_OPEN_SQ, T_REGISTER_PAIR_HL, T_CLOSE_SQ, T_EQUALS, T_HEX_LITERAL, T_SEMI }, 6, 0x36, Place_Direct_Byte_Opcode },
	{ (const unsigned char[]) { T_OPEN_SQ, T_REGISTER_PAIR_HL, T_CLOSE_SQ, T_EQUALS, T_ID, T_SEMI }, 6, 0x36, Place_Identifier_Byte_Opcode },
	LD_REG_VALUE(T_REGISTER_A, 0x3E),

	PUSH_POP(T_REGISTER_PAIR_BC, 0xC1),
	PUSH_POP(T_REGISTER_PAIR_DE, 0xD1),
	PUSH_POP(T_REGISTER_PAIR_HL, 0xE1),
	PUSH_POP(T_REGISTER_PAIR_AF, 0xF1),

	{ (const unsigned char[]) { T_SP, T_PLUS_EQUALS, T_NUMBER, T_SEMI }, 4, 0xE8, Place_Direct_Byte_Opcode },
	{ (const unsigned char[]) { T_SP, T_PLUS_EQUALS, T_HEX_LITERAL, T_SEMI }, 4, 0xE8, Place_Direct_Byte_Opcode },

	{ (const unsigned char[]) { T_REGISTER_PAIR_HL, T_EQUALS, T_SP, T_PLUS_EQUALS, T_NUMBER, T_SEMI }, 6, 0xF8, Place_Direct_Byte_Opcode },
	{ (const unsigned char[]) { T_REGISTER_PAIR_HL, T_EQUALS, T_SP, T_PLUS_EQUALS, T_HEX_LITERAL, T_SEMI }, 6, 0xF8, Place_Direct_Byte_Opcode },

	{ (const unsigned char[]) { T_SP, T_EQUALS, T_REGISTER_PAIR_HL, T_SEMI }, 4, 0xF9, Place_Opcode },

	{ (const unsigned char[]) { T_DISABLEI, T_SEMI }, 2, 0xF3, Place_Opcode },
	{ (const unsigned char[]) { T_ENABLEI, T_SEMI }, 2, 0xFB, Place_Opcode },

	{ (const unsigned char[]) { T_OPEN_SQ, T_HEX_LITERAL, T_PLUS_EQUALS, T_HEX_LITERAL, T_CLOSE_SQ, T_EQUALS, T_REGISTER_A, T_SEMI }, 8, 0xE0, Place_Direct_Byte_Opcode },
	{ (const unsigned char[]) { T_OPEN_SQ, T_HEX_LITERAL, T_PLUS_EQUALS, T_ID, T_CLOSE_SQ, T_EQUALS, T_REGISTER_A, T_SEMI }, 8, 0xE0, Place_Identifier_Byte_Opcode },
	{ (const unsigned char[]) { T_REGISTER_A, T_EQUALS, T_OPEN_SQ, T_HEX_LITERAL, T_PLUS_EQUALS, T_HEX_LITERAL, T_CLOSE_SQ, T_SEMI }, 8, 0xF0, Place_Direct_Byte_Opcode },
	{ (const unsigned char[]) { T_REGISTER_A, T_EQUALS, T_OPEN_SQ, T_HEX_LITERAL, T_PLUS_EQUALS, T_ID, T_CLOSE_SQ, T_SEMI }, 7, 0xF0, Place_Identifier_Byte_Opcode },

	{ (const unsigned char[]) { T_OPEN_SQ, T_HEX_LITERAL, T_PLUS_EQUALS, T_REGISTER_C, T_CLOSE_SQ, T_EQUALS, T_REGISTER_A, T_SEMI }, 8, 0xE2, Place_Opcode },
	{ (const unsigned char[]) { T_REGISTER_A, T_EQUALS, T_OPEN_SQ, T_HEX_LITERAL, T_PLUS_EQUALS, T_REGISTER_C, T_CLOSE_SQ, T_SEMI }, 7, 0xF2, Place_Opcode },

	{ (const unsigned char[]) { T_OPEN_SQ, T_HEX_LITERAL, T_CLOSE_SQ, T_EQUALS, T_REGISTER_A, T_SEMI }, 6, 0xEA, Place_Direct_Word_Opcode },
	{ (const unsigned char[]) { T_OPEN_SQ, T_ID, T_CLOSE_SQ, T_EQUALS, T_REGISTER_A, T_SEMI }, 6, 0xEA, Place_Identifier_Word_Opcode },
	{ (const unsigned char[]) { T_REGISTER_A, T_EQUALS, T_OPEN_SQ, T_HEX_LITERAL, T_CLOSE_SQ, T_SEMI }, 6, 0xFA, Place_Direct_Word_Opcode },
	{ (const unsigned char[]) { T_REGISTER_A, T_EQUALS, T_OPEN_SQ, T_ID, T_CLOSE_SQ, T_SEMI }, 6, 0xFA, Place_Identifier_Word_Opcode },



	{ (const unsigned char[]) { T_REGISTER_A, T_CARRY_LEFT_ROTATE, T_NUMBER, T_SEMI }, 4, 0x07, Place_CB_Opcode },
	{ (const unsigned char[]) { T_REGISTER_A, T_ROTATE_LEFT, T_NUMBER, T_SEMI }, 4, 0x17, Place_CB_Opcode },


	{ (const unsigned char[]) { T_REGISTER_A, T_CARRY_RIGHT_ROTATE, T_NUMBER, T_SEMI }, 4, 0x0F, Place_CB_Opcode },
	{ (const unsigned char[]) { T_REGISTER_A, T_ROTATE_RIGHT, T_NUMBER, T_SEMI }, 4, 0x1F, Place_CB_Opcode },


	DEFINE_CB_ROW(T_CARRY_LEFT_ROTATE, 0x00),
	DEFINE_CB_ROW(T_CARRY_RIGHT_ROTATE, 0x08),
	DEFINE_CB_ROW(T_ROTATE_LEFT, 0x10),
	DEFINE_CB_ROW(T_ROTATE_RIGHT, 0x18),
	DEFINE_CB_ROW(T_ARITHMETIC_RIGHT_SHIFT, 0x28),
	DEFINE_CB_ROW(T_LEFT_SHIFT, 0x20),
	DEFINE_CB_ROW(T_LOGICAL_RIGHT_SHIFT, 0x38),

	DEFINE_CB_BIT(T_REGISTER_B, 0),
	DEFINE_CB_BIT(T_REGISTER_C, 1),
	DEFINE_CB_BIT(T_REGISTER_D, 2),
	DEFINE_CB_BIT(T_REGISTER_E, 3),
	DEFINE_CB_BIT(T_REGISTER_H, 4),
	DEFINE_CB_BIT(T_REGISTER_L, 5),
	{ (const unsigned char[]) { T_OPEN_SQ, T_REGISTER_PAIR_HL, T_CLOSE_SQ, T_DOT, T_NUMBER, T_SEMI }, 6, 0x46, Place_CB_Bit_Opcode },
	{ (const unsigned char[]) { T_OPEN_SQ, T_REGISTER_PAIR_HL, T_CLOSE_SQ, T_DOT, T_HEX_LITERAL, T_SEMI }, 6, 0x46, Place_CB_Bit_Opcode },
	DEFINE_CB_BIT(T_REGISTER_A, 7)
};

// The assembler sifts through the identifiers and records them
// It also assigns data if there's a chunk of memory, declared by data{}
// If it's nothing like that, it'll check for operators here


char Does_Pattern_Match(Vector* Tokens, unsigned int* Token_Index, unsigned int Pattern)
{
	unsigned int Pattern_Token = 0;

	while (Pattern_Token < Patterns[Pattern].Token_Count)
	{
		if (((Token*)(Tokens->Data + Pattern_Token * sizeof(Token) + *Token_Index))->Token != Patterns[Pattern].Tokens[Pattern_Token])
			return 0;

		Pattern_Token++;
	}

	return 1;
}


const unsigned char* Get_Identifier_Representation(Unresolved_ID Unresolved, Vector* Identifiers)
{
	unsigned int Index = 0;

	Identifier* ID;

	while (Index < Identifiers->Size)
	{
		ID = (Identifier*)(Identifiers->Data + Index);

		if (!strcmp(Unresolved.Name, ID->Name))
			return ID->Representation;

		Index += sizeof(Identifier);
	}

	return NULL;
}

void Replace_Identifiers(unsigned char* ROM, Vector* Unresolved_IDs, Vector* Identifiers)
{
	unsigned int Index = 0;
	const unsigned char* Representation;
	unsigned int Value;
	Unresolved_ID* Unresolved;
	while (Index < Unresolved_IDs->Size)
	{
		Unresolved = (Unresolved_ID*)(Unresolved_IDs->Data + Index);

		Representation = Get_Identifier_Representation(*Unresolved, Identifiers);

		if (!Representation)
			printf("ERROR! Unresolved identifier: %s\n\n", Unresolved->Name);

		Value = Get_Value_From_String(Representation);

		if (Unresolved->Is_High)
			Value >>= 8u;

		if (Unresolved->Byte_Count == 1)
		{
			ROM[Unresolved->ROM_Index] = Value;
		}
		else
		{
			ROM[Unresolved->ROM_Index + 1] = Value >> 8u;
			ROM[Unresolved->ROM_Index] = Value & 0xFFu;
		}

		// writes the addresses accordingly!

		Index += sizeof(Unresolved_ID);
	}
}

void Replace_Local_Identifiers(unsigned char* ROM, Vector* Unresolved_IDs, Vector* Identifiers)
{
	unsigned int Index = 0;
	const unsigned char* Representation;
	unsigned int Value;
	Unresolved_ID* Unresolved;
	while (Index < Unresolved_IDs->Size)
	{
		Unresolved = (Unresolved_ID*)(Unresolved_IDs->Data + Index);

		Representation = Get_Identifier_Representation(*Unresolved, Identifiers);

		if (!Representation)
			Representation = Unresolved->Name;

		Value = Get_Value_From_String(Representation) - Unresolved->ROM_Index - 1;

		if (Unresolved->Is_High)
			Value >>= 8u;

		if (Unresolved->Byte_Count == 1)
		{
			ROM[Unresolved->ROM_Index] = Value;
		}
		else
		{
			ROM[Unresolved->ROM_Index + 1] = Value >> 8u;
			ROM[Unresolved->ROM_Index] = Value & 0xFFu;
		}

		// writes the addresses accordingly!

		Index += sizeof(Unresolved_ID);
	}
}

void Place_Pattern(unsigned char* ROM, unsigned int* ROM_Index, unsigned int* Token_Index, Vector* Tokens, Vector* Unresolved_IDs, Vector* Identifiers)
{
	unsigned int Pattern = 0;
	unsigned int Found = 0;

	while (sizeof(Token_Pattern) * Pattern < sizeof(Patterns))
	{
		if (Does_Pattern_Match(Tokens, Token_Index, Pattern))
		{
			Found++;
			break;
		}

		Pattern++;
	}

	if (!Found)
	{
		(*Token_Index) += sizeof(Token);
		return;
	}

	// Place pattern!

	(*Token_Index) += sizeof(Token) * Patterns[Pattern].Token_Count;

	Patterns[Pattern].Create_Function(ROM, Token_Index, ROM_Index, Unresolved_IDs, Tokens, Patterns[Pattern].Opcode);
}

//

//

void Generate_Byte_Code(Vector* Tokens, char* Buffer) // the buffer is 0x8000 bytes long
{
	Vector Unresolved[2] = { { 0, 0, 0 }, { 0, 0, 0 } }; // 0 is global, 1 is local

	unsigned int ROM_Index = 0;
	unsigned int Token_Index = 0;

	unsigned int Data_Flags = 0;

	Vector Identifiers[2] = { { 0, 0, 0 }, { 0, 0, 0 } }; // 0 is global, 1 is local

	Token* T;

	while (Token_Index < Tokens->Size)
	{
		T = (Token*)(Tokens->Data + Token_Index);

		if (T->Token == T_WORD || T->Token == T_BYTE)
		{
			Token_Index += sizeof(Token) * 5;
			continue;
		}

		if (T->Token == T_DATA)
		{
			Identifier ID;

			Data_Flags = 1;

			if (T[2].Token == T_HEX_LITERAL)
			{
				Token_Index += sizeof(Token);

				ROM_Index = Get_Value_From_String(T[2].Representation);
			}

			Token_Index += sizeof(Token) * 3;

			ID.Name = T[1].Representation;

			ID.Representation = Get_Hex_String_From_Word(ROM_Index);

			Vector_Push_Memory(Identifiers, &ID, sizeof(Identifier));

			continue;
		}

		if (T->Token == T_SUBROUTINE)
		{
			Identifier ID;

			if (T[2].Token == T_HEX_LITERAL)
			{
				Token_Index += sizeof(Token);

				ROM_Index = Get_Value_From_String(T[2].Representation);
			}

			Token_Index += sizeof(Token) * 3;

			ID.Name = T[1].Representation;

			ID.Representation = Get_Hex_String_From_Word(ROM_Index);

			Vector_Push_Memory(Identifiers, &ID, sizeof(Identifier));

			continue;
		}

		if (T->Token == T_CLOSE_SCOPE)
		{
			Data_Flags = 0;

			Token_Index += sizeof(Token);

			Replace_Local_Identifiers(Buffer, &Unresolved[1], &Identifiers[1]);

			Vector_Clear(Unresolved + 1); // clears local unresolved IDs
			Vector_Clear(Identifiers + 1); // cleras local identifiers

			continue;
		}

		if (T->Token == T_LABEL)
		{
			// 3 tokens

			//Unresolved_ID ID;

			//ID.ROM_Index = ROM_Index;
			//ID.Byte_Count = 1;
			//ID.Name = T[1].Representation;
			//ID.Is_High = 0;

			Identifier ID;

			ID.Name = T[1].Representation;
			ID.Representation = Get_Hex_String_From_Word(ROM_Index);

			Vector_Push_Memory(Identifiers + 1, &ID, sizeof(Identifier));

			Token_Index += sizeof(Token) * 3;

			continue;
		}

		if (Data_Flags)
		{
			if (T->Token == T_HIGH)
			{
				T += 2;
				if (T->Token == T_HEX_LITERAL || T->Token == T_NUMBER)
				{
					Buffer[ROM_Index++] = Get_Value_From_String(T->Representation) >> 8u;
					Token_Index += sizeof(Token) * 4;
				}
				else
				{
					Unresolved_ID ID;
					ID.Byte_Count = 1;
					ID.ROM_Index = ROM_Index++;
					ID.Is_High = 1;
					ID.Name = T->Representation;

					Vector_Push_Memory(Unresolved, &ID, sizeof(ID));

					Token_Index += sizeof(Token) * 4;
				}
			}
			else
			{
				if (T->Token == T_HEX_LITERAL || T->Token == T_NUMBER)
				{
					Buffer[ROM_Index++] = Get_Value_From_String(T->Representation);

					Token_Index += sizeof(Token);
				}
				else
				{
					Unresolved_ID ID;
					ID.Byte_Count = 1;
					ID.ROM_Index = ROM_Index++;
					ID.Is_High = 0;
					ID.Name = T->Representation;

					Vector_Push_Memory(Unresolved, &ID, sizeof(ID));

					Token_Index += sizeof(Token);
				}
			}
		}
		else
			Place_Pattern(Buffer, &ROM_Index, &Token_Index, Tokens, Unresolved, Identifiers);
	}

	Replace_Identifiers(Buffer, Unresolved, Identifiers);

	Generate_ROM_Header_Checksum(Buffer);
}

#endif
#ifndef C_VECTOR
#define C_VECTOR

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

struct Vector	// This is a wrapper for a vector data type
{
	unsigned char* Data;
	unsigned int Size;
	unsigned int Allocated_Memory;
};

typedef struct Vector Vector;

void Vector_Reserve(
	Vector* Target_Vector,
	unsigned int Size)
{
	unsigned char Realloc_Required = Size > Target_Vector->Allocated_Memory;

	while (Size > Target_Vector->Allocated_Memory)	// We may need to reserve new memory multiple times
	{
		Target_Vector->Allocated_Memory <<= 1u;
		Target_Vector->Allocated_Memory += Target_Vector->Allocated_Memory == 0; // If allocated memory is zero, just add one to get us some
		//Target_Vector->Data = (unsigned char*)realloc(Target_Vector->Data, Target_Vector->Allocated_Memory);
		// This allocates another chunk of memory to the vector
	}

	if (Realloc_Required)
	{
		Target_Vector->Data = (unsigned char*)realloc(Target_Vector->Data, Target_Vector->Allocated_Memory);
	}
}

void Vector_Resize(
	Vector* Target_Vector,
	unsigned int Size)
{
	Vector_Reserve(Target_Vector, Size);
	Target_Vector->Size = Size; // This actually changes the size of the vector
}

void Vector_Clear(Vector* Target_Vector)
{
	if (Target_Vector->Data)
		free(Target_Vector->Data);
	Target_Vector->Data = 0;
	Target_Vector->Allocated_Memory = 0;
	Target_Vector->Size = 0;
}

void Vector_Push_Memory(
	Vector* Target_Vector,
	const unsigned char* New_Data, // This is the data we're pushing to the vector
	unsigned int Size)		// This is the size in bytes of the memory
{
	Vector_Reserve(Target_Vector, Target_Vector->Size + Size);

	memcpy(Target_Vector->Data + Target_Vector->Size, New_Data, Size);
	Target_Vector->Size += Size;
}

void Vector_Push_Byte(Vector* Target_Vector, unsigned char Byte)
{
	unsigned int New_Size = Target_Vector->Size + 1;
	Vector_Reserve(Target_Vector, New_Size);
	Target_Vector->Data[Target_Vector->Size] = Byte;
	Target_Vector->Size = New_Size;
}

void Vector_Pop(Vector* Target_Vector, unsigned int Size)
{
	Target_Vector->Size -= Size;
}

void Vector_Pop_Byte(Vector* Target_Vector)
{
	Target_Vector->Size -= Target_Vector->Size != 0;
}


void Vector_From_String(Vector* Target_Vector, const char* String)
{
	unsigned int Index = 0;
	while (String[Index])
	{
		Vector_Push_Byte(Target_Vector, String[Index++]);
	}

	Vector_Push_Byte(Target_Vector, 0); // NULL terminate the string
}


#define ALPHANUMERIC_0 0
#define ALPHANUMERIC_LETTER 1
#define ALPHANUMERIC_NUMBER 2
unsigned char Is_Alpha_Numeric(unsigned char Character)
{
	if (Character > 0x2Fu && Character < 0x3Au)
		return ALPHANUMERIC_NUMBER;

	if (Character > 0x40u && Character < 0x5Bu ||
		Character > 0x60u && Character < 0x7Bu ||
		Character == '_')
		return ALPHANUMERIC_LETTER;

	return ALPHANUMERIC_0;
}

unsigned char Strict_Compare_Strings(unsigned char* String_1, const unsigned char* String_2)
{
	unsigned int X = 0;
	while (Is_Alpha_Numeric(String_1[X]) && Is_Alpha_Numeric(String_2[X]) && String_1[X] == String_2[X])
		X++;

	return (!(Is_Alpha_Numeric(String_1[X]) || Is_Alpha_Numeric(String_2[X]))) * X;
}

unsigned char Compare_Strings(unsigned char* String_1, const unsigned char* String_2)
{
	unsigned int X = 0;
	while (Is_Alpha_Numeric(String_1[X]) && Is_Alpha_Numeric(String_2[X]) && String_1[X] == String_2[X])
		X++;

	return (!String_2[X]) * X;
}

unsigned char String_Contains(unsigned char* String, unsigned char* Substring)
{
	unsigned int String_Index = 0;
	unsigned int Substring_Index = 0;

	while (String[String_Index])
	{
		if (Substring[Substring_Index] == String[String_Index])
			Substring_Index++;
		else
			Substring_Index = 0;

		if (!Substring[Substring_Index])
			return 1u;

		String_Index++;
	}

	return 0u;
}

unsigned char Characters_Until_Non_Alphanumeric(unsigned char* String)
{
	unsigned char X = 0;
	while (String[X] && Is_Alpha_Numeric(String[X]))
		X++;

	return X;
}

#endif
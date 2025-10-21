#include<stdio.h>

#include "C_Vector.h"
#include "Byte_Code_Generation.h"
#include "Syntax_Scanner.h"

void Get_User_Input_File(const char* Prompt, unsigned char* Buffer)
{
	unsigned int Index = 0;

	printf(Prompt);
	fgets(Buffer, 0xFFu, stdin);

	while (Buffer[Index]) Index++;

	Buffer[Index - 1] = '\0';
}

void Write_To_File(unsigned char* ROM, unsigned char* Buffer)
{
	FILE* File_Out = fopen(Buffer, "wb");

	fwrite(ROM, 0x8000, 1, File_Out);

	fclose(File_Out);
}

void main()
{
	unsigned char Buffer[0xFFu];

	Vector Tokens = { 0, 0, 0 };
	unsigned char ROM[0x8000u];

	memset(ROM, 0, 0x8000u);

	Get_User_Input_File("Please enter file directory to read!\n", Buffer);

	// Then we'll get the text document and analyse it character-by-character

	Tokenise(&Tokens, Buffer);
	Clean_Token_IDs(&Tokens);

	Generate_Byte_Code(&Tokens, ROM);

	Get_User_Input_File("Please enter file directory to write!\n", Buffer);

	Write_To_File(ROM, Buffer);
}
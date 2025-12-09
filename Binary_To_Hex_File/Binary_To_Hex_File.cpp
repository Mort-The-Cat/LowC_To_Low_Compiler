// Binary_To_Hex_File.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define _CRT_SECURE_NO_WRANINGS

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>

void Get_File_Contents(const char* File_Directory, std::vector<uint8_t>& Data)
{
    FILE* File = fopen(File_Directory, "rb");

    if (!File)
    {
        printf(" >> FATAL ERROR! Unable to open file %s\n", File_Directory);
        return;
    }

    fseek(File, 0, SEEK_END);

    size_t Size = ftell(File);

    fseek(File, 0, SEEK_SET);

    Data.resize(Size);

    fread(Data.data(), 1, Size, File);

    fclose(File);
}

void Write_File_Contents(const char* File_Directory, const std::string& File_Contents)
{
    FILE* File = fopen(File_Directory, "wt");

    if (!File)
    {
        printf(" >> FATAL ERROR! Unable to open file %s\n", File_Directory);
        return;
    }

    fwrite(File_Contents.data(), 1, File_Contents.size(), File);

    fclose(File);
}

std::string Byte_To_Hex(uint8_t Value)
{
    std::string Hex = "0x00";

    char Hex_Digits[] = "0123456789ABCDEF";

    Hex[2] = Hex_Digits[Value >> 4];
    Hex[3] = Hex_Digits[Value & 0xF];

    return Hex;
}

int main()
{
    std::string Input_File;
    std::string Output_File;

    std::cin >> Input_File >> Output_File;

    std::vector<uint8_t> Data;
    
    Get_File_Contents(Input_File.c_str(), Data);

    std::string Output_Hex = "\t";

    for (size_t W = 0; W < Data.size(); W++)
    {
        Output_Hex += Byte_To_Hex(Data[W]);

        if ((W + 1) & 7u)
            Output_Hex += ", ";
        else
            Output_Hex += ",\n\t";
    }

    Output_Hex.pop_back();

    Write_File_Contents(Output_File.c_str(), Output_Hex);
}

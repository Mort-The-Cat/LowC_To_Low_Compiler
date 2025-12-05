#include "Code_Trace_Handler.h"
#include "Code_Parser.h"
#include "LowC_Tokeniser.h"

size_t CON_NOT(size_t Condition)
{
	return (Condition + (2 - 4 * (Condition > 2)));
}

const char* Conditionals[] =
{
	"",
	" zero",
	" carry",
	" not_zero",
	" not_carry"
};

Trace* Register_From_Name(Tracer_Data& Tracer, char Letter)
{
	size_t Index = Letter - 'A';
	if (Letter == 'H')
		return &Tracer.Registers[5];
	if (Letter == 'L')
		return &Tracer.Registers[6];

	if (Index > 6)
		return nullptr;

	return &Tracer.Registers[Index];
}

void Store_Tracer_Register_Back_In_Memory(std::string& Output_Low_Code, Tracer_Data& Tracer, Trace* Variable);

void Analyse_Statements_LowC(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node);

#define MAKE_VALUE_HOT_NO_REQUIREMENTS 0
#define MAKE_VALUE_HOT_A_REG 1
#define MAKE_VALUE_HOT_HL_REG 2

#define MAKE_VALUE_HOT_REG_PAIR 4
#define MAKE_VALUE_HOT_REG 5

#define PULL_IDENTIFIER_COPY 1
#define PULL_IDENTIFIER_REF 0

Trace& Get_Free_Register(std::string& Output_Low_Code, Tracer_Data& Tracer, size_t Requirements = 0)
{
	// This looks for registers that suit the requirements

	// if there are none? make room by storing a hot value if possible

	switch (Requirements)
	{
	case MAKE_VALUE_HOT_A_REG:
		// If A isn't free? Store A in a register that IS free and clear 'A' on the tracer

		if (Tracer.Registers[0].Modified_Counter)
		{
			Trace* Temp_Reg = &Get_Free_Register(Output_Low_Code, Tracer);	// no requirements, just get a register we can temporarily store 'A' in
			Temp_Reg->Value = Tracer.Registers[0].Value;
			Temp_Reg->Modified_Counter = Tracer.Registers[0].Modified_Counter;
			Temp_Reg->Pointer_Flag = Tracer.Registers[0].Pointer_Flag;		// copies over everything EXCEPT the register name to the other register

			Output_Low_Code += "\t" + Temp_Reg->Name + " = A;\t\t# Frees up 'A' register for use\n";

			Tracer.Registers[0].Modified_Counter = 0;						// frees up this register for use!
		}

		return Tracer.Registers[0];

		//

	case MAKE_VALUE_HOT_HL_REG:

		if (Tracer.Registers[5].Modified_Counter || Tracer.Registers[6].Modified_Counter)
		{
			// if HL isn't free? find another register pair that is

			Trace* Temp_Reg = &Get_Free_Register(Output_Low_Code, Tracer, MAKE_VALUE_HOT_REG_PAIR);	// Note that HL doesn't necessarily always represent a 16-bit value, but it can be assumed to

			if (Temp_Reg->Name == "H")
				return *Temp_Reg;

			Temp_Reg[0].Value = Tracer.Registers[5].Value;
			Temp_Reg[1].Value = Tracer.Registers[6].Value;

			Temp_Reg[0].Modified_Counter = Tracer.Registers[5].Modified_Counter;
			Temp_Reg[1].Modified_Counter = Tracer.Registers[6].Modified_Counter;

			//Temp_Reg[0].Pointer_Flag = Tracer.Registers[5].Pointer_Flag;
			//Temp_Reg[1].Pointer_Flag = Tracer.Registers[6].Pointer_Flag;

			if (Temp_Reg[0].Modified_Counter) // any kinda value here?
				Output_Low_Code += "\t" + Temp_Reg[0].Name + " = H;\t\t# Frees up H register for use\n";

			if (Temp_Reg[1].Modified_Counter)
				Output_Low_Code += "\t" + Temp_Reg[1].Name + " = L;\t\t# Frees up L register for use\n";

			//Output_Low_Code +=
			//	"\t" + Temp_Reg[0].Name + " = H;\t\t# Frees up H register for use\n" +
			//	"\t" + Temp_Reg[1].Name + " = L;\t\t# Frees up L register for use\n";
		}

		Tracer.Registers[5].Modified_Counter = 0;
		Tracer.Registers[6].Modified_Counter = 0;

		return Tracer.Registers[5];	// return HL register pair

		//

	default:
	case MAKE_VALUE_HOT_NO_REQUIREMENTS:

		// iterate through registers and find one that isn't hot.
		// if we can't find one that isn't hot? find a value you can store back to memory
	{

		const size_t Indices[] = { 0, 6, 5, 4, 3, 2, 1 };

		int Tries = 8;

		while (Tries)
		{

			for (size_t Index = 0; Index < 7; Index++)
			{
				if (Tracer.Registers[Indices[Index]].Modified_Counter == 0)
					return Tracer.Registers[Indices[Index]];
			}

			// store something back into memory if possible

			for (size_t Index = 0; Index < 7; Index++)
				if (Tracer.Registers[Index].Modified_Counter == 1)
				{
					Store_Tracer_Register_Back_In_Memory(Output_Low_Code, Tracer, &Tracer.Registers[Index]);
					Tries--;

					break;
				}

			Tries--;
		}

		// I'll implement this later

		break;

	}

	//

	case MAKE_VALUE_HOT_REG_PAIR:

	{
		int Tries = 8;

		while (Tries)
		{

			for (long Index = 1; Index < 7; Index += 2)
			{
				if (Tracer.Registers[Index].Modified_Counter == 0 && Tracer.Registers[Index + 1].Modified_Counter == 0)
					return Tracer.Registers[Index];
			}


			for (long Index = 5; Index >= 0; Index -= 2)
				if (Tracer.Registers[Index].Modified_Counter == 1)
				{
					// Honestly... just push ts to the stack </3

					Output_Low_Code += "\tpush " + Tracer.Registers[Index].Name + Tracer.Registers[Index + 1].Name + ";\t\t# Push 16-bit word to stack to free up registers\n";

					Tracer.Stack.push_back(Trace("@panic@", Tracer.Registers[Index + 1].Value, 1));
					Tracer.Stack.push_back(Trace("@panic@", Tracer.Registers[Index].Value, 2));

					Tracer.Registers[Index].Modified_Counter = 0;
					Tracer.Registers[Index + 1].Modified_Counter = 0;

					// we can 'pop' it later 

					//Store_Tracer_Register_Back_In_Memory(Output_Low_Code, Tracer, &Tracer.Registers[Index]);

					Tries--;
					break;
				}

			Tries--;

		}

		break;

	}

	// this too
	// although this should be a little easier because I can instantly 'push' a reg pair to the stack effortlessly
	}
}

void Clear_Tracer_Registers(Tracer_Data& Tracer)
{
	for (size_t Register = 0; Register < Tracer.Registers.size(); Register++)
		if (Tracer.Registers[Register].Modified_Counter == 2)
			Tracer.Registers[Register].Modified_Counter = 0;
}

Trace* Find_Value_In_Tracer_Registers(Tracer_Data& Tracer, std::string Value)
{
	for (size_t Index = 0; Index < Tracer.Registers.size(); Index++)
	{
		if (Tracer.Registers[Index].Value == Value && Tracer.Registers[Index].Modified_Counter)
		{
			return &Tracer.Registers[Index];
		}
	}

	return nullptr;
}

long Find_Value_In_Tracer_Stack(Tracer_Data& Tracer, std::string Value)
{
	long Offset = 0;

	for (long Index = Tracer.Stack.size() - 1; Index >= 0; Index--, Offset++)
	{
		if (Tracer.Stack[Index].Value == Value && Tracer.Stack[Index].Name != "@panic@")
		{
			return Offset;
		}
	}

	return -1;
}

long Find_Value_Address_In_Tracer_Stack(Tracer_Data& Tracer, std::string Value)
{
	long Offset = 0;

	for (long Index = Tracer.Stack.size() - 1; Index >= 0; Index--, Offset++)
	{
		if (Tracer.Stack[Index].Name == Value)
		{
			return Offset;
		}
	}

	return -1;
}

bool Writeback_Panic_Push(std::string& Output_Low_Code, Tracer_Data& Tracer)	// This fixes any 'panic' pushes, grabbing the memory back from the stack and into a register
{
	if (!Tracer.Stack.size())
		return false;


	if (Tracer.Stack.back().Name == "@panic@")
	{
		if (Find_Value_In_Tracer_Registers(Tracer, Tracer.Stack.back().Value))
		{
			Output_Low_Code += "\tSP += 2;\t\t# Fixes 'panic' push\n";

			Tracer.Stack.pop_back();
			Tracer.Stack.pop_back();

			return true;
		}

		Trace* Register_Pair = &Get_Free_Register(Output_Low_Code, Tracer, MAKE_VALUE_HOT_REG_PAIR);

		Register_Pair[0].Value = Tracer.Stack.back().Value;
		Register_Pair[1].Value = Tracer.Stack.back().Value;

		Register_Pair[0].Modified_Counter = 1;
		Register_Pair[1].Modified_Counter = 1;

		Output_Low_Code += "\tpop " + Register_Pair[0].Name + Register_Pair[1].Name + ";\t\t# Fixes 'panic' push, places value back into register\n";

		Tracer.Stack.pop_back();
		Tracer.Stack.pop_back();

		return true;
	}

	return false;
}

void Store_Tracer_Register_Back_In_Memory(std::string& Output_Low_Code, Tracer_Data& Tracer, Trace* Variable)
{
	/*for (size_t Register = 0; Register < Tracer.Registers.size(); Register++)
		if (Tracer.Registers[Register].Modified_Counter == 1)					// If 'hot'
		{
			Tracer.Registers[Register].Modified_Counter = 0;


		}*/

		// 'address' is the register pair in which the address is stored in
		// if this is a stack variable (which it likely is)
		// then we need to

		// finds 'Variable' on stack

	long Stack_Position = Find_Value_In_Tracer_Stack(Tracer, Variable->Value);

	if (Stack_Position == -1)
	{

		printf(" >> SERIOUS ERROR! Unable to find variable %s on stack tracer!\n", Variable->Value.c_str());
	}

	Trace* Stack_Memory = &Tracer.Stack[Tracer.Stack.size() - Stack_Position - 1];

	std::string Value = Variable->Value;

	if (Stack_Memory->Pointer_Flag)	// Some kind of 16-bit value?
	{
		Get_Free_Register(Output_Low_Code, Tracer, MAKE_VALUE_HOT_HL_REG); // Frees up HL register

		Trace* New_Value_Address = Find_Value_In_Tracer_Registers(Tracer, Value);	// We MIGHT need to overwrite this?

		Output_Low_Code += "\tHL = SP + " + std::to_string(Stack_Position) + ";\t\t# Memory location of " + Value + "\n";

		Output_Low_Code += "\t[HL] = " + New_Value_Address[1].Name + ";\t\t# Stores " + Value + " back into memory\n";
		Output_Low_Code += "\tHL++;\n";
		Output_Low_Code += "\t[HL] = " + New_Value_Address[0].Name + ";\t\t# Stores upper byte of " + Value + " back into memory\n";

		New_Value_Address[0].Modified_Counter = 0;
		New_Value_Address[1].Modified_Counter = 0;
	}
	else
	{
		Get_Free_Register(Output_Low_Code, Tracer, MAKE_VALUE_HOT_HL_REG);

		Trace* New_Value_Address = Find_Value_In_Tracer_Registers(Tracer, Value);	// Me MIGHT need to overwrite this when freeing HL

		Output_Low_Code += "\tHL = SP + " + std::to_string(Stack_Position) + ";\t\t# Memory location of " + Value + "\n";
		Output_Low_Code += "\t[HL] = " + New_Value_Address->Name + ";\t\t# Stores " + Value + " back into memory\n";

		New_Value_Address[0].Modified_Counter = 0;
	}
}

const Parse_Node* Find_Value_In_Local_Consts(Tracer_Data& Tracer, std::string Value)
{
	for (size_t Index = 0; Index < Tracer.Local_Const_Declarations.size(); Index++)
	{
		if (Tracer.Local_Const_Declarations[Index]->Child_Nodes.at("id")[0].Value == (Local_Function_Scope_Name + Value))
			return Tracer.Local_Const_Declarations[Index];
	}

	return nullptr;
}

const Parse_Node* Find_Value_In_Global_Stack(Tracer_Data& Tracer, std::string Value)
{
	for (size_t Index = 0; Index < Tracer.Global_Const_Declarations.size(); Index++)
	{
		if (Tracer.Global_Const_Declarations[Index]->Child_Nodes.at("id")[0].Value == Value)
			return Tracer.Global_Const_Declarations[Index];
	}

	return nullptr;
}

std::string Tracer_Make_Value_Hot(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node, size_t Requirements = MAKE_VALUE_HOT_NO_REQUIREMENTS, size_t Copy_Requirements = PULL_IDENTIFIER_REF);

std::string Fit_Register_Requirements(std::string& Output_Low_Code, Tracer_Data& Tracer, std::string Register, size_t Requirements, size_t Copy_Requirements)
{
	// Checks if requirements are met

	Trace* Reg = Register_From_Name(Tracer, Register[0]);

	size_t Hot = Reg->Modified_Counter;

	switch (Requirements)
	{
	case MAKE_VALUE_HOT_REG:
	{
		std::string Lower_Byte = " ";
		Lower_Byte[0] = Register.back();

		if (Copy_Requirements == 1 && Hot == 1)
		{
			// We need to make a copy of this value...
			Reg->Modified_Counter = 3;
			if (Register.size() > 1)
				Reg[1].Modified_Counter = 3;

			Trace* Temp = &Get_Free_Register(Output_Low_Code, Tracer, 0);

			Output_Low_Code += "\t" + Temp->Name + " = " + Lower_Byte + "; \t\t# Makes a copy of " + Reg->Value + "\n";

			Temp->Value = Reg->Value;
			Temp->Modified_Counter = 2;

			Reg->Modified_Counter = 1;
			if (Register.size() > 1)
			{
				Reg[1].Modified_Counter = 1;
			}

			Lower_Byte[0] = Temp->Name[0];

			// 'Lower_Byte' is now just a copy of the value
		}

		return Lower_Byte;
	}

	//

		case MAKE_VALUE_HOT_A_REG:
		{
			if (Register == "A")	// We're exactly where we want us to be
			{
				if (Copy_Requirements == 1 && Hot == 1)
				{
					// We need to make a copy of this value... free up a register and make *that* the hot identifier

					Reg->Modified_Counter = 3;
					Trace& Temp = Get_Free_Register(Output_Low_Code, Tracer, 0);

					Output_Low_Code += "\t" + Temp.Name + " = A;\t\t# Makes 'A' a copy of " + Reg->Value + "\n";

					Temp.Value = Reg->Value;
					Temp.Modified_Counter = 1;

					Reg->Modified_Counter = 2;
				}

				return "A";
			}

			// otherwise, we want to store Reg into 'A'

			Reg->Modified_Counter = 3;

			std::string Reg_Value = Reg->Value;

			Reg->Value = "@register@";

			Get_Free_Register(Output_Low_Code, Tracer, MAKE_VALUE_HOT_A_REG); // Free up 'A' register

			Output_Low_Code += "\tA = " + Reg->Name + ";\t\t# Stores " + Reg->Value + " into 'A' register for use\n";

			if (Hot == 1 && Copy_Requirements == 1)
			{
				// If we gotta specify a copy?

				Reg = Find_Value_In_Tracer_Registers(Tracer, "@register@");

				Reg->Modified_Counter = 1;
				Reg->Value = Reg_Value;

				Tracer.Registers[0].Modified_Counter = 2;
				Tracer.Registers[0].Value = Reg->Value;
			}
			else
			{
				// Otherwise? just move the values over normally

				Tracer.Registers[0].Modified_Counter = 1;
				Tracer.Registers[0].Value = Reg_Value;

				Reg = Find_Value_In_Tracer_Registers(Tracer, "@register@");

				Reg->Modified_Counter = 0;	// We're not using this anymore, we've std::move'd it haha
			}

			return "A";
		}

		//

		case MAKE_VALUE_HOT_HL_REG:
		case MAKE_VALUE_HOT_REG_PAIR:
		{
			// add checks to see if this is a byte of some kind?

			if (Register.size() == 2)
			{
				// already stored as a word

				if ((Hot == 1 && Copy_Requirements == 1) || (Requirements == MAKE_VALUE_HOT_HL_REG && Register != "HL")) // We gotta make a copy and ensure the returned register is the copy
				{
					Reg[0].Modified_Counter = 3;
					Reg[1].Modified_Counter = 3;

					Trace* Copy_Register = &Get_Free_Register(Output_Low_Code, Tracer, Requirements);

					if (Copy_Register->Name != Reg->Name)
					{
						Output_Low_Code += "\t" + Copy_Register[0].Name + " = " + Reg[0].Name + ";\t\t# Creates copy of " + Reg[0].Value + "\n";
						Output_Low_Code += "\t" + Copy_Register[1].Name + " = " + Reg[1].Name + ";\t\t# Creates copy of " + Reg[1].Value + "\n";
					}

					Copy_Register[0].Value = Reg[0].Value;
					Copy_Register[1].Value = Reg[1].Value;

					if (Copy_Requirements == 1)
					{
						Copy_Register[0].Modified_Counter = 2;
						Copy_Register[1].Modified_Counter = 2;
						Reg[0].Modified_Counter = 1;
						Reg[1].Modified_Counter = 1;
					}
					else
					{
						Reg[0].Modified_Counter = 0;
						Reg[1].Modified_Counter = 0;
						Copy_Register[0].Modified_Counter = 1;
						Copy_Register[1].Modified_Counter = 1;
					}

					Register = Copy_Register[0].Name + Copy_Register[1].Name;
				}

				return Register;
			}
			else
			{
				// stored as a byte...?
				Reg->Modified_Counter = 3;
				Trace* Copy_Register = &Get_Free_Register(Output_Low_Code, Tracer, Requirements);

				Copy_Register[0].Modified_Counter = 2;
				if (Hot == 1 && Copy_Requirements == 1)
				{
					// we want to create a copy of the register in question

					Copy_Register[1].Modified_Counter = 2;

					Copy_Register[1].Value = Reg->Value;	// Only a copy
				}
				else
				{
					// we just want to 'move' the register over, discarding the old register

					Copy_Register[1].Modified_Counter = 1;
					Copy_Register[1].Value = Reg->Value;

					Reg->Modified_Counter = 0;
				}

				Output_Low_Code += "\t" + Copy_Register[0].Name + " = $00;\t\t# Zeroes high-byte\n";
				Output_Low_Code += "\t" + Copy_Register[1].Name + " = " + Reg->Name + ";\t\t# Writes " + Reg->Value + " to low byte\n";

				return Copy_Register[0].Name + Copy_Register[1].Name;
			}
		}

		default:
			return Register; // no issue!
	}
}

std::string Get_Back_Register(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node, size_t Requirements = MAKE_VALUE_HOT_NO_REQUIREMENTS, size_t Copy_Requirements = PULL_IDENTIFIER_REF)
{
	// Gets identifier from registers,
	// if not, gets them from stack

	std::string Register;

	Trace* Found; 

	if (Found = Find_Value_In_Tracer_Registers(Tracer, Node.Value))
	{
		// found it already!

		// try and fit it to requirements if it doesn't already
		if (Node.Syntax_ID == S_ID16)
			return Fit_Register_Requirements(Output_Low_Code, Tracer, Found[0].Name + Found[1].Name, Requirements, Copy_Requirements);
		else
			return Fit_Register_Requirements(Output_Low_Code, Tracer, Found[0].Name, Requirements, Copy_Requirements);
	}

	// not in current registers? check the stack...

	long Stack_Index = Find_Value_In_Tracer_Stack(Tracer, Node.Value);

	if (Stack_Index != -1)	// Found on stack!
	{
		// Get value off stack according to requirements...

		// check if it's a byte or a word

		Get_Free_Register(Output_Low_Code, Tracer, MAKE_VALUE_HOT_HL_REG);	// Need to free up HL to grab values off the stack
		Tracer.Registers[5].Modified_Counter = 3;
		Tracer.Registers[6].Modified_Counter = 3;

		Stack_Index = Find_Value_In_Tracer_Stack(Tracer, Node.Value);	// (It's quite likely that a 'panic' push was required to get this free register, so the SP needs to be updated)

		Found = &Tracer.Stack[Tracer.Stack.size() - 1 - Stack_Index];

		Output_Low_Code += "\tHL = SP + " + std::to_string(Stack_Index) + ";\t\t# Fetches " + Node.Value + " off the stack\n";

		if (Found->Pointer_Flag)
		{
			// if it's a word? just get a valid register and store it there

			Trace* Temp = &Get_Free_Register(Output_Low_Code, Tracer, 0); // just get a temp register to simplify things

			Output_Low_Code += "\t" + Temp->Name + " = [HL];\t\t# Store lower byte temporarily\n";
			Output_Low_Code += "\tHL++;\n\tH = [HL];\t\t# Store upper byte\n\tL = " + Temp->Name + ";\t\t# Move lower byte from temp register\n";

			Tracer.Registers[5].Value = Node.Value;
			Tracer.Registers[6].Value = Node.Value;

			Tracer.Registers[5].Modified_Counter = 1 + Copy_Requirements;
			Tracer.Registers[6].Modified_Counter = 1 + Copy_Requirements;

			return "HL";
		}
		else
		{
			if (Requirements == MAKE_VALUE_HOT_A_REG)	// we specifically need A register?
			{
				Trace* Temp = &Get_Free_Register(Output_Low_Code, Tracer, MAKE_VALUE_HOT_A_REG);

				Output_Low_Code += "\tA = [HL];\t\t# Store value into 'A' register\n";

				Temp->Modified_Counter = 1 + Copy_Requirements;
				Temp->Value = Node.Value;

				Tracer.Registers[5].Modified_Counter = 0;	// We're not using these anymore
				Tracer.Registers[6].Modified_Counter = 0;

				return "A";
			}
			else if (Requirements == MAKE_VALUE_HOT_HL_REG || Requirements == MAKE_VALUE_HOT_REG_PAIR)	// we specifically need HL?
			{
				Output_Low_Code += "\tL = [HL];\t\t# Stores " + Node.Value + " into lower byte of HL\n";
				Output_Low_Code += "\tH = $00;\t\t# Zeroes out upper byte\n";

				Tracer.Registers[5].Modified_Counter = 2;
				Tracer.Registers[6].Modified_Counter = 1 + Copy_Requirements;

				Tracer.Registers[5].Value = ""; //Node.Value;
				Tracer.Registers[6].Value = Node.Value;

				return "HL";
			}
			else // no issue
			{
				Tracer.Registers[5].Modified_Counter = 0;
				Tracer.Registers[6].Modified_Counter = 0;

				//std::string = Get_Free_Register()

				Trace* Register = &Get_Free_Register(Output_Low_Code, Tracer, 0);	// any kind of free register other than HL (but HL is still available)

				Output_Low_Code += "\t" + Register->Name + " = [HL];\t\t# Stores " + Node.Value + " into " + Register->Name + " register\n";

				//Tracer.Registers[5].Modified_Counter = 0;
				//Tracer.Registers[6].Modified_Counter = 1 + Copy_Requirements;

				//Tracer.Registers[5].Value = "";
				//Tracer.Registers[6].Value = Node.Value;

				Register->Modified_Counter = 1 + Copy_Requirements;
				Register->Value = Node.Value;

				return Register->Name;
			}
		}
	}

	// Then check stack addresses

	Stack_Index = Find_Value_Address_In_Tracer_Stack(Tracer, Node.Value);

	if (Stack_Index != -1)
	{
		// This can only be a register pair because it's an address!

		Get_Free_Register(Output_Low_Code, Tracer, MAKE_VALUE_HOT_HL_REG);	// Free up HL registers

		Stack_Index = Find_Value_Address_In_Tracer_Stack(Tracer, Node.Value); // (it's entirely possible that a 'panic' push was done, so double-check the stack pointer)

		Output_Low_Code += "\tHL = SP + " + std::to_string(Stack_Index) + ";\t\t# Gets " + Node.Value + " address\n";

		Tracer.Registers[5].Modified_Counter = 2;
		Tracer.Registers[6].Modified_Counter = 2;

		Tracer.Registers[5].Value = Node.Value;
		Tracer.Registers[6].Value = Node.Value;

		return Fit_Register_Requirements(Output_Low_Code, Tracer, "HL", Requirements, 0); // no requirements since this is never an identifier
	}

	// If not in registers or stack? Check global constant etc

	std::string Name = Node.Value;

	if (!Find_Value_In_Global_Stack(Tracer, Name))	// If not a global constant? check local constants
		Name = Local_Function_Scope_Name + Name;	// found the local constant!

	Trace* Temp = &Get_Free_Register(Output_Low_Code, Tracer, Requirements);

	Temp[0].Modified_Counter = 2;

	if (Requirements == MAKE_VALUE_HOT_HL_REG || Requirements == MAKE_VALUE_HOT_REG_PAIR)
	{
		// Need to do two
		Output_Low_Code += "\t" + Temp[0].Name + Temp[1].Name + " = " + Name + ";\n";

		Temp[1].Modified_Counter = 2;

		return Temp[0].Name + Temp[1].Name;
	}
	else
		Output_Low_Code += "\t" + Temp[0].Name + " = " + Name + ";\n";

	return Temp[0].Name;
}

std::string Tracer_Operator8(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node, size_t Requirements, size_t Copy_Requirements)
{
	const std::map<size_t, std::array<std::string, 2>> Operators = {
		{ S_PLUS8,		{ "+= ",	"S_PLUS8"		}	},
		{ S_MINUS8,		{ "-= ",	"S_MINUS8"		}	},
		{ S_OR8,		{ "|= ",	"S_OR8"			}	},
		{ S_AND8,		{ "&= ",	"S_AND8"		}	},
		{ S_XOR8,		{ "^= ",	"S_XOR8"		}	},
		{ S_LESS_THAN8,	{ "< ",	"S_LESS_THAN8"	}	}
	};

	// This will put 'left' into the A register and 'right' into a register of our choice
		// Then, it'll tell the tracer that the result is stored in 'A'

		//Register_From_Name(Tracer, Right_Operand_)

	std::string Right_Reg = Tracer_Make_Value_Hot(Output_Low_Code, Tracer, Node["right"][0], 0); // no requirements for the right value... algs!
	Trace* Right_Trace = Register_From_Name(Tracer, Right_Reg.back());

	std::string Right_Trace_Name;// = Right_Trace->Value;

	size_t Previous_Modified_Counter;

	if (Right_Trace)	// Note that this can be an int-literal
	{
		Previous_Modified_Counter = Right_Trace->Modified_Counter;

		Right_Trace_Name = Right_Trace->Value;

		Right_Trace->Value = "@value@";

		Right_Trace->Modified_Counter = 3;
	}
	else
		Previous_Modified_Counter = 0;

	Tracer_Make_Value_Hot(Output_Low_Code, Tracer, Node["left"][0], MAKE_VALUE_HOT_A_REG, Copy_Requirements);

	if (Right_Trace)	// note that this can be an int-literal
	{
		Right_Trace = Find_Value_In_Tracer_Registers(Tracer, "@value@");
		Right_Reg = Right_Trace->Name;
	}
	// this needs to be a copy because we're going to modify its value

	Output_Low_Code += "\tA " + Operators.at(Node.Syntax_ID)[0] + Right_Reg + ";\t\t# " + Operators.at(Node.Syntax_ID)[1] + "\n";

	if (Right_Trace)		// note that this can be an int-literal
	{
		Right_Trace->Modified_Counter = Previous_Modified_Counter;		// This returns it to its "pre-modified" state
		Right_Trace->Value = Right_Trace_Name;
	}

	return Fit_Register_Requirements(Output_Low_Code, Tracer, "A", Requirements, Copy_Requirements);
}

std::string Make_Int_Literal_Hot(std::string& Output_Low_Code, Tracer_Data& Tracer, std::string Value, size_t Requirements, size_t Copy_Requirements)
{
	if (Requirements == MAKE_VALUE_HOT_HL_REG || Requirements == MAKE_VALUE_HOT_REG_PAIR)
	{
		Trace* Output_Registers = &Get_Free_Register(Output_Low_Code, Tracer, Requirements);

		Output_Low_Code +=
			"\t" + Output_Registers[0].Name + Output_Registers[1].Name + " = " + Value + ";\n";

		Output_Registers[0].Value = "";
		Output_Registers[1].Value = "";

		Output_Registers[0].Modified_Counter = 2;
		Output_Registers[1].Modified_Counter = 2;

		return Output_Registers[0].Name + Output_Registers[1].Name;
	}
	else if (Requirements == MAKE_VALUE_HOT_A_REG)
	{
		Trace* Output_Register = &Get_Free_Register(Output_Low_Code, Tracer, Requirements);

		Output_Register[0].Value = "";

		Output_Register[0].Modified_Counter = 2;

		Output_Low_Code +=
			"\tA = " + Value + ";\n";

		return "A";
	}
	else if (Requirements == MAKE_VALUE_HOT_REG)
	{
		Trace* Output_Register = &Get_Free_Register(Output_Low_Code, Tracer, Requirements);

		Output_Register[0].Value = "";

		Output_Register[0].Modified_Counter = 2;

		Output_Low_Code +=
			"\t" + Output_Register->Name + " = " + Value + ";\n";

		return Output_Register->Name;
	}

	return Value;

	//
}

std::string Tracer_Shift(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node, size_t Requirements, size_t Copy_Requirements)
{
	// Check if this is a shift 16 or a shift 8

	// then use the appropriate left/right style shift

	std::string Value;

	if (Node.Syntax_ID == S_RIGHT_SHIFT16 || Node.Syntax_ID == S_LEFT_SHIFT16)
	{
		Value = Tracer_Make_Value_Hot(Output_Low_Code, Tracer, Node["value"][0], MAKE_VALUE_HOT_REG_PAIR, Copy_Requirements);

		if (Node.Syntax_ID == S_RIGHT_SHIFT16)
		{
			Output_Low_Code += "\t" + Value.substr(1, 1) + " >>= 1;\n";
			Output_Low_Code += "\t" + Value.substr(0, 1) + " |>><= 1;\n";
		}
		else
		{
			Output_Low_Code += "\t" + Value.substr(0, 1) + " <<= 1;\n";
			Output_Low_Code += "\t" + Value.substr(1, 1) + " <<>|= 1;\n";
		}
	}
	else
	{
		Value = Tracer_Make_Value_Hot(Output_Low_Code, Tracer, Node["value"][0], MAKE_VALUE_HOT_REG, Copy_Requirements);

		if (Node.Syntax_ID == S_RIGHT_SHIFT8)
		{
			Output_Low_Code += "\t" + Value + " >>= 1;\n";
		}
		else
		{
			Output_Low_Code += "\t" + Value + " <<= 1;\n";
		}
	}

	return Fit_Register_Requirements(Output_Low_Code, Tracer, Value, Requirements, Copy_Requirements);
}

std::string Tracer_Make_Value_Hot(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node, size_t Requirements, size_t Copy_Requirements)
{
	// If expression? evaluate and store in register under the name 'value' or whatever
	// If int-literal? perhaps value can be baked into code (instead of using a register or whatever)
	// 
	// if id? search for id:
	//		- in existing hot registers,
	//		- on the stack,
	//		- in global consts,
	//		- in local consts

	// Find register that fits condition (i.e. is wanted register and DOESN'T already contain a hot value)
	// Free up register for use (if necessary)
	// 

	// This will return the Low register/int-literal equivalent to this value
	// If getting this value requires additional instructions (or even just code to store a value in a register),
	// this function adds them automatically in order to fully evaluate the expression

	switch (Node.Syntax_ID)
	{
	case S_HIGH:
	{
		// This will get the value into a register pair
		// Then, it'll return the high value
		// If we need to make a copy? Do it

		// if we're applying this directly to it? just grab the high register

		if (!Copy_Requirements)
		{
			std::string Word = Tracer_Make_Value_Hot(Output_Low_Code, Tracer, Node["value"][0], MAKE_VALUE_HOT_REG_PAIR, 0);

			Trace* Word_Register = Register_From_Name(Tracer, Word[0]);

			//Word_Register[0].Modified_Counter = 1;
			//Word_Register[1].Modified_Counter = 1;	// We want to search for a free register (can include this one)

			/*Trace* Output_Register = &Get_Free_Register(Output_Low_Code, Tracer, 0);

			Output_Register->Modified_Counter = 2;

			if(Output_Register->Name[0] != Word[0])
				Output_Low_Code += "\t" + Output_Register->Name + " = " + Word[0] + ";\t\t# Moves high byte of " + Node["value"][0].Value + " into register\n";
			*/
			return Word_Register[0].Name;
		}
		else
		{
			// Otherwise? Make a copy

			std::string Word = Tracer_Make_Value_Hot(Output_Low_Code, Tracer, Node["value"][0], MAKE_VALUE_HOT_REG_PAIR, 0);
			Trace* Output_Register = &Get_Free_Register(Output_Low_Code, Tracer, 0);

			Output_Register->Modified_Counter = 2;

			Output_Low_Code += "\t" + Output_Register->Name + " = " + Word[0] + ";\t\t# Copies high byte of " + Node["value"][0].Value + " into register\n";
		
			return Output_Register->Name;
		}
	}

	case S_DEREF8:
	{
		Tracer_Make_Value_Hot(Output_Low_Code, Tracer, Node["address"][0], MAKE_VALUE_HOT_HL_REG, 1);
		Trace* Output = &Get_Free_Register(Output_Low_Code, Tracer, Requirements);

		if (Requirements == MAKE_VALUE_HOT_REG_PAIR || Requirements == MAKE_VALUE_HOT_HL_REG)
		{
			//Output;

			Output[1].Modified_Counter = 2;
			Output[0].Modified_Counter = 2;

			Output_Low_Code += "\t" + Output[0].Name + " = $00;\t\t# Zeros upper byte\n";
			Output_Low_Code += "\t" + Output[1].Name + " = [HL];\t\t# Dereferences address\n";

			return Output[0].Name + Output[1].Name;
		}
		else
		{
			Output[0].Modified_Counter = 2;

			Output_Low_Code += "\t" + Output[0].Name + " = [HL];\t\t# Dereferences address\n";

			return Output->Name;
		}
	}

	case S_RIGHT_SHIFT16:
	case S_RIGHT_SHIFT8:
	case S_LEFT_SHIFT16:
	case S_LEFT_SHIFT8:
	{
		return Tracer_Shift(Output_Low_Code, Tracer, Node, Requirements, Copy_Requirements);
	}

	case S_XOR8:
	case S_AND8:
	case S_OR8:
	case S_MINUS8:
	case S_PLUS8:
	{
		return Tracer_Operator8(Output_Low_Code, Tracer, Node, Requirements, Copy_Requirements);
	}

	case S_PLUS16_8:
	{
		// This will put 'left' into the HL register and 'right' into another register pair. Important that 'right' high is $00


		// return "HL";	// Stored in the HL register

		std::string Right_Registers = Tracer_Make_Value_Hot(Output_Low_Code, Tracer, Node["right"][0], MAKE_VALUE_HOT_REG_PAIR, 0);
		Trace* Right_Trace = Register_From_Name(Tracer, Right_Registers[0]);

		std::string Name[2] = { Right_Trace[0].Value, Right_Trace[1].Value };

		Right_Trace[0].Value = "@value@";
		Right_Trace[1].Value = "@value@";

		Tracer_Make_Value_Hot(Output_Low_Code, Tracer, Node["left"][0], MAKE_VALUE_HOT_HL_REG, Copy_Requirements);

		Right_Trace = Find_Value_In_Tracer_Registers(Tracer, "@value@");

		Right_Trace[0].Value = Name[0];
		Right_Trace[1].Value = Name[1];

		Right_Registers = Right_Trace[0].Name + Right_Trace[1].Name;

		Output_Low_Code += "\tHL += " + Right_Registers + ";\t\t# S_PLUS16\n";

		Clear_Tracer_Registers(Tracer);

		return Fit_Register_Requirements(Output_Low_Code, Tracer, "HL", Requirements, Copy_Requirements);
	}


	case S_ID8:
	case S_ID16:
	{
		return Get_Back_Register(Output_Low_Code, Tracer, Node, Requirements, Copy_Requirements);
	}

	//

	case S_INT_LITERAL:
	{

		return Make_Int_Literal_Hot(Output_Low_Code, Tracer, Node.Value, Requirements, Copy_Requirements);
	}

	//

	case S_SIZEOF:
	{

		// This will find and get the size of the identifier (if possible)

		const Parse_Node* Found = Find_Value_In_Global_Stack(Tracer, Node["id"][0].Value);

		if (!Found)
			Found = Find_Value_In_Local_Consts(Tracer, Node["id"][0].Value);

		return Make_Int_Literal_Hot(Output_Low_Code, Tracer, Found->Child_Nodes.at("size")[0].Value, Requirements, Copy_Requirements);
	}

	}
}

void Node_Dest_Assign_Statement(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node)
{
	// Evaluate expression for 'value'

	// Evaluate expression for 'dest'

	std::string Register = Tracer_Make_Value_Hot(Output_Low_Code, Tracer, Node["value"][0]);// , MAKE_VALUE_HOT_A_REG);

	Trace* Value = Register_From_Name(Tracer, Register[0]);

	std::string Destination_Registers;

	if (Value)
	{
		size_t Previous_Modified_Counter = Value->Modified_Counter;

		std::string Name = Value->Value;

		//if (Value->Modified_Counter != 1)
		Value->Value = "@statement_value@";
		Value->Modified_Counter = 3;

		if (Register.size() > 1)
		{
			Value[1].Value = "@statement_value@";
			Value[1].Modified_Counter = 3;
		}

		Clear_Tracer_Registers(Tracer);	// Any other hot registers we temporarily used to evaluate 'value' can be cleared (we don't need em)

		Destination_Registers = Tracer_Make_Value_Hot(Output_Low_Code, Tracer, Node["destination"][0], MAKE_VALUE_HOT_HL_REG);

		Value = Find_Value_In_Tracer_Registers(Tracer, "@statement_value@");

		Value->Modified_Counter = Previous_Modified_Counter;

		Value->Value = Name;

		if (Register.size() > 1)
		{
			Value[1].Value = Name;
			Value[1].Modified_Counter = Previous_Modified_Counter;

			Register = Value[1].Name;
		}
		else
			Register = Value->Name;
	}
	else
		Destination_Registers = Tracer_Make_Value_Hot(Output_Low_Code, Tracer, Node["destination"][0], MAKE_VALUE_HOT_HL_REG);

	Output_Low_Code += "\t[" + Destination_Registers + "] = " + Register + "; \t\t# Stores value into memory location\n";
}

void Store_High_Low_Statement(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node, size_t Is_Low)
{
	// This will make the ID 'hot' and store the byte 'value' into the higher register of the id16

	std::string Value_Name = Tracer_Make_Value_Hot(Output_Low_Code, Tracer, Node["value"][0]);

	Trace* Value = Register_From_Name(Tracer, Value_Name[0]);

	if (Value)
	{
		// We're dealing with an actual byte? Set the modified_counter to 3

		size_t Previous_Modified_Counter = Value->Modified_Counter;

		Value_Name = Value->Value;			// Temporarily store the name of the value stored in this register

		if (Previous_Modified_Counter != 1)
		{
			Value->Value = "@statement_value@";
		}

		Value->Modified_Counter = 3;

		// Now? Grab the id16

		std::string ID_16 = Tracer_Make_Value_Hot(Output_Low_Code, Tracer, Node["id"][0], MAKE_VALUE_HOT_REG_PAIR, PULL_IDENTIFIER_REF);

		Value = Find_Value_In_Tracer_Registers(Tracer, "@statement_value@");

		if (Is_Low)
			ID_16[0] = ID_16[1];

		ID_16.resize(1);

		// We want the actual reference of the id16

		const char* Text[] = {
			";\t\t# Moves value into high-byte of ",
			";\t\t# Moves value into low-byte of "
		};

		Output_Low_Code += "\t" + ID_16 + " = " + Value->Name + Text[Is_Low] + Node["id"][0].Value + "\n";

		Value->Modified_Counter = Previous_Modified_Counter;
		Value->Value = Value_Name;
	}
	else
	{
		// Otherwise? we're dealing with some kind of int-literal or other const value...

		// I won't worry about this for now
	}
}

void ID_Inc_Dec_Statement(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node)
{
	// This will get the value into a register and then just run an inc/dec operation on it
	// Easy!!

	std::string Register = Tracer_Make_Value_Hot(Output_Low_Code, Tracer, Node["id"][0], 0, 0);	// We wanna increment the value itself, not a copy

	const std::map<size_t, std::string> Operators =
	{
		{ S_PLUS8,		"++;\t\t# Increments "		},
		{ S_MINUS8,		"--;\t\t# Decrements "		}
	};

	Output_Low_Code += "\t" + Register + Operators.at(Node["operator"][0].Syntax_ID) + Node["id"][0].Value + "\n";
}

void Dest_Inc_Dec_Statement(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node)
{
	Tracer_Make_Value_Hot(Output_Low_Code, Tracer, Node["destination"][0], MAKE_VALUE_HOT_HL_REG, 0);

	const std::map<size_t, std::string> Operators =
	{
		{ S_PLUS8,		"++;\t\t# Increments "		},
		{ S_MINUS8,		"--;\t\t# Decrements "		}
	};

	Output_Low_Code += "\t[HL]" + Operators.at(Node["operator"][0].Syntax_ID) + "address\n";
}

void Node_ID_Assign_Statement(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node)
{
	// Write some kind of code to evaluate expressions and store them in registers etc

	// just set a register to this value but make it HOT so that the tracer knows to store it later

	// Stores expression into register
	// sets corresponding register to the corresponding ID

	if (Node["id"][0].Syntax_ID == S_ID8)
	{
		std::string Register = Tracer_Make_Value_Hot(Output_Low_Code, Tracer, Node["value"][0], MAKE_VALUE_HOT_REG, PULL_IDENTIFIER_COPY);

		Trace* Current_ID = Find_Value_In_Tracer_Registers(Tracer, Node["id"][0].Value);

		if (Current_ID)
		{
			Current_ID->Modified_Counter = 0;	// This ID is no longer 'hot', we've assigned this a new ID
		}

		Trace* Value = Register_From_Name(Tracer, Register.back());
		Value->Value = Node["id"][0].Value;
		Value->Modified_Counter = 1;
	}
	else
	{
		std::string Registers = Tracer_Make_Value_Hot(Output_Low_Code, Tracer, Node["value"][0], MAKE_VALUE_HOT_REG_PAIR, PULL_IDENTIFIER_COPY);

		Clear_Tracer_Registers(Tracer);

		Trace* Current_ID = Find_Value_In_Tracer_Registers(Tracer, Node["id"][0].Value);

		if (Current_ID)
		{
			Current_ID[0].Modified_Counter = 2;
			Current_ID[1].Modified_Counter = 2;
		}

		Trace* Value = Register_From_Name(Tracer, Registers[0]);
		Value[0].Value = Node["id"][0].Value;
		Value[1].Value = Node["id"][0].Value;
		Value[0].Modified_Counter = 1;
		Value[1].Modified_Counter = 1;
	}
}

void Node_Return_Statement(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node)
{
	// Roll the stack back to before we created the local variables

	if (Node.Child_Nodes.count("value"))
	{
		// If there's a value, we need to evaluate the expression and store it in either the A or HL registers

		Tracer_Make_Value_Hot(Output_Low_Code, Tracer, Node["value"][0], Tracer.Current_Scope_Function_Return_Type == S_BYTE ? MAKE_VALUE_HOT_A_REG : MAKE_VALUE_HOT_HL_REG);
	}

	// When there's no value, it's simpler! Just roll the stack back

	if (Tracer.Stack.size() > Tracer.Parameters_Count) // if we even initialised any local variables (other than the parameters)
	{
		Output_Low_Code += "\tSP += " + std::to_string(Tracer.Stack.size() - Tracer.Parameters_Count) + ";\t\t# Remove local variables from stack\n";
	}

	Output_Low_Code += "\treturn;\n";
}

void Node_Parameter_Dec(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node)
{
	switch (Node["type"][0].Syntax_ID)	// Very similar to stack dec except SP has already been incremented and assigned for us
	{
	case S_BYTE:
		Tracer.Stack.push_back(Trace("&" + Node["id"][0].Value, Node["id"][0].Value, 0));
		break;

		//

	case S_WORD:
	case S_BYTE_POINTER:
		Tracer.Stack.push_back(Trace("&" + Node["id"][0].Value, Node["id"][0].Value, 2));	// high
		Tracer.Stack.push_back(Trace("&" + Node["id"][0].Value, Node["id"][0].Value, 1));	// low
		break;

	default:
		printf("Error parameter type!\n");
		return;
	}

	Tracer.Parameters_Count = 0;
}

void Node_Parameters_Dec(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node)
{
	Node_Parameter_Dec(Output_Low_Code, Tracer, Node);

	//Tracer.Parameters_Count += 2 - (Node["type"][0].Syntax_ID == S_BYTE);

	if (Node.Child_Nodes.count("parameters"))
		Node_Parameters_Dec(Output_Low_Code, Tracer, Node["parameters"][0]);
}

void Call_Function_Handle_Parameter(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node, const Parse_Node& Parameter_Node)
{
	// is this a byte or a word we're pushing?

	if (Parameter_Node["type"][0].Syntax_ID == S_BYTE)
	{
		// byte

		Tracer.Stack.push_back(Trace("", "@parameter@", 0)); // Not a pointer of any kind

		std::string Register = Tracer_Make_Value_Hot(Output_Low_Code, Tracer, Node, MAKE_VALUE_HOT_REG); // For simplicity, store it in a register

		Trace* Value_Register = Register_From_Name(Tracer, Register[0]);

		std::string Name;// = Value_Register->Value;

		if (Value_Register->Modified_Counter != 1)
		{
			Name = "@parameter@";
		}
		else
			Name = Value_Register->Value;

		Value_Register->Modified_Counter = 3;
		Value_Register->Value = "@parameter@";

		Register_From_Name(Tracer, Register[0])->Modified_Counter = 3;
		Register_From_Name(Tracer, Register[0])->Value = "@parameter@";

		Get_Free_Register(Output_Low_Code, Tracer, MAKE_VALUE_HOT_HL_REG); // Frees up HL register pair

		Value_Register = Find_Value_In_Tracer_Registers(Tracer, "@parameter@");

		Output_Low_Code += "\tHL = SP + 255;\t\t# Makes room on stack for 1 byte\n";
		Output_Low_Code += "\t[HL] = " + Value_Register->Name + ";\t\t# Writes " + Parameter_Node["id"][0].Value + " to stack\n";
		Output_Low_Code += "\tSP = HL;\t\t# Update stack pointer\n";

		Value_Register->Modified_Counter = 2;

		/*if (Name == "@parameter@")
			Value_Register->Modified_Counter = 2;	// Not important anymore
		else
		{
			Value_Register->Value = Name;
			Value_Register->Modified_Counter = 1;
		}*/
	}
	else
	{
		// word

		std::string Register_Pair = Tracer_Make_Value_Hot(Output_Low_Code, Tracer, Node, MAKE_VALUE_HOT_REG_PAIR);

		Output_Low_Code += "\tpush " + Register_Pair + ";\t\t# Pushes " + Parameter_Node["id"][0].Value + " to the stack\n";

		Tracer.Stack.push_back(Trace("", "@parameter@", 2));
		Tracer.Stack.push_back(Trace("", "@parameter@", 1));
	}

	Clear_Tracer_Registers(Tracer);	// Afterwards, I'm pretty sure we're safe to do this

	// Writeback_Panic_Push(Output_Low_Code, Tracer);

	if (Parameter_Node.Child_Nodes.count("parameters"))
		Call_Function_Handle_Parameter(Output_Low_Code, Tracer, Node["parameters"][0], Parameter_Node["parameters"][0]);
}

void Call_Function_Statement(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node)
{
	// This will push all of the parameters to the stack

	// Then, it'll call the function

	// After that, it'll rewrite the tracer registers accordingly

	// Then, it'll deallocate the parameters that we just pushed to the stack

	std::vector<Parse_Node>& Function_Dec = Get_Function_Call(Node["id"][0].Value);

	size_t Pre_Function_Stack_Size = Tracer.Stack.size();

	for (size_t Index = 0; Index < 7; Index++)
		if (Tracer.Registers[Index].Modified_Counter == 1)
		{
			Store_Tracer_Register_Back_In_Memory(Output_Low_Code, Tracer, &Tracer.Registers[Index]);

			Writeback_Panic_Push(Output_Low_Code, Tracer);
		}

	// for each parameter...

	if (Function_Dec.size() == 3) // if there are parameters in this function node? Handle em
	{
		Call_Function_Handle_Parameter(Output_Low_Code, Tracer, Node["parameters"][0], Function_Dec[2]);
	}

	Output_Low_Code += "\tcall " + Node["id"][0].Value + ";\t\t# Calls function\n";

	if (Pre_Function_Stack_Size != Tracer.Stack.size())
	{
		Output_Low_Code += "\tSP += " + std::to_string(Tracer.Stack.size() - Pre_Function_Stack_Size) + ";\t\t# Removes pushed parameters from stack\n";

		Tracer.Stack.resize(Pre_Function_Stack_Size);
	}
}

void Node_Function_Definition(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node)
{
	// This will write the subroutine to the low-code
	// It'll also add the function declaration to the tracer code
	// once it's finished its statement analysis, clear stack again

	// Note that ALL return statements need to explicitly return the stack to its pre-call state

	// First, add parameters to code tracer stack

	//Tracer.Parameters_Count = 0;

	if (Node.Child_Nodes.count("parameters"))									// The function might not even have parameters!
		Node_Parameters_Dec(Output_Low_Code, Tracer, Node["parameters"][0]);

	Tracer.Stack.push_back(Trace("", "@function_return_pointer@", 2));
	Tracer.Stack.push_back(Trace("", "@function_return_pointer@", 1));

	Tracer.Parameters_Count = Tracer.Stack.size();

	Tracer.Current_Scope_Function_Return_Type = Node["return_type"][0].Syntax_ID;

	Local_Function_Scope_Name = "__" + Node["id"][0].Value + "__";

	Tracer.Global_Const_Declarations.push_back(&Node);

	Output_Low_Code += "subroutine " + Node["id"][0].Value + "\n{\n";

	for (size_t W = 0; W < Tracer.Registers.size(); W++)
		Tracer.Registers[W].Modified_Counter = 0;			// We enter the function with a blank slate

	Analyse_Statements_LowC(Output_Low_Code, Tracer, Node["statements"][0]);

	Tracer.Stack.clear();

	Output_Low_Code += "}\n\n";

	Local_Function_Scope_Name = "";

	// Now we can just add all of the local const declarations
}

void Node_ROM_V_Declaration(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node)
{
	// This should add a 'data' piece of code to the end of the file

	if (Local_Function_Scope_Name.empty())
	{
		Tracer.Global_Const_Declarations.push_back(&Node);
	}
	else
	{
		Tracer.Local_Const_Declarations.push_back(&Node);
	}
}

void Node_Stack_Declaration(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node)
{
	// This will add a value onto the stack

	// determine how large the stack value should be
	long Size = Get_Value_From_String(Node["size"][0].Value.c_str());

	Output_Low_Code += "\tSP += " + std::to_string(256 - Size) + ";\t\t# " + Node["type"][0].Value + " " + Node["id"][0].Value + ";\n";

	// Add 'size' bytes to the stack tracer
	// name them according to the type

	// add corresponding 'Low' code

	/*

	byte Variable;
	(trace):	(name)	-	-	(value)	-	(P. flag)
	SP + $00	&Variable		Variable	0


	byte* Variable:
	SP + $01	&Variable		Variable	2
	SP + $00	&Variable		Variable	1


	byte Variable[3];							// When dereferencing a value, look for its pointer in the 'name' table... If you can't find it there, look for the high/low bytes in the 'value' table etc
	SP + $02	Variable + 2	Variable[2]	0
	SP + $01	Variable + 1	Variable[1]	0
	SP + $00	Variable		Variable[0]	0

	*/

	switch (Node["type"][0].Syntax_ID)
	{
	case S_BYTE:
		Tracer.Stack.push_back(Trace("&" + Node["id"][0].Value, Node["id"][0].Value, 0));
		break;

		//

	case S_WORD:
	case S_BYTE_POINTER:
		Tracer.Stack.push_back(Trace("&" + Node["id"][0].Value, Node["id"][0].Value, 2));	// high
		Tracer.Stack.push_back(Trace("&" + Node["id"][0].Value, Node["id"][0].Value, 1));	// low
		break;

		//

	case S_BYTE_ARRAY:	// Assumes that 'size' is ALWAYS a valid size
		while (Size-- > 1)
		{
			Tracer.Stack.push_back(Trace(Node["id"][0].Value + "+" + std::to_string(Size), Node["id"][0].Value + "[" + std::to_string(Size) + "]", 0));
		}

		Tracer.Stack.push_back(Trace(Node["id"][0].Value, Node["id"][0].Value + "[0]", 0));

		break;
	}

	// Generates corresponding 'Low' code
}

void Restore_Register_Snapshot(std::string& Output_Low_Code, Tracer_Data& Tracer, std::vector<Trace>& ID_Snapshot)
{
	// This will make registers hot and swap them around as necessary until the previous register snapshot is restored.

	for (size_t Index = 0; Index < ID_Snapshot.size(); Index++)
	{
		Parse_Node Node;
		Node.Value = ID_Snapshot[Index].Value;
		Node.Syntax_ID = ID_Snapshot[Index].Pointer_Flag ? S_ID16 : S_ID8;

		Trace* Desired_Reg = Register_From_Name(Tracer, ID_Snapshot[Index].Name[0]);

		std::string Identifier = Get_Back_Register(Output_Low_Code, Tracer, Node, 0, 0); // This makes the value 'hot'

		Trace* ID_Reg = Register_From_Name(Tracer, Identifier[0]);

		for (size_t Reg = 0; Reg < 1 + ID_Snapshot[Index].Pointer_Flag/2; Reg++)
		{

			if (Desired_Reg[Reg].Value != ID_Snapshot[Index].Value || Desired_Reg[Reg].Modified_Counter != 1) // if not the 'hot' register
			{
				if (Desired_Reg[Reg].Modified_Counter)
				{
					// some kind of swap is required...

					Trace* Temp = &Get_Free_Register(Output_Low_Code, Tracer, 0);	// Gets a free register to temporarily use

					Output_Low_Code += "\t" + Temp->Name + " = " + Desired_Reg[Reg].Name + ";\t\t# Temporarily stores value here\n";
					Output_Low_Code += "\t" + Desired_Reg[Reg].Name + " = " + ID_Reg[Reg].Name + ";\t\t# Moves value into expected register\n";
					Output_Low_Code += "\t" + ID_Reg[Reg].Name + " = " + Temp->Name + ";\t\t# Swaps temporarily value\n";

					std::swap(ID_Reg[Reg].Value, Desired_Reg[Reg].Value); // This just swaps their values around

					Temp->Modified_Counter = 0;							// frees up 'temp'
				}
				else
				{
					// just get register and store it here!

					Desired_Reg[Reg].Value = ID_Reg[Reg].Value;
					Desired_Reg[Reg].Modified_Counter = 1;

					ID_Reg[Reg].Modified_Counter = 0;

					Output_Low_Code += "\t" + Desired_Reg[Reg].Name + " = " + ID_Reg[Reg].Name + ";\t\t# Moves value into expected register\n";
				}
			}
		}

		Writeback_Panic_Push(Output_Low_Code, Tracer);
	}


}

void Get_Register_Snapshot(Tracer_Data& Tracer, std::vector<Trace>& ID_Snapshot)
{
	for (size_t Index = 0; Index < 7; Index++)
	{
		if (Tracer.Registers[Index].Modified_Counter == 1)	// if this register is 'hot'
		{
			Trace* Stack_Value = &Tracer.Stack[Tracer.Stack.size() - 1 - Find_Value_In_Tracer_Stack(Tracer, Tracer.Registers[Index].Value)];

			Trace Snapshot = Tracer.Registers[Index];

			Snapshot.Pointer_Flag = 2 * Stack_Value->Pointer_Flag;

			if (Stack_Value->Pointer_Flag)
				Index++;

			ID_Snapshot.push_back(Snapshot);
		}
	}
}

size_t Evaluate_Condition(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node)	// Returns CON_ALWAYS, CON_ZERO, CON_CARRY, CON_NOT_ZERO, CON_NOT_CARRY etc etc
{

	switch (Node.Syntax_ID)
	{
		case S_NOT:
		{
			size_t Condition;

			Condition = Evaluate_Condition(Output_Low_Code, Tracer, Node["value"][0]);

			return CON_NOT(Condition);	// This just flips the condition
		}

		case S_BIT:
		{
			std::string Value = Tracer_Make_Value_Hot(Output_Low_Code, Tracer, Node["value"][0], MAKE_VALUE_HOT_REG);

			Output_Low_Code += "\t" + Value + "." + Node["bit"][0].Value + ";\t\t# Gets conditional\n";

			return CON_NOT_ZERO;	// expression is true when NOT_ZERO
		}

		//

		case S_LESS_THAN8:
		{
			Tracer_Operator8(Output_Low_Code, Tracer, Node, 0, 0); // This doesn't change any values so no copies are required

			return CON_CARRY;		// expression is LESS THAN when a < b results in a 'carry' flag
		}

		//

		case S_NOT_ZERO:
		{
			std::string Value = Tracer_Make_Value_Hot(Output_Low_Code, Tracer, Node["value"][0], MAKE_VALUE_HOT_A_REG, 1); // This needs to be a copy because otherwise it might try to truncate a 16-bit value to an 8-bit register

			Output_Low_Code += "\tA |= A;\t\t# This just gets the CPU flags\n";

			return CON_NOT_ZERO;
		}
	}
}

void Do_While_Statement(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node)
{
	// very similar to 'if' statement but backwards in a way

	Output_Low_Code += "\n";

	size_t Label = Tracer.Label_Count;

	std::string Label_Name = "__do_while_statement__" + std::to_string(Label);

	Tracer.Label_Count++;

	Output_Low_Code += "\tlabel " + Label_Name + ";\t\t# do/while loop label\n";

	std::vector<Trace> ID_Snapshot;

	Get_Register_Snapshot(Tracer, ID_Snapshot); // This records the current 'hot' registers and what identifiers are stored in them (if any)

	// writes statements code

	Analyse_Statements_LowC(Output_Low_Code, Tracer, Node["local_statements"][0]);

	// at the end of the if statement? return the registers to the snapshot state

	// get label
	std::string Condition = Conditionals[Evaluate_Condition(Output_Low_Code, Tracer, Node["condition"][0])];

	Clear_Tracer_Registers(Tracer);

	// we want to jump back to top if condition IS met

	Restore_Register_Snapshot(Output_Low_Code, Tracer, ID_Snapshot);	// After the local statement code is generated, set registers back to expected values

	Writeback_Panic_Push(Output_Low_Code, Tracer);

	Output_Low_Code += "\tjump" + Condition + " " + Label_Name + ";\t\t# do/while loop jump\n\n";
}

void If_Statement(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node)
{
	// This will look for values that are in a certain place

	// evaluate condition?

	Output_Low_Code += "\n";

	std::string Condition = Conditionals[CON_NOT(Evaluate_Condition(Output_Low_Code, Tracer, Node["condition"][0]))];

	Clear_Tracer_Registers(Tracer);
	// we only want to jump over the statement if the condition ISN'T met

	size_t Label = Tracer.Label_Count;

	std::string Label_Name = "__if_statement__" + std::to_string(Label);

	Output_Low_Code += "\tjump" + Condition + " " + Label_Name + ";\t\t# If statement jump\n";

	Tracer.Label_Count++;

	std::vector<Trace> ID_Snapshot;
	
	Get_Register_Snapshot(Tracer, ID_Snapshot); // This records the current 'hot' registers and what identifiers are stored in them (if any)

	// writes statements code

	Analyse_Statements_LowC(Output_Low_Code, Tracer, Node["local_statements"][0]);

	Restore_Register_Snapshot(Output_Low_Code, Tracer, ID_Snapshot);	// After the local statement code is generated, set registers back to expected values

	Writeback_Panic_Push(Output_Low_Code, Tracer);

	// at the end of the if statement? return the registers to the snapshot state

	// get label

	Output_Low_Code += "\tlabel " + Label_Name + ";\t\t# If statement label\n\n";
}

//

void Analyse_Statement_LowC(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node)
{
	if (Node.Syntax_ID == S_IF_STATEMENT)
	{
		If_Statement(Output_Low_Code, Tracer, Node);
		return;
	}

	if (Node.Syntax_ID == S_ID_INC_DEC)
	{
		ID_Inc_Dec_Statement(Output_Low_Code, Tracer, Node);
		return;
	}

	if (Node.Syntax_ID == S_DEST_INC_DEC)
	{
		Dest_Inc_Dec_Statement(Output_Low_Code, Tracer, Node);
		return;
	}

	if (Node.Syntax_ID == S_DO_WHILE_LOOP)
	{
		Do_While_Statement(Output_Low_Code, Tracer, Node);
		return;
	}

	if (Node.Syntax_ID == S_STACK_DECLARATION_STATEMENT)
	{
		Node_Stack_Declaration(Output_Low_Code, Tracer, Node);
		return;
	}

	if (Node.Syntax_ID == S_ROM_DECLARATION_STATEMENT)
	{
		Node_ROM_V_Declaration(Output_Low_Code, Tracer, Node);
		return;
	}

	if (Node.Syntax_ID == S_STORE_LOW)
	{
		Store_High_Low_Statement(Output_Low_Code, Tracer, Node, 1);
		return;
	}

	if (Node.Syntax_ID == S_STORE_HIGH)
	{
		Store_High_Low_Statement(Output_Low_Code, Tracer, Node, 0);
		return;
	}

	if (Node.Syntax_ID == S_ID_ASSIGN)
	{
		Node_ID_Assign_Statement(Output_Low_Code, Tracer, Node);
		return;
	}

	if (Node.Syntax_ID == S_DEST_ASSIGN)
	{
		Node_Dest_Assign_Statement(Output_Low_Code, Tracer, Node);
		return;
	}

	if (Node.Syntax_ID == S_FUNCTION_CALL)
	{
		Call_Function_Statement(Output_Low_Code, Tracer, Node);
		return;
	}

	if (Node.Syntax_ID == S_RETURN)
	{
		Node_Return_Statement(Output_Low_Code, Tracer, Node);
		return;
	}
}

void Analyse_Statements_LowC(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node)
{
	Analyse_Statement_LowC(Output_Low_Code, Tracer, Node);

	Clear_Tracer_Registers(Tracer);

	while (Writeback_Panic_Push(Output_Low_Code, Tracer));

	if (Node.Child_Nodes.count("statements"))
		Analyse_Statements_LowC(Output_Low_Code, Tracer, Node["statements"][0]);
}

void Analyse_Global_Declaration_LowC(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node)
{
	if (Node.Syntax_ID == S_FUNCTION_DEFINE)
	{
		Node_Function_Definition(Output_Low_Code, Tracer, Node);
		return;
	}

	if (Node.Syntax_ID == S_ROM_DECLARATION_STATEMENT)
	{
		Node_ROM_V_Declaration(Output_Low_Code, Tracer, Node);
		return;
	}
}

void Analyse_Global_Declarations_LowC(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node)
{
	Analyse_Global_Declaration_LowC(Output_Low_Code, Tracer, Node);

	if (Node.Child_Nodes.count("global_declarations"))
		Analyse_Global_Declarations_LowC(Output_Low_Code, Tracer, Node["global_declarations"][0]);
}

void Write_Const_Data_Definition(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node)
{
	if (!Node.Child_Nodes.count("data"))		// If we don't actually have any data to write, end early
		return;

	Output_Low_Code += "data " + Node["id"][0].Value + "\n{\n";

	// Write either string or int data accordingly

	if (Node["data"][0].Value[0] == '\"')
	{
		// This is a string that we want to null-terminate

		Output_Low_Code += "\t" + Node["data"][0].Value + " 0\n";
	}
	else
	{
		const Parse_Node* Current_Node = &Node["data"][0];

		Output_Low_Code += "\t" + Current_Node->Value + "\n";

		//while (Current_Node->Child_Nodes.count("int_literals"))
		//{
		//	Current_Node = &(*Current_Node)["int_literals"][0];
		//	Output_Low_Code += "\t" + Current_Node->Value + "\n";
		//}
	}

	Output_Low_Code += "}\n\n";

	// terminate 'data'
}

void Analyse_Parsed_LowC(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node)
{
	Local_Function_Scope_Name = "";
	Analyse_Global_Declarations_LowC(Output_Low_Code, Tracer, Node["global_declarations"][0]);
	//Analyse_Statements_LowC(Output_Low_Code, Tracer, Node["statements"][0]);

	// Add all of the data here!!!

	for (size_t Index = 0; Index < Tracer.Global_Const_Declarations.size(); Index++)
	{
		Write_Const_Data_Definition(Output_Low_Code, Tracer, *Tracer.Global_Const_Declarations[Index]);
	}

	for (size_t Index = 0; Index < Tracer.Local_Const_Declarations.size(); Index++)
	{
		Write_Const_Data_Definition(Output_Low_Code, Tracer, *Tracer.Local_Const_Declarations[Index]);
	}
}
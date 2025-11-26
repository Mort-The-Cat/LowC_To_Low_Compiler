#include "Code_Trace_Handler.h"
#include "Code_Parser.h"
#include "LowC_Tokeniser.h"

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

			Temp_Reg[0].Value = Tracer.Registers[5].Value;
			Temp_Reg[1].Value = Tracer.Registers[6].Value;

			Temp_Reg[0].Modified_Counter = Tracer.Registers[5].Modified_Counter;
			Temp_Reg[1].Modified_Counter = Tracer.Registers[6].Modified_Counter;

			Temp_Reg[0].Pointer_Flag = Tracer.Registers[5].Pointer_Flag;
			Temp_Reg[1].Pointer_Flag = Tracer.Registers[6].Pointer_Flag;

			Output_Low_Code +=
				"\t" + Temp_Reg[0].Name + " = H;\t\t# Frees up H register for use\n" +
				"\t" + Temp_Reg[1].Name + " = L;\t\t# Frees up L register for use\n";
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

			for(size_t Index = 0; Index < 7; Index++)
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

			for (long Index = 5; Index >= 0; Index -= 2)
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
		if (Tracer.Stack[Index].Value == Value)
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

		Output_Low_Code += "\tHL += " + std::to_string(Stack_Position) + ";\t\t# Memory location of " + Value + "\n";

		Output_Low_Code += "\t[HL] = " + New_Value_Address[1].Name + ";\t\t# Stores " + Value + "back into memory\n";
		Output_Low_Code += "\tHL++\n";
		Output_Low_Code += "\t[HL] = " + New_Value_Address[0].Name + ";\t\t# Stores upper byte of " + Value + "back into memory\n";

		New_Value_Address[0].Modified_Counter = 0;
		New_Value_Address[1].Modified_Counter = 0;
	}
	else
	{
		Get_Free_Register(Output_Low_Code, Tracer, MAKE_VALUE_HOT_HL_REG);

		Trace* New_Value_Address = Find_Value_In_Tracer_Registers(Tracer, Value);	// Me MIGHT need to overwrite this when freeing HL

		Output_Low_Code += "\tHL += " + std::to_string(Stack_Position) + ";\t\t# Memory location of " + Value + "\n";
		Output_Low_Code += "\t[HL] = " + New_Value_Address->Name + ";\t\t# Stores " + Value + "back into memory\n";

		New_Value_Address[0].Modified_Counter = 0;
	}
}

bool Find_Value_In_Global_Stack(Tracer_Data& Tracer, std::string Value)
{
	for (size_t Index = 0; Index < Tracer.Global_Const_Declarations.size(); Index++)
	{
		if (Tracer.Global_Const_Declarations[Index]->Value == Value)
			return true;
	}

	return false;
}

std::string Tracer_Make_Value_Hot(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node, size_t Requirements = 0, const char* Suggested_Name = nullptr)
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
	case S_DEREF8:
	{
		Tracer_Make_Value_Hot(Output_Low_Code, Tracer, Node["address"][0], MAKE_VALUE_HOT_HL_REG);
		Trace& Output = Get_Free_Register(Output_Low_Code, Tracer, Requirements);

		Output.Modified_Counter = 2;
		
		Output_Low_Code += "\t" + Output.Name + " = [HL];\t\t# Dereferences address\n";

		return Output.Name;
	}

	case S_PLUS8:
	{
		// This will put 'left' into the A register and 'right' into a register of our choice
		// Then, it'll tell the tracer that the result is stored in 'A'

		Tracer_Make_Value_Hot(Output_Low_Code, Tracer, Node["left"][0], MAKE_VALUE_HOT_A_REG);
		std::string Right_Operand_Val = Tracer_Make_Value_Hot(Output_Low_Code, Tracer, Node["right"][0]);

		Output_Low_Code += "\tA += " + Right_Operand_Val + ";\t\t# S_PLUS8\n";

		// output is in A register! Store it in the register we want

		return "A";		// Purely stored in the A register
	}

	case S_PLUS16_8:
	{
		// This will put 'left' into the HL register and 'right' into another register pair. Important that 'right' high is $00

		Tracer_Make_Value_Hot(Output_Low_Code, Tracer, Node["left"][0], MAKE_VALUE_HOT_HL_REG);
		std::string Right_Operand_Val = Tracer_Make_Value_Hot(Output_Low_Code, Tracer, Node["right"][0], MAKE_VALUE_HOT_REG_PAIR);	// must be a reg-pair

		Output_Low_Code += "\tHL += " + Right_Operand_Val + ";\t\t# S_PLUS16_8\n";

		return "HL";	// Stored in the HL register
	}


	case S_ID8:
	case S_ID16:
	{
		Trace* Found_Value = Find_Value_In_Tracer_Registers(Tracer, Node.Value);
		if (Found_Value)
		{
			// if it's in the register we want??

			if (Requirements == MAKE_VALUE_HOT_A_REG && Found_Value->Name != Tracer.Registers[0].Name)
			{
				Get_Free_Register(Output_Low_Code, Tracer, MAKE_VALUE_HOT_A_REG);	// Frees up A register

				Output_Low_Code += "\tA = " + Found_Value->Name + ";\t\t# Places " + Node.Value + " in 'A' register\n";
				Tracer.Registers[0].Value = Found_Value->Value;
				Tracer.Registers[0].Modified_Counter = Found_Value->Modified_Counter;
				Tracer.Registers[0].Pointer_Flag = Found_Value->Pointer_Flag;

				// Places value in 'A' register once it's free

				Found_Value->Modified_Counter = 0;
				//Found_Value->Value = "???";

				return "A";
			}

			if (Requirements == MAKE_VALUE_HOT_HL_REG && Found_Value->Name != Tracer.Registers[5].Name)
			{
				Get_Free_Register(Output_Low_Code, Tracer, MAKE_VALUE_HOT_HL_REG);	// Frees up HL register

				Output_Low_Code +=
					"\tH = " + Found_Value[0].Name +";\t\t# Places " + Node.Value + " in 'H' register\n" + 
					"\tL = " + Found_Value[1].Name + ";\t\t# Places " + Node.Value + " in 'L' register\n";

				return "HL";
			}

			if (Requirements == MAKE_VALUE_HOT_REG_PAIR && Node.Syntax_ID == S_ID8)
			{
				// We need to turn this into a register pair

				//Found_Value->Modified_Counter = 2
				Trace* Output_Register = &Get_Free_Register(Output_Low_Code, Tracer, MAKE_VALUE_HOT_REG_PAIR);
				Output_Register[0].Modified_Counter = 2;
				Output_Register[1].Modified_Counter = 1;	// Lower value is STILL holding the hot id, save it

				Output_Register[1].Value = Node.Value;

				Found_Value->Modified_Counter = 0;
				
				Output_Low_Code +=
					"\t" + Output_Register[0].Name + " = $00;\n\t" + Output_Register[1].Name + " = " + Found_Value->Name + ";\t\t# Places value into register pair\n";

				Found_Value = Output_Register;
			}

			if (Requirements == MAKE_VALUE_HOT_REG_PAIR || Requirements == MAKE_VALUE_HOT_HL_REG)
			{
				return Found_Value[0].Name + Found_Value[1].Name;	// Name of register pair that we want
			}

			return Found_Value->Name;
		}

		// Next, look for it on the stack

		long Stack_Index = Find_Value_In_Tracer_Stack(Tracer, Node.Value);

		if (Stack_Index != -1)
		{
			// Found on stack!

			Get_Free_Register(Output_Low_Code, Tracer, MAKE_VALUE_HOT_HL_REG);

			Tracer.Registers[5].Modified_Counter = 2;
			Tracer.Registers[6].Modified_Counter = 2;

			Trace* Output_Register;

			if (Requirements == MAKE_VALUE_HOT_HL_REG)
				Output_Register = &Get_Free_Register(Output_Low_Code, Tracer, MAKE_VALUE_HOT_REG_PAIR);
			else
				Output_Register = &Get_Free_Register(Output_Low_Code, Tracer, Requirements);

			Stack_Index = Find_Value_In_Tracer_Stack(Tracer, Node.Value);

			Output_Register->Value = Tracer.Stack[Tracer.Stack.size() - 1 - Stack_Index].Value;
			Output_Register->Modified_Counter = 2;
			Output_Register->Pointer_Flag = 2;// Tracer.Stack[Tracer.Stack.size() - 1 - Stack_Index].Pointer_Flag;

			if (Requirements == MAKE_VALUE_HOT_REG_PAIR || Requirements == MAKE_VALUE_HOT_HL_REG)
			{
				// we want to increment to next thing

				Output_Low_Code +=
					"\tHL = SP + " + std::to_string(Stack_Index) + ";\n" +
					"\t" + Output_Register[1].Name + " = [HL];\t\t# Places lower byte of " + Node.Value + " into register\n";

				Output_Register[1].Value = Tracer.Stack[Tracer.Stack.size() - 2 - Stack_Index].Value;
				Output_Register[1].Modified_Counter = 2;
				Output_Register[1].Pointer_Flag = 1;// Tracer.Stack[Tracer.Stack.size() - 2 - Stack_Index].Pointer_Flag;

				Output_Low_Code +=
					"\tHL++;\n\t" +
					Output_Register[0].Name + " = [HL];\t\t# Places " + Node.Value + " into register\n";

				//

				if (Requirements == MAKE_VALUE_HOT_HL_REG)
				{
					// move values back around accordingly

					Tracer.Registers[5].Value = Output_Register->Value;
					Tracer.Registers[6].Value = Output_Register[1].Value;

					Tracer.Registers[5].Modified_Counter = 2;
					Tracer.Registers[6].Modified_Counter = 2;

					Output_Register[0].Modified_Counter = 0;
					Output_Register[1].Modified_Counter = 0;

					Output_Low_Code +=
						"\tH = " + Output_Register[0].Name + ";\n\tL = " + Output_Register[1].Name + ";\t\t# Places register pair into HL\n";

					return "HL";
				}

				return Output_Register[0].Name + Output_Register[1].Name;
			}

			Output_Low_Code +=
				"\tHL = SP + " + std::to_string(Stack_Index) + ";\n" +
				"\t" + Output_Register[0].Name + " = [HL];\t\t# Places " + Node.Value + " into register\n";

			return Output_Register[0].Name;
		}

		Stack_Index = Find_Value_Address_In_Tracer_Stack(Tracer, Node.Value);

		if (Stack_Index != -1)
		{
			Get_Free_Register(Output_Low_Code, Tracer, MAKE_VALUE_HOT_HL_REG);

			Tracer.Registers[5].Modified_Counter = 2;	// This is just the address of the array, it never needs to be written back at all
			Tracer.Registers[6].Modified_Counter = 2;

			Output_Low_Code +=
				"\tHL = SP + " + std::to_string(Stack_Index) + ";\t\t# Gets array address into register\n";

			return "HL";
		}

		//if (!Find_Value_In_Global_Stack(Tracer, Node.Value))
		//	

			// Place value either into register/pair or just return how we want to

		std::string Name = Node.Value;

		if (!Find_Value_In_Global_Stack(Tracer, Node.Value))
			Name = Local_Function_Scope_Name + Node.Value;

		if (Requirements == MAKE_VALUE_HOT_HL_REG || Requirements == MAKE_VALUE_HOT_REG_PAIR)
		{
			Trace* Output_Registers = &Get_Free_Register(Output_Low_Code, Tracer, Requirements);

			Output_Registers[0].Pointer_Flag = 2;
			Output_Registers[1].Pointer_Flag = 1;
			Output_Registers[0].Modified_Counter = 2;
			Output_Registers[1].Modified_Counter = 2;

			Output_Low_Code += "\t" + Output_Registers[0].Name + Output_Registers[1].Name + " = " + Name + ";\n";

			return Output_Registers[0].Name + Output_Registers[1].Name;
		}
		else
		{
			Trace* Output_Register = &Get_Free_Register(Output_Low_Code, Tracer, Requirements);
			Output_Register->Modified_Counter = 2;

			Output_Low_Code += "\t" + Output_Register->Name + " = " + Name + ";\n";
		}

		return Name;
	}

		//

	case S_INT_LITERAL:
	{

		if (Requirements == MAKE_VALUE_HOT_HL_REG || Requirements == MAKE_VALUE_HOT_REG_PAIR)
		{
			Trace* Output_Registers = &Get_Free_Register(Output_Low_Code, Tracer, Requirements);

			Output_Low_Code +=
				"\t" + Output_Registers[0].Name + Output_Registers[1].Name + " = " + Node.Value + ";\n";

			return Output_Registers[0].Name + Output_Registers[1].Name;
		}
		else if(Requirements == MAKE_VALUE_HOT_A_REG)
		{
			Trace* Output_Register = &Get_Free_Register(Output_Low_Code, Tracer, Requirements);

			Output_Low_Code +=
				"\tA = " + Node.Value + ";\n";

			return "A";
		}
		else if (Requirements == MAKE_VALUE_HOT_REG)
		{
			Trace* Output_Register = &Get_Free_Register(Output_Low_Code, Tracer, Requirements);

			Output_Low_Code +=
				"\t" + Output_Register->Name + " = " + Node.Value + ";\n";

			return Output_Register->Name;
		}

		return Node.Value;

		break;

		//

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
		if(Value->Modified_Counter != 1)
			Value->Value = "@statement_value@";
		Value->Modified_Counter = 3;

		std::string Name = Value->Value;

		Clear_Tracer_Registers(Tracer);	// Any other hot registers we temporarily used to evaluate 'value' can be cleared (we don't need em)

		Destination_Registers = Tracer_Make_Value_Hot(Output_Low_Code, Tracer, Node["destination"][0], MAKE_VALUE_HOT_HL_REG);

		Value = Find_Value_In_Tracer_Registers(Tracer, Name);

		if (Value->Value == "@statement_value@")
			Value->Modified_Counter = 2;
		else
			Value->Modified_Counter = 1;

		Register = Value->Name;
	}
	else
		Destination_Registers = Tracer_Make_Value_Hot(Output_Low_Code, Tracer, Node["destination"][0], MAKE_VALUE_HOT_HL_REG);

	Output_Low_Code += "\t[" + Destination_Registers + "] = " + Register + "; \t\t# Stores value into memory location\n";
}

void Node_ID_Assign_Statement(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node)
{
	// Write some kind of code to evaluate expressions and store them in registers etc

	// just set a register to this value but make it HOT so that the tracer knows to store it later

	// Stores expression into register
	// sets corresponding register to the corresponding ID

	if (Node["id"][0].Syntax_ID == S_ID8)
	{

		std::string Register = Tracer_Make_Value_Hot(Output_Low_Code, Tracer, Node["value"][0]);

		Trace* Value = Register_From_Name(Tracer, Register[0]);
		Value->Value = Node["id"][0].Value;
		Value->Modified_Counter = 1;
	}
	else
	{
		std::string Registers = Tracer_Make_Value_Hot(Output_Low_Code, Tracer, Node["value"][0], MAKE_VALUE_HOT_REG_PAIR);

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

	if (Node.Child_Nodes.count("parameters"))
		Node_Parameters_Dec(Output_Low_Code, Tracer, Node["parameters"][0]);
}

void Node_Function_Definition(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node)
{
	// This will write the subroutine to the low-code
	// It'll also add the function declaration to the tracer code
	// once it's finished its statement analysis, clear stack again

	// Note that ALL return statements need to explicitly return the stack to its pre-call state

	// First, add parameters to code tracer stack

	Tracer.Parameters_Count = 0;

	if(Node.Child_Nodes.count("parameters"))									// The function might not even have parameters!
		Node_Parameters_Dec(Output_Low_Code, Tracer, Node["parameters"][0]);

	Tracer.Current_Scope_Function_Return_Type = Node["return_type"][0].Syntax_ID;

	Local_Function_Scope_Name = "__" + Node["id"][0].Value + "__";

	Tracer.Global_Const_Declarations.push_back(&Node);

	Output_Low_Code += "subroutine " + Node["id"][0].Value + "\n{\n";

	for (size_t W = 0; W < Tracer.Registers.size(); W++)
		Tracer.Registers[W].Modified_Counter = 0;			// We enter the function with a blank slate

	Analyse_Statements_LowC(Output_Low_Code, Tracer, Node["statements"][0]);

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

//

void Analyse_Statement_LowC(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node)
{
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

		while(Current_Node->Child_Nodes.count("int_literals"))
		{
			Current_Node = &(*Current_Node)["int_literals"][0];
			Output_Low_Code += "\t" + Current_Node->Value + "\n";
		}
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
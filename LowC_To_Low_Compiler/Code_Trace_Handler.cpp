#include "Code_Trace_Handler.h"
#include "Code_Parser.h"
#include "LowC_Tokeniser.h"

void Analyse_Statements_LowC(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node);

#define MAKE_VALUE_HOT_NO_REQUIREMENTS 0
#define MAKE_VALUE_HOT_A_REG 1
#define MAKE_VALUE_HOT_HL_REG 2

#define MAKE_VALUE_HOT_REG_PAIR 4

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

		for (size_t Index = 0; Index < Tracer.Registers.size(); Index++)
		{
			if (Tracer.Registers[Index].Modified_Counter == 0)
				return Tracer.Registers[Index];
		}

		// I'll implement this later

		break;

		//

	case MAKE_VALUE_HOT_REG_PAIR:

		for (size_t Index = 1; Index < Tracer.Registers.size(); Index += 2)
		{
			if (Tracer.Registers[Index].Modified_Counter == 0 && Tracer.Registers[Index + 1].Modified_Counter == 0)
				return Tracer.Registers[Index];
		}

		// this too
		// although this should be a little easier because I can instantly 'push' a reg pair to the stack effortlessly
	}
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

			Tracer.Registers[5].Modified_Counter = 1;
			Tracer.Registers[6].Modified_Counter = 1;

			Trace* Output_Register;

			if (Requirements == MAKE_VALUE_HOT_HL_REG)
				Output_Register = &Get_Free_Register(Output_Low_Code, Tracer, Requirements);
			else
				Output_Register = &Get_Free_Register(Output_Low_Code, Tracer, MAKE_VALUE_HOT_REG_PAIR);

			Stack_Index = Find_Value_In_Tracer_Stack(Tracer, Node.Value);

			Output_Register->Value = Tracer.Stack[Tracer.Stack.size() - 1 - Stack_Index].Value;
			Output_Register->Modified_Counter = 1;
			Output_Register->Pointer_Flag = 2;// Tracer.Stack[Tracer.Stack.size() - 1 - Stack_Index].Pointer_Flag;

			Output_Low_Code +=
				"\tHL = SP + " + std::to_string(Stack_Index) + ";\n" +
				"\t" + Output_Register[0].Name + " = [HL];\t\t# Places " + Node.Value + " into register\n";

			if (Node.Syntax_ID == S_ID16)
			{
				// we want to increment to next thing

				Output_Register[1].Value = Tracer.Stack[Tracer.Stack.size() - 2 - Stack_Index].Value;
				Output_Register[1].Modified_Counter = 1;
				Output_Register[1].Pointer_Flag = 1;// Tracer.Stack[Tracer.Stack.size() - 2 - Stack_Index].Pointer_Flag;

				Output_Low_Code +=
					"\tHL++;\n\t" +
					Output_Register[1].Name + " = [HL];\t\t# Places lower byte of " + Node.Value + " into register\n";

				//

				if (Requirements == MAKE_VALUE_HOT_HL_REG)
				{
					// move values back around accordingly

					Tracer.Registers[5].Value = Output_Register->Value;
					Tracer.Registers[6].Value = Output_Register[1].Value;

					Tracer.Registers[5].Modified_Counter = 1;
					Tracer.Registers[6].Modified_Counter = 1;

					Output_Register[0].Modified_Counter = 0;
					Output_Register[1].Modified_Counter = 0;

					Output_Low_Code +=
						"\tH = " + Output_Register[0].Name + ";\n\tL = " + Output_Register[1].Name; +";\t\t# Places register pair into HL\n";

					return "HL";
				}

				return Output_Register[0].Name + Output_Register[1].Name;
			}

			return Output_Register[0].Name;
		}

		if (Find_Value_In_Global_Stack(Tracer, Node.Value))
		{
			// Place value either into register/pair or just return how we want to

			if (Requirements == MAKE_VALUE_HOT_HL_REG || Requirements == MAKE_VALUE_HOT_REG_PAIR)
			{

			}
		}

		// After that, check global decs

		// if not there? use local dec naming scheme


		break;
	}

		//

	case S_INT_LITERAL:

		break;

		//


	}
}

void Node_ID_Assign_Statement(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node)
{
	// Write some kind of code to evaluate expressions and store them in registers etc

	// just set a register to this value but make it HOT so that the tracer knows to store it later


}

void Node_Return_Statement(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node)
{
	// Roll the stack back to before we created the local variables

	if (Node.Child_Nodes.count("value"))
	{
		// If there's a value, we need to evaluate the expression and store it in either the A or HL registers
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

	if (Node.Syntax_ID == S_RETURN)
	{
		Node_Return_Statement(Output_Low_Code, Tracer, Node);
		return;
	}
}

void Analyse_Statements_LowC(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node)
{
	Analyse_Statement_LowC(Output_Low_Code, Tracer, Node);

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
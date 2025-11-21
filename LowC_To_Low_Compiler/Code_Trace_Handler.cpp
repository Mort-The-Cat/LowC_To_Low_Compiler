#include "Code_Trace_Handler.h"
#include "Code_Parser.h"
#include "LowC_Tokeniser.h"

void Analyse_Statements_LowC(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node);

void Node_ID_Assign_Statement(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node)
{
	// Write some kind of code to evaluate expressions and store them in registers etc
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
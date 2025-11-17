#include "Code_Trace_Handler.h"
#include "Code_Parser.h"
#include "LowC_Tokeniser.h"

void Node_Function_Definition(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node)
{
	// This will write the subroutine to the low-code
	// It'll also add the function declaration to the tracer code
	// once it's finished its statement analysis, clear stack again
}

void Node_ROM_V_Declaration(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node)
{

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
		while (Size--)
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

void Analyse_Parsed_LowC(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node)
{
	Analyse_Global_Declarations_LowC(Output_Low_Code, Tracer, Node["global_declarations"][0]);
	//Analyse_Statements_LowC(Output_Low_Code, Tracer, Node["statements"][0]);
	// 
}
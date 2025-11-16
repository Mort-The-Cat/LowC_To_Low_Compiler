#ifndef CODE_TRACE_HANDLER
#define CODE_TRACE_HANDLER

#include<string>
#include<vector>
#include<array>

#include "Code_Parser.h"

class Trace
{
public:
	std::string Name;			// The name of the value (could also just be a byte-literal)
								// i.e. Pointer + 1 or 
	std::string Value;			// This is an expression of the value stored at this location (if garbage data, set to "?")
								// i.e. Data[1] etc 
	size_t Modified_Counter;	// How many times the value has changed
	unsigned char Pointer_Flag;	// 0 if not a pointer, 1 if low, 2 if high
	Trace()
	{
		Name = "???";
		Value = "???";
		Modified_Counter = 0;
		Pointer_Flag = 0;
	}
	Trace(std::string Namep, std::string Valuep, unsigned char Pointer_Flagp)
	{
		Name = Namep;
		Value = Valuep;
		Modified_Counter = 0;
		Pointer_Flag = Pointer_Flagp;
	}

	Trace(std::string Namep, unsigned char Pointer_Flagp)
	{
		Name = Namep;
		Value = "???";
		Modified_Counter = 0;
		Pointer_Flag = Pointer_Flagp;
	}
};

class Tracer_Data
{
public:
	std::vector<Trace> Stack;
	std::array<Trace, 7> Registers =
	{
		Trace("A", 0),
		Trace("B", 1),
		Trace("C", 0),
		Trace("D", 1),
		Trace("E", 0),
		Trace("H", 1),
		Trace("L", 0)
	};
};

// NOTE that local stack should NEVER be larger than 127 bytes
// If any larger, failures occur.

// For considerably larger buffers, just use pointers to access.

void Analyse_Parsed_LowC(std::string& Output_Low_Code, Tracer_Data& Tracer, const Parse_Node& Node);

#endif
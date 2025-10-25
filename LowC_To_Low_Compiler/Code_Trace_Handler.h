#ifndef CODE_TRACE_HANDLER
#define CODE_TRACE_HANDLER

#include<string>
#include<vector>
#include<array>

#include "LowC_Tokeniser.h"

class Trace
{
public:
	std::string Name;			// The name of the value (could also just be a byte-literal)
								// i.e. Pointer + 1 or 
	std::string Value;			// This is an expression of the value stored at this location (if garbage data, set to "?")
								// i.e. Data[1] etc 
	size_t Modified_Counter;	// How many times the value has changed
	unsigned char Is_High;
	Trace()
	{
		Name = "???";
		Value = "???";
		Modified_Counter = 0;
		Is_High = 0;
	}
	Trace(std::string Namep, unsigned char Is_Highp)
	{
		Name = Namep;
		Value = "???";
		Modified_Counter = 0;
		Is_High = Is_Highp;
	}
};

// NOTE that local stack should NEVER be larger than 127 bytes
// If any larger, failures occur.

// For considerably larger buffers, just use pointers to access.

void Analyse_LowC_Text(const std::vector<Token>& Tokens)
{
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

	// 
}

#endif
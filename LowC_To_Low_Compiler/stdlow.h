#ifndef STDLOW
#define STDLOW

void _call(const unsigned char* Function_Pointer);	// This needs a direct LOW implementation

#define call(function, return_type, parameters) _call(function);	// Function implementation included elsewhere (NOTE THIS RESULTS IN UNDEFINED BEHAVIOUR UNLESS DONE IN LOWC)

#define mem ((unsigned char*)(0))

// 	call(Function_Pointer, void, byte)(Value);
/*

	the compiler will produce something like this:

	SP += FF; # decrement 1
	HL = SP + 0;
	[HL] = byte; // whatever the value of 'byte' is
	HL = Function_Pointer; // HL needs to equal the function pointer we want or whatever
	call low_call;

	...
	...

	subroutine low_call
	{
		jump HL;
	}
*/

typedef unsigned char byte;		// 'byte' is the most common type you'll be using
typedef unsigned short word;	// 'word' is used for 16-bit integers
//

void memset(byte* Destination, byte Value, byte Length) // these functions don't need to be included in the codebase
{
	while (Length--)
		(Destination++)[0] = Value;
}

#endif
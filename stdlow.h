#ifndef STDLOW
#define STDLOW

void _call(const unsigned char* Function_Pointer);	// This needs a direct LOW implementation

#define call(function, return_type, parameters) ((return_type(*)parameters)function)

//_call(function);	// Function implementation included elsewhere (NOTE THIS RESULTS IN UNDEFINED BEHAVIOUR UNLESS DONE IN LOWC)

#define mem ((unsigned char*)(0))

#define addressof(location) (byte*)(location)

#define high(expression8) ((word)expression8 << 8)

#define store_high(id16, expression8)

#define bit(expression, bit_number) (1 & (expression >> bit_number))

#define push()

#define pop()

#define enablei()

#define disablei()

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

#endif
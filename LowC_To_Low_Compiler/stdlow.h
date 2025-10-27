#ifndef STDLOW
#define STDLOW

void _call(pointer Function_Pointer);	// This needs a direct LOW implementation

#define call(function, return_type, parameters) _call(function);	// Function implementation included elsewhere (NOTE THIS RESULTS IN UNDEFINED BEHAVIOUR UNLESS DONE IN LOWC)


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
typedef unsigned char* pointer; // Note that pointers on SM83 architectures are also 16 bits

//

#define INTERRUPT_ENABLE_REGISTER 0xFFFF 
	/*
		bit 0 - VBLANK
		bit 1 - LCD interrupts i.e. HBlank or LYC
	*/

#define BACKGROUND_SCROLL_X_REGISTER 0xFF43
#define BACKGROUND_SCROLL_Y_REGISTER 0xFF42

#define WINDOW_SCROLL_X_REGISTER 0xFF4B
#define WINDOW_SCROLL_Y_REGISTER 0xFF4A

#define BACKGROUND_PALETTE_REGISTER 0xFF47

#define OBJECT_PALETTE_0_REGISTER 0xFF48

#define OBJECT_PALETTE_1_REGISTER 0xFF49

#define LCDY_REGISTER 0xFF44	// current scanline
#define LYC_REGISTER 0xFF45		// this is the value that's compared against

#define LCD_STATUS_REGISTER 0xFF41

#define VBLANK_SCANLINE 0x90

#define LCDC_REGISTER 0xFF40

#define DMA_REGISTER 0xFF46

#define OAM 0xFE00

// C000 - DFFF is work RAM! we can do whatever we want there

#define HRAM 0xFF80 // FF80 - FFFE

#define VRAM_BLOCK_0 0x8000
#define VRAM_BLOCK_1 0x8800
#define VRAM_BLOCK_2 0x9000

#define VRAM_TILEM_0 0x9800
#define VRAM_TILEM_1 0x9C00

#define CONTROLLER_REGISTER 0xFF00

#define SQUARE_CHANNEL_0 0xFF10
#define SQUARE_CHANNEL_1 0xFF15
#define WAVE_CHANNEL 0xFF1A
#define NOISE_CHANNEL 0xFF1F

//

void memset(pointer Destination, byte Value, byte Length) // these functions don't need to be included in the codebase
{
	while (Length--)
		Destination++[0] = Value;
}

#endif
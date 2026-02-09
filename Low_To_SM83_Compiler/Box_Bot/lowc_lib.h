
// This header file declares a number of LowC functions
//  with an optimised direct Low implementation

#define DMA_HRAM_Function addressof(0xFF80)

#define VBLANK_HRAM_FUNCTION_JUMP addressof(0xFF85)
#define VBLANK_HRAM_FUNCTION_JUMP_ADDRESS addressof(0xFF86)

#define HBLANK_HRAM_FUNCTION_JUMP addressof(0xFF88)
#define HBLANK_HRAM_FUNCTION_JUMP_ADDRESS addressof(0xFF89)

// HRAM is free from 0xFF8B onwards

void init_screen_interrupts();

void Set_Interrupt_Function(byte* Address, byte* Function_Pointer)
{
	*Address = (byte)Function_Pointer;
	Address++;
	*Address = high(Function_Pointer);

	return;
}

void init_dma();    // This initialises the functions necessary for a simple DMG DMA transfer
                    // only needs to be called once (unless you intend to overwrite HRAM)

void dma_transfer();    // This runs a DMA transfer

void memcpy(byte* Destination, byte* Source, word Count);

void memset(byte* Destination, byte Value, word Count);

#define OAM_BUFFER_COUNTER addressof(0xC0A0)

void place_spritechain_in_oam_buffer(const byte* Spritechain, byte Count, byte Y, byte X);

word sub16(word Left, word Right);
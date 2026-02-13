#define INTERRUPT_ENABLE_REGISTER addressof(0xFFFF)
//	bit 0 - VBLANK
//	bit 1 - LCD interrupts i.e. HBlank or LYC


#define BACKGROUND_SCROLL_X_REGISTER addressof(0xFF43)
#define BACKGROUND_SCROLL_Y_REGISTER addressof(0xFF42)

#define WINDOW_SCROLL_X_REGISTER addressof(0xFF4B)
#define WINDOW_SCROLL_Y_REGISTER addressof(0xFF4A)

#define BACKGROUND_PALETTE_REGISTER addressof(0xFF47)

#define OBJECT_PALETTE_0_REGISTER addressof(0xFF48)

#define OBJECT_PALETTE_1_REGISTER addressof(0xFF49)

#define LCDY_REGISTER addressof(0xFF44)	// current scanline
#define LYC_REGISTER addressof(0xFF45)		// this is the value that's compared against

#define LCD_STATUS_REGISTER addressof(0xFF41)
//		bit 6 is set when you want a STAT interrupt to occur during LYC == LCDY
//		bit 3 is set when you want HBLANK interrupts 
//		bit 2 is set when LYC == LCDY
//		bits 1 and 0 determine the current PPU mode (0 if HBlank or currently disabled, 1 if VBlank)


#define VBLANK_SCANLINE addressof(0x90)

#define LCDC_REGISTER addressof(0xFF40)
//      bit 7 controls LCD and PPU			(MAY ONLY BE CHANGED DURING VBLANK!!)
	
//      bit 6 window tile map 9800-9BFF or 9C00-9FFF
	
//      bit 5 window enable
	
//      bit 4 controls addressing mode for tile *data
//          if bit 4 is not, 0-127 corresponds to $9000-97FF, and 128-255 corresponds to $8800-8FFF
//	        if bit 4 is set, 0-127 corresponds to $8000-87FF, and 128-255 corresponds to $8800-8FFF
//	            Note that objects always use the latter unsigned addressing mode for tile data
	
//      bit 3 background tile map 9800-9BFF or 9C00-9FF
	
//      bit 2 OBJ 8x8 or 8x1
	
//      bit 1 controls if objects are enable
	
//      bit 0 background/window enable

#define DMA_REGISTER addressof(0xFF46)

#define OAM addressof(0xFE00)
    // byte 0 is y-position + 16
	// byte 1 is x-position + 8
	// byte 2 is tile index
	// byte 3 are the flags
		// bit 7 is the priority bit (set this to decrease object priority)
	    // bit 6 is the Y flip
	    // bit 5 is the X flip
		// bit 4 is the palette (palette 0 or palette 1)

		// bit 3,2,1 is the CGB palette (between 0 and 7)

	// 4 bytes per object

// C000 - DFFF is work RAM! we can do whatever we want there

#define VRAM_BANK addressof(0xFF4F)
// This is used to swap the VRAM bank
// (write either 0 or 1 to the LSB)

#define CGB_BG_PALETTE_SPECIFIER addressof(0xFF68)
#define CGB_BG_PALETTE addressof(0xFF69)

#define CGB_OBJ_PALETTE_SPECIFIER addressof(0xFF6A)
#define CGB_OBJ_PALETTE addressof(0xFF6B)
// 5 bits per channel across 2 bytes

#define HRAM addressof(0xFF80) // FF80 - FFFE

#define VRAM_BLOCK_0 addressof(0x8000)
#define VRAM_BLOCK_1 addressof(0x8800)
#define VRAM_BLOCK_2 addressof(0x9000)

#define VRAM_TILEM_0 addressof(0x9800)
#define VRAM_TILEM_1 addressof(0x9C00)

#define CONTROLLER_REGISTER addressof(0xFF00)
	// bit 5 - clear this bit if you want the Start/Select/  B /    A buttons
	// bit 4 - clear this bit if you want the Down /  Up  /Left/Right buttons
	// bit 3 - Start		/ Down
	// bit 2 - Select	/ Up
	// bit 1 - B			/ Left
	// bit 0 - A			/ Right

	// Whichever button is read depends on which bit has just been cleared.
	// Takes a few clock cycles for the register to toggle properly.

#define CONTROLLER_SSBA_FLAG 0xD0
#define CONTROLLER_DPAD_FLAG 0xE0

#define CONTROLLER_BUTTON_START_BIT 0x03
#define CONTROLLER_BUTTON_DOWN_BIT 0x03
#define CONTROLLER_BUTTON_SELECT_BIT 0x02
#define CONTROLLER_BUTTON_UP_BIT 0x02
#define CONTROLLER_BUTTON_B_BIT 0x01
#define CONTROLLER_BUTTON_LEFT_BIT 0x01
#define CONTROLLER_BUTTON_A_BIT 0x00
#define CONTROLLER_BUTTON_RIGHT_BIT 0x00

#define SQUARE_CHANNEL_0 addressof(0xFF10)
#define SQUARE_CHANNEL_1 addressof(0xFF15)
#define WAVE_CHANNEL addressof(0xFF1A)
#define NOISE_CHANNEL addressof(0xFF1F)
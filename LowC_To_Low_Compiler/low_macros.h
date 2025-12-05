#define INTERRUPT_ENABLE_REGISTER addressof(0xFFFF)

//	bit 0 - VBLANK
//	bit 1 - LCD interrupts i.e. HBlank or LYC


#define BACKGROUND_SCROLL_X_REGISTER addressof(0xFF43)
#define BACKGROUND_SCROLL_Y_REGISTER addressof(0xFF42

#define WINDOW_SCROLL_X_REGISTER addressof(0xFF4B)
#define WINDOW_SCROLL_Y_REGISTER addressof(0xFF4A)

#define BACKGROUND_PALETTE_REGISTER addressof(0xFF47)

#define OBJECT_PALETTE_0_REGISTER addressof(0xFF48)

#define OBJECT_PALETTE_1_REGISTER addressof(0xFF49)

#define LCDY_REGISTER addressof(0xFF44)	// current scanline
#define LYC_REGISTER addressof(0xFF45)		// this is the value that's compared against

#define LCD_STATUS_REGISTER addressof(0xFF41)

#define VBLANK_SCANLINE addressof(0x90)

#define LCDC_REGISTER addressof(0xFF40)
/*
		# bit 7 controls LCD and PPU (MAY ONLY BE CHANGED DURING VBLANK!!)
		# bit 6 window tile map 9800 - 9BFF or 9C00 - 9FFF
		# bit 5 window enable
		# bit 4 controls addressing mode for tile * data *

		# if bit 4 is not, 0-127 corresponds to $9000-97FF, and 128-255 corresponds to $8800-8FFF
		# if bit 4 is set, 0-127 corresponds to $8000-87FF, and 128-255 corresponds to $8800-8FFF
		# Note that objects always use the latter unsigned addressing mode for tile data

		# bit 3 background tile map 9800 - 9BFF or 9C00 - 9FFF

		# bit 2 OBJ 8x8 or 8x16

		# bit 1 controls if objects are enabled

		# bit 0 background / window enable
*/

#define DMA_REGISTER addressof(0xFF46)

#define OAM addressof(0xFE00)

// C000 - DFFF is work RAM! we can do whatever we want there

#define HRAM addressof(0xFF80) // FF80 - FFFE

#define VRAM_BLOCK_0 addressof(0x8000)
#define VRAM_BLOCK_1 addressof(0x8800)
#define VRAM_BLOCK_2 addressof(0x9000)

#define VRAM_TILEM_0 addressof(0x9800)
#define VRAM_TILEM_1 addressof(0x9C00)

#define CONTROLLER_REGISTER addressof(0xFF00)

#define SQUARE_CHANNEL_0 addressof(0xFF10)
#define SQUARE_CHANNEL_1 addressof(0xFF15)
#define WAVE_CHANNEL addressof(0xFF1A)
#define NOISE_CHANNEL addressof(0xFF1F)
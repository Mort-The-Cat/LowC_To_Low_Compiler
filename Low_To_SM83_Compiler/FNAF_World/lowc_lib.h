
// This header file declares a number of LowC functions
//  with an optimised direct Low implementation

#define DMA_HRAM_Function 0xFF80

void init_dma();    // This initialises the functions necessary for a simple DMG DMA transfer
                    // only needs to be called once (unless you intend to overwrite HRAM)

void dma_transfer();    // This runs a DMA transfer

void memcpy(byte* Destination, byte* Source, word Count);

void memset(byte* Destination, byte Value, word Count);

word sub16(word Left, word Right);


void Generate_Palettes(byte* Destination)
{
	byte* Data;
	byte Count;

	*Destination = 0x80;	// start from the beginning and auto-increment to the end

	Destination++;

	Data = FNAF_World_Palette;
	Count = sizeof(FNAF_World_Palette);

	do
	{
		*Destination = *Data;
		Data++;
		Count--;
	} while (Count);

	return;
}

void Set_Interrupt_Function(byte* Address, byte* Function_Pointer)
{
	*Address = (byte)Function_Pointer;
	Address++;
	*Address = high(Function_Pointer);

	return;
}

#define Frame_Counter addressof(0xC100)	// This is a 2-byte value
#define Parallax_LYC_Data addressof(0xC103) // The LYC at which the next parallax interrupt should be triggered
// LYC pos
#define Parallax_Scroll_Data addressof(0xC107) // The scroll value that should be applied to the scanlines
#define Parallax_Index addressof(0xC102) // This is the index into the parallax_data table

void Test_Titlescreen_Vblank()
{
	push();

	dma_transfer();		// This places the OAM buffer into OAM

	*BACKGROUND_SCROLL_Y_REGISTER = 40;

	*BACKGROUND_SCROLL_X_REGISTER = *Parallax_Scroll_Data;

	*LYC_REGISTER = *Parallax_LYC_Data;

	pop();

	enablei();

	return;
}

void Test_Titlescreen_Hblank()
{
	push();

	byte* Address;
	byte* Index;

	Index = Parallax_Index;
	*Index++;
	Address = (Parallax_LYC_Data + *Index);

	*LYC_REGISTER = *Address;

	Address = (Parallax_Scroll_Data + *Index);

	*BACKGROUND_SCROLL_X_REGISTER = *Address;

	pop();

	enablei();

	return;
}

void Init_Parallax_Data()
{
	const byte Parallax_Data[] =
	{
		47, 63, 79, 255
	};

	memcpy(Parallax_LYC_Data, Parallax_Data, sizeof(Parallax_Data));

	return;
}

void Write_Parallax_Data()
{
	*Parallax_Index = 0;

	byte* Address;
	word Frame_Count;
	Address = Frame_Counter;
	Frame_Count = *Address;
	Address++;
	store_high(Frame_Count, *Address);

	// Frame_Count = (Frame_Count + Frame_Count);

	Address = Parallax_Scroll_Data;
	*Address = (byte)Frame_Count;	// Top set of clouds
	Address++;
	Frame_Count = shift_right(Frame_Count);
	*Address = (byte)Frame_Count;	// medium clouds
	Address++;
	Frame_Count = (byte)shift_right(Frame_Count);
	*Address = (byte)Frame_Count;	// bottom clouds
	Address++;
	*Address = 224;					// land, (no scroll)

	return;
}

void Increment_Frame_Counter()
{
	// *(Frame_Counter)++;

	byte* Address;
	word Frame_Count;

	Address = Frame_Counter;
	store_low(Frame_Count, *Address);
	Address++;
	store_high(Frame_Count, *Address);

	Frame_Count++;

	*Address = high(Frame_Count);
	Address--;
	*Address = (byte)Frame_Count;

	return;
}
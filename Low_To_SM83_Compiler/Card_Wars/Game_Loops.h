void Start_Table_Scene()
{
    Wait_For_VBlank();

    *(LCDC_REGISTER) = 0;

    Set_Interrupt_Function(HBLANK_HRAM_FUNCTION_JUMP_ADDRESS, Table_Scroll_HBlank_Function);

    memcpy(VRAM_BLOCK_2, Table_Graphics_Data, sizeof(Table_Graphics_Data));

    //memcpy(VRAM_BLOCK_1, Finny_Graphics_Data, sizeof(Finny_Graphics_Data));

    memcpy(VRAM_BLOCK_1, Alphabet_Graphics_Data, sizeof(Alphabet_Graphics_Data));

    memcpy(VRAM_BLOCK_1 + sizeof(Alphabet_Graphics_Data), Finny_Graphics_Data, sizeof(Finny_Graphics_Data));

    memcpy(VRAM_BLOCK_0, Open_Window_Graphics_Data, sizeof(Open_Window_Graphics_Data));

    // memcpy(VRAM_BLOCK_1 + sizeof(Alphabet_Graphics_Data), Title_Screen_UI_Blocks, sizeof(Title_Screen_UI_Blocks));

    memset(VRAM_TILEM_0, 0xFF, 0x400);

    memset(addressof(0x9820), 0x5B, 32);
    memset(addressof(0x98C0), 0xB2, 18);

    Copy_Tilemap(VRAM_TILEM_0, Table_Tilemap_Data, sizeof(Table_Tilemap_Data), Table_Tilemap_Width);

    // const byte Placeholder_Text[] = "INSERT FINN HERE!";

    Copy_Tilemap(addressof(0x9B41), Finny_Tilemap_Data, sizeof(Finny_Tilemap_Data), Finny_Tilemap_Width);

    //memcpy(addressof(0x9B60), Placeholder_Text, sizeof(Placeholder_Text));

    *(BACKGROUND_PALETTE_REGISTER) = b11100100;

    *(OBJECT_PALETTE_0_REGISTER) = b10011100;

    *(LCDC_REGISTER) = 0x87;

    *(LCD_STATUS_REGISTER) = 0x08;

    *(INTERRUPT_ENABLE_REGISTER) = 0x02;

    Test_Scroll_Loop();

    return;
}


void Start_Board_Scene()
{
    //Wait_For_VBlank();

    // *(LCDC_REGISTER) = 0x00;

    *BACKGROUND_SCROLL_X_REGISTER = 0;

    *(INTERRUPT_ENABLE_REGISTER) = 0x00;

    memcpy(VRAM_BLOCK_2, Test_Card_Graphics_Data, sizeof(Test_Card_Graphics_Data));

    memset(VRAM_TILEM_0, 0xFF, 0x400);

    // write 0x98E0 if you want the bottom of the screen, just below the table
    Copy_Tilemap(addressof(0x9820), Card_Tilemap_Data, sizeof(Card_Tilemap_Data), Card_Tilemap_Width);

    *(LCDC_REGISTER) = 0x87;

    byte Input;

    Input = 0;

    do
    {
        Input = Get_Controller_Input(CONTROLLER_SSBA_FLAG);
        Clean_OAM_Buffer();
        Wait_For_VBlank();
        dma_transfer();
    }while(bit(Input, CONTROLLER_BUTTON_START_BIT));

    return;
}


//

void Wait_For_Button_Release()
{
    byte Input;

    do
    {
        Input = Get_Controller_Input(CONTROLLER_SSBA_FLAG);
    }while(!bit(Input, CONTROLLER_BUTTON_START_BIT));             // While it's held down

    return;
}

void Alternate_Game_Loops()
{
    do
    {
        Start_Table_Scene();

        Wait_For_Button_Release();

        Start_Board_Scene();

        Wait_For_Button_Release();

    }while(true);

    return;
}
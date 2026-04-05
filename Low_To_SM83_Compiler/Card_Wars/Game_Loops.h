void Start_Table_Scene(byte* Game_Info)
{
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

    *(OBJECT_PALETTE_1_REGISTER) = b01000000;

    *(LCDC_REGISTER) = 0x87;

    *(LCD_STATUS_REGISTER) = 0x08;

    *(INTERRUPT_ENABLE_REGISTER) = 0x02;

    Test_Scroll_Loop();

    return;
}

//

void Test_Card_Menu(byte* Game_Info)
{
    //Wait_For_VBlank();

    //*(LCDC_REGISTER) = 0x00;

    *BACKGROUND_SCROLL_X_REGISTER = 0;

    *BACKGROUND_SCROLL_Y_REGISTER = 0;

    Copy_Tilemap(addressof(0x9800), Card_Menu_Tilemap, sizeof(Card_Menu_Tilemap), Card_Menu_Tilemap_Width);

    *(LCDC_REGISTER) = 0x87;

    byte Input;

    Input = 0;

    do
    {
        Clean_OAM_Buffer();

        Input = Get_Controller_Input(CONTROLLER_DPAD_FLAG);

        if(!bit(Input, CONTROLLER_BUTTON_DOWN_BIT))
        {
            *(BACKGROUND_SCROLL_Y_REGISTER)++;
        }

        if(!bit(Input, CONTROLLER_BUTTON_UP_BIT))
        {
            *(BACKGROUND_SCROLL_Y_REGISTER)--;
        }

        if(!bit(Input, CONTROLLER_BUTTON_RIGHT_BIT))
        {
            *(BACKGROUND_SCROLL_X_REGISTER)++;
        }

        if(!bit(Input, CONTROLLER_BUTTON_LEFT_BIT))
        {
            *(BACKGROUND_SCROLL_X_REGISTER)--;
        }

        Input = Get_Controller_Input(CONTROLLER_SSBA_FLAG);
        Wait_For_VBlank();
        dma_transfer();
    }while(bit(Input, CONTROLLER_BUTTON_START_BIT));

    *(LCDC_REGISTER) = 0x00;

    return;
}

//

void Select_Board_Card(byte Fresh_Inputs, byte* Selected_Card)
{
    byte Index;

    Index = *Selected_Card;

    if(!bit(Fresh_Inputs, CONTROLLER_BUTTON_DOWN_BIT))
    {
        //*(BACKGROUND_SCROLL_Y_REGISTER)++;
        if(Index < 12)
        {
            Index = Index + 4;
        }
    }
    
    if(!bit(Fresh_Inputs, CONTROLLER_BUTTON_UP_BIT))
    {
        if(Index > 3)
        {
            Index = Index - 4;
        }
    }
    
    if(!bit(Fresh_Inputs, CONTROLLER_BUTTON_RIGHT_BIT))
    {
        if((Index & 3) ^ 3)
        {
            Index++;
        }
    }
    
    if(!bit(Fresh_Inputs, CONTROLLER_BUTTON_LEFT_BIT))
    {
        if(Index & 3)
        {
            Index--;
        }
    }

    Index = Index & 15;    // Clamp between 0-15

    *Selected_Card = Index;

    const byte Vertical_Scroll[] = 
    {
        0x00,
        0x29,
        0x49,
        0x71
    };

    const byte Horizontal_Scroll[] = 
    {
        0x00,
        0x0A,
        0x16,
        0x21
    };

    byte Desired_X;
    byte Desired_Y;

    Desired_X = *(Horizontal_Scroll + (word)(Index & 3));
    Desired_Y = *(Vertical_Scroll + shift_right(shift_right((word)Index)));

    Desired_X = Desired_X + *(BACKGROUND_SCROLL_X_REGISTER);
    Desired_Y = Desired_Y + *(BACKGROUND_SCROLL_Y_REGISTER);

    *(BACKGROUND_SCROLL_X_REGISTER) = shift_right(Desired_X);
    *(BACKGROUND_SCROLL_Y_REGISTER) = shift_right(Desired_Y);

    return;
}

void Board_Scene_Selected_Card_Icon(byte* Game_Info, byte* Selected_Card)
{
    const byte Selected_Card_Spritechain[] =
    {
        0x11, 0x09, 0xAE, 0x10,
        0x11, 0x2F, 0xAF, 0x10,
        0x47, 0x09, 0xB0, 0x10,
        0x47, 0x2F, 0xB1, 0x10
    };

    const byte Sprite_Position_X[] =
    {
        0x00, 0x30, 0x60, 0x90
    };

    const byte Sprite_Position_Y[] =
    {
        0x00, 0x40, 0x80, 0xC0
    };

    byte X;

    X = (*Selected_Card) & 3;
    X = *(Sprite_Position_X + (word)X);

    byte Y;

    Y = *Selected_Card;
    Y = *(Sprite_Position_Y + shift_right(shift_right((word)Y)));

    place_spritechain_in_oam_buffer(Selected_Card_Spritechain, 4, Y - (*BACKGROUND_SCROLL_Y_REGISTER), X - (*BACKGROUND_SCROLL_X_REGISTER));

    if(!bit(*(Game_Info + Game_Info_SSBA_Fresh), CONTROLLER_BUTTON_A_BIT))
    {
        Place_Card_On_Board(Game_Info, *Selected_Card, ID_Strawman);
        Wait_For_VBlank();
        *(LCDC_REGISTER) = 0x00;
        Draw_Board_Cards(Game_Info);
        *(LCDC_REGISTER) = 0x83;
    }

    return;
}

void Start_Board_Scene(byte* Game_Info)
{
    *BACKGROUND_SCROLL_X_REGISTER = 0;

    *BACKGROUND_SCROLL_Y_REGISTER = 0;

    *(INTERRUPT_ENABLE_REGISTER) = 0x00;

    // memcpy(VRAM_BLOCK_2, Test_Card_Graphics_Data, sizeof(Test_Card_Graphics_Data));

    memcpy(addressof(0x9780), Card_Icon_Graphics_Data, sizeof(Card_Icon_Graphics_Data));

    memset(VRAM_TILEM_0, 0xFF, 0x400);

    // write 0x98E0 if you want the bottom of the screen, just below the table
    
    //Draw_Board_Creature_Card(0, Skeleton_Card_Data); // This will draw the creature to the screen
    //Draw_Board_Creature_Card(2, Ancient_Scholar_Card_Data); // This will draw the creature to the screen
    //Draw_Board_Creature_Card(13, Pig_Card_Data); // This will draw the creature to the screen
    //Draw_Board_Creature_Card(7, Skeleton_Card_Data); // This will draw the creature to the screen
    //Draw_Board_Creature_Card(5, Skeleton_Card_Data); // This will draw the creature to the screen
//
    //Draw_Board_Creature_Card(8, Mage_Card_Data); // This will draw the creature to the screen

    //Draw_Board_Creature_Card(0, Ancient_Scholar_Card_Data);
    //Draw_Board_Creature_Card(1, Strawman_Card_Data);
    //Draw_Board_Creature_Card(2, Starchy_Card_Data);
    //Draw_Board_Creature_Card(3, Mage_Card_Data);
    //Draw_Board_Creature_Card(4, Sir_Slicer_Card_Data);
    //Draw_Board_Creature_Card(5, Skeleton_Card_Data);
    //Draw_Board_Creature_Card(6, Skeleton_Card_Data);
    //Draw_Board_Creature_Card(7, Pig_Card_Data);
    //Draw_Board_Creature_Card(8, Skeleton_Card_Data);
    //Draw_Board_Creature_Card(9, Skeleton_Card_Data);
    //Draw_Board_Creature_Card(10, Bald_Man_Card_Data);
    //Draw_Board_Creature_Card(11, Skeleton_Card_Data);
    //Draw_Board_Creature_Card(12, Coffin_Card_Data);
    //Draw_Board_Creature_Card(13, Coffin_Card_Data);
    //Draw_Board_Creature_Card(14, Skeleton_Card_Data);
    //Draw_Board_Creature_Card(15, Coffin_Card_Data);

    Place_Card_On_Board(Game_Info, 0, ID_Ancient_Scholar);
    Place_Card_On_Board(Game_Info, 1, ID_Strawman);
    Place_Card_On_Board(Game_Info, 2, ID_Starchy);
    Place_Card_On_Board(Game_Info, 3, ID_Mage);
    Place_Card_On_Board(Game_Info, 4, ID_Sir_Slicer);
    Place_Card_On_Board(Game_Info, 5, ID_Skeleton);
    Place_Card_On_Board(Game_Info, 7, ID_Pig);
    Place_Card_On_Board(Game_Info, 8, ID_Skeleton);
    Place_Card_On_Board(Game_Info, 9, ID_Skeleton);
    Place_Card_On_Board(Game_Info, 10, ID_Bald_Man);
    Place_Card_On_Board(Game_Info, 12, ID_Coffin);
    Place_Card_On_Board(Game_Info, 13, ID_Coffin);

    Draw_Board_Cards(Game_Info);

    //Copy_Tilemap(addressof(0x9820), Card_Tilemap_Data, sizeof(Card_Tilemap_Data), Card_Tilemap_Width);

    *(LCDC_REGISTER) = 0x83;

    byte Input;

    Input = 0;

    byte Selected_Card; //[1];

    Selected_Card = 13;

    do
    {
        Clean_OAM_Buffer();

        Update_Player_Inputs(Game_Info);

        Select_Board_Card(*(Game_Info + Game_Info_DPAD_Fresh), &Selected_Card);
        Board_Scene_Selected_Card_Icon(Game_Info, &Selected_Card);

        Input = *(Game_Info + Game_Info_SSBA_Read);

        //Input = Get_Controller_Input(CONTROLLER_SSBA_FLAG);
        Wait_For_VBlank();
        dma_transfer();
    }while(bit(Input, CONTROLLER_BUTTON_START_BIT));

    *LCDC_REGISTER = 0x00;

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

void Alternate_Game_Loops(byte* Game_Info)
{
    do
    {
        Start_Table_Scene(Game_Info);

        Wait_For_Button_Release();

        Start_Board_Scene(Game_Info);

        Wait_For_Button_Release();

        Test_Card_Menu(Game_Info);

        Wait_For_Button_Release();

    }while(true);

    return;
}

void Update_Player_Inputs(byte* Game_Info)
{
    byte Previous_Value;
    byte Current_Value;

    Previous_Value = *(Game_Info + 4);

    Current_Value = Get_Controller_Input(CONTROLLER_DPAD_FLAG);
    Previous_Value = Current_Value | (Previous_Value ^ 255);         // falling edge detection

    *(Game_Info + 4) = Current_Value;
    *(Game_Info + 6) = Previous_Value;

    Previous_Value = *(Game_Info + 5);

    Current_Value = Get_Controller_Input(CONTROLLER_SSBA_FLAG);
    Previous_Value = Current_Value | (Previous_Value ^ 255);

    *(Game_Info + 5) = Current_Value;
    *(Game_Info + 7) = Previous_Value;

    return;
}
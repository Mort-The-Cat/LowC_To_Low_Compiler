void Test_Scroll_Loop(byte* Game_Info)
{
    byte Player_Input;

    *(LCDC_REGISTER) = 0x87;

    do
    {
        Update_Player_Inputs(Game_Info);

        Scroll_Table();
        Clean_OAM_Buffer();

        Wait_For_VBlank();
        dma_transfer();

        Player_Input = *(Game_Info + Game_Info_SSBA_Fresh);
    }while(bit(Player_Input, CONTROLLER_BUTTON_START_BIT));

    *LCDC_REGISTER = 0;

    return;
}

void Title_Screen_Loop(byte* Game_Info)        // This will wait for the player to press start and then enter a basic menu
{
    *(LCDC_REGISTER) = 0x00;

    Init_Table_Scene(Game_Info);

    Copy_Tilemap(addressof(0x9AA0), Title_Screen_Tilemap_Data, sizeof(Title_Screen_Tilemap_Data), Title_Screen_Tilemap_Width);

    Draw_Tilemap_Menu(addressof(0x98E0), 18, 2);

    *(BACKGROUND_SCROLL_Y_REGISTER) = 168;

    // Do some kind of menu here to select gamemode etc

    Test_Scroll_Loop(Game_Info);

    return;
}

// From here, we'll implement a kind of game loop, separate from the test 'game' loop

// This will have the functionality
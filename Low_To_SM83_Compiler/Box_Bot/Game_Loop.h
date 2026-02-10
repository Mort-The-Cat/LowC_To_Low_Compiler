void Clean_OAM_Buffer();
void Wait_For_VBlank();

void Render_Player(byte* Player_Data, word Camera_X, word Camera_Y);

void Render_Player(byte* Player_Data, word Camera_X, word Camera_Y)
{
    byte Screen_X;
    byte Screen_Y;

    word Position;

    Position = load_16(Player_Data);

    Camera_X = Camera_X;

    Screen_X = (byte)sub16( Position, Camera_X);

    Camera_Y = Camera_Y;

    Position = load_16(Player_Data + Player_Object_Y);

    Screen_Y = (byte)sub16(Position, Camera_Y);

    // then, check if out of range
    // do this by using the high byte of X and Y

    //if(high(Camera_X) | high(Camera_Y))
    //{
    //    return; // return early because yuck we can't even see it
    //}

    place_spritechain_in_oam_buffer(Bot_Walk_Right_2, shift_right( shift_right( (byte)sizeof(Bot_Walk_Right_2) ) ), 60, (Screen_X + 8) );

    return;
}

void Move_Player(byte* Player_Data)
{
    word Player_X;

    Player_X = load_16(Player_Data);
    Player_X++;
    write_16(Player_Data, Player_X);

    return;
}

void Test_Game_Loop()
{
    byte* Player_Data;

    word Camera_X;
    word Camera_Y;

    Camera_X = 0;
    Camera_Y = 0;

    Player_Data = malloc(Player_Object_Bytecount);

    memset(Player_Data, 0, Player_Object_Bytecount);

    do
    {
        Clean_OAM_Buffer();
        Render_Player(Player_Data, Camera_X, Camera_Y);
        Wait_For_VBlank();
        dma_transfer();

        Move_Player(Player_Data);

        //write_16( (Player_Data), (0x0001 + load_16( (Player_Data) )) ); // rewrites player x position
    }while(1);

    free(Player_Data);

    return;
}
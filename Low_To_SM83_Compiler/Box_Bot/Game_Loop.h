void Clean_OAM_Buffer();
void Wait_For_VBlank();

void Render_Player(byte* Player_Data, word Camera_X, word Camera_Y);

//void Draw_Player_Sprite(byte* Player_Data, byte Screen_X, byte Screen_Y)
//{
//    byte* Spritechain;
//    byte Spritecount;
//
//    const byte Sprite_Chains[] = 
//    {
//        Bot_Walk_Right_2, high(Bot_Walk_Right_2),
//        Bot_Walk_Left_2, high(Bot_Walk_Left_2)
//    };
//
//    Spritechain = Player_Data + Player_Object_State;
//
//    Spritechain = load_16( Sprite_Chains + shift_left(*Spritechain) );
//
//    Spritecount = 9; 
//    
//    //*(Sprite_Counts + *(Player_Data + Player_Object_State));
//
//    //Spritechain = Bot_Walk_Right_2;
//    //Spritecount = shift_right( shift_right( (byte)sizeof(Bot_Walk_Right_2) ) );
//
//    //if( *((Player_Data + Player_Object_State)) )
//    //{
//    //    Spritechain = Bot_Walk_Left_2;
//    //    Spritecount = shift_right( shift_right( (byte)sizeof(Bot_Walk_Left_2) ) );
//    //}
//
//    place_spritechain_in_oam_buffer(Spritechain, Spritecount, Screen_Y, Screen_X );
//
//    return;
//}

void Render_Player(byte* Player_Data, word Camera_X, word Camera_Y)
{
    byte Screen_X;
    byte Screen_Y;

    word Position;

    Position = load_16(Player_Data);

    Camera_X = Camera_X;

    Camera_X = sub16( Position, Camera_X);

    Camera_Y = Camera_Y;

    Position = load_16(Player_Data + Player_Object_Y);

    Camera_Y = sub16(Position, Camera_Y);

    // then, check if out of range
    // do this by using the high byte of X and Y

    //if(high(Camera_X) | high(Camera_Y))
    //{
    //    return; // return early because yuck we can't even see it
    //}

    Screen_X = (byte)Camera_X;

    Camera_X = shift_left(Camera_X);

    Camera_X = abs_16(Camera_X);

    Screen_Y = (byte)Camera_Y;

    Camera_Y = shift_left(Camera_Y);

    Camera_Y = abs_16(Camera_Y);

    byte Condition;

    Condition = high(Camera_X);
    Condition = Condition | high(Camera_Y);

    if(! Condition )
    {
        //place_spritechain_in_oam_buffer(Bot_Walk_Right_2, shift_right( shift_right( (byte)sizeof(Bot_Walk_Right_2) ) ), (Screen_Y + 88), (Screen_X + 88) );
        Draw_Player_Sprite(Player_Data, Screen_X + 88, Screen_Y + 88 );
    }

    return;
}

byte Get_Controller_Inputs(byte Flag)
{
    *(CONTROLLER_REGISTER) = Flag;

    byte Value_Back;

    Value_Back = *(CONTROLLER_REGISTER);
    Value_Back = *(CONTROLLER_REGISTER);
    Value_Back = *(CONTROLLER_REGISTER);

    return Value_Back;
}

void Set_Player_State(byte* Player_Data, byte State)
{
    *(Player_Data + Player_Object_State) = State;

    return;
}

void Move_Player(byte* Player_Data)
{
    word Player_X;

    byte Inputs;

    Inputs = Get_Controller_Inputs(CONTROLLER_DPAD_FLAG);

    Player_X = load_16(Player_Data);

    if(!bit(Inputs, CONTROLLER_BUTTON_LEFT_BIT))
    {
        Player_X--;

        // *(Player_Data + Player_Object_State) = 1;
    }

    if(!bit(Inputs, CONTROLLER_BUTTON_RIGHT_BIT))
    {
        Player_X++;

        // *(Player_Data + Player_Object_State) = 0;
    }

    write_16(Player_Data, Player_X);

    return;
}


const byte Stars_Song_0[] = 
{
    0x07,
    Phrase_0, high(Phrase_0),
    Phrase_1, high(Phrase_1),
    Phrase_2, high(Phrase_2),
    0x00, 0x00
};

const byte Stars_Song_3[] =
{
    0x07,
    Phrase_3, high(Phrase_3),
    Phrase_4, high(Phrase_4),
    Phrase_4, high(Phrase_4),
    0x00, 0x00
};

void Deallocate_Songs(byte* Songs)
{
    free(load_16(Songs));
    free(load_16(Songs + 2));
    free(load_16(Songs + 4));
    free(load_16(Songs + 6));

    free(Songs);

    return;
}

byte* Get_Songs(byte* Channel_0, byte* Channel_1, byte* Wave, byte* Noise)
{
    byte* Songs;

    Songs = malloc(8); // This is enough memory for pointers to 4 song handles

    memset(Songs, 0, 8);

    write_16(Songs, Get_Song_Handle(Channel_0));
    Songs = Songs + 2;
    write_16(Songs, Get_Song_Handle(Channel_1));
    Songs = Songs + 2;
    write_16(Songs, Get_Song_Handle(Wave));
    Songs = Songs + 2;
    write_16(Songs, Get_Song_Handle(Noise));
    
    return sub16(Songs, 6);
}

void Play_Songs(byte* Songs)
{
    byte* Song_Handle;
    Song_Handle = load_16(Songs);
    if((byte)Song_Handle | high(Song_Handle))
    {
        Play_Song_Handle(Song_Handle, 0x10, 0);
    }

    Songs++;
    Songs++;

    Song_Handle = load_16(Songs);
    if((byte)Song_Handle | high(Song_Handle))
    {
        Play_Song_Handle(Song_Handle, 0x15, 0);
    }

    Songs++;
    Songs++;

    Song_Handle = load_16(Songs);
    if((byte)Song_Handle | high(Song_Handle))
    {
        Play_Song_Handle(Song_Handle, 0x1A, 0);
    }

    Songs++;
    Songs++;

    Song_Handle = load_16(Songs);
    if((byte)Song_Handle | high(Song_Handle))
    {
        Play_Song_Handle(Song_Handle, 0x1F, 0);
    }

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

    write_16( Player_Data + Player_Object_Y, 0x30);

    *(Player_Data + Player_Object_State) = 5;

    byte Game_Timer;

    Game_Timer = 0;

    //

    //byte* Song_Handle;

    //Song_Handle = Get_Song_Handle(Stars_Song_0);

    //byte* Other_Song_Handle;

    //Other_Song_Handle = Get_Song_Handle(Stars_Song_3);

    byte* Songs;

    Songs = Get_Songs(Stars_Song_0, NULL, NULL, Stars_Song_3);

    do
    {
        Clean_OAM_Buffer();
        Render_Player(Player_Data, Camera_X, Camera_Y);
        Wait_For_VBlank();
        dma_transfer();

        Move_Player(Player_Data);

        Play_Songs(Songs);

        //Play_Song_Handle(Song_Handle, 0x10, 0);
        //Play_Song_Handle(Other_Song_Handle, 0x1F, 0);

        Game_Timer++;

        if(Game_Timer > 2)          // Halves the animation speed
        {
            State_Function(Player_Data);

            Game_Timer = 0;
        }

        //write_16( (Player_Data), (0x0001 + load_16( (Player_Data) )) ); // rewrites player x position
    }while(1);

    free(Player_Data);

    Deallocate_Songs(Songs);

    //free(Song_Handle);

    //free(Other_Song_Handle);

    return;
}
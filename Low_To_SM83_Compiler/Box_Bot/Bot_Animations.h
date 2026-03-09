
// Will use state table to keep track of functions for each state

// Player object data:
//      word X              (x position)
//      word Y              (y position)
//      byte State          (keeps track of what state the player object is in)
//      byte State_Timer    (this is used for animations etc between states)
//      byte X_Vel          (Delta X)
//      byte Y_Vel          (Delta Y)
//      

#define Player_Object_X 0x00
#define Player_Object_Y 0x02
#define Player_Object_State 0x04
#define Player_Object_State_Timer 0x05
#define Player_Object_X_Vel 0x06
#define Player_Object_Y_Vel 0x07
#define Player_Object_Direction 0x08

#define Player_Object_Bytecount 0x09

// Additional things such as weapons will be handled separately...

// 1 middle and 5 turnaround

// Player State
//  - Function pointer
//  - Graphic

void State_Function(byte* Player_Data);

const byte Player_State_Table[] = 
{   
    State_Function, high(State_Function),
    Bot_Jump_Left, high(Bot_Jump_Left),

    //

    State_Function, high(State_Function),           // State = 1
    Bot_Run_Left_0, high(Bot_Run_Left_0),
    
    State_Function, high(State_Function),
    Bot_Run_Left_1, high(Bot_Run_Left_1),
    
    State_Function, high(State_Function),
    Bot_Run_Left_2, high(Bot_Run_Left_2),

    State_Function, high(State_Function),           // State = 4
    Bot_Walk_Left_2, high(Bot_Walk_Left_2),

    State_Function, high(State_Function),
    Bot_Walk_Left_1, high(Bot_Walk_Left_1),

    State_Function, high(State_Function),
    Bot_Walk_Left_0, high(Bot_Walk_Left_0),

    State_Function, high(State_Function),           // State = 7
    Bot_Idle_Left, high(Bot_Idle_Left),

    State_Function, high(State_Function),
    Bot_Turn_Left_0, high(Bot_Turn_Left_0),

    State_Function, high(State_Function),
    Bot_Turn_Left_1, high(Bot_Turn_Left_1),

    State_Function, high(State_Function),
    Bot_Turn_Left_2, high(Bot_Turn_Left_2),

    State_Function, high(State_Function),
    Bot_Turn_Left_3, high(Bot_Turn_Left_3),

    State_Function, high(State_Function),
    Bot_Turn_Left_4, high(Bot_Turn_Left_4),

    State_Function, high(State_Function),           // State = 13
    Bot_Turn_Middle, high(Bot_Turn_Middle),

    State_Function, high(State_Function),
    Bot_Turn_Right_4, high(Bot_Turn_Right_4),

    //

    State_Function, high(State_Function),
    Bot_Turn_Right_3, high(Bot_Turn_Right_3),

    //

    State_Function, high(State_Function),
    Bot_Turn_Right_2, high(Bot_Turn_Right_2),

    //

    State_Function, high(State_Function),
    Bot_Turn_Right_1, high(Bot_Turn_Right_1),

    //

    State_Function, high(State_Function),
    Bot_Turn_Right_0, high(Bot_Turn_Right_0),

    State_Function, high(State_Function),           // State = 19
    Bot_Idle_Right, high(Bot_Idle_Right),

    State_Function, high(State_Function),
    Bot_Walk_Right_0, high(Bot_Walk_Right_0),

    State_Function, high(State_Function),
    Bot_Walk_Right_1, high(Bot_Walk_Right_1),

    State_Function, high(State_Function),
    Bot_Walk_Right_2, high(Bot_Walk_Right_2),       // 22

    State_Function, high(State_Function),
    Bot_Run_Right_2, high(Bot_Run_Right_2),         

    State_Function, high(State_Function),
    Bot_Run_Right_1, high(Bot_Run_Right_1),         

    State_Function, high(State_Function),
    Bot_Run_Right_0, high(Bot_Run_Right_0),         // 25

    //

    State_Function, high(State_Function),
    Bot_Jump_Right, high(Bot_Jump_Right)
};

byte Get_Controller_Inputs(byte Flag);

void State_Function(byte* Player_Data)
{

    // This will get player inputs and, depending on the input, change the player state

    // Then, it'll render the player

    byte Inputs;
    
    Inputs = Get_Controller_Inputs(CONTROLLER_DPAD_FLAG);

    byte Condition;

    byte* State_Pointer;
    
    State_Pointer = Player_Data + Player_Object_State;

    Condition = *State_Pointer;

    if(!bit(Inputs, CONTROLLER_BUTTON_LEFT_BIT))
    {
        *(State_Pointer + 4) = 0;   // This sets the direction flag to 'left'

        if(!(Condition - 1)) // If we're looping the run-left animation?
        {
            *State_Pointer = 5;
        }

        *(State_Pointer)--; // goes backwards in animation!

        return;
    }

    if(!bit(Inputs, CONTROLLER_BUTTON_RIGHT_BIT))
    {
        *(State_Pointer + 4) = 1;   // This sets the direction flag to 'right'

        if(!(Condition - 25))
        {
            *State_Pointer = 21;
        }

        *(State_Pointer)++;

        return;
    }

    if(*(State_Pointer + 4))
    {
        // If the player is facing right, 

        // get the player close to the right idle frame

        if(*(State_Pointer) > 19)
        {
            if(*(State_Pointer) > 22)
            {
                *(State_Pointer) = 22;
            }

            *(State_Pointer)--;
            return;
        }

        if(*(State_Pointer) < 19)
        {
            *(State_Pointer)++;

            return;
        }

        return;
    }

    if(*(State_Pointer) > 7)
    {
        *(State_Pointer)--;
        return;
    }

    if(*(State_Pointer) < 7)
    {
        if(*(State_Pointer) < 4)
        {
            *(State_Pointer) = 4;
        }

        *(State_Pointer)++;
        return;
    }

    // If NEITHER left or right is pressed,

    // approach idle_left or idle_right depending on player's direction

    return;
}

void Draw_Player_Sprite(byte* Player_Data, byte Screen_X, byte Screen_Y)
{
    byte* Spritechain;
    byte Spritecount;

    //const byte Sprite_Chains[] = 
    //{
    //    Bot_Walk_Right_2, high(Bot_Walk_Right_2),
    //    Bot_Walk_Left_2, high(Bot_Walk_Left_2)
    //};

    //Spritechain = Player_Data + Player_Object_State;

    //Spritechain = load_16( Sprite_Chains + shift_left(*Spritechain) );

    Spritechain = *(Player_Data + Player_Object_State);

    Spritechain = 2 + shift_left( shift_left(Spritechain) );

    Spritechain = load_16(Player_State_Table + Spritechain);

    Spritecount = 9; 
    
    //*(Sprite_Counts + *(Player_Data + Player_Object_State));

    //Spritechain = Bot_Walk_Right_2;
    //Spritecount = shift_right( shift_right( (byte)sizeof(Bot_Walk_Right_2) ) );

    //if( *((Player_Data + Player_Object_State)) )
    //{
    //    Spritechain = Bot_Walk_Left_2;
    //    Spritecount = shift_right( shift_right( (byte)sizeof(Bot_Walk_Left_2) ) );
    //}

    place_spritechain_in_oam_buffer(Spritechain, Spritecount, Screen_Y, Screen_X );

    return;
}
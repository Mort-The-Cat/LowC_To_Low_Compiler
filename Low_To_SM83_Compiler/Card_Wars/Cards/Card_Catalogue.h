// We'll store game info in a simple structure like so:
//      00 - Player_Deck pointer (this is a list of the cards that the player has in their deck)
//      02 - Board pointer
//      04 - Controller DPAD read
//      05 - Controller SSBA read
//      06 - Controller fresh DPAD
//      07 - Controller fresh SSBA
#define Game_Info_Bytecount 0x04

#define Game_Info_Board_Pointer 0x02

#define Game_Info_DPAD_Read 04
#define Game_Info_DPAD_Fresh 06
#define Game_Info_SSBA_Read 05
#define Game_Info_SSBA_Fresh 07

// Board info:
//      00 - Player_Deck_Pile pointer (list of cards that the player has in the deck pile in-game)
//      02 - Player_Hand
//      04 - Player_Discard_Pile
//      06 - Board_Card_Data
            // 16 cards each
            // 3 bytes per board-card
//    0x36 - Board_Card_Creature_Data_Buffer
            // 16 cards
            // 2 bytes each
//    0x56 - Friendly health
            // 1 unsigned byte
//      57 - Enemy health
            // 1 unsigned byte
//      58 - Blood counter
            // 1 unsigned byte
//      59 - Bone counter
            // 1 unsigned byte
        
#define Board_Info_Player_Deck_Pile 0x00
#define Board_Info_Player_Hand 0x02
#define Board_Info_Player_Discard 0x04

#define Board_Info_Board_Card_Data 0x06
#define Board_Info_Board_Creature_Data 0x36

#define Board_Info_Friendly_Health 0x56
#define Board_Info_Enemy_Health 0x57
#define Board_Info_Blood_Counter 0x58
#define Board_Info_Bone_Counter 0x59

#define Board_Info_Bytecount 0x59

byte Get_Length(byte Terminator, byte* List); // List with a certain terminator byte 

void Init_Game_Board(byte* Game_Info)
{
    // I'll sort out the card array pointers later

    byte* Pointer;

    Pointer = malloc(Board_Info_Bytecount);

    write_16(Game_Info + Game_Info_Board_Pointer, Pointer);

    memset(Pointer + Board_Info_Board_Card_Data, 0xFF, 48);   // sets all board spaces as 'blank'

    // Now, init all of the card/token/health info

    byte* New_List;

    byte Length;

    Pointer = load_16(Game_Info);           // Player deck

    Length = Get_Length(0xFF, Pointer);
    Length++;

    New_List = malloc((word)Length);
    memcpy(New_List, Pointer, (word)Length);    // needs to copy over corresponding cards

    Pointer = load_16(Game_Info + Game_Info_Board_Pointer);    // board info
    write_16(Pointer, New_List);            // writes player deck

    New_List = malloc(1);                   // only terminator byte
    *New_List = 0xFF;                       // empty list

    write_16(Pointer + Board_Info_Player_Hand, New_List); // writes player hand

    New_List = malloc(1);
    *New_List = 0xFF;

    write_16(Pointer + Board_Info_Player_Discard, New_List); // writes player discard

    const byte Initial_Health_Blood_Bone_Values[] =
    {
        0x0F, 0x0F, 0x00, 0x00
    };

    memcpy(Pointer + Board_Info_Friendly_Health, Initial_Health_Blood_Bone_Values, sizeof(Initial_Health_Blood_Bone_Values));

    return;
}

void Delete_Game_Board(byte* Game_Info)
{
    // card array pointers

    byte* Pointer;

    Pointer = load_16(Game_Info + Game_Info_Board_Pointer);

    byte Count;
    Count = 1;
    do
    {
        free( load_16(Pointer + (word)Count) );       // Deletes all cards on the board
        Count = Count + 3;
    }while(Count < 48);

    free( Pointer );

    free( Pointer + Board_Info_Player_Deck_Pile );
    free( Pointer + Board_Info_Player_Hand );
    free( Pointer + Board_Info_Player_Discard );

    // next, delete the deck, hand, and discard lists

    return;
}

void Delete_Game_Info(byte* Game_Info)
{
    free(load_16(Game_Info));

    free(Game_Info);

    return;
}

#define ID_Skeleton 0x00
#define ID_Pig 0x01
#define ID_Bald_Man 0x02
#define ID_Mage 0x03
#define ID_Ancient_Scholar 0x04
#define ID_Strawman 0x05
#define ID_Coffin 0x06
#define ID_Starchy 0x07
#define ID_Sir_Slicer 0x08
#define ID_Enchiridion 0x09
#define ID_Husk_Knight 0x0A
#define ID_Flame_Guy 0x0B
#define ID_Guard 0x0C
#define ID_Maize_God 0x0D

void Init_Game_Info(byte* Game_Info)
{
    const byte Player_Starting_Deck[] =
    {
        //ID_Skeleton,
        //ID_Skeleton,
        //ID_Pig,
        //ID_Pig,
        //ID_Bald_Man,
        //ID_Bald_Man,
        //ID_Mage,
        //ID_Mage,
        //ID_Ancient_Scholar,
        //ID_Strawman,
        //ID_Strawman,
        //ID_Starchy,
        //ID_Starchy,
        //ID_Sir_Slicer,
        //ID_Enchiridion,
        //0xFF

        //ID_Pig,
        //ID_Pig,
        //ID_Bald_Man,
        //ID_Bald_Man,
        //ID_Mage,
        //ID_Mage,
        //ID_Ancient_Scholar,
        //ID_Starchy,
        //ID_Starchy,
        //ID_Sir_Slicer,
        //ID_Skeleton,
        //ID_Skeleton,

        ID_Pig,
        ID_Pig,
        ID_Bald_Man,
        ID_Bald_Man,
        ID_Skeleton,
        ID_Skeleton,
        ID_Flame_Guy,
        ID_Husk_Knight,
        ID_Guard,
        ID_Maize_God,
        0xFF
    };

    byte* Data;
    Data = malloc(sizeof(Player_Starting_Deck));
    write_16(Game_Info, Data);

    memcpy(Data, Player_Starting_Deck, sizeof(Player_Starting_Deck));

    write_16(Game_Info + 2, 0x0000);

    return;
}

const byte Card_Catalogue[] =
{
    Skeleton_Card_Data, high(Skeleton_Card_Data),
    Pig_Card_Data, high(Pig_Card_Data),
    Bald_Man_Card_Data, high(Bald_Man_Card_Data),
    Mage_Card_Data, high(Mage_Card_Data),
    Ancient_Scholar_Card_Data, high(Ancient_Scholar_Card_Data),
    Strawman_Card_Data, high(Strawman_Card_Data),
    Coffin_Card_Data, high(Coffin_Card_Data),
    Starchy_Card_Data, high(Starchy_Card_Data),
    Sir_Slicer_Card_Data, high(Sir_Slicer_Card_Data),
    Enchidirion_Card_Data, high(Enchidirion_Card_Data),
    Husker_Knight_Card_Data, high(Husker_Knight_Card_Data),
    Flame_Guy_Card_Data, high(Flame_Guy_Card_Data),
    Guardian_Card_Data, high(Guardian_Card_Data),
    Maize_God_Card_Data, high(Maize_God_Card_Data),

    0x00, 0x00
};


// This will be where the card ID's are derived from
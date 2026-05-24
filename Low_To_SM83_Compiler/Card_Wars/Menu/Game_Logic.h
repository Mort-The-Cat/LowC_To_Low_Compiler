

// this will initialise a game and then start the player's turn

void Draw_Random_Card(byte* Board_Info)
{
    // checks first if the player deck has any cards in it

    byte* Deck;

    byte Length;

    Deck = load_16(Board_Info);         // player draw deck

    Length = Get_Length(0xFF, Deck);

    if(!Length)
    {
        return;                         // can't draw. Do nothing
    }

    byte Index;

    Index = random();
    Index = high(divide_8(Index, Length));  // make sure the value is within 0 and the length of the list

    byte* Hand;

    Length = *(Deck + (word)Index);   // get card id at that point

    Add_To_List(Length, Board_Info + Board_Info_Player_Hand);   // adds that card to the player hand

    Remove_From_List(Index, Board_Info);    // removes from player draw deck

    // Otherwise? Continue!

    return;
}

void Draw_Husk_Card(byte* Board_Info)
{
    Add_To_List(ID_Strawman, Board_Info + Board_Info_Player_Hand);

    return;
}

void Start_Card_Wars_Match_Draw_Player_Cards(byte* Board_Info)
{
    // draws random card from deck

    Draw_Husk_Card(Board_Info);

    byte Counter;

    Counter = 13;

    do
    {
        Draw_Random_Card(Board_Info);
        Counter--;
    }while(Counter);

    return;
}

byte Game_Round(byte* Game_Info)    // returns the state of the game- if someone has won or not
{
    // this handles the logic for a round of CW

    

    return 0x00;
}

void Start_Card_Wars_Match(byte* Game_Info)
{
    Init_Game_Board(Game_Info);

    // This will load the 'table-scene' and prompt the user for input

    byte Continue_Game;

    Continue_Game = 0;

    do
    {
        Continue_Game = Game_Round(Game_Info);
    }while(Continue_Game);

    return;
}
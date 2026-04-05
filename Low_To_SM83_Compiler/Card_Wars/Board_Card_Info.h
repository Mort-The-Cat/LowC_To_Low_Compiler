// When placing a creature card, it creates a copy of its const object_pointer data

// The board object is responsible for the allocation/deallocation of this object, NOT the deck etc

// The deck etc will store pointers to the const values
// Only the creature cards in play are copies

//



// Board structure:
//      00 - card id
//      01 - creature data pointer

void Draw_Card_Graphics_Tilemap(byte* Location, byte Value);

byte* Position_To_Tileset_Value(byte* Destinations, byte Position)
{
    byte* Address;
    Address = (word)Position;
    Address = Destinations + Address;
    Address = *Address;
    Address = shift_left(shift_left(shift_left(shift_left(Address))));
    Address = Address + 0x8800;
    return Address;
}

void Place_Card_On_Board(byte* Game_Info, byte Position, byte Card_ID)
{
    Position = Position + Position + Position;

    byte* Pointer;
    Pointer = load_16(Game_Info + (word)Game_Info_Board_Pointer);
    Pointer = Pointer + Board_Info_Board_Card_Data;

    Pointer = Pointer + (word)Position;

    *(Pointer) = Card_ID;

    Pointer++;

    //Card_ID = shift_left(Card_ID);

    byte* Card_Pointer;

    Card_Pointer = Card_Catalogue;

    Card_Pointer = load_16(Card_Pointer + shift_left((word)Card_ID));   // The card pointer
    Card_Pointer++;
    Card_Pointer = load_16( Card_Pointer );                             // The creature pointer

    byte* Copy_Pointer;

    if((byte)Card_Pointer | high(Card_Pointer))
    {
        // if it's an actual object? write it

        Game_Info = load_16(Game_Info + Game_Info_Board_Pointer);
        Game_Info = Game_Info + Board_Info_Board_Creature_Data;
        Copy_Pointer = Game_Info + shift_left((word)Position);

        memcpy(Copy_Pointer, Card_Pointer, 2);

        Card_Pointer = Copy_Pointer;
    }

    // Copy_Pointer = malloc(2); // allocates 2 bytes for the object data (if necessary)

    write_16(Pointer, Card_Pointer);                                    // simply copies creature pointer to the board

    // For now, this doesn't create a copy of the data

    // it'll just point to the original address

    return;
}

void Draw_Board_Creature_Card(byte Position, byte* Card_Data, byte* Creature_Data);

const byte Tilemap_Destinations[] =
{
    0x00, 0x98,
    0x06, 0x98,
    0x0C, 0x98,
    0x12, 0x98,

    0x00, 0x99,
    0x06, 0x99,
    0x0C, 0x99,
    0x12, 0x99,

    0x00, 0x9A,
    0x06, 0x9A,
    0x0C, 0x9A,
    0x12, 0x9A,

    0x00, 0x9B,
    0x06, 0x9B,
    0x0C, 0x9B,
    0x12, 0x9B
};

void Draw_Empty_Card_Space(byte Position)
{
    byte* Destination;

    Destination = Tilemap_Destinations + (word)Position;
    Destination = load_16(Destination);

    Copy_Tilemap(Destination, No_Card_Tilemap_Data, sizeof(No_Card_Tilemap_Data), No_Card_Tilemap_Width);

    return;
}

void Draw_Board_Cards_Iteration(byte* Pointer, byte Card_ID, byte Count)
{
    byte* Card_Data;

    if( Card_ID < 255 ) // If it's a valid card? draw it
    {
        Card_Data = load_16(Card_Catalogue + shift_left((word)Card_ID));

        Draw_Board_Creature_Card(Count, Card_Data, load_16(Pointer));
        return;
    }

    Draw_Empty_Card_Space(shift_left(Count));

    return;
}

void Draw_Board_Cards(byte* Game_Info)
{
    byte Count;

    byte* Pointer;

    Pointer = load_16(Game_Info + (word)Game_Info_Board_Pointer);//
    Pointer = Pointer + Board_Info_Board_Card_Data;

    byte Card_ID;

    Count = 0;

    do
    {
        Card_ID = *Pointer;
        Pointer++;

        // Draw_Empty_Card_Space(shift_left(Count));

        Draw_Board_Cards_Iteration(Pointer, Card_ID, Count);

        // Otherwise? Don't worry about it

        Pointer++;
        Pointer++;
        Count++;
    }while(Count < 16);

    return;
}

void Draw_Board_Creature_Card(byte Position, byte* Card_Data, byte* Creature_Data)   // 16 possible positions to place the card
{
    const byte Tileset_Destinations[] =
    {
        0x36, 0x42, 0x4E, 0x5A,
        0x66, 0x72, 0x80, 0x8C,
        0x98, 0xA4, 0xB0, 0xBC,
        0xC8, 0xD4, 0xE0, 0xEC
    };

    byte* Destination;

    byte* Number;

    Destination = Position_To_Tileset_Value(Tileset_Destinations, Position);

    Number = load_16(Card_Data + 9);

    memcpy(Destination, Number, 192);

    // This will get the graphics data
    // the HP/Attack
    // the cost perhaps

    Destination = Tilemap_Destinations + (word)shift_left(Position);

    //Position = *(Tileset_Destinations + (word)Position);

    Destination = load_16(Destination);

    Copy_Tilemap(Destination, Card_Tilemap_Data, sizeof(Card_Tilemap_Data), Card_Tilemap_Width);

    Number = Split_Byte(*Card_Data);            // Card cost
    
    if((byte)Number)                            // if there's no cost, don't display it
    {
        Number = Int_To_BCD( shift_right((byte)Number) );
        *(Destination + 35) = 0x80 | high(Number);
        *(Destination + 36) = 0x80 | (byte)Number;
        *(Destination + 34) = 0xA1;     // x symbol
        *(Destination + 33) = 0x78 | ((*Card_Data) & 0x01);
    }

    Destination = Destination + 65;
    Draw_Card_Graphics_Tilemap(Destination, *(Tileset_Destinations + (word)Position) );

    // Here, we'll check if this is a building or a creature.

    Number = Creature_Data; //load_16( Card_Data + 1);   // Gets attack/floop data from creature

    if(! ((byte)Number | high(Number))) // if 'creature data' is NULL, don't render it
    {
        return;
    }

    Destination = Destination + 97;

    Number = Split_Byte( *(Number + 1) ); // Splits packed attack/floop data

    Number = Int_To_BCD( high(Number) );// converts the attack value to a BCD

    //Number = Int_To_BCD(*(load_16(Card_Data + (word)1) + (word)1));

    *Destination = 0x80 | high(Number);
    Destination++;
    *Destination = 0x80 | (byte)Number;
    Destination = Destination + 31;

    Number = Creature_Data; //load_16( Card_Data + 1);   // HP

    Number = Int_To_BCD(0x3F & (*Number));

    // Number = Int_To_BCD(*load_16(Card_Data + (word)1));

    *Destination = 0x80 | high(Number);
    Destination++;
    *Destination = 0x80 | (byte)Number;
    Destination++;

    Number = Creature_Data; //load_16( Card_Data + 1 );
    
    byte Movement_Flag;

    Movement_Flag = b11000000 & (*Number);

    // b10 is rock          (0x7C)
    // b11 is move left     (0x7A)
    // b00 is normal        (0xFF)
    // b01 is move right    (0x7B)

    if(bit(Movement_Flag, 6)) // some kind of movement sigil
    {
        if(bit(Movement_Flag, 7))
        {
            *Destination = 0x7A;    // move left
            return;
        }

        *Destination = 0x7B;        // move right
        return;
    }

    if(bit(Movement_Flag, 7))
    {
        *Destination = 0x7C;        // rock
    }

    return;
}
// When placing a creature card, it creates a copy of its const object_pointer data

// The board object is responsible for the allocation/deallocation of this object, NOT the deck etc

// The deck etc will store pointers to the const values
// Only the creature cards in play are copies

void Draw_Card_Graphics_Tilemap(byte* Location, byte Value);

byte* Position_To_Tileset_Value(byte* Destinations, byte Position)
{
    byte* Address;
    Address = (word)Position;
    Address = Destinations + Address;
    Address = *Address;
    Address = shift_left(shift_left(shift_left(shift_left(Address))));
    Address = Address + 0x8000;
    return Address;
}

void Draw_Board_Creature_Card(byte Position, byte* Card_Data)   // 16 possible positions to place the card
{

    const byte Tileset_Destinations[] = // tile ID
    {
        0x00, 0x0C, 0x18, 0x24,
        0x30, 0x3C, 0x48, 0x54,
        0x60, 0x6C, 0xB6, 0xC2,
        0xCE, 0xDA, 0xE6, 0xF2
    }; // $8B60 is start of second tilemap (tile $B6)

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
    Number = Int_To_BCD( shift_right((byte)Number) );
    *(Destination + 35) = 0x80 | high(Number);
    *(Destination + 36) = 0x80 | (byte)Number;
    *(Destination + 34) = 0xA1;     // x symbol
    *(Destination + 33) = 0x78 | ((*Card_Data) & 0x01);

    Destination = Destination + 65;
    Draw_Card_Graphics_Tilemap(Destination, *(Tileset_Destinations + (word)Position) );

    Destination = Destination + 97;

    Number = load_16( Card_Data + 1);   // Gets attack/floop data from creature

    Number = Split_Byte( *(Number + 1) ); // Splits packed attack/floop data

    Number = Int_To_BCD( high(Number) );// converts the attack value to a BCD

    //Number = Int_To_BCD(*(load_16(Card_Data + (word)1) + (word)1));

    *Destination = 0x80 | high(Number);
    Destination++;
    *Destination = 0x80 | (byte)Number;
    Destination = Destination + 31;

    Number = load_16( Card_Data + 1);   // HP

    Number = Int_To_BCD(0x3F & (*Number));

    // Number = Int_To_BCD(*load_16(Card_Data + (word)1));

    *Destination = 0x80 | high(Number);
    Destination++;
    *Destination = 0x80 | (byte)Number;
    Destination++;

    Number = load_16( Card_Data + 1 );
    
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
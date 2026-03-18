// class 'Creature Card Data' ()
//      HP      : 8-bit signed integre
//      ATK     : 4-bit unsigned integre        (Won't be BCD, regular integre)
//      Flp Cost: 3-bit unsigned integre
//      Flipped : 1-bit flag                    (if flipped already)

// class Card (10 bytes long)
//      00  Element_Type    : 4-bit integre     (corn, sky, swamp, etc)
//      00  Cost            : 4-bit unsigned integre

//      01  Object_Pointer  : 16-bit pointer    (pointer to the creature object (if applicable) )
//              if Object_Pointer is NULL, this isn't a creature
//      03  Function        : 16-bit pointer    (pointer to the function that handles floop/spell/building/attack/etc etc)
//      05  Name            : 16-bit pointer to string
//      07  Description     : 16-bit pointer to string - 
//              Explains what the card does (i.e. building function, spell function, or floop ability)
//      09  Graphics        : 16-bit pointer to tileset (i.e. what tiles the card uses for its graphics)

const byte Card_Name[] = "CARDTEST!";
const byte Card_Description[] = "PLACEHOLDER TEXT";

const byte Placeholder_Creature_Data[] =
{
    0x10,
    0x52
};

const byte Placeholder_Card_Data[] = 
{
    0x01,       // Element type and cost
    Placeholder_Creature_Data, high(Placeholder_Creature_Data), // no object pointer
    0x00, 0x00, // no function pointer yet
    Card_Name, high(Card_Name),
    Card_Description, high(Card_Description),
    Skeleton_Graphics_Data, high(Skeleton_Graphics_Data)
};

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

    memcpy(Destination, Skeleton_Graphics_Data, 192);

    // This will get the graphics data
    // the HP/Attack
    // the cost perhaps

    Destination = Tilemap_Destinations + ((word)shift_left(Position));

    //Position = *(Tileset_Destinations + (word)Position);

    Destination = load_16(Destination);

    Copy_Tilemap(Destination, Card_Tilemap_Data, sizeof(Card_Tilemap_Data), Card_Tilemap_Width);

    Number = Split_Byte(*Card_Data);
    Number = Int_To_BCD( (byte)Number );
    *(Destination + 34) = 0x80 | high(Number);
    *(Destination + 35) = 0x80 | (byte)Number;

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

    Number = load_16( Card_Data + 1);

    Number = Int_To_BCD(*Number);

    // Number = Int_To_BCD(*load_16(Card_Data + (word)1));

    *Destination = 0x80 | high(Number);
    Destination++;
    *Destination = 0x80 | (byte)Number;

    return;
}
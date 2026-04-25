// class 'Creature Card Data' ()
//      Movement: 2-bit signed value            (-2, -1, 0, 1)
//      HP      : 6-bit signed integre          (31 health is max)
//      ATK     : 4-bit unsigned integre        (Won't be BCD, regular integre)
//      Flooped : 1-bit flag                    (if flooped already)
//      Flp Cost: 3-bit unsigned integre


// class Card (10 bytes long)
//      00  Element_Type    : 3-bit integre     (corn, sky, swamp, etc)
//      00  Building/Spell  : 1-bit             (if Object_Pointer is NULL, this states whether or not this is a building (0) or a spell (1))
//      00  Cost            : 3-bit unsigned integre
//      00  Blood/Bone      : 1-bit bool        (false = blood, true = bones)

//      01  Object_Pointer  : 16-bit pointer    (pointer to the creature object (if applicable) )
//              if Object_Pointer is NULL, this isn't a creature
//      03  Function        : 16-bit pointer    (pointer to the function that handles floop/spell/building/attack/etc etc)
//      05  Name            : 16-bit pointer to string
//      07  Description     : 16-bit pointer to string - 
//              Explains what the card does (i.e. building function, spell function, or floop ability)
//      09  Graphics        : 16-bit pointer to tileset (i.e. what tiles the card uses for its graphics)
#define Card_Data_Name 05

//

#define Blood_Char "\x78"
#define Skull_Char "\x79"
#define Left_Char "\x7A"
#define Right_Char "\x7B"
#define Stone_Char "\x7C"

const byte Enchiridion_Name[] = "ENCHIRIDION";
const byte Enchiridion_Description[] = "DISCARD YOUR HAND" Newline "AND DRAW 7 CARDS";
const byte Enchidirion_Card_Data[] =
{
    0x10,           // is a spell and costs nothing
    0x00, 0x00,
    0x00, 0x00,     // will add function later
    Enchiridion_Name, high(Enchiridion_Name),
    Enchiridion_Description, high(Enchiridion_Description),
    Enchiridion_Graphics_Data, high(Enchiridion_Graphics_Data)
};


const byte Sir_Slicer_Name[] = "SIR-SLICER";
const byte Sir_Slicer_Description[] = "THIS KNIGHT WILL" Newline "MOVE " Left_Char " OR " Right_Char " AT THE" Newline "END OF YOUR TURN";
//"MOVES " Left_Char " OR " Right_Char " AT" Newline "THE END OF YOUR" Newline "TURN.";
const byte Sir_Slicer_Creature_Data[] =
{
    0x47,
    0x30
};
const byte Sir_Slicer_Card_Data[] =
{
    0x06,
    Sir_Slicer_Creature_Data, high(Sir_Slicer_Creature_Data),
    0x00, 0x00,
    Sir_Slicer_Name, high(Sir_Slicer_Name),
    Sir_Slicer_Description, high(Sir_Slicer_Description),
    Sir_Slicer_Graphics_Data, high(Sir_Slicer_Graphics_Data)
};


const byte Starchy_Name[] = "STARCHY!";
const byte Starchy_Description[] = "COLLECTS 1 " Skull_Char " AT" Newline "THE END OF YOUR" Newline "TURN.";
const byte Starchy_Creature_Data[] =
{
    0x02,
    0x10
};
const byte Starchy_Card_Data[] =
{
    0x02,
    Starchy_Creature_Data, high(Starchy_Creature_Data),
    0x00, 0x00,
    Starchy_Name, high(Starchy_Name),
    Starchy_Description, high(Starchy_Description),
    Starchy_Graphics_Data, high(Starchy_Graphics_Data)
};


const byte Coffin_Name[] = "COFFIN";    // This DOESN'T have creature data because it's a card
const byte Coffin_Description[] = "THIS BUILDING WILL" Newline "COLLECT 1 " Skull_Char " AT THE" Newline "END OF YOUR TURN.";
const byte Coffin_Card_Data[] =
{
    0x03,                               // high nibble LSB is 0 because it's a building (see class Card definition)
    0x00, 0x00,                          // no creature data (it's a building)
    0x00, 0x00,
    Coffin_Name, high(Coffin_Name),
    Coffin_Description, high(Coffin_Description),
    Coffin_Graphics_Data, high(Coffin_Graphics_Data)
};

const byte Strawman_Name[] = "STRAWMAN";
const byte Strawman_Description[] = "CAN BE PLACED FOR" Newline "FREE.";
const byte Strawman_Creature_Data[] =
{
    0x01,
    0x00
};
const byte Strawman_Card_Data[] =
{
    0x00,
    Strawman_Creature_Data, high(Strawman_Creature_Data),
    0x00, 0x00,
    Strawman_Name, high(Strawman_Name),
    Strawman_Description, high(Strawman_Description),
    Strawman_Graphics_Data, high(Strawman_Graphics_Data)
};

const byte Skeleton_Name[] = "SKELETON";
const byte Skeleton_Description[] = "AN UNDEAD PILE OF" Newline "BONES.REQUIRES" Newline "ONLY 1 " Skull_Char " TO PLAY.";

const byte Skeleton_Creature_Data[] =
{
    0x01,
    0x11
};

const byte Skeleton_Card_Data[] = 
{
    0x03,       // Element type and cost
    Skeleton_Creature_Data, high(Skeleton_Creature_Data), // no object pointer
    0x00, 0x00, // no function pointer yet
    Skeleton_Name, high(Skeleton_Name),
    Skeleton_Description, high(Skeleton_Description),
    Skeleton_Graphics_Data, high(Skeleton_Graphics_Data)
};

//

const byte Ancient_Scholar_Creature_Data[] =
{
    0x07,
    0x32
};
const byte Ancient_Scholar_Name[] = "ARCH SCHOLAR";
const byte Ancient_Scholar_Description[] = "EVERY TURN,SEARCH" Newline "DISCARD PILE FOR A" Newline "CARD,AND DRAW IT.";
const byte Ancient_Scholar_Card_Data[] = 
{
    0x06,       // Element type and cost
    Ancient_Scholar_Creature_Data, high(Ancient_Scholar_Creature_Data), // no object pointer
    0x00, 0x00, // no function pointer yet
    Ancient_Scholar_Name, high(Ancient_Scholar_Name),
    Ancient_Scholar_Description, high(Ancient_Scholar_Description),
    Ancient_Scholar_Graphics_Data, high(Ancient_Scholar_Graphics_Data)
};

//

const byte Mage_Creature_Data[] =
{
    0x03,
    0x52
};
const byte Mage_Name[] = "MAGE";
const byte Mage_Description[] = "AN APPRENTICE FROM" Newline "WIZARD CITY.HE IS" Newline "STILL LEARNING.";
const byte Mage_Card_Data[] = 
{
    0x04,       // Element type and cost
    Mage_Creature_Data, high(Mage_Creature_Data), // no object pointer
    0x00, 0x00, // no function pointer yet
    Mage_Name, high(Mage_Name),
    Mage_Description, high(Mage_Description),
    Mage_Graphics_Data, high(Mage_Graphics_Data)
};

//

const byte Pig_Creature_Data[] =
{
    0x06,
    0x11
};
const byte Pig_Name[] = "PIG";
const byte Pig_Description[] = "OINK OINK.";
const byte Pig_Card_Data[] = 
{
    0x02,       // Element type and cost
    Pig_Creature_Data, high(Pig_Creature_Data), // no object pointer
    0x00, 0x00, // no function pointer yet
    Pig_Name, high(Pig_Name),
    Pig_Description, high(Pig_Description),
    The_Pig_Graphics_Data, high(The_Pig_Graphics_Data)
};

//

const byte Bald_Man_Creature_Data[] =
{
    0x45,
    0x20
};
const byte Bald_Man_Name[] = "BALD MAN";
const byte Bald_Man_Description[] = "THIS GUY WANDERS " Newline Left_Char " OR " Right_Char " AT THE END" Newline "OF YOUR TURN."; 
const byte Bald_Man_Card_Data[] =
{
    0x02,
    Bald_Man_Creature_Data, high(Bald_Man_Creature_Data),
    0x00, 0x00,
    Bald_Man_Name, high(Bald_Man_Name),
    Bald_Man_Description, high(Bald_Man_Description),
    Bald_Man_Graphics_Data, high(Bald_Man_Graphics_Data)
};
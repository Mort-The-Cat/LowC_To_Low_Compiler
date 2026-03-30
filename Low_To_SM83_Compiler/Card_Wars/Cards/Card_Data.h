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


// 



const byte Skeleton_Name[] = "SKELETON";
const byte Skeleton_Description[] = "PLACEHOLDER TEXT";

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
const byte Ancient_Scholar_Name[] = "ANCIENT SCHOLAR";
const byte Ancient_Scholar_Card_Data[] = 
{
    0x06,       // Element type and cost
    Ancient_Scholar_Creature_Data, high(Ancient_Scholar_Creature_Data), // no object pointer
    0x00, 0x00, // no function pointer yet
    Ancient_Scholar_Name, high(Ancient_Scholar_Name),
    Skeleton_Description, high(Skeleton_Description),
    Ancient_Scholar_Graphics_Data, high(Ancient_Scholar_Graphics_Data)
};

//

const byte Mage_Creature_Data[] =
{
    0x05,
    0x52
};
const byte Mage_Name[] = "MAGE";
const byte Mage_Card_Data[] = 
{
    0x04,       // Element type and cost
    Mage_Creature_Data, high(Mage_Creature_Data), // no object pointer
    0x00, 0x00, // no function pointer yet
    Mage_Name, high(Mage_Name),
    Skeleton_Description, high(Skeleton_Description),
    Mage_Graphics_Data, high(Mage_Graphics_Data)
};

//

const byte Pig_Creature_Data[] =
{
    0x06,
    0x11
};
const byte Pig_Name[] = "PIG";
const byte Pig_Card_Data[] = 
{
    0x02,       // Element type and cost
    Pig_Creature_Data, high(Pig_Creature_Data), // no object pointer
    0x00, 0x00, // no function pointer yet
    Pig_Name, high(Pig_Name),
    Skeleton_Description, high(Skeleton_Description),
    The_Pig_Graphics_Data, high(The_Pig_Graphics_Data)
};

//

const byte Bald_Man_Creature_Data[] =
{
    0x45,
    0x20
};
const byte Bald_Man_Name[] = "BALD MAN";
const byte Bald_Man_Card_Data[] =
{
    0x02,
    Bald_Man_Creature_Data, high(Bald_Man_Creature_Data),
    0x00, 0x00,
    Bald_Man_Name, high(Bald_Man_Name),
    Skeleton_Description, high(Skeleton_Description),
    Bald_Man_Graphics_Data, high(Bald_Man_Graphics_Data)
};
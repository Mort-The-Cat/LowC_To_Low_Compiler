// class 'Creature Card Data' ()
//      HP      : 8-bit signed integre
//      ATK     : 4-bit unsigned integre        (Won't be BCD, regular integre)
//      Flp Cost: 3-bit unsigned integre
//      Flipped : 1-bit flag                    (if flipped already)

// class Card (10 bytes long)
//      Element_Type    : 4-bit integre     (corn, sky, swamp, etc)
//      Cost            : 4-bit unsigned integre
//      Object_Pointer  : 16-bit pointer    (pointer to the creature object (if applicable) )
//              if Object_Pointer is NULL, this isn't a creature
//      Function        : 16-bit pointer    (pointer to the function that handles floop/spell/building/attack/etc etc)
//      Name            : 16-bit pointer to string
//      Description     : 16-bit pointer to string - 
//              Explains what the card does (i.e. building function, spell function, or floop ability)
//      Graphics        : 16-bit pointer to tileset (i.e. what tiles the card uses for its graphics)

// When placing a creature card, it creates a copy of its const object_pointer data

// The board object is responsible for the allocation/deallocation of this object, NOT the deck etc

// The deck etc will store pointers to the const values
// Only the creature cards in play are copies
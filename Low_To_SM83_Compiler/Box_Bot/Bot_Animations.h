
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

#define Player_Object_Bytecount 0x08

// Additional things such as weapons will be handled separately...
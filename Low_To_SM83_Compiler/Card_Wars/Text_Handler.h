#define Newline "\x0A"

void Write_Text_Tiles(byte* Destination, byte* String);

void Draw_Tilemap_Menu(byte* Address, byte Width, byte Height);

void Add_To_List(byte Value, byte* List_Pointer);           // Pointer to the list, not the list itself
void Remove_From_List(byte Index, byte* List_Pointer);      // Pointer to the list, not the list itself
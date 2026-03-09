#define Table_Near_Scroll 0xFF90
#define Table_Far_Scroll 0xFF91
#define Table_Delta_Scroll 0xFF92
#define Table_Denom_Scroll 0xFF93

#define Table_Denom_Default 40

#define Table_Scroll_Register 0xFF90
                            //0xFF91 as well
    // top is 64 pixels             8 tiles not including sides i.e. 10 tiles
    // table is 48 pixels tall      6 tiles
    // bottom is 144 pixels         18 tiles total

    // 5/3 gradient

    // 
    // for each entry in the table, it specifies
    // x scroll
    // y scroll
    // palette (maybe)

    // 

//



void Table_Scroll_HBlank_Function();


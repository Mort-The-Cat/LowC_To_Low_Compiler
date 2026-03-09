class Note
    {

        // Value is the frequency value
        // which we want to feet the gameboy APU
        // in order to play this note
        constructor(Name, Value)
        {
            this.Name = Name;
            this.Value = Value;
        }
    }

const Notes = 
[
    new Note("", 0x00),
    new Note("C-3", 44      ),
    new Note("C#3", 157     ),
    new Note("D-3", 263     ),
    new Note("D#3", 363     ),
    new Note("E-3", 457     ),
    new Note("F-3", 547     ),
    new Note("F#3", 631     ),
    new Note("G-3", 711     ),
    new Note("G#3", 786     ),
    new Note("A-3", 856     ),
    new Note("A#3", 923     ),
    new Note("B-3", 986     ),
    new Note("C-4", 1046    ),
    new Note("C#4", 1102    ),
    new Note("D-4", 1155    ),
    new Note("D#4", 1205    ),
    new Note("E-4", 1253    ),
    new Note("F-4", 1297    ),
    new Note("F#4", 1339    ),
    new Note("G-4", 1379    ),
    new Note("G#4", 1417    ),
    new Note("A-4", 1452    ),
    new Note("A#4", 1486    ),
    new Note("B-4", 1517    ),
    new Note("C-5", 1547    ),
    new Note("C#5", 1575    ),
    new Note("D-5", 1602    ),
    new Note("D#5", 1627    ),
    new Note("E-5", 1650    ),
    new Note("F-5", 1673    ),
    new Note("F#5", 1694    ),
    new Note("G-5", 1714    ),
    new Note("G#5", 1732    ),
    new Note("A-5", 1750    ),
    new Note("A#5", 1767    ),
    new Note("B-5", 1783    ),
    new Note("C-6", 1798    ),
    new Note("C#6", 1812    ),
    new Note("D-6", 1825    ),
    new Note("D#6", 1837    ),
    new Note("E-6", 1849    ),
    new Note("F-6", 1860    ),
    new Note("F#6", 1871    ),
    new Note("G-6", 1881    ),
    new Note("G#6", 1890    ),
    new Note("A-6", 1899    ),
    new Note("A#6", 1907    ),
    new Note("B-6", 1915    ),
    new Note("C-7", 1923    ),
    new Note("C#7", 1930    ),
    new Note("D-7", 1936    ),
    new Note("D#7", 1943    ),
    new Note("E-7", 1949    ),
    new Note("F-7", 1954    ),
    new Note("F#7", 1959    ),
    new Note("G-7", 1964    ),
    new Note("G#7", 1969    ),
    new Note("A-7", 1974    ),
    new Note("A#7", 1978    ),
    new Note("B-7", 1982    ),
    new Note("C-8", 1985    ),
    new Note("C#8", 1989    ),
    new Note("D-8", 1992    ),
    new Note("D#8", 1995    ),
    new Note("E-8", 1998    ),
    new Note("F-8", 2001    ),
    new Note("F#8", 2004    ),
    new Note("G-8", 2006    ),
    new Note("G#8", 2009    ),
    new Note("A-8", 2011    ),
    new Note("A#8", 2013    ),
    new Note("B-8", 2015    ),
    new Note("C-9", 2017    ),
    new Note("C#9", 2018    ),
    new Note("D-9", 2020    ),
    new Note("D#9", 2022    ),
    new Note("E-9", 2023    ),
    new Note("F-9", 2025    ),
    new Note("F#9", 2026    ),
    new Note("G-9", 2027    ),
    new Note("G#9", 2028    ),
    new Note("A-9", 2029    ),
    new Note("A#9", 2030    ),
    new Note("B-9", 2031    ),
    new Note("C-10", 2032   ),
    new Note("C#10", 2033   ),
    new Note("D-10", 2034   ),
    new Note("D#10", 2035   ),
    new Note("E-10", 2036   ),
    new Note("F-10", 2036   ),
    new Note("F#10", 2037   ),
    new Note("G-10", 2038   )
];

const Noise_Notes = 
[
    new Note("", 0),
    new Note("0", 247), // f = 1.142857 Hz
    new Note("1", 246), // f = 1.333333 Hz
    new Note("2", 245), // f = 1.600000 Hz
    new Note("3", 244), // f = 2.000000 Hz
    new Note("4", 231), // f = 2.285714 Hz
    new Note("5", 230), // f = 2.666667 Hz
    new Note("6", 229), // f = 3.200000 Hz
    new Note("7", 228), // f = 4.000000 Hz
    new Note("8", 215), // f = 4.571429 Hz
    new Note("9", 214), // f = 5.333333 Hz
    new Note("10", 213), // f = 6.400000 Hz
    new Note("11", 212), // f = 8.000000 Hz
    new Note("12", 199), // f = 9.142858 Hz
    new Note("13", 198), // f = 10.666667 Hz
    new Note("14", 197), // f = 12.800000 Hz
    new Note("15", 196), // f = 16.000000 Hz
    new Note("16", 183), // f = 18.285715 Hz
    new Note("17", 182), // f = 21.333334 Hz
    new Note("18", 181), // f = 25.600000 Hz
    new Note("19", 180), // f = 32.000000 Hz
    new Note("20", 167), // f = 36.571430 Hz
    new Note("21", 166), // f = 42.666668 Hz
    new Note("22", 165), // f = 51.200001 Hz
    new Note("23", 164), // f = 64.000000 Hz
    new Note("24", 151), // f = 73.142860 Hz
    new Note("25", 150), // f = 85.333336 Hz
    new Note("26", 149), // f = 102.400002 Hz
    new Note("27", 148), // f = 128.000000 Hz
    new Note("28", 135), // f = 146.285721 Hz
    new Note("29", 134), // f = 170.666672 Hz
    new Note("30", 133), // f = 204.800003 Hz
    new Note("31", 132), // f = 256.000000 Hz
    new Note("32", 119), // f = 292.571442 Hz
    new Note("33", 118), // f = 341.333344 Hz
    new Note("34", 117), // f = 409.600006 Hz
    new Note("35", 116), // f = 512.000000 Hz
    new Note("36", 103), // f = 585.142883 Hz
    new Note("37", 102), // f = 682.666687 Hz
    new Note("38", 101), // f = 819.200012 Hz
    new Note("39", 100), // f = 1024.000000 Hz
    new Note("40", 87), // f = 1170.285767 Hz
    new Note("41", 86), // f = 1365.333374 Hz
    new Note("42", 85), // f = 1638.400024 Hz
    new Note("43", 84), // f = 2048.000000 Hz
    new Note("44", 71), // f = 2340.571533 Hz
    new Note("45", 70), // f = 2730.666748 Hz
    new Note("46", 69), // f = 3276.800049 Hz
    new Note("47", 68), // f = 4096.000000 Hz
    new Note("48", 55), // f = 4681.143066 Hz
    new Note("49", 54), // f = 5461.333496 Hz
    new Note("50", 53), // f = 6553.600098 Hz
    new Note("51", 52), // f = 8192.000000 Hz
    new Note("52", 39), // f = 9362.286133 Hz
    new Note("53", 38), // f = 10922.666992 Hz
    new Note("54", 37), // f = 13107.200195 Hz
    new Note("55", 36), // f = 16384.000000 Hz
    new Note("56", 23), // f = 18724.572266 Hz
    new Note("57", 22), // f = 21845.333984 Hz
    new Note("58", 21), // f = 26214.400391 Hz
    new Note("59", 20), // f = 32768.000000 Hz
    new Note("60", 7),  // f = 37449.144531 Hz
    new Note("61", 6),  // f = 43690.667969 Hz
    new Note("62", 5),  // f = 52428.800781 Hz
    new Note("63", 4),  // f = 65536.000000 Hz
    new Note("64", 3),  // f = 87381.335938 Hz
    new Note("65", 2),  // f = 131072.000000 Hz
    new Note("66", 1),  // f = 262144.000000 Hz
];
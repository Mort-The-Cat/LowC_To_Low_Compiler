

// How sound/music handles are stored:
//   word -  Song file pointer HL 
//   byte - Frame counter (1-256)
//   byte - Current phrase offset
//   byte - Note offset (this is the memory offset of the current note) (NOT the index of the current note)
//   byte - Previous sweep byte
//   byte - Previous duty/length byte
//   byte - Previous volume byte

#define Song_Handle_Bytecount 0x08

// Song file pointer:
//      byte - frames per beat (i.e. tempo)
//      phrase pointer array pointer

// Phrase pointer array:
//      phrase pointer ...
//      $0000 (i.e. end of file terminator)

// See "Song_Handle.low" for phrase implementation

byte* Get_Song_Handle(byte* Song);

void Play_Song_Handle(byte* Song_Handle, byte Audio_Register, byte Silence);

void Reset_Song_Handle(byte* Song_Handle);
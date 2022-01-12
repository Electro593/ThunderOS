/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                         **
**  Author: Aria Seiler                                                    **
**                                                                         **
**  This program is in the public domain. There is no implied warranty,    **
**  so use it at your own risk.                                            **
**                                                                         **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

internal r32
R32_Floor(r32 N)
{
    if(N > 0) return (r32)((s32)N);
    if(N - (s32)N == 0) return N;
    return (r32)((s32)N - 1);
}

internal r32
R32_Ceil(r32 N)
{
    if(N < 0) return (r32)((s32)N);
    if(N - (s32)N == 0) return N;
    return (r32)((s32)N + 1);
}

internal r32
R32_Sqrt(r32 N)
{
    r32 Result;
    Result = R128_Sqrt_4(R128_Set(N,0,0,0))[0];
    return Result;
}

internal r32
R32_Abs(r32 N)
{
    u32 Binary = FORCE_CAST(u32, N);
    Binary &= 0x7FFFFFFF;
    return FORCE_CAST(r32, Binary);
}

#define STBTT_pow(x,y) x
#define STBTT_fmod(x,y) x
#define STBTT_cos(x) x
#define STBTT_acos(x) x
#define STBTT_assert(x) x
#define STBTT_strlen(x) x
#define STBTT_ifloor(x) (s32)R32_Floor(x)
#define STBTT_iceil(x) (s32)R32_Ceil(x)
#define STBTT_sqrt(x) R32_Sqrt(x)
#define STBTT_fabs(x) R32_Abs(x)
#define STBTT_malloc(x,u) Context.Allocate(x)
#define STBTT_free(x,u)
#define STBTT_memcpy Mem_Cpy
#define STBTT_memset Mem_Set
#define size_t u64
#define STB_TRUETYPE_IMPLEMENTATION
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
#include <libraries/stb_truetype.h>
#pragma GCC diagnostic pop
#undef STBTT_ifloor
#undef STBTT_iceilSTBTT_sqrt
#undef STBTT_pow
#undef STBTT_fmod
#undef STBTT_cos
#undef STBTT_acos
#undef STBTT_fabs
#undef STBTT_malloc
#undef STBTT_free
#undef STBTT_assert
#undef STBTT_strlen
#undef STBTT_memcpy
#undef STBTT_memset
#undef STB_TRUETYPE_IMPLEMENTATION

/* Format

Font data
  s32 Ascent
  s32 Descent
  s32 Linegap
Character data ...
  c08 Codepoint
  s32 Advance
  s32 BearingX
  v2u32 Pos
  v2u32 Size
  u64 KerningFileOffset
  u64 BitmapFileOffset
Kerning Table ...
Character bitmaps ...

*/

typedef struct font_header {
    v2u32 BitmapSize;
    s32 Ascent;
    s32 Descent;
    s32 Linegap;
    s32 MaxAdvanceX;
} font_header;

typedef struct font_character {
    c08 Codepoint;
    s32 Advance;
    s32 BearingX;
    u64 KerningFileOffset;
    u64 BitmapFileOffset;
    v2s32 Pos;
    v2u32 Size;
} font_character;

internal b08
CreateFontFile(vptr FontData,
               vptr *DataOut,
               u64 *DataOutSize,
               r32 CharHeight,
               bitmap_header *BitmapHeaderOut)
{
    stbtt_fontinfo Font;
    if(!stbtt_InitFont(&Font, FontData, stbtt_GetFontOffsetForIndex(FontData, 0))) {
        return FALSE;
    }
    
    r32 Scale = stbtt_ScaleForPixelHeight(&Font, CharHeight);
    
    s32 SX, SY, EX, EY;
    stbtt_GetFontBoundingBox(&Font, &SX, &SY, &EX, &EY);
    u32 CellWidth = (EX - SY) * Scale;
    u32 CellHeight = (EY - SY) * Scale;
    
    u32 BitmapWidth = CellWidth * 8;
    u32 BitmapHeight = (((127-33)/8)+1) * CellHeight;
    u64 BitmapSize = BitmapWidth * BitmapHeight;
    
    u64 BitmapFileOffset = sizeof(font_header) + (127-32)*sizeof(font_character);
    u64 FileSize = BitmapFileOffset + BitmapSize;
    vptr FileData = Context.Allocate(FileSize);
    font_header *Header = (font_header*)FileData;
    font_character *Character = (font_character*)(Header+1);
    u08 *Bitmap = (u08*)(Character+(127-32));
    
    Header->BitmapSize = (v2u32){BitmapWidth, BitmapHeight};
    Header->MaxAdvanceX = CellWidth;
    
    s32 Ascent, Descent, Linegap;
    stbtt_GetFontVMetrics(&Font, &Ascent, &Descent, &Linegap);
    Header->Ascent  = (s32)((r32)Ascent  * Scale);
    Header->Descent = (s32)((r32)Descent * Scale);
    Header->Linegap = (s32)((r32)Linegap * Scale);
    
    u32 Cell = 0;
    for(u32 Codepoint = 32; Codepoint < 127; ++Codepoint)
    {
        Character->Codepoint = Codepoint;
        
        u32 GlyphIndex = stbtt_FindGlyphIndex(&Font, Codepoint);
        if(GlyphIndex == 0) continue;
        
        s32 Advance, BearingX;
        stbtt_GetGlyphHMetrics(&Font, GlyphIndex, &Advance, &BearingX);
        Character->Advance  = (s32)((r32)Advance  * Scale);
        Character->BearingX = (s32)((r32)BearingX * Scale);
        
        // stbtt_GetGlyphBitmapBox(&Font, GlyphIndex, Scale, Scale, &SX, &SY, &EX, &EY);
        // u32 GlyphWidth = EX - SX;
        // u32 GlyphHeight = EY - SY;
        
        if(stbtt_IsGlyphEmpty(&Font, GlyphIndex)) {
            Character->Size = (v2u32){0};
            Character->Pos = (v2s32){0};
            Character->BitmapFileOffset = 0;
        } else {
            stbtt_GetGlyphBitmapBox(&Font, GlyphIndex, Scale, Scale, &SX, &SY, &EX, &EY);
            Character->Pos = (v2s32){SX, -EY};
            Character->Size = (v2u32){EX - SX, EY - SY};
            
            u64 BitmapOffset = INDEX_2D((Cell%8)*CellWidth, (Cell/8)*CellHeight, BitmapWidth);
            u08 *CharBitmap = Bitmap + BitmapOffset;
            Character->BitmapFileOffset = BitmapFileOffset + BitmapOffset;
            stbtt_MakeGlyphBitmap(&Font, CharBitmap, CellWidth, CellHeight, BitmapWidth, Scale, Scale, GlyphIndex);
            
            
            // u08 *Temp = Context.Allocate(CellWidth * CellHeight);
            // stbtt_MakeGlyphBitmap(&Font, Temp, CellWidth, CellHeight, CellWidth, Scale, Scale, GlyphIndex);
            // for(u32 i = 0; i < CellHeight; ++i) {
            //     u08 *Cursor = CharBitmap;
            //     for(u32 j = 0; j < CellWidth; ++j) {
            //         *Cursor++ = *Temp++;
            //     }
            //     CharBitmap += BitmapWidth;
            // }
            
            
            Cell++;
        }
        
        Character->KerningFileOffset = 0;
        
        Character++;
    }
    
    BitmapHeaderOut->Signature[0] = 'B';
    BitmapHeaderOut->Signature[1] = 'M';
    BitmapHeaderOut->FileSize = sizeof(bitmap_header) + BitmapSize;
    BitmapHeaderOut->Reserved = 0;
    BitmapHeaderOut->DataOffset = sizeof(bitmap_header);
    BitmapHeaderOut->Size = 40;
    BitmapHeaderOut->Width = BitmapWidth;
    BitmapHeaderOut->Height = -BitmapHeight;
    BitmapHeaderOut->Planes = 1;
    BitmapHeaderOut->BitsPerPixel = 8;
    BitmapHeaderOut->Compression = 0;
    BitmapHeaderOut->ImageSize = BitmapSize;
    BitmapHeaderOut->XPixelsPerM = 0;
    BitmapHeaderOut->YPixelsPerM = 0;
    BitmapHeaderOut->ColorsUsed = 0;
    BitmapHeaderOut->ImportantColors = 0;
    
    *DataOut = FileData;
    *DataOutSize = FileSize;
}
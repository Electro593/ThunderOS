/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                         **
**  Author: Aria Seiler                                                    **
**                                                                         **
**  This program is in the public domain. There is no implied warranty,    **
**  so use it at your own risk.                                            **
**                                                                         **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef struct terminal {
    u64 MaxLines;
    u64 LineNum;
    c08 **Lines;
    
    u64 MaxChars;
    u64 CharCount;
    c08 *Text;
} terminal;

internal void
WriteToTerminal(terminal *Terminal,
                c08 *TextToWrite,
                u32 CharCount)
{
    if(Terminal->CharCount + CharCount >= Terminal->MaxChars) {
        ASSERT(FALSE);
    }
    
    c08 *C = TextToWrite;
    c08 *Buf = Terminal->Text;
    while(*C)
    {
        if(*C == '\n') {
            if(Terminal->LineNum + 1 >= Terminal->MaxLines) {
                ASSERT(FALSE);
            }
            
            c08 **Line = Terminal->Lines + Terminal->LineNum;
            *Line = Buf + 1;
            Terminal->LineNum++;
        }
        
        *Buf++ = *C++;
    }
    
    Terminal->CharCount += CharCount;
}

internal void
DrawTerminal(u32 *Framebuffer,
             v2u32 BufferSize,
             terminal *Terminal,
             c08 *Text,
             font_header *Font)
{
    c08 *C = Text;
    
    font_character *Characters = (font_character*)(Font+1);
    
    s32 AdvanceY = Font->Ascent - Font->Descent + Font->Linegap;
    
    // u32 LinesToShow = BufferSize.X / AdvanceY;
    // if(Terminal->LineNum < LinesToShow) LinesToShow = Terminal->LineNum;
    // c08 *C = Terminal->Lines[Terminal->LineNum - LinesToShow];
    
    v2u32 Pos = {0, Font->Ascent + Terminal->LineNum*AdvanceY};
    while(*C) {
        if(*C == '\n') {
            Pos.X = 0;
            Pos.Y += AdvanceY;
            C++;
            continue;
        }
        
        if(*C == '\t') {
            Pos.X += 4 * Characters[' ' - ' '].Advance;
            C++;
            continue;
        }
        
        if(*C == ' ') {
            Pos.X += Characters[' ' - ' '].Advance;
            C++;
            continue;
        }
        
        if(Pos.X >= BufferSize.X) {
            Pos.X = 0;
            Pos.Y += AdvanceY;
        }
        
        if(Pos.Y - Font->Descent >= BufferSize.Y) {
            break;
        }
        
        v4u08 CurrColor;
        font_character Char = Characters[*C - ' '];
        u08 *Bitmap = (u08*)Font + Char.BitmapFileOffset;
        
        for(s32 Y = 0; Y < Char.Size.Y; Y++)
        {
            for(s32 X = 0; X < Char.Size.X; X++)
            {
                u32 _X = Pos.X + Char.BearingX + X;
                u32 _Y = Pos.Y - Char.Pos.Y - Char.Size.Y + Y;
                u32 PixelIndex = INDEX_2D(_X, _Y, BufferSize.X);
                u32 *Pixel = Framebuffer + PixelIndex;
                
                u08 Grayscale = Bitmap[INDEX_2D(X, Y, Font->BitmapSize.X)];
                CurrColor = (v4u08){Grayscale, Grayscale, Grayscale, 0};
                
                *Pixel = MAKE_COLOR(PixelFormat_BGRX_8, CurrColor);
            }
        }
        
        Pos.X += Char.Advance;
        C++;
    }
}
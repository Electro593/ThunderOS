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
    c08 *Cursor;
    
    font_header *Font;
    v2u32 BufferSize;
    u32 PosX;
} terminal;

internal terminal
InitTerminal(u32 MaxLines,
             u32 MaxChars,
             font_header *Font,
             v2u32 BufferSize)
{
    terminal Terminal;
    Terminal.MaxChars = MaxChars;
    Terminal.MaxLines = MaxLines;
    Terminal.CharCount = 0;
    Terminal.LineNum   = 1;
    Terminal.Cursor = Context.Allocate(MaxChars * sizeof(c08));
    Terminal.Lines  = Context.Allocate(MaxLines * sizeof(c08*));
    Terminal.Cursor[0] = 0;
    Terminal.Lines[0]  = Terminal.Cursor;
    Terminal.Font = Font;
    Terminal.BufferSize = BufferSize;
    Terminal.PosX = 0;
    return Terminal;
}

internal void
WriteToTerminal(terminal *Terminal,
                c08 *TextToWrite,
                u32 CharCount)
{
    if(CharCount == 0) {
        c08 *C = TextToWrite;
        while(*C++) CharCount++;
    }
    
    u32 PosX = Terminal->PosX;
    font_header *Font = Terminal->Font;
    font_character *Characters = (font_character*)(Terminal->Font+1);
    
    u32 NewCharCount = Terminal->CharCount + CharCount;
    ASSERT(NewCharCount < Terminal->MaxChars);
    
    font_character Char;
    c08 *C = TextToWrite;
    c08 *Buf = Terminal->Cursor;
    while(*C)
    {
        b08 AddDifference = FALSE;
        u32 ToAdvance = 0;
        
        if(*C == '\t') {
            ToAdvance = 4 * Characters[' ' - ' '].Advance;
        } else if(*C == '\n') {
            ToAdvance = Terminal->BufferSize.X;
        } else {
            ToAdvance = Characters[*C - ' '].Advance;
            AddDifference = TRUE;
        }
        
        if(PosX + ToAdvance >= Terminal->BufferSize.X) {
            if(AddDifference) {
                PosX = PosX + ToAdvance - Terminal->BufferSize.X;
            } else {
                PosX = 0;
            }
            
            ASSERT(Terminal->LineNum < Terminal->MaxLines);
            Terminal->Lines[Terminal->LineNum++] = Buf;
        } else {
            PosX += ToAdvance;
        }
        
        *Buf++ = *C++;
    }
    
    *Buf = 0;
    Terminal->Cursor = Buf;
    Terminal->CharCount = NewCharCount;
    Terminal->PosX = PosX;
}

internal void
DrawTerminal(u32 *Framebuffer,
             terminal *Terminal)
{
    v2u32 BufferSize = Terminal->BufferSize;
    font_header *Font = Terminal->Font;
    font_character *Characters = (font_character*)(Font+1);
    
    u32 LinesToShow = BufferSize.Y / Font->AdvanceY;
    if(Terminal->LineNum < LinesToShow) LinesToShow = Terminal->LineNum;
    
    v2u32 Pos = (v2u32){0, Font->Ascent};
    c08 **Line = Terminal->Lines + Terminal->LineNum - LinesToShow;
    for(u32 LineIndex = 0; LineIndex < LinesToShow; LineIndex++)
    {
        Pos.X = 0;
        c08 *C = Line[LineIndex];
        c08 *NextLine;
        if(LineIndex == LinesToShow - 1) {
            NextLine = Terminal->Cursor;
        } else {
            NextLine = Line[LineIndex+1];
        }
        
        while(C < NextLine)
        {
            if(*C == '\n') {
                C++;
                continue;
            }
            
            if(*C == '\t') {
                Pos.X += 4 * Characters[' ' - ' '].Advance;
                C++;
                continue;
            }
            
            if(Pos.X >= Terminal->BufferSize.X) {
                break;
            }
            
            font_character Char = Characters[*C - ' '];
            if(Char.BitmapFileOffset)
            {
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
                        v4u08 CurrColor = (v4u08){Grayscale, Grayscale, Grayscale, 0};
                        
                        *Pixel = MAKE_COLOR(PixelFormat_BGRX_8, CurrColor);
                    }
                }
            }
            
            Pos.X += Char.Advance;
            C++;
        }
        
        Pos.Y += Font->AdvanceY;
    }
}
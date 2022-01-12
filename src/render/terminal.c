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
    
    font_header *Font;
    v2u32 BufferSize;
    v2u32 Pos;
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
    Terminal.LineNum = 0;
    Terminal.Text = Context.Allocate(MaxChars * sizeof(c08));
    Terminal.Lines = Context.Allocate(MaxLines * sizeof(c08*));
    Terminal.Text[0] = 0;
    Terminal.Lines[0] = Terminal.Text;
    Terminal.Font = Font;
    Terminal.BufferSize = BufferSize;
    return Terminal;
}

internal void
WriteToTerminal(terminal *Terminal,
                c08 *TextToWrite,
                u32 CharCount)
{
    v2u32 Pos = Terminal->Pos;
    font_header *Font = Terminal->Font;
    font_character *Characters = (font_character*)(Terminal->Font+1);
    
    u32 NewCharCount = Terminal->CharCount + CharCount - 1;
    ASSERT(NewCharCount < Terminal->MaxChars);
    
    font_character Char;
    c08 *C = TextToWrite;
    c08 *Buf = Terminal->Text + Terminal->CharCount;
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
        
        if(Pos.X + ToAdvance >= Terminal->BufferSize.X) {
            if(AddDifference) {
                Pos.X = ToAdvance - Terminal->BufferSize.X;
            } else {
                Pos.X = 0;
            }
            
            ASSERT(Terminal->LineNum+1 < Terminal->MaxLines);
            Terminal->Lines[++Terminal->LineNum] = Buf;
        } else {
            Pos.X += ToAdvance;
        }
        
        *Buf++ = *C++;
    }
    
    *Buf = 0;
    Terminal->CharCount = NewCharCount;
    Terminal->Pos = Pos;
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
    
    c08 **Line = Terminal->Lines[Terminal->LineNum - LinesToShow];
    for(u32 LineIndex = 0; LineIndex < LinesToShow; LineIndex++)
    {
        c08 *C = Line[LineIndex];
        c08 *NextLine;
        if(LineIndex == LinesToShow - 1) {
            NextLine = Terminal->Text;
        } else {
            NextLine = Line[LineIndex+1];
        }
        
        while(C < NextLine)
        {
            
            
            C++;
        }
    }
    
    c08 *C = Terminal->Lines[Terminal->LineNum - LinesToShow];
    
    v2u32 Pos = {0, Font->Ascent};
    while(*C) {
        if(*C == '\n') {
            // Already been processed, so continue
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
            Pos.Y += Font->AdvanceY;
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
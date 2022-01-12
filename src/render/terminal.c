/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                         **
**  Author: Aria Seiler                                                    **
**                                                                         **
**  This program is in the public domain. There is no implied warranty,    **
**  so use it at your own risk.                                            **
**                                                                         **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef struct terminal {
    v2u32 Pos;
    
    v2u32 CellSize;
    
    v3u08 BackgroundColor;
    v4u08 ForegroundColor;
    
    c08 *Text;
} terminal;

internal void
DrawTerminal(u32 *Framebuffer,
             v2u32 BufferSize,
             terminal Terminal,
             vptr *Bitmaps)
{
    c08 *C = Terminal.Text;
    r32 Opacity = Terminal.ForegroundColor.W / 255.0f;
    v3u08 BackgroundColor = V3u08_Mul_VS(Terminal.BackgroundColor, 1 - Opacity);
    v3u08 ForegroundColor = V3u08_Mul_VS(FORCE_CAST(v3u08, Terminal.ForegroundColor), Opacity);
    v3u08 Color = V3u08_Add(BackgroundColor, ForegroundColor);
    
    v2u32 CellCount = {BufferSize.X / Terminal.CellSize.X,
                       BufferSize.Y / Terminal.CellSize.Y};
    
    u32 Col = Terminal.Pos.X;
    u32 Row = Terminal.Pos.Y;
    while(*C) {
        if(Row >= CellCount.Y) break;
        
        if(*C == '\n') {
            Col = 0;
            Row++;
            C++;
            continue;
        }
        
        if(*C == '\t') {
            Col += 4;
            C++;
            continue;
        }
        
        if(*C == ' ') {
            Col++;
            C++;
            continue;
        }
        
        if(Col >= CellCount.X) {
            Col = 0;
            Row++;
        }
        
        v4u08 CurrColor;
        bitmap_header *Header = (bitmap_header*)(Bitmaps[*C]);
        
        for(s32 Y = Terminal.CellSize.Y; Y > Terminal.CellSize.Y - Header->Height; Y--)
        {
            for(u32 X = 0; X < Header->Width; X++)
            {
                u32 PixelIndex = INDEX_2D(X+Col*Terminal.CellSize.X, Y+Row*Terminal.CellSize.Y, BufferSize.X);
                u32 *Pixel = Framebuffer + PixelIndex;
                
                // v4u08 CurrColor = {64*((*C>>0)&3) + 64*((*C>>6)&3),
                //                    64*((*C>>2)&3) + 64*((*C>>6)&3),
                //                    64*((*C>>4)&3) + 64*((*C>>6)&3),
                //                    0};
                
                v4u08 *Pixels = (v4u08*)(Header+1);
                CurrColor = Pixels[INDEX_2D(X, Terminal.CellSize.Y - Y, Header->Width)];
                
                *Pixel = MAKE_COLOR(PixelFormat_BGRX_8, CurrColor);
            }
        }
        
        Col++;
        C++;
    }
}
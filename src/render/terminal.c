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
    v2u32 CellCount;
    
    v3u08 BackgroundColor;
    v4u08 ForegroundColor;
    
    c08 *Text;
} terminal;

internal void
DrawTerminal(v3u08 *Framebuffer,
             v2u32 BufferSize,
             terminal Terminal)
{
    c08 *C = Terminal.Text;
    r32 Opacity = Terminal.ForegroundColor.W / 255.0f;
    v3u08 BackgroundColor = V3u08_Mul_VS(Terminal.BackgroundColor, 1 - Opacity);
    v3u08 ForegroundColor = V3u08_Mul_VS(FORCE_CAST(v3u08, Terminal.ForegroundColor), Opacity);
    v3u08 Color = V3u08_Add(BackgroundColor, ForegroundColor);
    
    u32 Y = Terminal.Pos.Y;
    for(u32 Row = 0; Row < Terminal.CellCount.Y; Row++)
    {
        if(Y >= BufferSize.Y) break;
        
        u32 X = Terminal.Pos.X;
        for(u32 Col = 0; Col < Terminal.CellCount.X; Col++)
        {
            if(X >= BufferSize.X) break;
            
            for(u32 _Y = Y; _Y < Y + Terminal.CellSize.Y; ++_Y) {
                for(u32 _X = X; _X < X + Terminal.CellSize.X; ++_X) {
                    v3u08 *Pixel = Framebuffer + INDEX_2D(_X, _Y, BufferSize.X);
                    
                    v3u08 CurrColor = {64 * (*C>>0)&3 + 64*(*C>>6)&3,
                                       64 * (*C>>2)&3 + 64*(*C>>6)&3,
                                       64 * (*C>>4)&3 + 64*(*C>>6)&3};
                    
                    *Pixel = CurrColor;
                }
            }
            
            X += Terminal.CellSize.X;
            C++;
        }
        
        Y += Terminal.CellSize.Y;
    }
}
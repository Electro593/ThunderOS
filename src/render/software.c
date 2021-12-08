/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
**                                                                         **
**  Author: Aria Seiler                                                    **
**                                                                         **
**  This program is in the public domain. There is no implied warranty,    **
**  so use it at your own risk.                                            **
**                                                                         **
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef enum pixel_format {
    PixelFormat_RGBX_8,
    PixelFormat_BGRX_8,
} pixel_format;

#define _MAKE_COLOR(_0, _1, _2, _3) (((_0) << 24) | ((_1) << 16) | ((_2) << 8) | (_3))
#define MAKE_COLOR(F, V) (F == PixelFormat_RGBX_8 ? _MAKE_COLOR(0, V.Z, V.Y, V.X) : _MAKE_COLOR(0, V.X, V.Y, V.Z))

typedef struct software_renderer {
    u08 *Framebuffer;
    
    v2u32 Size;
    u32 Pitch;
    u32 BackgroundColor;
    pixel_format Format;
} software_renderer;

internal void
DrawLine(software_renderer *Renderer,
         v2r32 Point0,
         v2r32 Point1,
         v4u08 Color0,
         v4u08 Color1)
{
    v2u32 P0 = V2r32_ToV2u32(V2r32_Mul_VV(Point0, V2u32_ToV2r32(Renderer->Size)));
    v2u32 P1 = V2r32_ToV2u32(V2r32_Mul_VV(Point1, V2u32_ToV2r32(Renderer->Size)));
    s32 DX = P1.X - P0.X; // Delta X
    s32 DY = P1.Y - P0.Y; // Delta Y
    s32 D = 2*DY - DX; // Difference between points
    u32 C0;
    if(Renderer->Format == EFI_GraphicsPixelFormat_BlueGreenRedReserved8BitPerColor) {
        C0 = (Color0.X << 16) | (Color0.Y << 8) | Color0.Z;
    } else {
        C0 = (Color0.Z << 16) | (Color0.Y << 8) | Color0.X;
    }
    
    u32 Y = P0.Y;
    for(u32 X = P0.X; X < P1.X; ++X) {
        *((u32*)(Renderer->Framebuffer + Y*Renderer->Pitch + X*4)) = C0;
        if(D > 0) {
            Y++;
            D -= 2*DX;
        }
        D += 2*DY;
    }
}

internal void
Rasterize(software_renderer *Renderer,
          v3r64 *Vertices,
          v4u08 *Colors,
          u32 TriangleCount)
{
    u32 C0, C1, C2;
    v3r64 P0, P1, P2, P;
    r64 Xi, Xf, Yi, Yf;
    v3r64 View = {(r64)Renderer->Size.X, (r64)Renderer->Size.Y, -1};
    
    for(u32 TriangleIndex = 0;
        TriangleIndex < TriangleCount;
        ++TriangleIndex)
    {
        C0 = MAKE_COLOR(Renderer->Format, Colors[0]);
        C1 = MAKE_COLOR(Renderer->Format, Colors[1]);
        C2 = MAKE_COLOR(Renderer->Format, Colors[2]);
        
        P0 = V3r64_Mul_VV(Vertices[0], View);
        P1 = V3r64_Mul_VV(Vertices[1], View);
        P2 = V3r64_Mul_VV(Vertices[2], View);
        Xi = P0.X;
        Xf = P0.X;
        Yi = P0.Y;
        Yf = P0.Y;
        if(P1.X < Xi) Xi = P1.X;
        if(P2.X < Xi) Xi = P2.X;
        if(Xf < P1.X) Xf = P1.X;
        if(Xf < P2.X) Xf = P2.X;
        if(P1.Y < Yi) Yi = P1.Y;
        if(P2.Y < Yi) Yi = P2.Y;
        if(Yf < P1.Y) Yf = P1.Y;
        if(Yf < P2.Y) Yf = P2.Y;
        
        #define EDGE(P, V0, V1) (((P.X-V0.X)*(V1.Y-V0.Y))-((P.Y-V0.Y)*(V1.X-V0.X)))
        for(P.Y = Yi; P.Y < Yf; ++P.Y)
        {
            for(P.X = Xi; P.X < Xf; ++P.X)
            {
                s32 E01 = EDGE(P, P0, P1);
                s32 E02 = EDGE(P, P0, P2);
                s32 E12 = EDGE(P, P1, P2);
                
                if(E01 >= 0 && E02 >= 0 && E12 >= 0)
                {
                    *((u32*)Renderer->Framebuffer + (u32)P.Y*Renderer->Size.X + (u32)P.X) = C0;
                }
            }
        }
        #undef EDGE
        
        Vertices += 3;
        Colors += 3;
    }
}

internal r64
R64_Abs(r64 S)
{
    u64 B = FORCE_CAST(u64, S);
    B = BIT_CLEAR(B, 63);
    return FORCE_CAST(r64, B);
}

internal void
Raytrace(software_renderer *Renderer,
         v3r64 *Vertices,
         v4u08 Color,
         u32 TriangleCount)
{
    #define PI 3.14159265359
    #define R64_MAX LITERAL_CAST(r64, u64, 0x7FEFFFFFFFFFFFFF)
    #define R64_EPSILON LITERAL_CAST(r64, u64, 7)
    
    v2u32 Size = Renderer->Size;
    v3r64 View = {(r64)Size.X, (r64)Size.Y, 0};
    u32 C = MAKE_COLOR(Renderer->Format, Color);
    u32 VertexCount = 3 * TriangleCount;
    
    // P = <((X / S.X) * 2) - 1, ((Y / S.Y) * 2) - 1, S.Z>
    
    //       Origin = O = <IN>
    //        Pixel = P = <IN>
    //          Ray = R = P - O
    //   Vertex 0-3 = V = <IN>
    //       Normal = N = (V2-V0) x (V1-V0)
    //                t = (N*V0 - N*O) / (N*R)
    // Intersection = I = O + t*R
    
    v3r64 O, P, R, V0,V1,V2, N, I;
    r64 t, tN, NR;
    
    
    O = V3r64(0, 0, 1);
    
    for(u32 Y = 0; Y < Size.Y; ++Y)
    {
        u32 WriteOffset = (Size.Y - Y - 1)*Size.X;
        
        for(u32 X = 0; X < Size.X; ++X)
        {
            P = V3r64((((r64)X/View.X)*2)-1, (((r64)Y/View.Y)*2)-1, View.Z);
            R = V3r64_Sub(P, O);
            tN = R64_MAX;
            
            for(u32 K = 0; K < VertexCount; K += 3)
            {
                V0 = Vertices[K+0];
                V1 = Vertices[K+1];
                V2 = Vertices[K+2];
                N = V3r64_Cross(V3r64_Sub(V2, V0), V3r64_Sub(V1, V0));
                NR = V3r64_Dot(N, R);
                if(NR < R64_EPSILON) continue; // Facing the wrong way
                t = V3r64_Dot(N, V3r64_Sub(V0, O)) / NR;
                if(t < 0) continue; // Triangle is behind the viewer
                I = V3r64_Add(O, V3r64_Mul_VS(R, t));
                if(V3r64_Dot(N, V3r64_Cross(V3r64_Sub(V2, V0), V3r64_Sub(I, V0))) < 0 ||
                   V3r64_Dot(N, V3r64_Cross(V3r64_Sub(V0, V1), V3r64_Sub(I, V1))) < 0 ||
                   V3r64_Dot(N, V3r64_Cross(V3r64_Sub(V1, V2), V3r64_Sub(I, V2))) < 0)
                    continue;
                
                if(t < tN)
                    tN = t;
            }
            
            if(tN == R64_MAX)
                *((u32*)Renderer->Framebuffer + WriteOffset + X) = Renderer->BackgroundColor;
            else
                *((u32*)Renderer->Framebuffer + WriteOffset + X) = C;
        }
    }
}
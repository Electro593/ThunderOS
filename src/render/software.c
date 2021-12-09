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
    v4u08 BackgroundColor;
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
         v3r64 Eye,
         v3r64 *Vertices,
         v4u08 *Colors,
         u32 TriangleCount)
{
    #define PI 3.14159265359
    #define R64_MAX LITERAL_CAST(r64, u64, 0x7FEFFFFFFFFFFFFF)
    #define R64_EPSILON LITERAL_CAST(r64, u64, 7)
    
    typedef struct _raytracer_triangle {
        v4r64 C0, C1, C2;
        v3r64 V0;
        v3r64 N;
        v3r64 NcV02;
        v3r64 V01cN;
        r64 NdN;
        r64 tNumerator;
    } _raytracer_triangle;
    
    u32 BackgroundColor = MAKE_COLOR(Renderer->Format, Renderer->BackgroundColor);
    v2u32 Size = Renderer->Size;
    v3r64 View = {(r64)Size.X, (r64)Size.Y, 0};
    v3r64 O = Eye;
    v3r64 P; P.Z = View.Z;
    
    _raytracer_triangle *Ts = Context.Allocate(TriangleCount * sizeof(_raytracer_triangle));
    for(u32 K = 0; K < TriangleCount; ++K)
    {
        Ts[K].C0 = V4u08_ToV4r64(Colors[(K*3)+0]);
        Ts[K].C1 = V4u08_ToV4r64(Colors[(K*3)+1]);
        Ts[K].C2 = V4u08_ToV4r64(Colors[(K*3)+2]);
        Ts[K].V0 = Vertices[(K*3)+0];
        v3r64 V1 = Vertices[(K*3)+1];
        v3r64 V2 = Vertices[(K*3)+2];
        v3r64 V01 = V3r64_Sub(V1, Ts[K].V0);
        v3r64 V02 = V3r64_Sub(V2, Ts[K].V0);
        Ts[K].N = V3r64_Cross(V02, V01);
        Ts[K].NcV02 = V3r64_Cross(Ts[K].N, V02);
        Ts[K].V01cN = V3r64_Cross(V01, Ts[K].N);
        Ts[K].NdN = V3r64_Dot(Ts[K].N, Ts[K].N);
        Ts[K].tNumerator = V3r64_Dot(Ts[K].N, V3r64_Sub(Ts[K].V0, O));
    }
    
    for(u32 Y = 0; Y < Size.Y; ++Y)
    {
        u32 WriteOffset = (Size.Y - Y - 1)*Size.X;
        P.Y = (((r64)Y / View.Y) * 2) - 1;
        
        for(u32 X = 0; X < Size.X; ++X)
        {
            P.X = (((r64)X / View.X) * 2) - 1;
            v3r64 R = V3r64_Sub(P, O);
            r64 tN = R64_MAX;
            r64 uN = 0, vN = 0, wN = 0;
            _raytracer_triangle *TN = Ts;
            
            for(u32 K = 0; K < TriangleCount; ++K)
            {
                r64 NR = V3r64_Dot(Ts[K].N, R);
                if(NR > R64_EPSILON)
                    continue; // Facing the wrong way
                
                r64 t = Ts[K].tNumerator / NR;
                if(t < 0)
                    continue; // Triangle is behind the viewer
                
                v3r64 I = V3r64_Add(O, V3r64_Mul_VS(R, t));
                v3r64 V0I = V3r64_Sub(I, Ts[K].V0);
                r64 u = V3r64_Dot(Ts[K].V01cN, V0I) / Ts[K].NdN;
                r64 v = V3r64_Dot(Ts[K].NcV02, V0I) / Ts[K].NdN;
                r64 w = 1 - u - v;
                if(u < 0 || v < 0 || w < 0)
                    continue; // Outside of triangle
                
                if(t < tN)
                {
                    tN = t;
                    TN = Ts + K;
                    uN = u;
                    vN = v;
                    wN = w;
                }
            }
            
            v4u08 C0 = V4r64_ToV4u08(V4r64_Mul_VS(TN->C0, wN));
            v4u08 C1 = V4r64_ToV4u08(V4r64_Mul_VS(TN->C1, vN));
            v4u08 C2 = V4r64_ToV4u08(V4r64_Mul_VS(TN->C2, uN));
            v4u08 C = V4u08_Add(V4u08_Add(C0, C1), C2);
            u32 Color = MAKE_COLOR(Renderer->Format, C);
            
            if(tN == R64_MAX)
                *((u32*)Renderer->Framebuffer + WriteOffset + X) = BackgroundColor;
            else
                *((u32*)Renderer->Framebuffer + WriteOffset + X) = Color;
        }
    }
}
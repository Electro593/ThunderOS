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
          v3r32 *Vertices,
          v4u08 *Colors,
          u32 TriangleCount)
{
    u32 C0, C1, C2;
    v3r32 P0, P1, P2, P;
    r32 Xi, Xf, Yi, Yf;
    v3r32 View = {(r32)Renderer->Size.X, (r32)Renderer->Size.Y, -1};
    
    for(u32 TriangleIndex = 0;
        TriangleIndex < TriangleCount;
        ++TriangleIndex)
    {
        C0 = MAKE_COLOR(Renderer->Format, Colors[0]);
        C1 = MAKE_COLOR(Renderer->Format, Colors[1]);
        C2 = MAKE_COLOR(Renderer->Format, Colors[2]);
        
        P0 = V3r32_Mul_VV(Vertices[0], View);
        P1 = V3r32_Mul_VV(Vertices[1], View);
        P2 = V3r32_Mul_VV(Vertices[2], View);
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

internal void
Raytrace(software_renderer *Renderer,
         v3r32 Eye,
         v3r32 *Vertices,
         v3r32 *Normals,
         v4u08 *Colors,
         u32 TriangleCount)
{
    #define PI 3.14159265359
    #define R32_MAX LITERAL_CAST(r32, u32, 0x7F7FFFFF)
    #define R32_EPSILON LITERAL_CAST(r32, u32, 7)
    
    typedef struct _raytracer_triangle {
        v4r32 C0, C1, C2;
        v3r32 N0, N1, N2;
        v3r32 V0;
        v3r32 N;
        v3r32 NcV02;
        v3r32 V01cN;
        r32 NdN;
        r32 tNumerator;
    } _raytracer_triangle;
    
    if(!Vertices || !Colors || !Renderer)
        return;
    
    v2u32 Size = Renderer->Size;
    v3r32 View = {(r32)Size.X, (r32)Size.Y, 0};
    v3r32 O = Eye;
    v3r32 P; P.Z = View.Z;
    
    _raytracer_triangle *Ts = Context.Allocate(TriangleCount * sizeof(_raytracer_triangle));
    for(u32 K = 0; K < TriangleCount; ++K)
    {
        Ts[K].C0 = V4u08_ToV4r32(Colors[(K*3)+0]);
        Ts[K].C1 = V4u08_ToV4r32(Colors[(K*3)+1]);
        Ts[K].C2 = V4u08_ToV4r32(Colors[(K*3)+2]);
        Ts[K].N0 = Normals[(K*3)+0];
        v3r32 N1 = Normals[(K*3)+1];
        v3r32 N2 = Normals[(K*3)+2];
        Ts[K].V0 = Vertices[(K*3)+0];
        v3r32 V1 = Vertices[(K*3)+1];
        v3r32 V2 = Vertices[(K*3)+2];
        v3r32 V01 = V3r32_Sub(V1, Ts[K].V0);
        v3r32 V02 = V3r32_Sub(V2, Ts[K].V0);
        Ts[K].N = V3r32_Cross(V02, V01);
        Ts[K].NcV02 = V3r32_Cross(Ts[K].N, V02);
        Ts[K].V01cN = V3r32_Cross(V01, Ts[K].N);
        Ts[K].NdN = V3r32_Dot(Ts[K].N, Ts[K].N);
        Ts[K].tNumerator = V3r32_Dot(Ts[K].N, V3r32_Sub(Ts[K].V0, O));
    }
    
    #if 0
    
    // SIMD
    
    #define EPV 4
    u32 X,Y,K;
    r128 PX,PY,PZ, RX,RY,RZ;
    r128 u,v,w, uN,vN,wN;
    r128 V0IX,V0IY,V0IZ;
    r128 IX,IY,IZ, t,tN, NR;
    s128 C0X,C0Y,C0Z, C1X,C1Y,C1Z;
    s128 C2X,C2Y,C2Z;
    u128 CX,CY,CZ, Color;
    u128 WriteMask;
    s128 C;
    u32 WriteOffset, ColorMask;
    
    PZ = R128_Set1(0);
    RZ = PZ - O.Z;
    
    for(Y = 0; Y < Size.Y; ++Y)
    {
        WriteOffset = (Size.Y - Y - 1) * Size.X;
        PY = R128_Set1(Y);
        PY = ((PY / View.Y) * 2.0f) - 1.0f;
        RY = PY - O.Y;
        
        for(X = 0; X < Size.X; X += EPV)
        {
            WriteMask = U128_Set(-1, -(X+1<Size.X), -(X+2<Size.X), -(X+3<Size.X));
            PX = R128_Set(X+0, X+1, X+2, X+3);
            PX = ((PX / View.X) * 2.0f) - 1.0f;
            RX = PX - O.X;
            tN = R128_Set1(R32_MAX);
            
            // By setting only w to 1, only C0[XYZ] need to be set as defaults
            C0X = S128_Set1(Renderer->BackgroundColor.X);
            C0Y = S128_Set1(Renderer->BackgroundColor.Y);
            C0Z = S128_Set1(Renderer->BackgroundColor.Z);
            uN = R128_Set1(0);
            vN = R128_Set1(0);
            wN = R128_Set1(1);
            
            for(K = 0; K < TriangleCount; ++K)
            {
                NR = (Ts[K].N.X * RX) +
                     (Ts[K].N.Y * RY) +
                     (Ts[K].N.Z * RZ);
                C = NR > R32_EPSILON; // Facing the wrong way
                
                t = Ts[K].tNumerator / NR;
                C |= t < 0.0f; // Triangle is behind the viewer
                
                IX = O.X + (RX * t);
                IY = O.Y + (RY * t);
                IZ = O.Z + (RZ * t);
                
                V0IX = IX - Ts[K].V0.X;
                V0IY = IY - Ts[K].V0.Y;
                V0IZ = IZ - Ts[K].V0.Z;
                
                u = ((Ts[K].V01cN.X * V0IX) + 
                     (Ts[K].V01cN.Y * V0IY) + 
                     (Ts[K].V01cN.Z * V0IZ)) / Ts[K].NdN;
                
                v = ((Ts[K].NcV02.X * V0IX) + 
                     (Ts[K].NcV02.Y * V0IY) + 
                     (Ts[K].NcV02.Z * V0IZ)) / Ts[K].NdN;
                
                w = 1.0f - u - v;
                
                // Outside of triangle
                C |= u < 0.0f;
                C |= v < 0.0f;
                C |= w < 0.0f;
                
                // False if tN needs to get updated (t < tN)
                C |= t >= tN;
                
                tN = (r128)(((s128)t & ~C) | ((s128)tN & C));
                uN = (r128)(((s128)u & ~C) | ((s128)uN & C));
                vN = (r128)(((s128)v & ~C) | ((s128)vN & C));
                wN = (r128)(((s128)w & ~C) | ((s128)wN & C));
                C0X = (S128_Set1(Ts[K].C0.X) & ~C) | (C0X & C);
                C1X = (S128_Set1(Ts[K].C1.X) & ~C) | (C1X & C);
                C2X = (S128_Set1(Ts[K].C2.X) & ~C) | (C2X & C);
                C0Y = (S128_Set1(Ts[K].C0.Y) & ~C) | (C0Y & C);
                C1Y = (S128_Set1(Ts[K].C1.Y) & ~C) | (C1Y & C);
                C2Y = (S128_Set1(Ts[K].C2.Y) & ~C) | (C2Y & C);
                C0Z = (S128_Set1(Ts[K].C0.Z) & ~C) | (C0Z & C);
                C1Z = (S128_Set1(Ts[K].C1.Z) & ~C) | (C1Z & C);
                C2Z = (S128_Set1(Ts[K].C2.Z) & ~C) | (C2Z & C);
            }
            
            CX = R128_ToU128((S128_ToR128(C0X)*wN) + (S128_ToR128(C1X)*vN) + (S128_ToR128(C2X)*uN));
            CY = R128_ToU128((S128_ToR128(C0Y)*wN) + (S128_ToR128(C1Y)*vN) + (S128_ToR128(C2Y)*uN));
            CZ = R128_ToU128((S128_ToR128(C0Z)*wN) + (S128_ToR128(C1Z)*vN) + (S128_ToR128(C2Z)*uN));
            
            ColorMask = Renderer->Format == PixelFormat_RGBX_8;
            Color = ( ColorMask & ((CZ << 16) | (CY << 8) | CX)) |
                    (~ColorMask & ((CX << 16) | (CY << 8) | CZ));
            
            U128_MaskMove(Color, WriteMask,
                          ((u32*)Renderer->Framebuffer + WriteOffset + X));
        }
    }
    
    #else
    
    // LINEAR
    
    u32 BackgroundColor = MAKE_COLOR(Renderer->Format, Renderer->BackgroundColor);
    
    for(u32 Y = 0; Y < Size.Y; ++Y)
    {
        u32 WriteOffset = (Size.Y - Y - 1)*Size.X;
        P.Y = (((r32)Y / View.Y) * 2) - 1;
        
        for(u32 X = 0; X < Size.X; ++X)
        {
            P.X = (((r32)X / View.X) * 2) - 1;
            v3r32 R = V3r32_Sub(P, O);
            r32 tN = R32_MAX;
            r32 uN = 0, vN = 0, wN = 0;
            _raytracer_triangle *TN = Ts;
            
            for(u32 K = 0; K < TriangleCount; ++K)
            {
                #if 0
                v3r32 V0 = Vertices[(K*3)+0];
                v3r32 V1 = Vertices[(K*3)+1];
                v3r32 V2 = Vertices[(K*3)+2];
                v3r32 V01 = V3r32_Sub(V1, V0);
                v3r32 V02 = V3r32_Sub(V2, V0);
                v3r32 V0O = V3r32_Sub(O, V0);
                
                r32 D = V3r32_Dot(V3r32_Cross(V01, V02), R);
                if(D < R32_EPSILON) continue;
                r32 DR = 1 / D;
                
                r32 u = V3r32_Dot(V3r32_Cross(V01, V0O), R) * DR;
                if(u < 0) continue;
                
                r32 v = V3r32_Dot(V3r32_Cross(V0O, V02), R) * DR;
                if(v < 0) continue;
                
                r32 w = 1 - u - v;
                if(w < 0) continue;
                
                r32 t = V3r32_Dot(V3r32_Cross(V01, V0O), V02) * DR;
                
                #else
                
                r32 NR = V3r32_Dot(Ts[K].N, R);
                if(NR > R32_EPSILON)
                    continue; // Facing the wrong way
                
                r32 t = Ts[K].tNumerator / NR;
                if(t < 0)
                    continue; // Triangle is behind the viewer
                
                v3r32 I = V3r32_Add(O, V3r32_Mul_VS(R, t));
                v3r32 V0I = V3r32_Sub(I, Ts[K].V0);
                r32 u = V3r32_Dot(Ts[K].V01cN, V0I) / Ts[K].NdN;
                r32 v = V3r32_Dot(Ts[K].NcV02, V0I) / Ts[K].NdN;
                r32 w = 1 - u - v;
                if(u < 0 || v < 0 || w < 0)
                    continue; // Outside of triangle
                
                #endif
                
                if(t < tN)
                {
                    tN = t;
                    TN = Ts + K;
                    uN = u;
                    vN = v;
                    wN = w;
                }
            }
            
            v4u08 C0 = V4r32_ToV4u08(V4r32_Mul_VS(TN->C0, wN));
            v4u08 C1 = V4r32_ToV4u08(V4r32_Mul_VS(TN->C1, vN));
            v4u08 C2 = V4r32_ToV4u08(V4r32_Mul_VS(TN->C2, uN));
            v4u08 C = V4u08_Add(V4u08_Add(C0, C1), C2);
            u32 Color = MAKE_COLOR(Renderer->Format, C);
            
            if(tN == R32_MAX)
                *((u32*)Renderer->Framebuffer + WriteOffset + X) = BackgroundColor;
            else
                *((u32*)Renderer->Framebuffer + WriteOffset + X) = Color;
        }
    }
    
    #endif
}

typedef struct window {
    v2u32 Pos;
    v2u32 Size;
    
    v2u32 BufferSize;
    v4u08 *Framebuffer;
} window;

internal void
DrawWindow(v4u08 *Framebuffer,
           v2u32 BufferSize,
           window Window)
{
    u32 Size = Window.Size.X * Window.Size.Y * sizeof(v4u08);
    u32 WritePos = INDEX_2D(Window.Pos.X, Window.Pos.Y, BufferSize.Y);
    
    Mem_Cpy(Framebuffer + WritePos, Window.Framebuffer, Size);
}
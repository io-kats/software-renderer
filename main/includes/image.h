#ifndef IMAGE_H
#define IMAGE_H

#include "ers/typedefs.h"
#include "ers/macros.h"
#include "ers/common.h"
#include "ers/allocators.h"
#include "ers/vec.h"

struct Color3
{
    u8 r;
    u8 g;
    u8 b;

    inline Color3 operator+(const Color3 col2) const 
    { 
        Color3 result;
        result.r = (r > 255 - col2.r) ? 255 : (r + col2.r); 
        result.g = (g > 255 - col2.g) ? 255 : (g + col2.g); 
        result.b = (b > 255 - col2.b) ? 255 : (b + col2.b); 
        return result;    
    }

    inline Color3 operator*(const Color3 col2) const 
    {
        Color3 result;
        result.r = (u8)(((f32)r / 255.0f) * (f32) col2.r);
        result.g = (u8)(((f32)g / 255.0f) * (f32) col2.g);
        result.b = (u8)(((f32)b / 255.0f) * (f32) col2.b);
        return result;
    }

	inline Color3 operator*(f32 s) const { s = ers::clamp(s, 0.0f, 1.0f); return { (u8)(s * (f32) r), (u8)(s * (f32) g), (u8)(s * (f32) b) }; }
};

Color3 color3_mul(const Color3& col1, const Color3& col2);
Color3 color3_mul(f32 x, const Color3& col2);

struct Color4
{
    u8 r;
    u8 g;
    u8 b;
    u8 a;
};

Color4 vec3_to_color4(const ers::vec3& in);

typedef bool (*fragment_shader_t)(void*, f32, f32, f32, Color4&);

constexpr Color3 
RED = { 255, 0, 0 },
GREEN = { 0, 255, 0 },
BLUE = { 0, 0, 255 },
WHITE = { 255, 255, 255 },
BLACK = { 0, 0, 0 };
constexpr Color4 EMPTY = { 0, 0, 0, 0 };

class Image
{
public:
    enum class Format : u8
    {
        DUMMY = 0,
        GRAYSCALE,
        GRAYSCALE_WITH_ALPHA,
        RGB,
        RGBA
    };

    enum class Range : u8
    {
        LDR = 0,
        HDR
    };

    Image(ers::IAllocator* alloc = &ers::default_alloc);
    Image(s32 width, s32 height, Format type = Format::RGBA, Range range = Range::LDR, ers::IAllocator* alloc = &ers::default_alloc);
    Image(const char* filename, ers::IAllocator* alloc = &ers::default_alloc);
    ~Image();

    Image(Image&& im_in) noexcept;
    Image& operator=(Image&& im_in) noexcept;

    Image(const Image& im_in);
    Image& operator=(const Image& im_in);

    void Get(f32 s, f32 t, f32& out) const;
    void Get(f32 s, f32 t, ers::vec3& out) const;
    void Get(f32 s, f32 t, ers::vec4& out) const;

    void Get(s32 x, s32 y, u8& out) const;
    void Get(s32 x, s32 y, Color3& out) const;
    void Get(s32 x, s32 y, Color4& out) const; 

    void Set(s32 x, s32 y, f32 mag, f32 alpha = 1.0f);
    void Set(s32 x, s32 y, const ers::vec3& color, f32 alpha = 1.0f);
    void Set(s32 x, s32 y, const ers::vec4& color);

    void Set(s32 x, s32 y, u8 mag, u8 alpha = 255);
    void Set(s32 x, s32 y, const Color3& color, u8 alpha = 255);
    void Set(s32 x, s32 y, const Color4& color);

    void Clear();
    void Clear(u8 mag);
    void Clear(const Color3& color);
    void Clear(const Color4& color);

    void* GetData();

    s32 GetWidth();
    s32 GetHeight();
    s32 GetSize();
    s32 GetChannels();

    void Write(const char* filename, bool flip = true);

private:
    void* m_data;
    s32 m_width;
    s32 m_height;
    s32 m_channels;
    Range m_range;
    ers::IAllocator* m_alloc;

    void* allocate(s32 width_, s32 height_);
    size_t getIndexFromST(f32 s, f32 t) const;
    size_t getIndexFromXY(s32 x, s32 y) const;
    u8* GetDataLDR() const;
    f32* GetDataHDR() const;
};

#endif // IMAGE_H
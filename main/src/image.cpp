#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Actual modular arithmetic:
#define ERS_IMAGE_MOD(x, y) (((x) % (y) + (y)) % (y))


Color3 color3_mul(f32 s, const Color3& col2)
{
    Color3 result;
    s = ers::clamp(s, 0.0f, 1.0f);
    result.r = (u8)(s * (f32) col2.r);
    result.g = (u8)(s * (f32) col2.g);
    result.b = (u8)(s * (f32) col2.b);
    return result;
}

Color3 color3_mul(const Color3& col1, const Color3& col2)
{
    Color3 result;
    result.r = (u8)(((f32)col1.r / 255.0f) * (f32) col2.r);
    result.g = (u8)(((f32)col1.g / 255.0f) * (f32) col2.g);
    result.b = (u8)(((f32)col1.b / 255.0f) * (f32) col2.b);
    return result;
}

Color4 vec3_to_color4(const ers::vec3& in)
{
    Color4 result;
    result.r = (s32)(ers::clamp(in.x(), 0.0f, 1.0f) * 255.999f); 
    result.g = (s32)(ers::clamp(in.y(), 0.0f, 1.0f) * 255.999f); 
    result.b = (s32)(ers::clamp(in.z(), 0.0f, 1.0f) * 255.999f); 
    result.a = 255;
    return result;
}

Image::Image(ers::IAllocator* alloc)
    : m_data(nullptr), m_width(0), m_height(0), m_channels(0), m_range(Range::LDR), m_alloc(alloc)
{
    
}

Image::Image(s32 width, s32 height, Format type, Range range, ers::IAllocator* alloc)
    : m_data(nullptr), m_width(width), m_height(height), m_channels(0), m_range(range), m_alloc(alloc)
{
    if (type == Format::GRAYSCALE)
        m_channels = 1;
    else if (type == Format::GRAYSCALE_WITH_ALPHA)
        m_channels = 2;
    else if (type == Format::RGB)
        m_channels = 3;
    else if (type == Format::RGBA)
        m_channels = 4;
    else   
        ERS_ASSERT(false);

    m_data = allocate(width, height);  

    //Clear();
}

Image::Image(const char* filename, ers::IAllocator* alloc)
    : m_range(Range::LDR), m_alloc(alloc)
{
    // Load image
	stbi_set_flip_vertically_on_load(true);
	u8* data = stbi_load(filename, &m_width, &m_height, &m_channels, 0);
	ERS_ASSERTF(data != nullptr, "Image::Image: Failed to load texture: %s", filename);

    size_t image_size = sizeof(u8) * m_width * m_height * m_channels;
    m_data = reinterpret_cast<u8*>(alloc->Allocate(image_size, alignof(u8)));
    ERS_ASSERT(m_data != nullptr);
    memcpy(m_data, data, image_size);
    stbi_image_free(data);
}

Image::~Image()
{
    m_alloc->Deallocate(m_data);
}

Image::Image(Image&& im_in) noexcept
 :
    m_data(im_in.m_data),
    m_width(im_in.m_width),
    m_height(im_in.m_height),
    m_channels(im_in.m_channels),
    m_range(im_in.m_range),
    m_alloc(im_in.m_alloc)
{
    im_in.m_data = nullptr;
    im_in.m_width = 0;
    im_in.m_height = 0;
    im_in.m_channels = 0;
}

Image& Image::operator=(Image&& im_in) noexcept
{
    if (this != &im_in)
    {
        void* temp_data = m_data;
        s32 temp_width = m_width;
        s32 temp_height = m_height;
        s32 temp_channels = m_channels;
        Range temp_range = m_range;
        ers::IAllocator* temp_alloc = im_in.m_alloc;

        m_data = im_in.m_data;
        m_width = im_in.m_width;
        m_height = im_in.m_height;
        m_channels = im_in.m_channels;
        m_range = im_in.m_range;
        m_alloc = im_in.m_alloc;

        im_in.m_data = temp_data;
        im_in.m_width = temp_width;
        im_in.m_height = temp_height;
        im_in.m_channels = temp_channels;
        im_in.m_range = temp_range;
        im_in.m_alloc = temp_alloc;
    }
    return *this;
}

Image::Image(const Image& im_in)
 :
    m_width(im_in.m_width),
    m_height(im_in.m_height),
    m_channels(im_in.m_channels),
    m_range(im_in.m_range),
    m_alloc(im_in.m_alloc)
{
    m_data = allocate(m_width, m_height);

    for (s32 y = 0; y < m_height; ++y)
	{
        for (s32 x = 0; x < m_width; ++x)
        {
            if (m_channels == 1)
            {
                u8 mag;
                im_in.Get(x, y, mag);
                Set(x, y, mag);
            }
            else if (m_channels == 3)
            {
                Color3 col;
                im_in.Get(x, y, col);
                Set(x, y, col);
            }  
            else if (m_channels == 4)
            {
                Color4 col;
                im_in.Get(x, y, col);
                Set(x, y, col);
            }              
        }
    }
}

Image& Image::operator=(const Image& im_in)
{
    ERS_ASSERT(m_width == im_in.m_width && m_height == im_in.m_height);
    if (this != &im_in)
    {
        for (s32 y = 0; y < m_height; ++y)
        {
            for (s32 x = 0; x < m_width; ++x)
            {
                if (m_channels == 1)
                {
                    u8 mag;
                    im_in.Get(x, y, mag);
                    Set(x, y, mag);
                }
                else if (m_channels == 3)
                {
                    Color3 col;
                    im_in.Get(x, y, col);
                    Set(x, y, col);
                }  
                else if (m_channels == 4)
                {
                    Color4 col;
                    im_in.Get(x, y, col);
                    Set(x, y, col);
                }              
            }
        }
    }
    return *this;
}

void Image::Get(f32 s, f32 t, f32& out) const
{
    //ERS_ASSERT(m_channels == 1);
    size_t position = getIndexFromST(s, t);

    if (m_range == Range::LDR)
    {
        u8* p = GetDataLDR();
        out = (f32)p[position] / 255.0f; 
    }
    else
    {
        f32* p = GetDataHDR();
        out = p[position]; 
    }
}

void Image::Get(f32 s, f32 t, ers::vec3& out) const
{
    ERS_ASSERT(m_channels >= 3);
    size_t position = getIndexFromST(s, t);

    if (m_range == Range::LDR)
    {
        u8* p = GetDataLDR();
        out.x() = (f32)p[position] / 255.0f; 
        out.y() = (f32)p[position + 1] / 255.0f; 
        out.z() = (f32)p[position + 2] / 255.0f;   
    }
    else
    {
        f32* p = GetDataHDR();
        out.x() = p[position]; 
        out.y() = p[position + 1]; 
        out.z() = p[position + 2];   
    }
}

void Image::Get(f32 s, f32 t, ers::vec4& out) const
{
    ERS_ASSERT(m_channels % 2 == 0);
    size_t position = getIndexFromST(s, t);

    if (m_range == Range::LDR)
    {
        u8* p = GetDataLDR();
        if (m_channels == 4)
        {
            out.x() = (f32)p[position] / 255.0f; 
            out.y() = (f32)p[position + 1] / 255.0f; 
            out.z() = (f32)p[position + 2] / 255.0f;
            out.w() = (f32)p[position + 3] / 255.0f;
        }
        else
        {
            out.x() = (f32)p[position] / 255.0f; 
            out.y() = out.x(); 
            out.z() = out.x();
            out.w() = (f32)p[position + 1] / 255.0f;
        }
    }
    else
    {
        f32* p = GetDataHDR();
        if (m_channels == 4)
        {
            out.x() = p[position]; 
            out.y() = p[position + 1]; 
            out.z() = p[position + 2];
            out.w() = p[position + 3];
        }
        else
        {
            out.x() = p[position]; 
            out.y() = out.x(); 
            out.z() = out.x();
            out.w() = p[position + 1];
        }
    }
}

void Image::Get(s32 x, s32 y, u8& out) const
{
    ERS_ASSERT(m_channels == 1 && m_range == Range::LDR);
    size_t position = getIndexFromXY(x, y);
    u8* p = GetDataLDR();
    out = p[position];
}

void Image::Get(s32 x, s32 y, Color3& out) const
{
    ERS_ASSERT(m_channels > 1 && m_range == Range::LDR);
    size_t position = getIndexFromXY(x, y);
    u8* p = GetDataLDR();
    out = { p[position], p[position + 1], p[position + 2] };
}

void Image::Get(s32 x, s32 y, Color4& out) const
{
    ERS_ASSERT(m_channels % 2 == 0 && m_range == Range::LDR);
    size_t position = getIndexFromXY(x, y);
    u8* p = GetDataLDR();
    if (m_channels == 4)
    {
        out = { p[position], p[position + 1], p[position + 2], p[position + 3] };
    }
    else
    {
        out.r = p[position]; 
        out.g = out.r; 
        out.b = out.r;
        out.a = p[position + 1];
    }  
}

void Image::Set(s32 x, s32 y, f32 mag, f32 alpha)
{
    ERS_ASSERT(m_channels <= 2);
    size_t position = getIndexFromXY(x, y);

    mag = ers::clamp(mag, 0.0f, 1.0f);
    alpha = ers::clamp(alpha, 0.0f, 1.0f);
    if (m_range == Range::LDR)
    {
        u8* p = GetDataLDR();
        p[position] = (u8)(mag * 255.999f);
        if (m_channels == 2)
            p[position + 1] = (u8)(alpha * 255.999f);
    }
    else
    {
        f32* p = GetDataHDR();
        p[position] = mag;
        if (m_channels == 2)
            p[position + 1] = alpha;
    }
}

void Image::Set(s32 x, s32 y, const ers::vec3& color, f32 alpha)
{
    ERS_ASSERT(m_channels > 1);
    size_t position = getIndexFromXY(x, y);

    if (m_range == Range::LDR)
    {
        u8* p = GetDataLDR();
        p[position] = (u8)(ers::clamp(color.x(), 0.0f, 1.0f) * 255.999f);
        p[position + 1] = (u8)(ers::clamp(color.y(), 0.0f, 1.0f) * 255.999f);
        p[position + 2] = (u8)(ers::clamp(color.z(), 0.0f, 1.0f) * 255.999f);
        if (m_channels == 4)
            p[position + 3] = (u8)(ers::clamp(alpha, 0.0f, 1.0f) * 255.999f);
    }
    else
    {
        f32* p = GetDataHDR();
        p[position] = ers::clamp(color.x(), 0.0f, 1.0f);
        p[position + 1] = ers::clamp(color.y(), 0.0f, 1.0f);
        p[position + 2] = ers::clamp(color.z(), 0.0f, 1.0f);
        if (m_channels == 4)
            p[position + 3] = ers::clamp(alpha, 0.0f, 1.0f);
    }
}

void Image::Set(s32 x, s32 y, const ers::vec4& color)
{
    ERS_ASSERT(m_channels == 4);
    size_t position = getIndexFromXY(x, y);

    if (m_range == Range::LDR)
    {
        u8* p = GetDataLDR();
        p[position] = (u8)(ers::clamp(color.x(), 0.0f, 1.0f) * 255.999f);
        p[position + 1] = (u8)(ers::clamp(color.y(), 0.0f, 1.0f) * 255.999f);
        p[position + 2] = (u8)(ers::clamp(color.z(), 0.0f, 1.0f) * 255.999f);
        p[position + 3] = (u8)(ers::clamp(color.w(), 0.0f, 1.0f) * 255.999f);
    }
    else
    {
        f32* p = GetDataHDR();
        p[position] =     ers::clamp(color.x(), 0.0f, 1.0f);
        p[position + 1] = ers::clamp(color.y(), 0.0f, 1.0f);
        p[position + 2] = ers::clamp(color.z(), 0.0f, 1.0f);
        p[position + 3] = ers::clamp(color.w(), 0.0f, 1.0f);
    }
}

void Image::Set(s32 x, s32 y, u8 mag, u8 alpha)
{
    ERS_ASSERT(m_channels <= 2 && m_range == Range::LDR);
    size_t position = getIndexFromXY(x, y);
    u8* p = GetDataLDR();
    p[position] = mag;
    if (m_channels == 2)
        p[position + 1] = alpha;
}

void Image::Set(s32 x, s32 y, const Color3& color, u8 alpha)
{
    ERS_ASSERT(m_channels > 1 && m_range == Range::LDR);
    size_t position = getIndexFromXY(x, y);
    u8* p = GetDataLDR();
    p[position] = color.r;
    p[position + 1] = color.g;
    p[position + 2] = color.b;
    if (m_channels == 4)
        p[position + 3] = alpha;
}

void Image::Set(s32 x, s32 y, const Color4& color)
{
    ERS_ASSERT(m_channels == 4 && m_range == Range::LDR);
    size_t position = getIndexFromXY(x, y);
    u8* p = GetDataLDR();
    p[position] = color.r;
    p[position + 1] = color.g;
    p[position + 2] = color.b;
    p[position + 3] = color.a;
}

void Image::Clear()
{
    for (s32 y = 0; y < m_height; ++y)
	{
        for (s32 x = 0; x < m_width; ++x)
        {
            if (m_channels == 1)
                Set(x, y, 0.0f);
            else
                Set(x, y, ers::vec3(0.0f));
        }
    }
}

void Image::Clear(u8 mag)
{
    for (s32 y = 0; y < m_height; ++y)
	{
        for (s32 x = 0; x < m_width; ++x)
        {
            if (m_channels == 1)
            {
                Set(x, y, mag);
            }
            else
            {
                const Color3 col = { mag, mag, mag };
                Set(x, y, col);
            }              
        }
    }
}

void Image::Clear(const Color3& color)
{
    for (s32 y = 0; y < m_height; ++y)
		for (s32 x = 0; x < m_width; ++x)
			Set(x, y, color);
}

void Image::Clear(const Color4& color)
{
    for (s32 y = 0; y < m_height; ++y)
		for (s32 x = 0; x < m_width; ++x)
			Set(x, y, color);
}

void* Image::GetData()
{
    return m_data;
}

u8* Image::GetDataLDR() const
{
    return reinterpret_cast<u8*>(m_data);
}

f32* Image::GetDataHDR() const
{
    return reinterpret_cast<f32*>(m_data);
}

s32 Image::GetWidth()
{
    return m_width;
}

s32 Image::GetHeight()
{
    return m_height;
}

s32 Image::GetSize()
{
    return m_width * m_height * m_channels;
}

s32 Image::GetChannels()
{
    return m_channels;
}

void Image::Write(const char* filename, bool flip)
{
	stbi_flip_vertically_on_write(flip);
	s32 rc = stbi_write_png(
        filename, 
        m_width, 
        m_height, 
        m_channels, 
        reinterpret_cast<const void*>(m_data), 
        m_channels * m_width
    );
	ERS_PANIC(rc != 0);
}

void* Image::allocate(s32 width_, s32 height_)
{
    size_t image_size;
    size_t alignment;
    if (m_range == Range::LDR)
    {
        image_size = sizeof(u8) * width_ * height_ * m_channels;
        alignment = alignof(u8);
    }
    else
    {
        image_size = sizeof(f32) * width_ * height_ * m_channels;
        alignment = alignof(f32);
    }
    void* p = m_alloc->Allocate(image_size, alignment);
    ERS_ASSERT(p != nullptr);    
    memset(p, 0, image_size);
    return p;
}

size_t Image::getIndexFromST(f32 s, f32 t) const
{
    s32 x = (s32)(s * ((f32)m_width  - 0.001f));
    s32 y = (s32)(t * ((f32)m_height - 0.001f));
    x = ERS_IMAGE_MOD(x, m_width);
    y = ERS_IMAGE_MOD(y, m_height);
    size_t position = m_width * y + x;  
    position *= m_channels;
    return position;
}

size_t Image::getIndexFromXY(s32 x, s32 y) const
{
    x = ERS_IMAGE_MOD(x, m_width);
    y = ERS_IMAGE_MOD(y, m_height);
    size_t position = m_width * y + x;  
    position *= m_channels;
    return position;
}
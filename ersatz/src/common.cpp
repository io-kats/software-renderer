#include "../include/ers/common.h"

namespace ers
{
	// ----------------------------
	// ::::::::FILE READER:::::::::
	// ----------------------------

    FileReader::FileReader() 
        : size(size_t(0)), data(nullptr) { }

	FileReader::FileReader(const char* filepath) : size(size_t(0)), data(nullptr)
	{
		int rc = ReadFile(filepath);
		ERS_ASSERTF(rc == 0, "FileReader::FileReader: Couldn't open file: \n%s\n", filepath);
	}

	FileReader::~FileReader()
	{
		free(data);
		size = 0;
		data = nullptr;
	}

	int FileReader::ReadFile(const char* filepath)
	{
        ERS_ASSERTF(data == nullptr, "%s", "FileReader::ReadFile: Sanity check.");

        // Open file.
		FILE* f = fopen(filepath, "rb");
		if (f == nullptr)
		{
			return -1;
		}

        // Set the file position indicator's position to be EOF.
		if (fseek(f, 0, SEEK_END) < 0)
		{
			fclose(f);
			return -1;
		}

        // Since we are at EOF, the file position indicator
        // should equal the length of the file.
		long file_length = ftell(f);
		if (file_length < 0)
		{
			fclose(f);
			return -1;
		}

        // Allocate enough bytes for the whole file.
		char* temp_buffer = static_cast<char*>(malloc(((size_t)file_length + 1) * sizeof(char)));
		if (temp_buffer == nullptr)
		{
			fclose(f);
			return -1;
		}

        // Set the file position indicator's position to be
        // at the beginning of the file.
		if (fseek(f, 0, SEEK_SET) < 0)
		{
            free(temp_buffer);
			fclose(f);
			return -1;
		}

        // Read file into temporary buffer.
		long n = (long)fread(temp_buffer, 1, (size_t)file_length, f);
		if (n != file_length || ferror(f) < 0)
		{
            free(temp_buffer);
			fclose(f);
			return -1;
		}

        // Remember: zero-termination.
        temp_buffer[file_length] = '\0';

        // Update object and cleanup.
		size = (size_t)file_length;
		data = temp_buffer;
		
        temp_buffer = nullptr;
		fclose(f);
		return 0;
	}

	char* FileReader::GetData()
	{
		return data;
	}

	size_t FileReader::GetSize()
	{
		return size;
	}

	const char* FileReader::GetCstr() const
	{
		return static_cast<const char*>(data);
	}

    StringView FileReader::GetStringView() const
    {
        return StringView(static_cast<const char*>(data), size);
    }

    long get_file_length(FILE* file)
    {
        if (file == nullptr)
            return 0;

        if (fseek(file, 0, SEEK_END) < 0)
			return 0;

		long file_length = ftell(file); // negative on failure
		if (file_length < 0)
			return 0;

        if (fseek(file, 0, SEEK_SET) < 0)
			return 0;

        return file_length;
    }

    int read_file_into_buffer(FILE* file, char* read_buffer, size_t buffer_size)
	{
		if (file == nullptr || read_buffer == nullptr || buffer_size == 0)
            return 0;

		size_t n = fread(read_buffer, 1, (size_t)buffer_size, file);
		if (n != buffer_size || ferror(file) < 0)
			return 0;

		return 1;
	}

	// ----------------------------
	// :::::::::::STRING:::::::::::
	// ----------------------------

    String::String() : size(size_t(0)), capacity(size_t(0)), data(nullptr), alloc(&default_alloc)
    { 
        Set();
    }

    String::String(IAllocator* alloc_) : size(size_t(0)), capacity(size_t(0)), data(nullptr), alloc(alloc_)
    {
        Set();
    }

    String::String(const size_t& capacity_, IAllocator* alloc_) : size(size_t(0)), capacity(capacity_ + 3), data(nullptr), alloc(alloc_)
    { 
        Set(); 
    }

    String::String(const char* c, IAllocator* alloc_) : size(strlen(c)), capacity(size_t(0)), data(nullptr), alloc(alloc_)
    {
        Set();
        memcpy(data, c, size);
        nullTerminate();
    }

    String::String(const char* c, size_t c_len, IAllocator* alloc_)
        : size(c_len), capacity(size_t(0)), data(nullptr), alloc(alloc_)
    {
        Set();
        memcpy(data, c, size);
        nullTerminate();
    }

    String::String(FILE* file_, IAllocator* alloc_)
        : capacity(size_t(0)), data(nullptr), alloc(alloc_)
    {
        size = (size_t)get_file_length(file_);
        Set();
        int rc = read_file_into_buffer(file_, data, size);
	    ERS_ASSERTF(rc == 1, "%s", "String::String: Couldn't read file.");
        nullTerminate();
    }

    String::String(const String& s_in) : size(s_in.size), capacity(size_t(0)), data(nullptr), alloc(s_in.alloc)
    {
        Set();
        memcpy(data, s_in.data, size);
        nullTerminate();
    }

    String::String(const StringView& sv_in, IAllocator* alloc_)
        : size(sv_in.size), capacity(size_t(0)), data(nullptr), alloc(alloc_)
    {
        Set();
        memcpy(data, sv_in.data, size);
        nullTerminate();
    }

    String::String(String&& s_in) noexcept
    {
        size = s_in.size;
        capacity = s_in.capacity;
        data = s_in.data;
        alloc = s_in.alloc;

        s_in.size = 0;
        s_in.capacity = 0;
        s_in.data = nullptr;
    }

    String::~String()
    {
        Destroy();
    }

    // Figure out capacity and allocate zero-initialized memory.
    void String::Set()
    {
        // If you've constructed the string by giving it a capacity,
        // it won't be affected.
        if (capacity == 0)
            capacity = (size < ERS_SMALL_STRING) ? ERS_SMALL_STRING : (size + 1);

        // Use calloc to zero initialize the string.
        //data = static_cast<char*>(calloc(capacity, sizeof(char)));
        data = static_cast<char*>(alloc->Allocate(capacity * sizeof(char), alignof(char)));
        ERS_ASSERTF(data != nullptr, "%s", "String::Set: Failed to allocate memory.");
        for (size_t i = 0; i < capacity; ++i)
            data[i] = 0;        
    }   

    String& String::operator=(const char* c)
    {
        if (data != c)
        {
            const size_t c_len = strlen(c);
            Resize(c_len);
            size = c_len;
            memcpy(data, c, size);
            nullTerminate();
        }
        return *this;
    }   

    String& String::operator=(const String& s_in)
    {
        if (this != &s_in)
        {
            Resize(s_in.size);
            memcpy(data, s_in.data, size);
            nullTerminate();
        }
        return *this;
    }

    String& String::operator=(String&& s_in) noexcept
    {
        if (this != &s_in)
        {
            size_t temp_size = size;
            size_t temp_capacity = capacity;
            char* temp_data = data;
            IAllocator* temp_alloc = alloc;

            size = s_in.size;
            capacity = s_in.capacity;
            data = s_in.data;
            alloc = s_in.alloc;

            s_in.size = temp_size;
            s_in.capacity = temp_capacity;
            s_in.data = temp_data;
            s_in.alloc = temp_alloc;
        }
        return *this;
    }

    String& String::operator=(const StringView& sv_in)
    {
        if (data != sv_in.data)
        {
            Resize(sv_in.size);
            memcpy(data, sv_in.data, size);
            nullTerminate();
        }
        return *this;
    }

    char& String::operator[] (size_t i)
    {
        ERS_ASSERTF(i < size, "%s", "String::operator[]: Out of bounds.");
        return data[i];
    }

    char String::operator[] (size_t i) const
    {
        ERS_ASSERTF(i < size, "%s", "String::operator[]: Out of bounds.");
        return data[i];
    }

    String String::operator+(const char* c) const
    {
        const size_t c_len = strlen(c);
        String s_out(size + c_len, alloc);
        s_out.size = size + c_len;
        memcpy(s_out.data, data, size);
        memcpy(s_out.data + size, c, c_len);
        s_out.nullTerminate();
        return s_out;
    }

    String String::operator+(const String& s_rhs) const
    {
        const size_t s_len = s_rhs.size;
        String s_out(size + s_len, alloc);
        s_out.size = size + s_len;
        memcpy(s_out.data, data, size);
        memcpy(s_out.data + size, s_rhs.data, s_len);
        s_out.nullTerminate();
        return s_out;
    }

    String& String::operator+=(const char* c)
    {
        const size_t c_len = strlen(c);
        const size_t old_size = size;
        Resize(size + c_len);
        memcpy(data + old_size, c, c_len);
        nullTerminate();
        return *this;
    }

    String& String::operator+=(const String& s_in)
    {
        const size_t s_len = s_in.size;
        const size_t old_size = size;
        Resize(size + s_len);
        memcpy(data + old_size, s_in.data, s_len);
        nullTerminate();
        return *this;
    }

    String& String::operator+=(char c)
    {     
        Resize(size + 1);
        data[size - 1] = c;
        nullTerminate();
        return *this;
    }   

    bool String::operator==(const String& s_in) const
    {
        return size == s_in.size && memcmp(data, s_in.data, size) == 0;
    }

    bool String::operator!=(const String& s_in) const
    {
        return size != s_in.size || memcmp(data, s_in.data, size) != 0;
    }

    bool String::operator==(const char* c) const
    {
        return size == strlen(c) && memcmp(data, c, size) == 0;
    }

    bool String::operator!=(const char* c) const
    {
        return size != strlen(c) || memcmp(data, c, size) != 0;
    }

    bool String::operator==(const StringView& sv_in) const
    {
        return size == sv_in.size && memcmp(data, sv_in.data, size) == 0;
    }

    bool String::operator!=(const StringView& sv_in) const
    {
        return size != sv_in.size || memcmp(data, sv_in.data, size) != 0;
    }

    void String::Resize(const size_t& new_size)
    {
        ERS_ASSERTF(data != nullptr, "%s", "String::Resize: Sanity check.");

        // If the new size is less than the existing capacity (minus 1), just update the size.
        if (new_size < capacity)
        {
            size = new_size;
            return;
        }

        // Keep doubling capacity until it fits at least (new_size + 3) chars.
        while (capacity <= new_size)
            capacity *= 2;

        // Allocate new memory and zero-initialize. 
        //char* new_data = static_cast<char*>(calloc(capacity, sizeof(char)));
        char* new_data = static_cast<char*>(alloc->Allocate(capacity * sizeof(char), alignof(char)));
        if (new_data == nullptr)
        {
            alloc->Deallocate(data);
            fprintf(stderr, "%s", "[ERROR] String::Resize: Failed to allocate more memory.");
            exit(EXIT_FAILURE);
        }

        for (size_t i = 0; i < capacity; i++)
            new_data[i] = 0;

        // Copy data from old to new area and free the old one.
        memcpy(new_data, data, size);
        alloc->Deallocate(data);

        // Update object state.
        size = new_size;
        data = new_data;
        new_data = nullptr;
    }

    void String::Destroy()
    {
        if (data != nullptr)
        {
            alloc->Deallocate(data);
            size = 0;
            capacity = 0;
            data = nullptr;
        }      
    }

    void String::Reverse()
    {      
        if (size > 1)
        {
            for (size_t i = 0; i < size / 2; i++)
            {
                char temp = data[i];
                data[i] = data[size - 1 - i];
                data[size - 1 - i] = temp;
            }
        }       
    }

    void String::Clear()
    {      
        size = 0;
        nullTerminate();
    }

    char* String::GetData() const
    {
        return data;
    }

    const char* String::GetCstr() const
    {
        return static_cast<const char*>(data);
    }

    StringView String::GetStringView() const
    {
        return ers::StringView(static_cast<const char*>(data), size);
    }

    size_t String::GetSize() const
    {
        return size;
    }

    void String::AppendFile(FILE* file)
    {
        size_t file_size = (size_t)ers::get_file_length(file);		
		char* file_buffer = (char*)alloc->Allocate(file_size, alignof(char));
		int rc = ers::read_file_into_buffer(file, file_buffer, file_size);
	    ERS_ASSERTF(rc == 1, "%s", "String::AppendFile: Couldn't read file into buffer.");
        AppendSprintf("%.*s", file_size, (const char*)file_buffer);
        nullTerminate();
    }

    void String::AppendCodepoint(u32 cp)
    {
        char buffer[4] = { 0 }; // worst case, the codepoint coresponds to 4 bytes.
        int len;

        ERS_ASSERTF(cp < 0x110000 && (cp < 0xd800 || cp > 0xdbff), 
            "%s", "String::AppendCodepoint: Invalid codepoint.");

        if (cp < 0x80)
        {
            buffer[0] = (char)cp;
            len = 1;
        }
        else if (cp < 0x800)
        {
            buffer[1] = (char)((cp & 0x3f) | 0x80);
            buffer[0] = (char)(((cp >> 6) & 0x1f) | 0xc0);
            len = 2;
        }
        else if (cp < 0x10000)
        {
            buffer[2] = (char)((cp & 0x3f) | 0x80);
            buffer[1] = (char)(((cp >> 6) & 0x3f) | 0x80);
            buffer[0] = (char)(((cp >> 12) & 0x0f) | 0xe0);
            len = 3;
        }
        else if (cp < 0x110000)
        {
            buffer[3] = (char)((cp & 0x3f) | 0x80);
            buffer[2] = (char)(((cp >> 6) & 0x3f) | 0x80);
            buffer[1] = (char)(((cp >> 12) & 0x3f) | 0x80);
            buffer[0] = (char)(((cp >> 18) & 0x07) | 0xf0);
            len = 4;
        }

        char* old_end = data + size;
        Resize(size + len);
        memcpy(old_end, &buffer[0], len);
        nullTerminate();        
    }

    void String::Sprintf(const char* fmt, ...)
    {
        size = 0;
        va_list args;
        va_start(args, fmt);
        vsnprintfWrapper(fmt, args, 0);
        va_end(args);
    }

    void String::AppendSprintf(const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        vsnprintfWrapper(fmt, args, size);
        va_end(args);
    }

    void String::vsnprintfWrapper(const char* fmt, va_list args, size_t index)
    {
        va_list args2;
        va_copy(args2, args);

        int actual_size = vsnprintf(data + index, capacity - index, fmt, args);
        ERS_ASSERTF(actual_size >= 0, "%s", "String::vsnprintfWrapper: encoding error");
        if (actual_size < (int)(capacity - index))
        {
            size = index + actual_size;
        }
        else
        {
            Resize(index + actual_size);
            actual_size = (size_t)vsnprintf(data + index, capacity - index, fmt, args2);
            ERS_ASSERTF(actual_size >= 0, "%s", "String::vsnprintfWrapper: encoding error");            
        }
        nullTerminate();
        va_end(args2);
    }

    void String::nullTerminate()
    {
        data[size] = '\0';
    }

    String u64_to_string(u64 n)
    {
        String s_out(20);
        do {
            char remainder = static_cast<char>(n % 10);
            s_out += '0' + remainder;
            n /= 10;
        } while (n != 0);
        s_out.Reverse();
        return s_out;
    }

    String int_to_string(int n)
    {
        if (n == INT32_MIN)
        {
            size_t fixed_n = static_cast<size_t>(-(n + 1)) + 1;
            return "-" + u64_to_string(fixed_n);
        }

        bool is_negative = false;
        if (n < 0)
        {
            is_negative = true;
            n = -n;
        }
        
        String s_out(10);
        do {
            char remainder = static_cast<char>(n % 10);
            s_out += '0' + remainder;
            n /= 10;
        } while (n != 0);
        s_out.Reverse();

        if (is_negative)
            return "-" + s_out;
        else
            return s_out;
    }

    String operator+(const char* c, const String& s)
    {
        const size_t c_len = strlen(c);
        String s_out(s.size + c_len);
        s_out.size = s.size + c_len;
        memcpy(s_out.data, c, c_len);
        memcpy(s_out.data + c_len, s.data, s.size);
        s_out.nullTerminate();
        return s_out;
    }

    size_t utf8_length(const u8* s)
    {
        const u8 c = (s[0] >> 3);
        static const size_t length_from_msb[32] =
        {
            1, 1, 1, 1, 1, 1, 1, 1,	1, 1, 1, 1,	1, 1, 1, 1,	 // 0xxx xxxx
            0, 0, 0, 0,	0, 0, 0, 0, // 10xx xxxx, invalid
            2, 2, 2, 2, // 110x xxxx
            3, 3, // 1110 xxxx
            4, // 1111 0xxx
            0 // 1111 1xxx, invalid
        };
        return length_from_msb[c];
    }

    u32 utf8_to_codepoint(const u8* s, size_t length)
    {
        static const u8 masks[5] = { 0x00, 0x7f, 0x1f, 0x0f, 0x07 }; // masks[0] is a dummy value.
        static const u8 shift_values[7] = { 0, 0, 0, 0, 6, 12, 18 };

        const u8* str = s;
        const size_t len = (1 - (size_t)(length > 4)) * length;
    
        u8 buffer[4];
        buffer[0] = (u8)(len > 0) * str[0];
        buffer[1] = (u8)(len > 1) * str[1];
        buffer[2] = (u8)(len > 2) * str[2];
        buffer[3] = (u8)(len > 3) * str[3];

        u32 result = (buffer[0] & masks[len]) << shift_values[len + 2];
        result |= (buffer[1] & 0x3f) << shift_values[len + 1];
        result |= (buffer[2] & 0x3f) << shift_values[len];
        result |= (buffer[3] & 0x3f);

        return result;
    }

    u32 utf8_to_codepoint(const u8* s, size_t* index)
    {
        static const size_t length_from_msb[32] =
        {
            1, 1, 1, 1, 1, 1, 1, 1,	1, 1, 1, 1,	1, 1, 1, 1,	 // 0xxx xxxx
            0, 0, 0, 0,	0, 0, 0, 0, // 10xx xxxx, invalid
            2, 2, 2, 2, // 110x xxxx
            3, 3, // 1110 xxxx
            4, // 1111 0xxx
            0 // 1111 1xxx, invalid
        };

        static const u8 masks[5] = { 0x00, 0x7f, 0x1f, 0x0f, 0x07 }; // masks[0] is a dummy value.
        static const u8 shift_values[7] = { 0, 0, 0, 0, 6, 12, 18 };

        const u8* str = s + *index;
        const size_t len = length_from_msb[str[0] >> 3];
     
        u8 buffer[4]; u8 m;
        m = (u8)(len > 0); buffer[0] = m * str[m * 0];
        m = (u8)(len > 1); buffer[1] = m * str[m * 1];       
        m = (u8)(len > 2); buffer[2] = m * str[m * 2];
        m = (u8)(len > 3); buffer[3] = m * str[m * 3];

        u32 result = (buffer[0] & masks[len]) << shift_values[len + 2];
        result |= (buffer[1] & 0x3f) << shift_values[len + 1];
        result |= (buffer[2] & 0x3f) << shift_values[len];
        result |= (buffer[3] & 0x3f);

        *index += len;
        return result;
    }

    u32 utf16_to_codepoint(const u8* s, bool swap)
    {
        const u8* str = s;
        const int sw = (int)swap;

        u8 buffer[2];
        buffer[0] = str[0 + sw];
        buffer[1] = str[1 - sw];

        u16 curr = 0; 
        u8* p_curr = (u8*)&curr;
        memcpy(p_curr, &buffer[0], sizeof(u16));

        u32 result = (u32)curr;

        // Check if codepoint is in a supplementary plane.
        u8 isSupplementaryPlane = (u8)((result >= 0xd800) && (result <= 0xdbff));
        if (isSupplementaryPlane)
        {
            buffer[0] = str[2 + sw];
            buffer[1] = str[3 - sw];
            memcpy(p_curr, &buffer[0], sizeof(u16));
            result = ((result - 0xd800) << 10) + ((u32)curr - 0xdc00) + 0x10000;
        }      
        return result;
    }

    u32 utf16_to_codepoint(const u8* s, size_t* index, bool swap)
    {
        const u8* str = s + *index;
        const int sw = (int)swap;

        u8 buffer[2];
        buffer[0] = str[0 + sw];
        buffer[1] = str[1 - sw];

        u16 curr = 0; 
        u8* p_curr = (u8*)&curr;
        memcpy(p_curr, &buffer[0], sizeof(u16));

        u32 result = (u32)curr;
        *index += 2;

        // Check if codepoint is in a supplementary plane.
        u8 isSupplementaryPlane = (u8)((result >= 0xd800) && (result <= 0xdbff));
        if (isSupplementaryPlane)
        {
            buffer[0] = str[2 + sw];
            buffer[1] = str[3 - sw];
            memcpy(p_curr, &buffer[0], sizeof(u16));
            result = ((result - 0xd800) << 10) + ((u32)curr - 0xdc00) + 0x10000;
            *index += 2;
        }

        return result;
    }

    inline size_t utf16_length(u32 codepoint)
    {
        return (codepoint >= 0x10000) ? 4 : 2;
    }

        bool is_digit(char c)
    {
        return (c >= '0' && c <= '9');
    }

    bool is_hex_digit(char c)
    {
        return (c >= '0' && c <= '9') || (c >= 'a' && c >= 'f') || (c >= 'A' && c >= 'F');
    }

    bool is_space(char c)
    {
        return (c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '\v' || c == '\f');
    }

    // ----------------------------
	// ::::::::STRING VIEW:::::::::
	// ----------------------------

    StringView::StringView(const char* s_)
        : size(strlen(s_)), data(s_) { }
    StringView::StringView(const char* s_, size_t size_)
        : size(size_), data(s_) { }
    StringView::StringView(const char* start_, const char* end_)
        : size(end_ - start_), data(start_) { }
    StringView::StringView(String s_)
        : size(s_.GetSize()), data(s_.GetCstr()) { }
		
    bool StringView::operator==(const StringView& sv_in) const
    {
        return size == sv_in.size && memcmp(data, sv_in.data, size) == 0;
    }

    bool StringView::operator!=(const StringView& sv_in) const
    {
        return size != sv_in.size || memcmp(data, sv_in.data, size) != 0;
    }

    bool StringView::operator==(const char* s_in) const
    {
        return size == strlen(s_in) && memcmp(data, s_in, size) == 0;
    }

    bool StringView::operator!=(const char* s_in) const
    {
        return size != strlen(s_in) || memcmp(data, s_in, size) != 0;
    }

    bool StringView::operator==(const String& s_in) const
    {
        return size == s_in.GetSize() && memcmp(data, s_in.GetData(), size) == 0;
    }

    bool StringView::operator!=(const String& s_in) const
    {
        return size != s_in.GetSize() || memcmp(data, s_in.GetData(), size) != 0;
    }

    bool StringView::StartsWith(const char* s_)
    {
        const size_t s_len = strlen(s_);
        const bool cond = (s_len <= size);
        bool starts_with = true;
        if (cond)
        {
            for (size_t i = 0; i < s_len; i++)
            {
                starts_with = (s_[i] == data[i]);
                if (!starts_with)
                    break;
            }
        }       
        return cond && starts_with;
    }

    bool StringView::StartsWith(const char* s_, size_t s_len)
    {    
        const bool cond = (s_len <= size);
        bool starts_with = true;
        if (cond)
        {
            for (size_t i = 0; i < s_len; i++)
            {
                starts_with = (s_[i] == data[i]);
                if (!starts_with)
                    break;
            }
        }       
        return cond && starts_with;
    }


	// ----------------------------
	// ::::::::MATH HELPERS::::::::
	// ----------------------------

    // Wanted to test this. Turns out, at least on MSVC,
    // 1.0f/std::sqrt(x) is faster, if my benchmark wasn't bad.
    // TODO: search for better constants
    f32 fast_rsqrt(f32 x)
    {
        const f32 threehalfs = 1.5f;
        const f32 x2 = x * 0.5f;
        const u32 constant = 0x5F3759DFu;

        f32 y = x;       
        u32 i = ers::f32_to_u32(y);
        i = constant - (i >> 1);
        y = ers::u32_to_f32(i);

        y = y * (threehalfs - (x2 * y * y));
        //y = y * (threehalfs - (x2 * y * y));
        return y;
    }

      u32 f32_to_u32(f32 x)
    {
        u32 y;
        u8* c_in = (u8*)&x;
        u8* c_out = (u8*)&y;
        memcpy(c_out, c_in, sizeof(f32));
        return y;
    }

	u64 f64_to_u64(f64 x)
    {       
        u64 y;
        u8* c_in = (u8*)&x;
        u8* c_out = (u8*)&y;
        memcpy(c_out, c_in, sizeof(f64));
        return y;
    }

    f32 u32_to_f32(u32 x)
    {
        f32 y = 0;
        u8* c_in = (u8*)&x;
        u8* c_out = (u8*)&y;
        memcpy(c_out, c_in, sizeof(u32));
        return y;
    }

    f64 u64_to_f64(u64 x)
    {
        f64 y = 0;
        u8* c_in = (u8*)&x;
        u8* c_out = (u8*)&y;
        memcpy(c_out, c_in, sizeof(u64));
        return y;
    }
    
    // ----------------------------
    // :::::::NUMBER PARSERS:::::::
    // ----------------------------
    
    s64 parse_int(const char* start, const char* finish)
    {       
        bool sign = false;
        if (*start == '-')
        {
            sign = true;
            start++;
        }

        s64 result = 0;
        while (start != finish && *start != '\0')
        {
            const int temp = static_cast<int>(*(start++) - '0');
            result = result * 10 + static_cast<s64>(temp);
        }
            
        if (sign)
            result = -result;
        return result;
    }

    float parse_float(const char* start, const char* finish)
    {
        return static_cast<float>(parse_double(start, finish));
    }

    double parse_double(const char* start, const char* finish)
    {
        bool sign = false;
        if (*start == '-')
        {
            sign = true;
            ++start;
        }

        const u32 five_power_of_ten = 100000; // 10**5
        u32 loops_out = 0;
        u32 loops_in = 0;
        double result = 0.0;
        
        u32 before_decimal = 0;
        while (start != finish && *start >= '0' && *start <= '9')
        {
            before_decimal = before_decimal * 10 + ((u32)(*(start++)) - '0');
            ++loops_out;
            if (loops_out == 5)
            {
                result *= five_power_of_ten;
                result += (double)before_decimal;

                loops_out = 0;
                before_decimal = 0;
            }
        }

        u32 power_of_ten = 1;
        for (u32 i = 0; i < loops_out; ++i)
            power_of_ten *= 10;

        result *= power_of_ten;
        result += (double)before_decimal;
               
        if (*start == '.')
        {
            ++start;
            loops_out = 0;
            loops_in = 0;
            u32 after_decimal = 0;
            while (start != finish && *start >= '0' && *start <= '9')
            {                              
                after_decimal = after_decimal * 10 + ((u32)(*(start++)) - '0');
                ++loops_out; // power_of_ten *= 10;
                if (loops_out == 5)
                {
                    ++loops_in;
                    double temp = (double)after_decimal;
                    for (u32 i = 0; i < loops_in; ++i)
                        temp /= five_power_of_ten;
                    result += temp;

                    loops_out = 0;
                    after_decimal = 0;
                }
            }

            power_of_ten = 1;
            for (u32 i = 0; i < loops_out; ++i)
                power_of_ten *= 10;
            
            double temp = (double)after_decimal / (double)power_of_ten;
            for (u32 i = 0; i < loops_in; ++i)
                temp /= five_power_of_ten;
            result += temp;                
        }

        if (start != finish && (*start == 'e' || *start == 'E'))
        {
            ++start;
            if (*start == '+')
                ++start;
            const s32 exp = (s32)parse_int(start, finish);

            if (exp > 0)
                result *= pow(10, exp);
            else
                result /= pow(10, -exp);
        }
   
        return (sign) ? -result : result;
    }

    String parse_string(const char* start, const char* finish)
    {
        String result(finish - start);
        while (start != finish && *start != '\0')
            result += *start++;
        return result;
    }

    // ----------------------------
    // :::::STRING PROCESSOR:::::::
    // ----------------------------

    StringProcessor::StringProcessor()
        : start(nullptr), end(nullptr), position(nullptr), lookahead(nullptr) { }

    StringProcessor::StringProcessor(const char* start_, size_t size)
        : start(start_), end(start_ + size), position(start_), lookahead(nullptr) { }

    StringProcessor::StringProcessor(const char* start_, size_t size, const char* position_)
        : start(start_), end(start_ + size), position(position_), lookahead(nullptr) { }

    StringProcessor::StringProcessor(const char* start_, const char* end_, const char* position_)
        : start(start_), end(end_), position(start_), lookahead(nullptr) { }

    void StringProcessor::SkipTo(char delimiter)
    {
        while (position != end && *position != delimiter)
            ++position;
    }

    void StringProcessor::SkipLine()
    {
        while (position != end && *position != '\n')
            ++position;
        if (position != end)
            ++position;
    }

    void StringProcessor::SkipWhitespace()
    {
        while (position != end && is_space(*position))
            ++position;
    }

    void StringProcessor::SkipToWhitespace()
    {
        while (position != end && !is_space(*position))
            ++position;
    }

    void StringProcessor::Reset()
    {
        position = start;
        lookahead = nullptr;
    }

    void StringProcessor::getLookahead(char delimiter)
    {
        lookahead = position;
        if (delimiter == -1)
        {
            while (lookahead != end && !is_space(*lookahead))
                ++lookahead;
        }
        else
        {
            while (lookahead != end && *lookahead != delimiter)
                ++lookahead;
        }
    }

    float StringProcessor::GetFloat(const char delimiter)
    {
        getLookahead(delimiter);
        float result = static_cast<float>(ers::parse_float(position, lookahead));
        position = lookahead;
        return result;
    }

    int StringProcessor::GetInt(const char delimiter)
    {
        getLookahead(delimiter);
        int result = static_cast<int>(ers::parse_int(position, lookahead));
        position = lookahead;
        return result;
    }

    String StringProcessor::GetString(const char delimiter)
    {
        getLookahead(delimiter);
        String result = parse_string(position, lookahead);
        position = lookahead;
        return result;
    }

    char StringProcessor::Current()
    {
        return *position;
    }

    char StringProcessor::Next()
    {
        if (1 < end - position)
            return *(position + 1);
        else
            return -1;
    }

    void StringProcessor::Advance(ptrdiff_t n)
    {
        if (n <= end - position)
            position += n;
    }

    bool StringProcessor::CanAdvance()
    {
        return position != end;
    }

    bool StringProcessor::StartsWith(const char* s)
    {
        const size_t s_len = strlen(s);
        const bool cond = ((ptrdiff_t)s_len <= end - position);
        bool starts_with = true;
        if (cond)
        {
            for (size_t i = 0; i < s_len; i++)
            {
                starts_with = (*(s + i) == *(position + i));
                if (!starts_with)
                    break;
            }               
        }
        return cond && starts_with;
    }

    const char* StringProcessor::GetPosition()
    {
        return position;
    }

    // ----------------------------
    // ::::::::::::RNG:::::::::::::
    // ----------------------------

    // Pseudo-random number generation based on the
    // Lehmer random number generator.
    u32 r_state;
    bool first_time_rand_initialization = true;

    void init_rand()
    {
        if (first_time_rand_initialization)
        {
            // Only initialize once. Otherwise, do nothing.
            first_time_rand_initialization = false;

            // We use time(nullptr) to retrieve the current time.
            u64 t = static_cast<u64>(time(nullptr));
            r_state = static_cast<u32>(t & 0x7fffffff);

            // But it seems that, since it's in seconds, subsequent runs *of the program*
            // close to one another (or possibly by using this function to reseed the rng)
            // will produce a seed close to the previous one. This seems to produce a first
            // pseudo-random number close to the previous first one.
            // To solve the aforementioned problem, we run the randomizer once
            // before returning any other numbers. 
            lcg_parkmiller(r_state);

            // This is not enough, because the first numbers 
            // produced by it are also seemingly close to each other, 
            // which is mostly a problem for random_at_most() and random_range(),
            // but that is solved by random(), where the rng is called
            // a second time before a number is ever returned, which
            // seems to produce better results.
        }       
    }
  
    // From wikipedia: 
    // https://en.wikipedia.org/wiki/Lehmer_random_number_generator#Sample_C99_code
    // 'state' needs to be a positive number smaller than 0x7fffffff.
    void lcg_parkmiller(u32& state)
    {
        u64 product = static_cast<u64>(state) * 48271;
        u32 x = static_cast<u32>((product & 0x7fffffff) + (product >> 31));
        state = (x & 0x7fffffff) + (x >> 31);
    }

    u32 random()
    {
        lcg_parkmiller(r_state);
        return r_state;
    }

    // Should produce a random double in [0, 1].
    double random_frac()
    {
        lcg_parkmiller(r_state);
        return ERS_PROBABILITY * static_cast<double>(r_state);
    }

    // Should produce a random double in [min, max].
    double random_frac(double min, double max)
    {
        double t = random_frac();
        return min + t * (max - min);
    }

    // From here: 
    // https://stackoverflow.com/questions/2509679/how-to-generate-a-random-integer-number-from-within-a-range
    // Return an unsigned 32-bit integer in [0, max], for 'max' not greater than ERS_MAX_RAND.
    u32 random_at_most(u32 max)
    {
        ERS_ASSERT(max <= ERS_MAX_RAND);
        u32 num_bins = max + 1;
        u32 num_rand = ERS_MAX_RAND + 1;
        u32 bin_size = num_rand / num_bins;
        num_rand -= num_rand % num_bins; // defect

        u32 x;
        do {
            x = random();
        } while (x >= num_rand);

        return x / bin_size;
    }

    // Returns an unsigned 32-bit integer in [min, max].
    u32 random_range(u32 min, u32 max)
    {      
        u32 difference = max - min;
        // ERS_ASSERT(difference > 0);
        return min + random_at_most(difference);
    }

    // ----------------------------
    // :::::::HASH FUNCTIONS:::::::
    // ----------------------------

    size_t hash(u8 n)
    {
        size_t result = 0;
        hash_combine(result, n);
        return result;
    }

    size_t hash(s8 n)
    {
        u8 val = (n > 0) ? static_cast<u8>(n) : static_cast<u8>(INT8_MAX) + static_cast<u8>(-(n + 1)) + 1;
        return hash(val);
    }

    size_t hash(u16 n)
    {
        size_t result = 0;
        do
        {           
            hash_combine(result, n);
            n >>= 8;
        } while (n > 0);
        return result;
    }

    size_t hash(s16 n)
    {
        u16 val = (n > 0) ? 
            static_cast<u16>(n) 
            : 
            static_cast<u16>(INT8_MAX) + static_cast<u16>(-(n + 1)) + 1;
        return hash(val);
    }

    size_t hash(u32 n)
    {
        size_t result = 0;
        do
        {           
            hash_combine(result, n);
            n >>= 8;
        } while (n > 0);
        return result;
    }

    size_t hash(s32 n)
    {
        u32 val = (n > 0) 
            ? 
            static_cast<u32>(n) 
            : 
            static_cast<u32>(INT8_MAX) + static_cast<u32>(-(n + 1)) + 1;
        return hash(val);
    }

    size_t hash(u64 n)
    {
        size_t result = 0;
        do
        {          
            hash_combine(result, n);
            n >>= 8;
        } while (n > 0);
        return result;
    }

    size_t hash(s64 n)
    {
        u64 val = (n > 0) ? static_cast<u64>(n) : static_cast<u64>(INT8_MAX) + static_cast<u64>(-(n + 1)) + 1;
        return hash(val);
    }

    size_t hash(f32 n)
    {
        int exp;
        f32 temp = frexp(n, &exp);
        if (temp < 0)
        {
            exp += 254;
            temp = -temp;
        }
       
        size_t exp2 = static_cast<size_t>(exp);       
        size_t mantissa2 = static_cast<size_t>(f32_to_u32(temp));

        size_t result = 0;
        while (mantissa2 > 0)
        {
            hash_combine(result, mantissa2);
            mantissa2 >>= 6;
        }

        while (exp2 > 0)
        {
            hash_combine(result, exp2);
            exp2 >>= 3;
        }
        return result;
    }

    size_t hash(f64 n)
    {
        int exp;
        f64 temp = frexp(n, &exp);
        if (temp < 0)
        {
            exp += 2046;
            temp = -temp;
        }

        size_t exp2 = static_cast<size_t>(exp);
        size_t mantissa2 = static_cast<size_t>(f64_to_u64(temp));
             
        size_t result = 0;
        do
        {
            hash_combine(result, mantissa2);
            mantissa2 >>= 10;
        } while (mantissa2 > 0);

        do
        {
            hash_combine(result, exp2);
            exp2 >>= 4;
        } while (exp2 > 0);
        return result;
    }

    size_t hash(const char* s)
    {
        size_t result = 0;
        const size_t s_len = strlen(s);
        for (size_t i = 0; i < s_len; i++)
            hash_combine(result, s[i]); //hash_value = 31 * hash_value + s[i];
        return result;
    }

    size_t hash(const ers::String& str)
    {       
        const char* s = str.GetCstr();
        const size_t s_len = str.GetSize();
        size_t result = 0;
        for (size_t i = 0; i < s_len; i++)
            hash_combine(result, s[i]); //hash_value = 31 * hash_value + s[i];
        return result;
    }

    size_t hash(const ers::StringView& sv)
    {      
        const char* s = sv.data;
        const size_t s_len = sv.size;
        size_t result = 0;
        for (size_t i = 0; i < s_len; i++)
            hash_combine(result, s[i]); //hash_value = 31 * hash_value + s[i];
        return result;
    }

    size_t hash(void* p)
    {
        return hash((u64)(uintptr_t)p);
    }

    void hash_combine(size_t& seed, size_t value)
    {
        // 0x9e3779b9 = inverse golden ratio
        seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    u32 pcg_hash(u32 input)
	{
		u32 state = input * 747796405u + 2891336453u;
		u32 word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
		return (word >> 22u) ^ word;
	}

    // ----------------------------
    // :::::::::ENDIANNESS:::::::::
    // ----------------------------

    Endianness get_endianness() 
    {
        static Endianness platform_endianness = Endianness::UNKNOWN_ENDIAN;
        static int was_endianness_cached = 0;
        if (!was_endianness_cached)
        {
            const u16 test_value = 0x1234;
            const u8* test_value_as_char = reinterpret_cast<const u8*>(&test_value);
            platform_endianness = test_value_as_char[0] == 0x34 ? Endianness::LITTLE_ENDIAN :
                (test_value_as_char[0] == 0x12 ? Endianness::BIG_ENDIAN : Endianness::UNKNOWN_ENDIAN);
            was_endianness_cached = 1;
        }
        return platform_endianness;
    }
}
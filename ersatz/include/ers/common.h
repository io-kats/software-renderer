#ifndef COMMON_H
#define COMMON_H

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstdarg>
#include <cstring>
#include <ctime>
#include <float.h>
#include <cmath>
#include "./ers/macros.h"
#include "./ers/typedefs.h"
#include "./ers/allocators.h"

namespace ers
{
	// ----------------------------
	// ::::::::::::PAIR::::::::::::
	// ----------------------------
	
	template <typename A, typename B>
	struct Pair
	{
		A first;
		B second;
	};

	// ----------------------------
	// ::::::MACHINE EPSILON:::::::
	// ----------------------------

	template<typename T>
	class eps
	{
	public:
		static constexpr T val() { return T(0); }
	};

	template<>
	class eps<f32>
	{
	public:
		static constexpr f32 val() { return 1.1920928955078125e-7f; } // ers::u32_to_f32(ers::f32_to_u32(1.0f) + 1) - 1.0f
	};

	template<>
	class eps<f64>
	{
	public:
		static constexpr f64 val() { return 2.220446049250313080847263336181640625e-16; } // ers::u64_to_f64(ers::f64_to_u64(1.0) + 1) - 1.0
	};

	// ----------------------------
	// ::::::PI, HALF PI, TAU::::::
	// ----------------------------

	template<typename T>
	class PI
	{
	public:
		static constexpr T val() { return T(3); }
	};

	template<>
	class PI<f32>
	{
	public:
		static constexpr f32 val() { return 3.14159265f; }
	};

	template<>
	class PI<f64>
	{
	public:
		static constexpr f64 val() { return 3.141592653589793; } 
	};

	template<typename T>
	class HALF_PI
	{
	public:
		static constexpr T val() { return T(2); }
	};

	template<>
	class HALF_PI<f32>
	{
	public:
		static constexpr f32 val() { return 1.57079632f; }
	};

	template<>
	class HALF_PI<f64>
	{
	public:
		static constexpr f64 val() { return 1.5707963267948966; }
	};

	template<typename T>
	class TAU
	{
	public:
		static constexpr T val() { return T(6); }
	};

	template<>
	class TAU<f32>
	{
	public:
		static constexpr f32 val() { return 6.2831853f; }
	};

	template<>
	class TAU<f64>
	{
	public:
		static constexpr f64 val() { return 6.283185307179586; }
	};

	//extern DefaultAllocator default_alloc;

	// ----------------------------
	// :::::::::OPTIONAL:::::::::::
	// ----------------------------

	// Wrapper class for values to test for validity.
	// Used when reading values from e.g. files 
	// that might not be present.
	template <typename T>
	class Optional
	{
	private:
		T value;
		bool has_value;
		
	public:
		Optional() : value(T()), has_value(false) { }
		void operator =(T rhs) { value = rhs; has_value = true; }
		bool HasValue() { return has_value; }
		T& GetValue() { ERS_WARNF(has_value, "%s", "Optional::GetValue: Value invalid.");  return value; }
	};

	// ----------------------------
	// ::::::::STRING VIEW:::::::::
	// ----------------------------

	class String;

	struct StringView
	{
		StringView() = default;
		StringView(const char* s_);
		StringView(const char* s_, size_t size);
		StringView(const char* start_, const char* end_);
		StringView(String s_);

		bool StartsWith(const char* s_);
		bool StartsWith(const char* s_, size_t s_len);

		bool operator==(const StringView& sv_in) const;
		bool operator!=(const StringView& sv_in) const;

		bool operator==(const char* s_in) const;
		bool operator!=(const char* s_in) const;

		bool operator==(const String& s_in) const;
		bool operator!=(const String& s_in) const;

		size_t size;
		const char* data;
	};

	// ----------------------------
	// :::::::::::STRING:::::::::::
	// ----------------------------

	// Class for a buffer containing a C-style string.
	// Allocates size of string + 3 bytes (1 byte for zero termination and 2 extra bytes for branchless utf-8/16 encoding),
	// with a minimum capacity of 16, size of 13 bytes.
	constexpr size_t ERS_SMALL_STRING = 16;
	class String
	{
	private:
		size_t size;
		size_t capacity; // max_size + 3
		char* data;
		IAllocator* alloc;

		void Set();

	public:
		String();
		String(IAllocator* alloc_);
		String(const size_t& capacity_, IAllocator* alloc_ = &default_alloc);
		String(const char* c, IAllocator* alloc_ = &default_alloc); // copies c.
		String(const char* c, size_t c_len, IAllocator* alloc_ = &default_alloc);
		String(FILE* file_, IAllocator* alloc_ = &default_alloc);
		String(const String& s_in);
		String(const StringView& sv_in, IAllocator* alloc_ = &default_alloc);
		String(String&& s_in) noexcept;
		~String();

		String& operator=(const char* c);
		String& operator=(const String& s_in);
		String& operator=(String&& s_in) noexcept;
		String& operator=(const StringView& sv_in);

		char& operator[] (size_t i);
		char operator[] (size_t i) const;

		String operator+(const char* c) const;
		String operator+(const String& s_rhs) const;

		String& operator+=(const char* c);
		String& operator+=(const String& s_in);
		String& operator+=(char c);

		bool operator==(const String& s_in) const;
		bool operator!=(const String& s_in) const;

		bool operator==(const char* c) const;
		bool operator!=(const char* c) const;

		bool operator==(const StringView& sv_in) const;
		bool operator!=(const StringView& sv_in) const;

		void Resize(const size_t& new_size);
		void Destroy();

		void Reverse();
		void Clear();

		char* GetData() const;
		const char* GetCstr() const;
		StringView GetStringView() const;
		size_t GetSize() const;

		void AppendFile(FILE* file);
		void AppendCodepoint(u32 cp);
		void Sprintf(const char* fmt, ...); // copies formatted string to the data buffer.
		void AppendSprintf(const char* fmt, ...); // appends formatted string to the data buffer.

		friend String operator+(const char* c, const String& s);

	private:
		void vsnprintfWrapper(const char*  fmt, va_list args, size_t index);
		inline void nullTerminate(); // 3 extra bytes for branchless utf-8/16 decoding.
	};

	String u64_to_string(u64 n); // u64 to string
	String int_to_string(int n); // int to string	

	/*void u64_to_string(char* s, u64 n);
	void s64_to_string(char* s, s64 n);
	void f64_to_string(char* s, f64 n);
	void string_to_string(char* s, StringView from);*/

	size_t utf8_length(const u8* s); // length for utf8_to_codepoint and for incementing index.
	u32 utf8_to_codepoint(const u8* s, size_t length);
	u32 utf8_to_codepoint(const u8* s, size_t* index);
	// e.g.:
	// const u8* s = reinterpret_cast<const u8*>(file.GetData());
	// size_t advance = 0;
	// for (size_t i = 0; i < size; i += advance)
	// {
	// 	advance = utf8_length(s + i);
	// 	codepoint = utf8_to_codepoint(s + i, advance);
	// 	etc...
	// }

	u32 utf16_to_codepoint(const u8* s, bool swap); // returns unicode codepoint for utf16. swap = true if endianness is opposite to the platoform's.	
    inline size_t utf16_length(u32 codepoint); // returns length in u8/char based on codepoint from utf16_to_codepoint, used for incementing index.
	u32 utf16_to_codepoint(const u8* s, size_t* index, bool swap); // same as utf16_to_codepoint, doesn't need utf16_length to use.
	// e.g.:
	// const u8* s = reinterpret_cast<const u8*>(file.GetData());
	// size_t advance = 0;
	// for (size_t i = 0; i < size; i += advance)
	// {
	// 	codepoint = utf16_to_codepoint(s + i, true);
	//  advance = utf16_length(codepoint);
	// 	etc...
	// }

	bool is_digit(char c);
	bool is_hex_digit(char c);
	bool is_space(char c);

	// ----------------------------
	// ::::::::FILE READER:::::::::
	// ----------------------------

	// Small throwaway class for reading files with zero-termination.
	// TODO: Deprecate it, since its functionality was replaced by using a special String constructor
	// and the get_file_length/read_file_into_buffer functions.
	class FileReader
	{
	private:
		size_t size;
		char* data;		

	public:
		FileReader();
		FileReader(const char* filepath);
		~FileReader();

		int ReadFile(const char* filepath);

		char* GetData();
		size_t GetSize();

		const char* GetCstr() const;
		StringView GetStringView() const;
	};
	
	long get_file_length(FILE* file);
	int read_file_into_buffer(FILE* file, char* read_buffer, size_t buffer_size);
	// Simple non-class based API to read files into buffers. Uses fread, ftell, fseek under the hood.
	// The users have to provide a FILE pointer and allocate storage themselves.
	// get_file_length: Returns 0 for failure, positive number for file length.
	// read_file_into_buffer: Returns 0 for failure, positive number for success.
	// Failure is propagated through read_file_into_buffer, so you only really have to check after using both functions.
	// E.g.:
	// FILE* f = fopen(filepath, "rb");
	// size_t buffer_size = (size_t)ers::get_file_length(f);
	// char* buffer = new char[buffer_size + 1]; // can use malloc/free too or any other allocator.
	// int rc = ers::read_file_into_buffer(f, buffer, buffer_size);
	// if (rc == 0)
	//     delete[] buffer;
	// else
	//     buffer[buffer_size] = 0;
	// fclose(f);

	// ----------------------------
	// ::::::::MATH HELPERS::::::::
	// ----------------------------

	// Bit-casting-like functions to access the bit representation
	// of floats (to avoid unions or weird casting). And their inverses.
	u32 f32_to_u32(f32 x);
	u64 f64_to_u64(f64 x);

	f32 u32_to_f32(u32 x);
	f64 u64_to_f64(u64 x);

	template <typename T>
	inline T saw(T time, T min_val, T max_val, T period, T peak_time_perc)
	{
		peak_time_perc = clamp(peak_time_perc, T(0.001), T(0.999));
        time = fmod(time, period);
        T diff = max_val - min_val;
        if (time > peak_time_perc * period)
        {
            peak_time_perc -= T(1);
            min_val -= diff / peak_time_perc;
        }
        return time * diff / (peak_time_perc * period) + min_val;
	}

	template <typename T>
	inline T tri(T time, T min_val, T max_val, T period)
    {
        return saw(time, min_val, max_val, period, T(0.5));
    }

	template <typename T>
	inline T sin_norm(T time, T min_val, T max_val, T period)
    {
        return (sinf(ers::TAU<T>::val() * time / period) * T(0.5) + T(0.5)) * (max_val - min_val) + min_val;
    }

	template <typename T>
	inline T radians(T angle)
    {
        return ers::PI<T>::val() * angle / T(180.0);
    }

	f32 fast_rsqrt(f32 x);

	template <typename T>
	inline void swap(T& x, T& y)
	{
		T temp(std::move(x));
		x = std::move(y);
		y = std::move(temp);
	}

	template <typename T>
	inline T step(T edge, T x)
	{
		return T(x < edge);
	}

	template <typename T>
	inline T clamp(T val, T min_val, T max_val)
	{
		return (val > max_val) 
		? 
		max_val 
		: 
		((val < min_val) ? min_val : val);
	}

	template <typename T>
	inline T smoothstep(T edge0, T edge1, T x)
	{
		T t = clamp((x - edge0) / (edge1 - edge0), T(0), T(1));
		return t * t * (T(3) - t * T(2));
	}

	template <typename T>
	inline T lerp(T t, T a, T b) { return a + (b - a) * t; }

	template <typename T>
	inline T abs(T x) { return (x < T(0) ? -x : x); }

	template <typename T>
	inline T min(T a, T b) { return a < b ? a : b; }

	template <typename T>
	inline T max(T a, T b) { return a > b ? a : b; }

	template <typename T>
	inline T map(T val, T x0, T xf, T new_x0, T new_xf)
	{
		ERS_ASSERT(xf != x0);
		return new_x0 + ((new_xf - new_x0) / (xf - x0)) * (val - x0);
	}

	template <typename T>
	inline T approach(T from, T to, T amount) 
	{ 
		T result;
		if (from == to)
		{
			result = from;	
		}
		else if (from > to)
		{
			from -= amount;
			result = (from < to) ? to : from;
		}
		else
		{
			from += amount;
			result = (from > to) ? to : from;			
		}

		return result; 
	}

	template <typename T>
	inline T equals_epsilon(T x, T y, T epsilon = eps<T>::val())
	{
		return abs(x - y) <= epsilon * max(abs(x), max(abs(y), T(1)));
	}

	template <typename T>
	inline T equals_epsilon_approx(T x, T y, T epsilon = eps<T>::val())
	{
		return abs(x - y) <= epsilon * (abs(x) + abs(y) + T(1));
	}

	template <typename T>
	inline T copy_sign(T x, T y) // returns sign(y) * abs(x)
	{
		x = ers::abs(x);
		return y < T(0) ? -x : (y > T(0) ? x : T(0));
	}

	template <typename T>
	inline T sign(T x)
	{
		return x < T(0) ? -T(1) : (x > T(0) ? T(1) : T(0));
	}

	// ----------------------------
	// ::::::::::PARSERS:::::::::::
	// ----------------------------

	// Helper functions for parsing text into a type.
	// TODO: u64, u32 and s64 parsers.
	// TODO: deprecate parse_string? seems way too trivial
	s64 parse_int(const char* start, const char* finish);	
	f64 parse_double(const char* start, const char* finish); // incorrect in general (doubles), but works for what I want it for, i.e. seems to be able to parse all floats correctly (except NaN/+-Inf).
	f32 parse_float(const char* start, const char* finish);
	String parse_string(const char* start, const char* finish);

	// ----------------------------
	// :::::STRING PROCESSOR:::::::
	// ----------------------------

	// Lightweight StringView-like class for manually parsing strings.
	// Used to e.g. parse text files copied into a buffer with FileReader.
	// It does not allocate any extra memory.
	// TODO: rework into a stream-like base class.
	class StringProcessor
	{
	private:
		const char* start;
		const char* end;
		const char* position;
		const char* lookahead;

	public:
		StringProcessor();
		StringProcessor(const char* start_, size_t size);
		StringProcessor(const char* start_, size_t size, const char* position_);
		StringProcessor(const char* start_, const char* end_, const char* position_);

		void SkipTo(char delimiter); // skip to first occurence of delimiter from the current position
		void SkipLine(); // skip to first occurence of '\n' from the current position and advance by 1.
		void SkipWhitespace(); // skip to first occurence of non-whitespace from the current position.
		void SkipToWhitespace(); // skip to first occurence of whitespace from the current position.
		void Reset(); // reset 'position' to 'start'
		char Current(); // get the character currently pointed to.
		char Next(); // get the character next to the one currently pointed to.
		void Advance(ptrdiff_t n = 1); // advance by 'n' from the current position.
		bool CanAdvance(); // check if we've gone past the end of the buffer.
		bool StartsWith(const char* s); // check if the current position starts with the string pointed to by s.
		const char* GetPosition(); // get the current position

		// The delimiter's default value stands for whitespace.
		f32 GetFloat(const char delimiter = -1); // parses the following characters as if they represented a float, up to and excluding 'delimiter'
		int GetInt(const char delimiter = -1); // parses the following characters as if they represented an int, up to and excluding 'delimiter'
		String GetString(const char delimiter = -1); // parses the following characters as if they represented a string, up to and excluding 'delimiter'
	
	private:
		void getLookahead(char delimiter); // helper function for parsing floats, ints and strings.
	};

	// ----------------------------
	// ::::::::::::RNG:::::::::::::
	// ----------------------------

	// Lehmer random number generator.
	// Taken from wikipedia: https://en.wikipedia.org/wiki/Lehmer_random_number_generator#Sample_C99_code	
	constexpr u32 ERS_MAX_RAND = 0x7fffffff;
	constexpr double ERS_PROBABILITY = 1.0 / static_cast<double>(ERS_MAX_RAND + 1); // Probability that a certain number is produced.
	void lcg_parkmiller(u32& state);
	void init_rand();
	u32 random();
	double random_frac();
	double random_frac(double min, double max);
	u32 random_at_most(u32 max);
	u32 random_range(u32 min, u32 max);

	// ----------------------------
	// :::::::HASH FUNCTIONS:::::::
	// ----------------------------
	size_t hash(u8 n);
	size_t hash(s8 n);
	size_t hash(u16 n);
	size_t hash(s16 n);
	size_t hash(u32 n);
	size_t hash(s32 n);
	size_t hash(u64 n);
	size_t hash(s64 n);
	size_t hash(f32 n);
	size_t hash(f64 n);
	size_t hash(const char* s);
	size_t hash(const ers::String& s);	
	size_t hash(const ers::StringView& s);
	size_t hash(void* p);

	// Boost's hash_combine: https://www.boost.org/doc/libs/1_64_0/boost/functional/hash/hash.hpp
	// License here (also to be found in the the ./include/ers folder): https://www.boost.org/LICENSE_1_0.txt
	void hash_combine(size_t& seed, size_t value);

	// https://www.reedbeta.com/blog/hash-functions-for-gpu-rendering/
	u32 pcg_hash(u32 input);

	// ----------------------------
	// :::::::::ENDIANNESS:::::::::
	// ----------------------------

	enum class Endianness
	{
		LITTLE_ENDIAN,
		BIG_ENDIAN,
		UNKNOWN_ENDIAN
	};
	
	Endianness get_endianness();
}

#endif // COMMON_H

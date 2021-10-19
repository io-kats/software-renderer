#ifndef ERS_VECTOR_H
#define ERS_VECTOR_H

#include "./ers/common.h"
#include "./ers/allocators.h"
#include "./ers/macros.h"

namespace ers
{	
	//extern DefaultAllocator default_alloc;
	constexpr size_t ERS_MINIMUM_VECTOR_CAPACITY = 4;

	// ----------------------------
	// :::::::::::VECTOR:::::::::::
	// ----------------------------	

	// Simple std::vector lookalike class. Atm more like a stack.

	// Interface.
	template <typename T>
	class Vector
	{
	private:
		size_t size;
		size_t capacity;
		T* data;
		IAllocator* alloc;

	public:
		Vector();
		Vector(IAllocator* alloc_);
		Vector(size_t size_, IAllocator* alloc_ = &default_alloc);

		virtual ~Vector();

		Vector(Vector&& v) noexcept;
		Vector& operator =(Vector&& v) noexcept;

		Vector(const Vector& v);
		Vector& operator =(const Vector& v);

		T& operator[] (size_t i);
		const T& operator[] (size_t i) const;

		// For ranged-for loops and iteration.
		T* begin();
		const T* begin() const;
		T* end();
		const T* end() const;

		inline void Clear();
		inline void Destroy();

		inline void PushFront(const T& value);
		inline void PushFront(T&& value);

		inline T PopFront();

		inline void PushBack(const T& value);
		inline void PushBack(T&& value);

		inline T PopBack();
		inline void Reserve(size_t new_size); // reserves enough space to fit the new size.
		inline void Resize(size_t new_size);

		inline T* Find(const T& val);
		inline size_t FindIndex(const T& val);

		inline void EraseShift(T* it, size_t amount = 1);
		inline void EraseShift(size_t idx, size_t amount = 1);

		inline void EraseSwap(T* it, size_t amount = 1);
		inline void EraseSwap(size_t idx, size_t amount = 1);

		inline size_t GetSize() const;
		inline size_t GetCapacity() const;
	};

	/////////////////////////////////////////////////////////////////////////////////////////
	// Implementation.

	template <typename T>
	Vector<T>::Vector()
		: size(0), capacity(0), data(nullptr), alloc(&default_alloc)
	{
		Reserve(ERS_MINIMUM_VECTOR_CAPACITY);
	}

	template <typename T>
	Vector<T>::Vector(IAllocator* alloc_)
		: size(0), capacity(0), data(nullptr), alloc(alloc_)
	{
		Reserve(ERS_MINIMUM_VECTOR_CAPACITY);
	}

	template <typename T>
	Vector<T>::Vector(size_t size_, IAllocator* alloc_)
		: size(0), capacity(0), data(nullptr), alloc(alloc_)
	{
		Reserve(size_);
	}

	template <typename T>
	Vector<T>::~Vector()
	{
		Destroy();
	}

	template <typename T>
	Vector<T>::Vector(Vector&& v) noexcept
	{
		size = v.size;
		capacity = v.capacity;
		data = v.data;
		alloc = v.alloc;

		v.size = 0;
		v.capacity = 0;
		v.data = nullptr;
	}

	template <typename T>
	Vector<T>& Vector<T>::operator =(Vector&& v) noexcept
	{
		if (this != &v)
		{
			size_t temp_size = size;
			size_t temp_capacity = capacity;
			T* temp_data = data;
			IAllocator* temp_alloc = alloc;

			size = v.size;
			capacity = v.capacity;
			data = v.data;
			alloc = v.alloc;

			v.size = temp_size;
			v.capacity = temp_capacity;
			v.data = temp_data;
			v.alloc = temp_alloc;
		}
		return *this;
	}

	template <typename T>
	Vector<T>::Vector(const Vector& v)
		: size(0), capacity(0), data(nullptr), alloc(v.alloc)
	{
		Reserve(v.capacity);
		size = v.size;
		for (size_t i = 0; i < size; ++i)
			new(data + i) T(v.data[i]);
	}

	template <typename T>
	Vector<T>& Vector<T>::operator =(const Vector& v)
	{
		if (this != &v)
		{
			Clear();
			Reserve(v.capacity);
			size = v.size;
			for (size_t i = 0; i < size; ++i)
				new(data + i) T(v.data[i]);			
		}
		return *this;
	}

	template <typename T>
	T& Vector<T>::operator[](size_t i)
	{
		ERS_ASSERTF(i < size, "%s", "Vector::operator[]: Out of bounds.");
		return data[i];
	}

	template <typename T>
	const T& Vector<T>::operator[](size_t i) const
	{
		ERS_ASSERTF(i < size, "%s", "Vector::operator[]: Out of bounds.");
		return data[i];
	}

	template <typename T>
	T* Vector<T>::begin()
	{
		return data;
	}

	template <typename T>
	const T* Vector<T>::begin() const
	{
		return data;
	}

	template <typename T>
	T* Vector<T>::end()
	{
		return data + size;
	}

	template <typename T>
	const T* Vector<T>::end() const
	{
		return data + size;
	}

	template <typename T>
	inline void Vector<T>::Clear()
	{
		for (size_t i = 0; i < size; ++i)
		{
			data[i].~T();
		}
		size = 0;
	}

	template <typename T>
	inline void Vector<T>::Destroy()
	{
		Clear();
		alloc->Deallocate((void*)data);
	}

	template <typename T>
	inline void Vector<T>::PushFront(const T& value)
	{		
		Reserve(size + 1);		
		new(data + size) T(std::move(data[size - 1]));
		for (size_t i = size - 1; i > 0; i--)
			data[i] = std::move(data[i - 1]);		
		++size;
		data->~T();
		new(data) T(value);
	}

	template <typename T>
	inline void Vector<T>::PushFront(T&& value)
	{
		Reserve(size + 1);
		new(data + size) T(std::move(data[size - 1]));
		for (size_t i = size - 1; i > 0; i--)
			data[i] = std::move(data[i - 1]);
		++size;
		data->~T();
		new(data) T(value);
	}

	template <typename T>
	inline T Vector<T>::PopFront()
	{
		ERS_ASSERTF(size > 0, "%s", "Vector::PopFront: Empty vector.");
		T out = T(std::move(data[0]));
		EraseShift(data);
		return out;
	}

	template <typename T>
	inline void Vector<T>::PushBack(const T& value)
	{
		Reserve(size + 1);
		new(data + size) T(value);
		++size;
	}

	template <typename T>
	inline void Vector<T>::PushBack(T&& value)
	{
		Reserve(size + 1);
		new(data + size) T(std::move(value));
		++size;
	}

	template <typename T>
	inline T Vector<T>::PopBack()
	{
		ERS_ASSERTF(size > 0, "%s", "Vector::PopBack: Empty vector.");
		--size;
		T out = T(std::move(data[size]));
		data[size].~T();		
		return out;
	}

	template <typename T>
	inline void Vector<T>::Reserve(size_t new_size)
	{
		if (new_size > capacity)
		{
			// At construction.
			if (capacity == 0) 
				capacity = new_size;
				//capacity = ERS_MINIMUM_VECTOR_CAPACITY;

			// Double capacity, until it fits the new size.
			while (new_size > capacity)
				capacity *= 2;

			// Reallocate.
			T* temp_buffer = (T*)alloc->Allocate(capacity * sizeof(T), alignof(T));
			ERS_ASSERTF(temp_buffer != nullptr, "%s", "Vector::Reserve: Could not allocate memory.");
			for (size_t i = 0; i < size; ++i)
			{
				new(temp_buffer + i) T(std::move(data[i]));
				data[i].~T();
			}

			alloc->Deallocate((void*)data);
			data = temp_buffer;
		}
	}

	template <typename T>
	inline void Vector<T>::Resize(size_t new_size)
	{
		if (new_size <= size)
		{
			for (size_t i = new_size; i < size; ++i)
				data[i].~T();
		}
		else
		{
			Reserve(new_size);
			for (size_t i = size; i < new_size; ++i)
				new(data + i) T();
		}
		size = new_size;
	}

	template <typename T>
	inline T* Vector<T>::Find(const T& val)
	{
		for (size_t i = 0; i < size; ++i)
			if (data[i] == val)
				return data + i;
		return nullptr;
	}

	template <typename T>
	inline size_t Vector<T>::FindIndex(const T& val)
	{
		for (size_t i = 0; i < size; ++i)
			if (data[i] == val)
				return i;
		return capacity;
	}

	template <typename T>
	inline void Vector<T>::EraseShift(T* it, size_t amount)
	{
		
		ERS_ASSERTF(amount <= size && it - data <= size - amount, "%s", "Vector::EraseShift: Out of bounds.");
		if (amount > 0)
		{
			T* finish = data + size;
			T* from_end = finish - amount;
			for (T* d = it; d != from_end; d++)
				*d = std::move(*(d + amount));
			for (T* d = from_end; d != finish; d++)
				d->~T();
			size -= amount;
		}
	}

	template <typename T>
	inline void Vector<T>::EraseShift(size_t idx, size_t amount)
	{
		ERS_ASSERTF(amount <= size && idx <= size - amount, "%s", "Vector::EraseShift: Out of bounds.");
		if (amount > 0)
		{
			size_t from_end = size - amount;
			for (size_t i = idx; i < from_end; i++)
				data[i] = std::move(data[i + amount]);
			for (size_t i = from_end; i < size; i++)
				data[i].~T();		
			size -= amount;
		}
	}

	template <typename T>
	inline void Vector<T>::EraseSwap(T* it, size_t amount)
	{
		ERS_ASSERTF(amount <= size && it - data <= size - amount, "%s", "Vector::EraseSwap: Out of bounds.");
		if (amount > 0)
		{
			T* from_end = data + size;
			T* p = it;
			T* stop = it + amount;
			while (p != stop)
			{
				*p++ = std::move(*--from_end);
				from_end->~T();
			}
			size -= amount;			
		}
	}

	template <typename T>
	inline void Vector<T>::EraseSwap(size_t idx, size_t amount)
	{
		ERS_ASSERTF(amount <= size && idx <= size - amount, "%s", "Vector::EraseSwap: Out of bounds.");
		if (amount > 0)
		{
			T* from_end = data + size;
			T* p = data + idx;
			T* stop = p + amount;
			while (p != stop)
			{
				*p++ = std::move(*--from_end);
				from_end->~T();
			}
			size -= amount;
		}
	}

	template <typename T>
	inline size_t Vector<T>::GetSize() const
	{
		return size;
	}

	template <typename T>
	inline size_t Vector<T>::GetCapacity() const
	{
		return capacity;
	}
}

#endif // ERS_VECTOR_H

#ifndef ALLOCATORS_H
#define ALLOCATORS_H

#include <new>
#include <malloc.h>
#include "./ers/macros.h"
#include "./ers/typedefs.h"

// Allocators for raw memory. Under the hood using malloc/realloc/free or new/delete.
namespace ers
{
	u8* next_aligned(u8* p, size_t alignment);

	// ----------------------------
	// :::::ALLOCATOR INTERFACE::::
	// ----------------------------

	class IAllocator 
	{
	public:
		virtual void* Allocate(size_t size, size_t alignment) = 0;
		virtual void* Reallocate(void* p, size_t size, size_t alignment) = 0;
		virtual void Deallocate(void* p) = 0;
	};

	// ----------------------------
	// :::::::::NEW/DELETE:::::::::
	// ----------------------------

	// Wrapper allocator class for new/delete.
	class NewDeleteAllocator : public IAllocator
	{
	public:
		NewDeleteAllocator();

		NewDeleteAllocator(NewDeleteAllocator&&) = default;
		NewDeleteAllocator& operator =(NewDeleteAllocator&&) = default;

		NewDeleteAllocator(const NewDeleteAllocator&) = delete;
		NewDeleteAllocator& operator =(const NewDeleteAllocator&) = delete;

		~NewDeleteAllocator();

		virtual void* Allocate(size_t size, size_t alignment);
		virtual void* Reallocate(void* p, size_t new_size, size_t alignment); // 
		virtual void Deallocate(void* p);
	};

	// ----------------------------
	// :::::::::::MALLOC:::::::::::
	// ----------------------------

	// Wrapper allocator class for malloc.
	class MallocAllocator : public IAllocator
	{
	public:
		MallocAllocator();

		MallocAllocator(MallocAllocator&&) = default;
		MallocAllocator& operator =(MallocAllocator&&) = default;

		MallocAllocator(const MallocAllocator&) = delete;
		MallocAllocator& operator =(const MallocAllocator&) = delete;

		~MallocAllocator();

		virtual void* Allocate(size_t size, size_t alignment);
		virtual void* Reallocate(void* p, size_t new_size, size_t alignment);
		virtual void Deallocate(void* p);
	};

	// ----------------------------
	// :::::::VERBOSE MALLOC:::::::
	// ----------------------------

	// Wrapper allocator class for malloc.
	// Atm only keeps track of how many allocations and 
	// deallocations took place, as well as how many bytes were allocated.
	class VerboseMalloc : public IAllocator
	{
	private:
		size_t m_countAllocations;
		size_t m_countDeallocations;
		size_t m_countBytesAllocated;
		size_t m_countBytesDeallocated;

	public:
		VerboseMalloc();

		VerboseMalloc(VerboseMalloc&&) = default;
		VerboseMalloc& operator =(VerboseMalloc&&) = default;

		VerboseMalloc(const VerboseMalloc&) = delete;
		VerboseMalloc& operator =(const VerboseMalloc&) = delete;

		~VerboseMalloc();

		virtual void* Allocate(size_t size, size_t alignment);
		virtual void* Reallocate(void* p, size_t new_size, size_t alignment);
		virtual void Deallocate(void* p);

		void Print();
	};

	// ----------------------------
	// ::::::MONOTONIC MANAGER:::::
	// ----------------------------
	// Monotonic allocator -manager-. 
	// Accepts buffer base and capacity.
	// Does not release memory on deallocation OR destruction,
	// as it should not own the buffer (does provide a helper function for it though).
	// Does not support reallocation.
	class MonotonicAllocatorManager : public IAllocator
	{
	public:
		struct Marker
		{
			size_t bytes_occupied;
		};

		size_t bytes_occupied;
		size_t capacity;
		u8* buffer;

		MonotonicAllocatorManager(u8* buffer_, size_t capacity_);

		MonotonicAllocatorManager(const MonotonicAllocatorManager&) = delete;
		MonotonicAllocatorManager& operator =(const MonotonicAllocatorManager&) = delete;

		MonotonicAllocatorManager(MonotonicAllocatorManager&& m) noexcept;
		MonotonicAllocatorManager& operator =(MonotonicAllocatorManager&& m) noexcept;

		virtual ~MonotonicAllocatorManager();

		virtual void* Allocate(size_t size, size_t alignment);
		virtual void* Reallocate(void* p, size_t new_size, size_t alignment);
		virtual void Deallocate(void* p);
		void Clear();
		void Destroy();

		Marker GetMarker();
		void SetMarker(Marker marker);
	};

	// ----------------------------
	// :::::::::MONOTONIC::::::::::
	// ----------------------------
	// Monotonic allocator. Does not release memory on deallocation.
	// Allows reallocation, but does not release the now freed memory.
	class MonotonicAllocator : public IAllocator
	{
	public:
		struct Marker
		{
			size_t bytes_occupied;
		};

		size_t bytes_occupied;
		size_t capacity;
		u8* buffer;

		MonotonicAllocator(size_t capacity_);

		MonotonicAllocator(const MonotonicAllocator&) = delete;
		MonotonicAllocator& operator =(const MonotonicAllocator&) = delete;

		MonotonicAllocator(MonotonicAllocator&& m) noexcept;
		MonotonicAllocator& operator =(MonotonicAllocator&& m) noexcept;

		virtual ~MonotonicAllocator();

		virtual void* Allocate(size_t size, size_t alignment);
		virtual void* Reallocate(void* p, size_t new_size, size_t alignment);
		virtual void Deallocate(void* p);
		void Clear();
		void Destroy();

		Marker GetMarker();
		void SetMarker(Marker marker);	
	};

	// ----------------------------
	// :::::::::::STACK::::::::::::
	// ----------------------------
	// Stack allocator. Releases memory on deallocation only the pointer passed
	// as an argument for deallocation was the last one allocated.
	class StackAllocator : public IAllocator
	{
	public:
		struct Marker
		{
			size_t bytes_occupied;
			u8* previous;
		};

		StackAllocator(size_t capacity_);

		StackAllocator(const StackAllocator&) = delete;
		StackAllocator& operator =(const StackAllocator&) = delete;

		StackAllocator(StackAllocator&& m) noexcept;
		StackAllocator& operator =(StackAllocator&& m) noexcept;

		virtual ~StackAllocator();

		virtual void* Allocate(size_t size, size_t alignment);
		virtual void* Reallocate(void* p, size_t new_size, size_t alignment);
		virtual void Deallocate(void* p);
		void Clear();
		void Destroy();

		Marker GetMarker();
		void SetMarker(Marker marker);	

	private:
		size_t bytes_occupied;
		size_t capacity;
		u8* buffer;
		u8* previous;
	};

	// ----------------------------
	// ::::::::::::ARENA:::::::::::
	// ----------------------------	
	// Linked list of monotonic allocators.
	// TODO: rework the whole class.
	constexpr size_t ERS_MINIMUM_REGION_CAPACITY = 512;
	class Arena : public IAllocator
	{
	public:
		struct Region 
		{
			Region* next;
			MonotonicAllocator* m;		
		};

		Region* head;
		Region* tail;
		size_t region_count;
		size_t minimum_region_capacity;

		Arena();
		Arena(size_t minimum_region_capacity_);
		virtual ~Arena();

		Arena(const Arena&) = delete;
		Arena& operator=(const Arena&) = delete;

		Arena(Arena&& m) noexcept;
		Arena& operator=(Arena&& m) noexcept;

		virtual void* Allocate(size_t size, size_t alignment);
		virtual void* Reallocate(void* p, size_t new_size, size_t alignment);
		virtual void Deallocate(void* p);
		void Clear();
	};

	// ----------------------------
	// --DEFINE ALLOCATOR FOR APP--
	// ----------------------------	
	typedef MallocAllocator DefaultAllocator;
	extern DefaultAllocator default_alloc;
}

#endif // ALLOCATORS_H

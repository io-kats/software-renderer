#include "../include/ers/allocators.h"
namespace ers
{
	DefaultAllocator default_alloc = DefaultAllocator();

	u8* next_aligned(u8* p, size_t alignment)
	{
		ERS_ASSERTF((alignment & (alignment - 1)) == 0, 
			"%s", 
			"MonotonicAllocator::Allocate: Alignment must be a power of 2.");
		u8* next_aligned_position = (u8*)(((uintptr_t)p + alignment - 1) & ~(alignment - 1));
		ERS_ASSERTF((uintptr_t)next_aligned_position % alignment == 0, 
			"%s", 
			"MonotonicAllocator::Allocate: Alignment failure.");
		return next_aligned_position;
	}
	
	// ----------------------------
	// :::::::::NEW/DELETE:::::::::
	// ----------------------------

	NewDeleteAllocator::NewDeleteAllocator() { }

	NewDeleteAllocator::~NewDeleteAllocator() { }

	inline void* NewDeleteAllocator::Allocate(size_t size, size_t alignment)
	{
		ERS_UNUSED(alignment);
		u8* pt = new u8[size];
		return reinterpret_cast<void*>(pt);
	}

	void* NewDeleteAllocator::Reallocate(void* p, size_t size, size_t alignment)
	{
		ERS_UNUSED(p);
		ERS_UNUSED(size);
		ERS_UNUSED(alignment);
		ERS_ASSERT(false);
		return nullptr;
	}

	inline void NewDeleteAllocator::Deallocate(void* p)
	{
		u8* pt = reinterpret_cast<u8*>(p);
		delete[] pt;
	}

	// ----------------------------
	// :::::::::::MALLOC:::::::::::
	// ----------------------------

	MallocAllocator::MallocAllocator() { }

	MallocAllocator::~MallocAllocator() { }

	inline void* MallocAllocator::Allocate(size_t size, size_t alignment)
	{
		ERS_UNUSED(alignment);
		return malloc(size);
	}

	void* MallocAllocator::Reallocate(void* p, size_t size, size_t alignment)
	{
		ERS_UNUSED(alignment);
		return realloc(p, size);
	}

	inline void MallocAllocator::Deallocate(void* p)
	{
		free(p);
	}

	// ----------------------------
	// :::::::VERBOSE MALLOC:::::::
	// ----------------------------

	VerboseMalloc::VerboseMalloc()
	  : m_countAllocations(0), 
		m_countDeallocations(0), 
		m_countBytesAllocated(0),
		m_countBytesDeallocated(0) 
	{ 
	
	}

	VerboseMalloc::~VerboseMalloc() 
	{ 
		Print();
	}

	inline void* VerboseMalloc::Allocate(size_t size, size_t alignment)
	{
		ERS_UNUSED(alignment);
		++m_countAllocations;
		u8* p = (u8*)malloc(size + sizeof(size_t));
		ERS_ASSERT(p != nullptr);
		size_t* size_position = reinterpret_cast<size_t*>(p);
		*size_position = size;
		m_countBytesAllocated += size;
		return reinterpret_cast<void*>(size_position + 1);
	}

	void* VerboseMalloc::Reallocate(void* p, size_t size, size_t alignment)
	{
		ERS_UNUSED(p);
		ERS_UNUSED(size);
		ERS_UNUSED(alignment);
		ERS_ASSERT(false);
		return nullptr;
	}

	inline void VerboseMalloc::Deallocate(void* p)
	{
		if (p != nullptr)
		{
			++m_countDeallocations;
			size_t* size_position = reinterpret_cast<size_t*>(p) - 1;
			m_countBytesDeallocated += *size_position;
			free(reinterpret_cast<u8*>(size_position));
		}		
	}

	void VerboseMalloc::Print()
	{
		printf("Allocations: %zu\nDeallocations: %zu\nBytes allocated: %zu\nBytes deallocated: %zu\n", 
			m_countAllocations, 
			m_countDeallocations, 
			m_countBytesAllocated,
			m_countBytesDeallocated);
	}

	// ----------------------------
	// :::::::PURE MONOTONIC:::::::
	// ----------------------------

	MonotonicAllocatorManager::MonotonicAllocatorManager(u8* buffer_, size_t capacity_)
		: bytes_occupied(0), capacity(capacity_), buffer(buffer_)
	{
		buffer = buffer_;
		ERS_ASSERTF(buffer != nullptr, 
			"%s", 
			"MonotonicAllocator::MonotonicAllocator: Could not allocate memory.");
		ERS_ASSERTF((uintptr_t)buffer % alignof(u8) == 0, 
			"%s", 
			"MonotonicAllocator::MonotonicAllocator: Alignment failure.");
	}

	MonotonicAllocatorManager::MonotonicAllocatorManager(MonotonicAllocatorManager&& m) noexcept
	{
		bytes_occupied = m.bytes_occupied;
		capacity = m.capacity;
		buffer = m.buffer;

		m.bytes_occupied = 0;
		m.capacity = 0;
		m.buffer = nullptr;
	}

	MonotonicAllocatorManager& MonotonicAllocatorManager::operator =(MonotonicAllocatorManager&& m) noexcept
	{

		size_t temp_bytes_occupied = bytes_occupied;
		size_t temp_capacity = capacity;
		u8* temp_buffer = buffer;

		bytes_occupied = m.bytes_occupied;
		capacity = m.capacity;
		buffer = m.buffer;

		m.bytes_occupied = temp_bytes_occupied;
		m.capacity = temp_capacity;
		m.buffer = temp_buffer;

		return *this;
	}

	MonotonicAllocatorManager::~MonotonicAllocatorManager()
	{
		
	}

	inline void* MonotonicAllocatorManager::Allocate(size_t size, size_t alignment)
	{
		ERS_ASSERTF((alignment & (alignment - 1)) == 0, "%s", "MonotonicAllocator::Allocate: Alignment must be a power of 2.");
		u8* current_position = buffer + bytes_occupied;
		u8* next_aligned_position = (u8*)(((uintptr_t)current_position + alignment - 1) & ~(alignment - 1));
		ERS_ASSERTF((uintptr_t)next_aligned_position % alignment == 0, "%s", "MonotonicAllocator::Allocate: Alignment failure.");

		if (next_aligned_position + size <= buffer + capacity)
		{
			bytes_occupied += (next_aligned_position - current_position) + size;
			return reinterpret_cast<void*>(next_aligned_position);
		}
		else
		{
			return nullptr;
		}
	}

	inline void* MonotonicAllocatorManager::Reallocate(void* p, size_t size, size_t alignment)
	{
		ERS_UNUSED(p);
		ERS_UNUSED(size);
		ERS_UNUSED(alignment);
		return nullptr;
	}

	inline void MonotonicAllocatorManager::Deallocate(void* p)
	{
		ERS_UNUSED(p);
		return; // no op.
	}

	void MonotonicAllocatorManager::Clear()
	{
		bytes_occupied = 0;
	}

	void MonotonicAllocatorManager::Destroy()
	{
		if (buffer != nullptr)
		{
			free(buffer);
			buffer = nullptr;
			bytes_occupied = 0;
		}
	}

	MonotonicAllocatorManager::Marker MonotonicAllocatorManager::GetMarker()
	{
		return { bytes_occupied };
	}

	void MonotonicAllocatorManager::SetMarker(MonotonicAllocatorManager::Marker marker)
	{
		bytes_occupied = marker.bytes_occupied;
	}

	// ----------------------------
	// :::::::::MONOTONIC::::::::::
	// ----------------------------

	MonotonicAllocator::MonotonicAllocator(size_t capacity_)
		: bytes_occupied(0), capacity(capacity_), buffer(nullptr)
	{
		buffer = static_cast<u8*>(calloc(capacity, sizeof(u8)));
		ERS_ASSERTF(buffer != nullptr, "%s", "MonotonicAllocator::MonotonicAllocator: Could not allocate memory.");
		ERS_ASSERTF((uintptr_t)buffer % alignof(u8) == 0, "%s", "MonotonicAllocator::MonotonicAllocator: Alignment failure.");
	}

	MonotonicAllocator::MonotonicAllocator(MonotonicAllocator&& m) noexcept
	{
		bytes_occupied = m.bytes_occupied;
		capacity = m.capacity;
		buffer = m.buffer;

		m.bytes_occupied = 0;
		m.capacity = 0;
		m.buffer = nullptr;
	}

	MonotonicAllocator& MonotonicAllocator::operator =(MonotonicAllocator&& m) noexcept
	{

		size_t temp_bytes_occupied = bytes_occupied;
		size_t temp_capacity = capacity;
		u8* temp_buffer = buffer;

		bytes_occupied = m.bytes_occupied;
		capacity = m.capacity;
		buffer = m.buffer;

		m.bytes_occupied = temp_bytes_occupied;
		m.capacity = temp_capacity;
		m.buffer = temp_buffer;

		return *this;
	}

	MonotonicAllocator::~MonotonicAllocator()
	{
		Destroy();
	}

	inline void* MonotonicAllocator::Allocate(size_t size, size_t alignment)
	{
		ERS_ASSERTF((alignment & (alignment - 1)) == 0, "%s", "MonotonicAllocator::Allocate: Alignment must be a power of 2.");
		u8* current_position = buffer + bytes_occupied;
		u8* next_position = current_position + sizeof(size_t);
		u8* next_aligned_position = (u8*)(((uintptr_t)next_position + alignment - 1) & ~(alignment - 1));
		ERS_ASSERTF((uintptr_t)next_aligned_position % alignment == 0, "%s", "MonotonicAllocator::Allocate: Alignment failure.");

		size_t* size_position = reinterpret_cast<size_t*>(next_aligned_position) - 1;
		*size_position = (next_aligned_position - current_position) + size;

		if (next_aligned_position + size <= buffer + capacity)
		{
			bytes_occupied += (next_aligned_position - current_position) + size;
			return reinterpret_cast<void*>(next_aligned_position);
		}
		else
		{
			return nullptr;
		}
	}

	inline void* MonotonicAllocator::Reallocate(void* p, size_t new_size, size_t alignment)
	{
		void* result = nullptr;
		if (p == nullptr)
		{
			result = Allocate(new_size, alignment);
		}
		else
		{
			size_t* old_size_pointer = reinterpret_cast<size_t*>(p) - 1;
			size_t old_size = *old_size_pointer;
			size_t min_size = (new_size < old_size) ? new_size : old_size;
			u8* p_new = reinterpret_cast<u8*>(Allocate(new_size, alignment));
			memcpy(p_new, p, min_size);	
			result = reinterpret_cast<void*>(p_new);
		}		
		return result;
	}

	inline void MonotonicAllocator::Deallocate(void* p) 
	{ 
		ERS_UNUSED(p);
	}

	void MonotonicAllocator::Clear()
	{
		bytes_occupied = 0;
	}

	void MonotonicAllocator::Destroy()
	{
		if (buffer != nullptr)
		{
			free(buffer);
			buffer = nullptr;
			bytes_occupied = 0;
		}
	}

	MonotonicAllocator::Marker MonotonicAllocator::GetMarker()
	{
		return { bytes_occupied };
	}

	void MonotonicAllocator::SetMarker(MonotonicAllocator::Marker marker)
	{
		bytes_occupied = marker.bytes_occupied;
	}

	// ----------------------------
	// :::::::::::STACK::::::::::::
	// ----------------------------

	StackAllocator::StackAllocator(size_t capacity_)
		: bytes_occupied(0), capacity(capacity_), buffer(nullptr), previous(nullptr)
	{
		buffer = static_cast<u8*>(calloc(capacity, sizeof(u8)));
		ERS_ASSERTF(buffer != nullptr, "%s", "StackAllocator::StackAllocator: Could not allocate memory.");
		ERS_ASSERTF((uintptr_t)buffer % alignof(u8) == 0, "%s", "StackAllocator::StackAllocator: Alignment failure.");
	}

	StackAllocator::StackAllocator(StackAllocator&& m) noexcept
	{
		bytes_occupied = m.bytes_occupied;
		capacity = m.capacity;
		buffer = m.buffer;
		previous = m.previous;

		m.bytes_occupied = 0;
		m.capacity = 0;
		m.buffer = nullptr;
		m.previous = nullptr;
	}

	StackAllocator& StackAllocator::operator =(StackAllocator&& m) noexcept
	{
		size_t temp_bytes_occupied = bytes_occupied;
		size_t temp_capacity = capacity;
		u8* temp_buffer = buffer;
		u8* temp_previous = previous;

		bytes_occupied = m.bytes_occupied;
		capacity = m.capacity;
		buffer = m.buffer;
		previous = m.previous;

		m.bytes_occupied = temp_bytes_occupied;
		m.capacity = temp_capacity;
		m.buffer = temp_buffer;
		m.previous = temp_previous;

		return *this;
	}

	StackAllocator::~StackAllocator()
	{
		Destroy();
	}

	inline void* StackAllocator::Allocate(size_t size, size_t alignment)
	{
		ERS_ASSERTF((alignment & (alignment - 1)) == 0, "%s", "StackAllocator::Allocate: Alignment must be a power of 2.");
		u8* current_position = buffer + bytes_occupied;
		u8* next_position = current_position + sizeof(size_t) + sizeof(u8*);
		u8* next_aligned_position = (u8*)(((uintptr_t)next_position + alignment - 1) & ~(alignment - 1));
		ERS_ASSERTF((uintptr_t)next_aligned_position % alignment == 0, "%s", "StackAllocator::Allocate: Alignment failure.");

		size_t* size_pointer = reinterpret_cast<size_t*>(next_aligned_position) - 1;
		*size_pointer = (next_aligned_position - current_position) + size;
		u8** previous_pointer = reinterpret_cast<u8**>(size_pointer) - 1;
		*previous_pointer = previous;

		if (next_aligned_position + size <= buffer + capacity)
		{
			previous = next_aligned_position;
			bytes_occupied += (next_aligned_position - current_position) + size;
			return reinterpret_cast<void*>(next_aligned_position);
		}
		else
		{
			return nullptr;
		}
	}

	inline void* StackAllocator::Reallocate(void* p, size_t new_size, size_t alignment)
	{
		void* result = nullptr;
		if (p == nullptr)
		{
			result = Allocate(new_size, alignment);
		}
		else
		{
			u8* p_new = reinterpret_cast<u8*>(p);
			size_t* old_size_pointer = reinterpret_cast<size_t*>(p) - 1;
			if (previous != nullptr && p_new == previous)
			{			
				bytes_occupied += new_size - *old_size_pointer;
				*old_size_pointer = new_size;			
			}
			else
			{
				size_t old_size = *old_size_pointer;
				size_t min_size = (new_size < old_size) ? new_size : old_size;
				u8* p_new = reinterpret_cast<u8*>(Allocate(new_size, alignment));
				memcpy(p_new, p, min_size);
			}		
			result = reinterpret_cast<void*>(p_new);
		}		
		return result;
	}

	inline void StackAllocator::Deallocate(void* p) 
	{ 
		if (previous != nullptr && (u8*)p == previous)
		{
			bytes_occupied = previous - buffer;
			size_t* size_position = reinterpret_cast<size_t*>(p) - 1;
			u8** previous_position = reinterpret_cast<u8**>(size_position) - 1;
			previous = *previous_position;
		}
	}

	void StackAllocator::Clear()
	{
		previous = nullptr;
		bytes_occupied = 0;
	}

	void StackAllocator::Destroy()
	{
		if (buffer != nullptr)
		{
			free(buffer);
			buffer = nullptr;
			previous = nullptr;
			bytes_occupied = 0;
		}
	}

	StackAllocator::Marker StackAllocator::GetMarker()
	{
		return { bytes_occupied, previous };
	}

	void StackAllocator::SetMarker(StackAllocator::Marker marker)
	{
		bytes_occupied = marker.bytes_occupied;
		previous = marker.previous;
	}

	// ----------------------------
	// ::::::::::::ARENA:::::::::::
	// ----------------------------	

	Arena::Arena()
		: head(nullptr), tail(nullptr), region_count(0), minimum_region_capacity(ERS_MINIMUM_REGION_CAPACITY) { }

	Arena::Arena(size_t minimum_region_capacity_)
		: head(nullptr), tail(nullptr), region_count(0), minimum_region_capacity(minimum_region_capacity_) { }

	Arena::Arena(Arena&& m) noexcept
	{
		head = m.head;
		tail = m.tail;
		region_count = m.region_count;
		minimum_region_capacity = m.minimum_region_capacity;

		m.head = nullptr;
		m.tail = nullptr;
		m.region_count = 0;
		m.minimum_region_capacity = 0;
	}

	Arena& Arena::operator=(Arena&& m) noexcept
	{
		Region* temp_head = head;
		Region* temp_tail = tail;
		size_t temp_region_count = region_count;
		size_t temp_minimum_region_capacity = minimum_region_capacity;

		head = m.head;
		tail = m.tail;
		region_count = m.region_count;
		minimum_region_capacity = m.minimum_region_capacity;

		m.head = temp_head;
		m.tail = temp_tail;
		m.region_count = temp_region_count;
		m.minimum_region_capacity = temp_minimum_region_capacity;

		return *this;
	}

	Arena::~Arena()
	{
		Region* current_region = head;
		while (current_region != nullptr)
		{
			Region* next = current_region->next;
			current_region->m->~MonotonicAllocator();
			free(current_region->m);
			free(current_region);
			current_region = next;
		}
	}

	inline void* Arena::Allocate(size_t size, size_t alignment)
	{
		// Initialize.
		if (head == nullptr)
		{
			ERS_ASSERTF(tail == nullptr, "%s", "Arena::Allocate: If head is null, so must tail also be.");
			head = (Region*)malloc(sizeof(Region));
			ERS_ASSERTF(head != nullptr, "%s", "Arena::Allocate: Could not allocate memory.");
			head->m = (MonotonicAllocator*)malloc(sizeof(MonotonicAllocator));
			if (head->m == nullptr)
			{
				free(head);
				fprintf(stderr, "%s", "Arena::Allocate: Could not allocate more regions.");
				exit(EXIT_FAILURE);
			}
			new(head->m) MonotonicAllocator(size > minimum_region_capacity ? size : minimum_region_capacity);
			head->next = nullptr;
			tail = head;
			++region_count;
		}

		// Find region with enough memory.
		Region* current_region = head;
		while (current_region != nullptr && current_region->m->bytes_occupied + size > current_region->m->capacity)
			current_region = current_region->next;

		// If found, allocate.
		if (current_region != nullptr)
			return current_region->m->Allocate(size, alignment);

		// If not found, create new region.
		size_t necessary_capacity = minimum_region_capacity;
		while (size > necessary_capacity)
			necessary_capacity *= 2;
		current_region = (Region*)malloc(sizeof(Region));
		ERS_ASSERTF(current_region != nullptr, "%s", "Arena::Allocate: Could not allocate more regions.");
		current_region->m = (MonotonicAllocator*)malloc(sizeof(MonotonicAllocator));
		if (current_region->m == nullptr)
		{
			free(current_region);
			fprintf(stderr, "%s", "Arena::Allocate: Could not allocate more regions.");
			exit(EXIT_FAILURE);
		}
		new(current_region->m) MonotonicAllocator(necessary_capacity);
		current_region->next = nullptr;
		tail->next = current_region;
		tail = current_region;
		++region_count;
		return tail->m->Allocate(size, alignment);
	}

	inline void* Arena::Reallocate(void* p, size_t size, size_t alignment)
	{
		ERS_UNUSED(p);
		ERS_UNUSED(size);
		ERS_UNUSED(alignment);
		return nullptr;
	}

	inline void Arena::Deallocate(void* p) 
	{ 
		tail->m->Deallocate(p);
	}

	void Arena::Clear()
	{
		Region* current_region = head;
		while (current_region != nullptr)
		{
			current_region->m->Clear();
			current_region = current_region->next;
		}
	}
}



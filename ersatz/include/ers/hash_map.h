#ifndef HASH_MAP_H
#define HASH_MAP_H

#include "./ers/common.h"
#include "./ers/allocators.h"
#include "./ers/vector.h"

namespace ers
{
	// ----------------------------
	// :::::::::HASH MAP:::::::::::
	// ----------------------------

	// Open hash map using vectors for buckets,
	// storing key-value pairs in them.
	// TODO: Re-hashing after load factor surpasses ~50%.
	template <typename K, typename V>
	class HashMap;

	template <typename K, typename V>
	class HashMapIterator
	{
		friend class HashMap<K, V>;

	public:
		using bucket_item_t = Pair<K, V>;
		using bucket_t = Vector<bucket_item_t>;

	private:
		size_t m_currentPair;
		bucket_t** m_currentBucket;
		bucket_t** m_endBucket;

	public:
		HashMapIterator(size_t pair_, bucket_t** current_bucket_, bucket_t** end_bucket_)
			:
			m_currentPair(pair_),
			m_currentBucket(current_bucket_),
			m_endBucket(end_bucket_)
		{

		}

		bucket_item_t& operator*() { return (*m_currentBucket)->operator[](m_currentPair); }
		bucket_item_t* operator->() { return &((*m_currentBucket)->operator[](m_currentPair)); }

		HashMapIterator& operator++()
		{
			const size_t count = (*m_currentBucket)->GetSize();
			if (m_currentPair < count)
				++m_currentPair;

			if (m_currentPair >= count)
			{
				++m_currentBucket;
				while (m_currentBucket != m_endBucket && (*m_currentBucket)->GetSize() == 0)
					++m_currentBucket;
				m_currentPair = (m_currentBucket == m_endBucket) ? size_t(-1) : 0;
			}
			return *this;
		}

		HashMapIterator operator++(int)
		{
			HashMapIterator temp(*this);
			++(*this);
			return temp;
		}

		bool operator==(const HashMapIterator& it_rhs) const
		{
			return m_currentBucket == it_rhs.m_currentBucket
				&& m_endBucket == it_rhs.m_endBucket
				&& m_currentPair == it_rhs.m_currentPair;
		}

		bool operator!=(const HashMapIterator& it_rhs) const
		{
			return m_currentBucket != it_rhs.m_currentBucket
				|| m_endBucket != it_rhs.m_endBucket
				|| m_currentPair != it_rhs.m_currentPair;
		}
	};

	constexpr size_t ERS_MIN_BUCKET_COUNT = 7;
	constexpr size_t ERS_MIN_BUCKET_SIZE = 8;
	constexpr size_t ERS_MAX_BUCKET = 1023;
	template <typename K, typename V>
	class HashMap
	{
	public:
		using bucket_item_t = Pair<K, V>;
		using bucket_t = Vector<bucket_item_t>;
		using iterator_t = HashMapIterator<K, V>;

	private:
		Vector<bucket_t*> m_buckets;
		size_t m_size;
		size_t m_bucketCount;
		IAllocator* m_alloc;

	public:
		HashMap();
		HashMap(IAllocator* alloc_);
		HashMap(size_t bucket_count_, IAllocator* alloc_ = &default_alloc);
		~HashMap();

		HashMap(HashMap&& hm_rhs) noexcept;
		HashMap& operator =(HashMap&& hm_rhs) noexcept;

		HashMap(const HashMap& hm_rhs);
		HashMap& operator =(const HashMap& hm_rhs);

		bool Insert(const bucket_item_t& p);
		bool Insert(bucket_item_t&& p);

		void Insert(const bucket_item_t& p, iterator_t& hint);
		void Insert(bucket_item_t&& p, iterator_t& hint);

		ers::Pair<HashMap<K, V>::iterator_t, bool> Find(const K& key);

	private:
		bucket_item_t* findKeyValuePair(const K& key, bucket_t* bucket);

	public:
		template<typename U>
		ers::Pair<HashMap<K, V>::iterator_t, bool>
			FindAs(const U& key, size_t hash_code)
		{
			bucket_t** bucket = &m_buckets[hash_code % m_bucketCount];
			bucket_t** bucket_end = m_buckets.end();
			bucket_item_t* pair_found = findKeyValuePair(key, *bucket);

			bool found = (pair_found != (*bucket)->end());
			iterator_t result(found ? (pair_found - (*bucket)->begin()) : size_t(-1), bucket, bucket_end);

			return { result, found };
		}

		template<typename U>
		ers::Pair<HashMap<K, V>::iterator_t, bool>
			FindAs(const U& key, size_t hash_code, bool (*equal_if)(K, U))
		{
			bucket_t** bucket = m_buckets.begin() + hash_code % m_bucketCount;
			bucket_item_t* p = (*bucket)->begin();
			bucket_item_t* q = (*bucket)->end();
			while (p != q)
			{
				if (equal_if(p->first, key))
					break;
				++p;
			}

			iterator_t result((p != q) ? (p - (*bucket)->begin()) : size_t(-1), bucket, m_buckets.end());
			return { result, p != q };
		}

		// Semantics for [] different to unordered_map (I think). 
		// Here, sort of like in a vector. 
		V& operator [](const K& key);
		const V& operator [](const K& key) const;

		void Destroy();

		size_t GetSize() const;

		#if 0
		void PrintStatistics();
		#endif

		// For ranged-for loops and simple forward iteration.
		iterator_t begin();
		const iterator_t begin() const;
		iterator_t end();
		const iterator_t end() const;
	};

	/////////////////////////////////////////////////////////////////////////////////////////
	// Implementation.
	template <typename K, typename V>
	HashMap<K, V>::HashMap()
		:
		m_buckets(Vector<bucket_t*>(ERS_MIN_BUCKET_COUNT)),
		m_size(0),
		m_bucketCount(ERS_MIN_BUCKET_COUNT),
		m_alloc(&default_alloc)
	{
		for (size_t i = 0; i < m_bucketCount; ++i)
		{
			bucket_t* new_bucket = static_cast<bucket_t*>(m_alloc->Allocate(sizeof(bucket_t), alignof(bucket_t)));
			new(new_bucket) bucket_t(ERS_MIN_BUCKET_SIZE);
			m_buckets.PushBack(new_bucket);
		}
	}

	template <typename K, typename V>
	HashMap<K, V>::HashMap(IAllocator* alloc_)
		:
		m_buckets(Vector<bucket_t*>(ERS_MIN_BUCKET_COUNT, alloc_)),
		m_size(0),
		m_bucketCount(ERS_MIN_BUCKET_COUNT),
		m_alloc(alloc_)
	{
		for (size_t i = 0; i < m_bucketCount; ++i)
		{
			bucket_t* new_bucket = static_cast<bucket_t*>(m_alloc->Allocate(sizeof(bucket_t), alignof(bucket_t)));
			new(new_bucket) bucket_t(ERS_MIN_BUCKET_SIZE, alloc_);
			m_buckets.PushBack(new_bucket);
		}
	}

	template <typename K, typename V>
	HashMap<K, V>::HashMap(size_t bucket_count_, IAllocator* alloc_)
		:
		m_buckets(Vector<bucket_t*>(bucket_count_, alloc_)),
		m_size(0),
		m_bucketCount(bucket_count_),
		m_alloc(alloc_)
	{
		for (size_t i = 0; i < bucket_count_; ++i)
		{
			bucket_t* new_bucket = static_cast<bucket_t*>(m_alloc->Allocate(sizeof(bucket_t), alignof(bucket_t)));
			new(new_bucket) bucket_t(ERS_MIN_BUCKET_SIZE, alloc_);
			m_buckets.PushBack(new_bucket);
		}
	}

	template <typename K, typename V>
	HashMap<K, V>::~HashMap()
	{
		Destroy();
	}

	template <typename K, typename V>
	HashMap<K, V>::HashMap(HashMap&& hm_rhs) noexcept
		:
		m_buckets(std::move(hm_rhs.m_buckets)),
		m_size(hm_rhs.m_size),
		m_bucketCount(hm_rhs.m_bucketCount),
		m_alloc(hm_rhs.m_alloc)
	{
		hm_rhs.m_size = 0;
		hm_rhs.m_bucketCount = 0;
	}

	template <typename K, typename V>
	HashMap<K, V>& HashMap<K, V>::operator=(HashMap&& hm_rhs) noexcept
	{
		if (this != &hm_rhs)
		{
			size_t temp_size = m_size;
			IAllocator* temp_alloc = m_alloc;
			size_t temp_m_bucketCount = m_bucketCount;

			m_size = hm_rhs.m_size;
			m_alloc = hm_rhs.m_alloc;
			m_bucketCount = hm_rhs.m_bucketCount;

			hm_rhs.m_size = temp_size;
			hm_rhs.m_alloc = temp_alloc;
			hm_rhs.m_bucketCount = temp_m_bucketCount;

			m_buckets = std::move(hm_rhs.m_buckets);
		}
		return *this;
	}

	template <typename K, typename V>
	HashMap<K, V>::HashMap(const HashMap& hm_rhs)
		:
		m_buckets(Vector<bucket_t*>(hm_rhs.m_bucketCount, hm_rhs.m_alloc)),
		m_size(hm_rhs.m_size),
		m_bucketCount(hm_rhs.m_bucketCount),
		m_alloc(hm_rhs.m_alloc)
	{
		for (size_t i = 0; i < m_bucketCount; i++)
		{
			bucket_t* new_bucket = static_cast<bucket_t*>(
				m_alloc->Allocate(sizeof(bucket_t), 
					alignof(bucket_t))
			);
			new(new_bucket) bucket_t(*(hm_rhs.m_buckets[i]));
			m_buckets.PushBack(new_bucket);
		}
	}

	template <typename K, typename V>
	HashMap<K, V>& HashMap<K, V>::operator=(const HashMap& hm_rhs)
	{
		if (this != &hm_rhs)
		{
			Destroy();
			m_size = hm_rhs.m_size;
			m_alloc = hm_rhs.m_alloc;
			m_bucketCount = hm_rhs.m_bucketCount;

			m_buckets = Vector<bucket_t*>(hm_rhs.m_bucketCount, hm_rhs.m_alloc);
			for (size_t i = 0; i < m_bucketCount; i++)
			{
				bucket_t* new_bucket = static_cast<bucket_t*>(
					m_alloc->Allocate(sizeof(bucket_t), 
						alignof(bucket_t))
				);
				new(new_bucket) bucket_t(*(hm_rhs.m_buckets[i]));
				m_buckets.PushBack(new_bucket);
			}
		}
		return *this;
	}

	template <typename K, typename V>
	bool HashMap<K, V>::Insert(const bucket_item_t& p)
	{
		bucket_t* bucket = m_buckets[hash(p.first) % m_bucketCount];
		bucket_item_t* pair_found = findKeyValuePair(p.first, bucket);
		const bool result = (pair_found == bucket->end());
		if (result)
		{
			bucket->PushBack(bucket_item_t(p));
			++m_size;
		}
		return result;
	}

	template <typename K, typename V>
	bool HashMap<K, V>::Insert(bucket_item_t&& p)
	{
		bucket_t* bucket = m_buckets[hash(p.first) % m_bucketCount];
		bucket_item_t* pair_found = findKeyValuePair(p.first, bucket);
		const bool result = (pair_found == bucket->end());
		if (result)
		{
			bucket->PushBack(bucket_item_t(std::move(p)));
			++m_size;
		}
		return result;
	}

	template <typename K, typename V>
	void HashMap<K, V>::Insert(const bucket_item_t& p, iterator_t& hint)
	{
		bucket_t* bucket = *(hint.m_currentBucket);
		hint.m_currentPair = bucket->GetSize();
		bucket->PushBack(bucket_item_t(p));
		++m_size;
	}

	template <typename K, typename V>
	void HashMap<K, V>::Insert(bucket_item_t&& p, iterator_t& hint)
	{
		bucket_t* bucket = *(hint.m_currentBucket);
		hint.m_currentPair = bucket->GetSize();
		bucket->PushBack(bucket_item_t(std::move(p)));
		++m_size;
	}

	template <typename K, typename V>
	auto HashMap<K, V>::Find(const K& key) -> ers::Pair<HashMap<K, V>::iterator_t, bool>
	{
		bucket_t** bucket = &m_buckets[hash(key) % m_bucketCount];
		bucket_t** bucket_end = m_buckets.end();
		bucket_item_t* pair_found = findKeyValuePair(key, *bucket);

		bool found = (pair_found != (*bucket)->end());
		iterator_t result(found ? (pair_found - (*bucket)->begin()) : size_t(-1), bucket, bucket_end);

		return { result, found };
	}

	template <typename K, typename V>
	V& HashMap<K, V>::operator[](const K& key)
	{
		bucket_t* bucket = m_buckets[hash(key) % m_bucketCount];
		bucket_item_t* pair_found = findKeyValuePair(key, bucket);
		if (pair_found != bucket->end())
		{
			return pair_found->second;
		}
		else
		{
			const size_t bucket_size = bucket->GetSize();
			bucket->PushBack(bucket_item_t({ key, V() }));
			++m_size;
			return bucket->operator[](bucket_size).second;
		}
	}

	template <typename K, typename V>
	void HashMap<K, V>::Destroy()
	{
		for (size_t i = 0; i < m_bucketCount; ++i)
		{
			bucket_t* bucket_to_delete = m_buckets.PopBack();
			bucket_to_delete->~bucket_t();
			m_alloc->Deallocate(bucket_to_delete);
		}
		m_bucketCount = 0;
		m_size = 0;
	}

	template <typename K, typename V>
	size_t HashMap<K, V>::GetSize() const
	{
		return m_size;
	}

	template <typename K, typename V>
	auto HashMap<K, V>::begin() -> HashMap<K, V>::iterator_t
	{
		iterator_t result(0, m_buckets.begin(), m_buckets.end());
		if (m_buckets[0]->GetSize() == 0)
			++result;
		return result;
	}

	template <typename K, typename V>
	auto HashMap<K, V>::begin() const -> const HashMap<K, V>::iterator_t
	{
		iterator_t result(0, m_buckets.begin(), m_buckets.end());
		if (m_buckets[0]->GetSize() == 0)
			++result;
		return result;
	}

	template <typename K, typename V>
	auto HashMap<K, V>::end() -> HashMap<K, V>::iterator_t
	{
		bucket_t** finish = m_buckets.end();
		return iterator_t(size_t(-1), finish, finish);
	}

	template <typename K, typename V>
	auto HashMap<K, V>::end() const -> const HashMap<K, V>::iterator_t
	{
		bucket_t** finish = m_buckets.end();
		return iterator_t(size_t(-1), finish, finish);
	}

	template <typename K, typename V>
	auto HashMap<K, V>::findKeyValuePair(const K& key, bucket_t* bucket) -> HashMap<K, V>::bucket_item_t*
	{
		bucket_item_t* p = bucket->begin();
		bucket_item_t* q = bucket->end();
		while (p != q)
		{
			if (p->first == key)
				break;
			++p;
		}
		return p;
	}

	#if 0
	template <typename K, typename V>
	void HashMap<K, V>::PrintStatistics() //needs <algorithm> for the median bucket size
	{
		size_t empty = 0;
		size_t min = 0 - 1;
		size_t max = 0;
		size_t size2 = 0;
		size_t* sizes = new size_t[m_bucketCount];

		for (size_t i = 0; i < m_bucketCount; ++i)
		{
			auto current = m_buckets[i];
			auto count = current->GetSize();
			empty += (count == 0) ? 1 : 0;
			sizes[i] = count;
			if (max < count)
				max = count;
			if (min > count)
				min = count;
			size2 += count;
		}

		ERS_ASSERT(m_size == size2);
		printf("Size: %zu\n", size2);
		printf("Empty buckets: %f\n", (double)empty / (double)m_bucketCount);
		printf("Minimum size: %zu\n", min);
		printf("Maximum size: %zu\n", max);
		printf("Load factor: %f\n", (double)size2 / (double)m_bucketCount);

		for (size_t i = 0; i < 16; ++i)
		{
			size_t idx = i * 16;
			for (size_t j = 0; j < 16; ++j)
			{
				if (idx >= m_bucketCount)
					break;
				printf("%3zu ", sizes[i * 16 + j]);
				++idx;
			}
			printf("\n");
			if (idx >= m_bucketCount)
				break;
		}

		// std::sort(sizes, sizes + m_bucketCount);
		// size_t half = m_bucketCount / 2;
		// size_t median = (m_bucketCount % 2 == 0) ? (sizes[half] + sizes[half - 1]) / 2 : sizes[half];
		// printf("Median size: %llu\n", median);
	}
	#endif
}

#endif // HASH_MAP_H

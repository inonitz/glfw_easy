#include <benchmark/benchmark.h>  // for State, Benchmark, BENCHMARK, BENCHM...
#include <mm_malloc.h>            // for _mm_malloc, _mm_free
#include <stdio.h>                // for size_t, fprintf, stderr, printf
#include <algorithm>              // for fill
#include <array>                  // for array
#include <cstdint>                // for int32_t, uint32_t, int16_t, int64_t
#include <functional>             // for function
#include <random>                 // for uniform_int_distribution, random_de...
#include <stdexcept>              // for runtime_error
#include <type_traits>            // for conditional, is_integral
#include <vector>                 // for vector


// compile flags were -Os -std=c++17 -pthread
// Used Google Benchmark[Library] + Include-What-You-Use[tool]
// google benchmark guide: https://github.com/google/benchmark/blob/main/docs/user_guide.md#passing-arguments



#define ifcrashdbg(condition, ...) /* Using this as a common denominator across all ifcrash* macros. */ \
	if(!!(condition)) { \
		fprintf(stderr, "[IFCRASH_DEBUG] [FROM] %s [LINE] %u\n", __FILE__, __LINE__); \
		__VA_ARGS__; \
		throw std::runtime_error(""); \
	} \


#define likely(cond)   __builtin_expect(boolean(cond), 1)
#define unlikely(cond) __builtin_expect(boolean(cond), 0)




#define boolean(arg) !!(arg)
#define KB           	   (1024llu)
#define MB           	   (KB*KB)
#define GB           	   (MB*MB)
#define CACHE_LINE_BYTES   (64ul)
#define DEFAULT8           (0xAA)
#define DEFAULT16          (0xF00D)
#define DEFAULT32          (0xBABEBABE)
#define DEFAULT64          (0xFACADE00FACADE00)
#define DEFAULT128         (0xBEBC0FFEEAC1DBEB)


#define __hot           __attribute__((hot))
#define __cold          __attribute__((cold))
#ifndef __unused
#define __unused        __attribute__((unused)) /* more appropriate for functions		    */
#endif
#define notused         __attribute__((unused)) /* more appropriate for function parameters */
#define pack            __attribute__((packed))
#define alignpk(size)   __attribute__((packed, aligned(size)))
#define alignsz(size)   __attribute__((aligned(size)))

#ifndef __force_inline 
#define __force_inline inline __attribute__((always_inline))
#else
#define __force_inline __always_inline
#endif


#define isaligned(ptr, alignment) boolean( (  reinterpret_cast<size_t>(ptr) & (static_cast<size_t>(alignment) - 1llu)  ) == 0 )
#define __scast(type, val) static_cast<type>(val)
#define __rcast(type, val) reinterpret_cast<type>(val)


typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;
typedef int64_t  i64;
typedef int32_t  i32;
typedef int16_t  i16;
typedef int8_t   i8;
typedef float    f32;
typedef double   f64;


template<typename T> using ref 		 = typename std::conditional<sizeof(T) <= 8, T, T&		>::type;
template<typename T> using const_ref = typename std::conditional<sizeof(T) <= 8, T, T const&>::type;
template<typename T> using value_ptr = typename std::conditional<sizeof(T) <= 8, T, T*>::type;


template<typename T> constexpr T round2(T v) {
	static_assert(std::is_integral<T>::value, "Value must be an Integral Type! (Value v belongs to group N [0 -> +inf]. ");
	--v;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	++v;
	return v;
}




template<typename T, bool DontFreeCreationPointer = false> struct StaticPoolAllocator
{
private:
	using optionalOwnership = void;
	struct Node {
		i64   index;
		Node* next;
	};

	
	Node* freelist; /* Book-keeping -> Each node points to a T object in memory. */
	Node* available;
	T*    buffer;   /* The actual memory allocated. */
	u64   elemCount;
	u64   freeBlk;


	bool   occupied(size_t idx) 		      { return freelist[idx].index < 0; 	     }
	size_t index_from_pointer(T const* p) { return __scast(size_t, (buffer - p) ); }

	void common_init(size_t amountOfElements)
	{
		ifcrashdbg(amountOfElements == 0);
		elemCount = amountOfElements; 
		freeBlk   = amountOfElements;
		for(size_t i = 0; i < amountOfElements - 1; ++i)
		{
			freelist[i].index = i + 1;
			freelist[i].next = &freelist[i + 1];
		}
		freelist[elemCount - 1] = { __scast(i64, elemCount), nullptr }; /* last element shouldn't pointer anywhere */
		available = &freelist[0];
		return;
	}

public:
	void create(size_t amountOfElements)
	{
		buffer   = __scast(T*,    _mm_malloc(sizeof(T)    * amountOfElements, sizeof(T)    ));
		freelist = __scast(Node*, _mm_malloc(sizeof(Node) * amountOfElements, sizeof(Node) ));
		common_init(amountOfElements);
		return;
	}


	void create(optionalOwnership* __proper_memory, size_t amountOfElements)
	{
		buffer   = __scast(T*, __proper_memory);
		freelist = __scast(Node*,  _mm_malloc(sizeof(Node) * amountOfElements, sizeof(Node))  );
		common_init(amountOfElements);
		return;
	}


	void destroy() 
	{
		if constexpr(!DontFreeCreationPointer) {
			_mm_free(buffer);
		}
		
		_mm_free(freelist);
		available = nullptr;
		elemCount = 0;
		freeBlk   = 0;
		return;
	}


	T* allocate() 
	{
		ifcrashdbg(!freeBlk);
		T* v = &buffer[available->index - 1];
		available->index *= -1; /* now occupied */

		available = available->next;
		--freeBlk;
		return v;
	}


	size_t allocate_index()
	{
		ifcrashdbg(!freeBlk);
		size_t v = available->index - 1;
		available->index *= -1; /* now occupied */

		available = available->next;
		--freeBlk;
		return v;	
	}


	void free_index(size_t idx)
	{
		ifcrashdbg(!occupied(idx) || freeBlk == elemCount || idx >= elemCount);
		freelist[idx].index *= -1;
		freelist[idx].next = available;
		available = &freelist[idx];
		++freeBlk;

		memset(&buffer[idx], DEFAULT8, sizeof(T)); /* completely wipe the block of old data */
		return;
	}


	void free(T* ptr)
	{
		size_t idx = index_from_pointer(ptr);
		ifcrashdbg(!isaligned(ptr, sizeof(T)) || !occupied(idx) || (freeBlk == elemCount));
		freelist[idx].index *= -1;
		freelist[idx].next = available;
		available = &freelist[idx];
		++freeBlk;
		
		memset(ptr, DEFAULT8, sizeof(T)); /* completely wipe the block of old data */
		return;
	}


	size_t availableBlocks() const { return freeBlk;   }
	size_t size()    	     const { return elemCount; }

	
	void print()
	{
		static const char* strs[2] = { "Occupied", "Free    " };
		bool tmp = false;
		printf("Static Pool Allocator:\nObject Array[%llu]: %p\n    Free:     %u\n    Occupied: %u\n    ", buffer, elemCount, freeBlk, elemCount - freeBlk);
		for(size_t i = 0; i < elemCount; ++i)
		{
			tmp = boolean(freelist[i].index > 0);
			printf("    Object [i = %llu] [%s] => Object [%llu]\n", i, strs[tmp], __scast(u64, freelist[i].index));
		}
		return;
	}
};






template<typename Key, typename Value>
struct flat_hash
{
public:
    using HashFunction    = std::function<u64(Key)>;
    using CompareFunction = std::function<bool(Key, Key)>; /* MUST return 1 if keys match! */

#define INSERT_SUCCESS 0
#define INSERT_FAIL_TABLE_FULL 1
#define INSERT_FAIL_BUCKET_FULL 2

#define DELETE_SUCCESS 0
#define DELETE_FAIL_TABLE_EMPTY 1
#define DELETE_FAIL_BUCKET_EMPTY 2
#define DELETE_FAIL_KEY_NOT_FOUND 3

private:
	using ValueManager    = StaticPoolAllocator<Value, true>;
	static constexpr f32 cm_growth_factor = 1.5f;
    static constexpr f32 cm_load_factor   = 0.70f;


	size_t current_load() { return m_tableSize / m_buckets.size(); }


	constexpr size_t maxKeysPerBucket(size_t nodes, size_t buckets) {
		f32 out = __scast(f32, nodes) / buckets;
		return __scast(size_t, (out > 0.0f && out < 1.0f) + out); 
	}

	constexpr size_t minimumNodesFromBuckets(size_t buckets) {
		return __scast(size_t, __scast(f32, buckets) * (1 / cm_load_factor) );
	}




	struct alignpk(64) KeyGroup {
		struct pack {
			u8 bits; /* first 5 bits: occupation of each node. */
			u8 pad[3];
		} ctrl_t;
		struct {
			Key key;
			u32 index;
		} Node[5];


		KeyGroup() : ctrl_t{0b00000}, Node{ 
			{ 0, DEFAULT32 },
			{ 0, DEFAULT32 },
			{ 0, DEFAULT32 },
			{ 0, DEFAULT32 },
			{ 0, DEFAULT32 }
		} {}

		__force_inline u8 first_available() const { return __builtin_ffs(~ctrl_t.bits) - 1; } /* get index to first available block.     */
		__force_inline u8 first_occupied()  const { return __builtin_ffs(ctrl_t.bits) - 1;  } /* make sure NOT to call when bits == 0 :) */
		bool full()  const { return ctrl_t.bits == 0b11111; }
		bool empty() const { return ctrl_t.bits == 0b00000; }
		void push(const_ref<Key> key, u32 idx)
		{
			u8 ins = first_available();
			Node[ins]	 = { key, idx };
			ctrl_t.bits |= (1 << ins); /* set bit to 1, meaning occupied. */
			return;
		}
	};


	u8 bucket_lookup(u32 bidx, const_ref<Key> key) {
		u8 i = 5;
		
		i *= (m_buckets[bidx].ctrl_t.bits == 0);
		while(cmp(key, m_buckets[bidx].Node[i].key) != true && i < 5) ++i;
		return i;
	}


	u32 bucket_del(u32 bidx, const_ref<Key> key) { /* assume the KeyGroup will ALWAYS contain atleast a single element before deletion. */
		u32 i = 0, valueIdx = 0;
		while(cmp(key, m_buckets[bidx].Node[i].key) != true && i < 5) ++i;


		if(unlikely(i == 5)) /* i.e not found. */
			return DEFAULT32;
		else {
			m_buckets[bidx].ctrl_t.bits &= ~(1 << i); /* clear bit[i] to 0, meaning free. */
			valueIdx = m_buckets[bidx].Node[i].index;

			m_buckets[bidx].Node[i] = { 0, DEFAULT32 };
			return valueIdx;
		}
	}


	u32 bucket_del_nocleanup(u32 bidx, const_ref<Key> key)
	{
		u32 i = 0;
		while( cmp(key, m_buckets[bidx].Node[i].key) != true && i < 5) ++i;

		if(unlikely(i == 5)) return DEFAULT32;

		m_buckets[bidx].ctrl_t.bits &= ~(1 << i);
		m_buckets[bidx].Node[i].key = 0;
		return m_buckets[bidx].Node[i].index;
	}



	/* [NOTE]: 
		Theres always the problem of, will the re-hash actually satisfy the requirement. 
		In most cases - it will. In VERY UNLIKELY cases, it won't.
		Thats why I'm letting the user take care of this >:)
	*/
	bool grow_buckets_rehash(size_t newSize) {
		std::vector<KeyGroup> newTable(newSize);
		u64 bidx = 0;
		u8  ctrl = 0, node;
		bool bucketNotFull = true;


		m_tableSize = 0;
		auto& bucket = m_buckets[0];
		for(size_t i = 0; bucketNotFull && i < m_buckets.size(); ++i) 
		{
			bucket = m_buckets[i];
			ctrl   = bucket.ctrl_t.bits;
			while(ctrl && bucketNotFull) { /* find every occupied node and copy it to its new position */
				node = bucket.first_occupied();
				bidx = hash(bucket.Node[node].key) % newTable.size();


				/* push first occupied node in current bucket (b) to new position in new table (newTable) */
				newTable[bidx].push(
					bucket.Node[node].key, 
					bucket.Node[node].value
				);
				++m_tableSize;

				bucketNotFull = !newTable[bidx].full();
				ctrl &= ~(1 << node); /* 'delete' the node from the bitmap, so we can go to next node. */
			}
		}
		m_rehashed += bucketNotFull;
		
		
		return bucketNotFull;
	}




private:
	HashFunction    hash;
	CompareFunction cmp;


	/* Value allocator & Value buffer (we only Own the pointer, we do not Manage it) */
	ValueManager 		  m_vmng;
	Value*                m_values;
	
	std::vector<KeyGroup> m_buckets;
	u32 				  m_tableSize;
    u8                    m_rehashed;
	u8 					  pad[3];


public:
    void overrideFunctors(const_ref<HashFunction> hasher, const_ref<CompareFunction> comparer) {
        hash = hasher;
        cmp  = comparer;
        return;
    }


	void create(u32 buckets, u32 suggestedNodeAmount = 0)
	{
		m_buckets.resize(buckets);
		
		suggestedNodeAmount += (suggestedNodeAmount == 0) * minimumNodesFromBuckets(buckets);
		m_values = __scast(Value*,   _mm_malloc(sizeof(Value) * suggestedNodeAmount, round2(sizeof(Value)))  );
		m_vmng.create(m_values, suggestedNodeAmount);

		m_tableSize = 0;
		m_rehashed  = 0;

		static const KeyGroup tmp{};
		std::fill(m_buckets.begin(), m_buckets.end(), tmp);
		return;
	}


	void destroy()
	{
		m_tableSize = 0;
		m_rehashed  = 0;
		m_buckets.clear();
		m_vmng.destroy();
		_mm_free(m_values);
		return;
	}

	
	Value* lookup(const_ref<Key> k) {
		u32 bidx = hash(k) % m_buckets.size(), vidx = 0;
		u8  nidx = m_buckets[bidx].lookup(k);
		bool found = (nidx != 5);


		/* instead of the if statement, we'll multiply each index value by the condition, that way we can avoid it. */
		bidx *= found; /* found=false: will give bucket 0 */
		nidx *= found; /* found=false: will give node   0 */
		vidx = found * m_buckets[bidx].Node[nidx].index; /* found=false: will give m_values[0] */
		return found * &m_values[vidx];					 /* found=false: will return nullptr   */
	}


	u8 insert(const_ref<Key> k, const_ref<Value> v)
	{
		if(unlikely(m_vmng.size() == m_tableSize)) /* Hashtable needs resize */
			return INSERT_FAIL_TABLE_FULL;

		u32 idx = hash(k) % m_buckets.size();
		if(unlikely(m_buckets[idx].full())) 
			return INSERT_FAIL_BUCKET_FULL; /* Hashtable needs resize */

		
		u32 vidx = m_vmng.allocate_index(); /* get new value from alloc */
		m_buckets[idx].push(k, vidx); 	    /* push into available position in bucket */
		m_values[vidx] = v; 		  	    /* set value to what we were given. */
		++m_tableSize;						/* table increased in size.         */


		// if(current_load() > cm_load_factor) { /* hash table re-hashing will be the users' job */
		// 	grow_buckets_rehash(__scast(size_t, __scast(f32, m_tableSize) * cm_growth_factor ));
		// }
		return INSERT_SUCCESS;
	}


	u8 del(const_ref<Key> k)
	{
		if(unlikely(m_tableSize == 0)) return DELETE_FAIL_TABLE_EMPTY;
		
		u32 idx = hash(k) % m_buckets.size();
		if(unlikely(m_buckets[idx].empty())) 
			return DELETE_FAIL_BUCKET_EMPTY;
		
		u32 vidx = m_buckets[idx].del(k);
		if(unlikely(vidx == DEFAULT32))
			return DELETE_FAIL_KEY_NOT_FOUND;


		m_vmng.free_index(vidx);
		return DELETE_SUCCESS;
	}


	bool rehash(size_t suggestedNewSize = 0)
	{
		size_t tmp = __scast(size_t, __scast(f32, m_tableSize) * cm_growth_factor );
		suggestedNewSize += (suggestedNewSize == 0) * tmp;
		return grow_buckets_rehash(suggestedNewSize);
	}


	bool   need_rehash() const { return current_load() > cm_load_factor; }
	bool   full() const  { return m_tableSize == m_vmng.size(); }
	bool   empty() const { return m_tableSize == 0; }
	size_t size() const  { return m_tableSize;      }
	Value* data() const  { return m_values;    	    }


	#undef INSERT_FAIL_BUCKET_FULL
	#undef INSERT_FAIL_TABLE_FULL
	#undef INSERT_SUCCESS
	#undef DELETE_FAIL_KEY_NOT_FOUND
	#undef DELETE_FAIL_BUCKET_EMPTY
	#undef DELETE_FAIL_TABLE_EMPTY
	#undef DELETE_SUCCESS
};




/* Source: https://github.com/hhrhhr/MurmurHash-for-Lua/blob/master/MurmurHash64A.c */
__force_inline u64 MurmurHash64A(const void * key, i32 len, u64 seed)
{
	const u64 m = 0xc6a4a7935bd1e995LLU;
	const int r = 47;

	u64 h = seed ^ (len * m), k = 0;

	const u64 * data = (const u64 *)key;
	const u64 * end = (len >> 3) + data;

	while(data != end)
	{
		k = *data++;

		k *= m; 
		k ^= k >> r; 
		k *= m; 

		h ^= k;
		h *= m; 
	}

	const u8* data2 = (const u8*)data;
	switch(len & 7)
	{
	case 7: h ^= (u64)(data2[6]) << 48;
	case 6: h ^= (u64)(data2[5]) << 40;
	case 5: h ^= (u64)(data2[4]) << 32;
	case 4: h ^= (u64)(data2[3]) << 24;
	case 3: h ^= (u64)(data2[2]) << 16;
	case 2: h ^= (u64)(data2[1]) << 8;
	case 1: h ^= (u64)(data2[0]);
			h *= m;
	};

	h ^= h >> r;
	h *= m;
	h ^= h >> r;

	return h;
} 


__force_inline size_t readTimestampCounter() {
    u32 lo, hi;
    __asm__ __volatile__("rdtsc" : "=a" (lo), "=d" (hi));
    return ((size_t)hi << 32) | lo;
}




struct GridCell {
    f32 p, v;
    std::array<f32, 2> vx;
    std::array<f32, 2> vy;
};




template<class Generator = std::mt19937>
struct NumberGenerator
{
    using intDistResultType = std::uniform_int_distribution<int32_t>::result_type;
    using uintDistResultType = std::uniform_int_distribution<uint32_t>::result_type;
    using fDistResultType = std::uniform_real_distribution<f32>::result_type;
    size_t                                  seed;
    Generator                               generator;
    std::uniform_int_distribution<uint32_t> uintdist;
    std::uniform_int_distribution<int32_t>  intdist;
    std::uniform_real_distribution<f32>     floatdist;


    NumberGenerator(size_t initialSeed = 0)
    {
        initialSeed += (initialSeed == 0) * std::random_device()(); 
        seed = initialSeed;
        generator.seed(initialSeed);
        return;
    }


    intDistResultType  randu() { return uintdist(generator);  }
    uintDistResultType randi() { return intdist(generator);   }
    fDistResultType    randf() { return floatdist(generator); }

};




namespace Context {
	static NumberGenerator<> __rand{readTimestampCounter()};
	static flat_hash<u64, GridCell> table;
	static bool initOnce = true;
	
	/* For Insert */
	static size_t randCellCount = 256;
    static std::vector<GridCell> randCells(randCellCount);


	void Setup()
	{
		if(!initOnce) return;


		table.overrideFunctors(
			[](size_t key) -> u64 { static const u64 seed = readTimestampCounter(); return MurmurHash64A(&key, 8, Context::__rand.seed); },
			[](size_t k0, size_t k1) -> bool { return k0 == k1; }
		);
		
		
		for(auto& e : randCells) 
		{
			static std::array<f32, 4> tmp = {
				__rand.randf(),
				__rand.randf(),
				__rand.randf(),
				__rand.randf()
			};
			e = {  tmp[0], tmp[1], { tmp[2] }, { tmp[3] }  };
		}


		initOnce = false;
	}


};


static void HashInsert(benchmark::State& state)
{
    Context::table.create(state.range(0));

    u64	     idx = 0;
    GridCell toCache;
    for (auto _ : state) {
        state.PauseTiming();
        toCache = Context::randCells[idx % Context::randCellCount];
        ++idx;
        state.ResumeTiming();

        Context::table.insert(idx, toCache);
    }
}
// BENCHMARK(HashInsert)->Arg(32768)->Arg(65536)->Arg(131072)->Arg(262144)->Arg(524288)->Arg(1048576)->Arg(2097152);
BENCHMARK(HashInsert)->Arg(32768)->Arg(65536)->Arg(131072);


// static void HashLookup(benchmark::State& state)
// {
//     for (auto _ : state) {

//     }
// }
// BENCHMARK(HashLookup);


// static void HashDelete(benchmark::State& state)
// {
//     for (auto _ : state) {

//     }  
// }
// BENCHMARK(HashDelete);


static void HashCreate(benchmark::State& state)
{
	Context::Setup();
    for (auto _ : state) {
        Context::table.create(state.range(0));

        state.PauseTiming();
        Context::table.destroy();
        state.ResumeTiming();
    }
}
// BENCHMARK(HashCreate)->Arg(32768)->Arg(65536)->Arg(131072)->Arg(262144)->Arg(524288)->Arg(1048576)->Arg(2097152);
// BENCHMARK(HashCreate)->Arg(32768)->Arg(65536);


static void HashDestroy(benchmark::State& state)
{
	Context::Setup();
    for (auto _ : state) {
        state.PauseTiming();
        Context::table.create(state.range(0));
        state.ResumeTiming();

        Context::table.destroy();
    }
}
// BENCHMARK(HashDestroy)->Arg(32768)->Arg(65536)->Arg(131072)->Arg(262144)->Arg(524288)->Arg(1048576)->Arg(2097152);
// BENCHMARK(HashDestroy)->Arg(32768)->Arg(65536);




BENCHMARK_MAIN();
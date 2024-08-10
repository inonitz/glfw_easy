#ifndef __UTIL_POOL_ALLOCATOR__
#define __UTIL_POOL_ALLOCATOR__
#include "base.hpp"


namespace detail {


template<u32 objectSizeInBytes> struct CommonPoolDef
{
public:
	void* allocate();
	void free(void* ptr);

	u64 allocate_index();
	void free_index(u64 idx);

	u64  availableBlocks() const { return m_freeBlk;         }
	u64  size()    	       const { return m_elemCount;       }
	u64  objectSize() 	   const { return objectSizeInBytes; }
	u64  bytes() 		   const { return size() * objectSize(); }	
	void print() 		   const;

protected:
	
	struct Node {
		i64   index;
		Node* next;
	};
	
	Node* m_freelist; /* Book-keeping -> Each node points to a T object in memory. */
	Node* m_available;
	byte* m_buffer;   /* The actual memory allocated. */
	u64   m_elemCount;
	u64   m_freeBlk;


	bool occupied(u64 idx) const { 
		return m_freelist[idx].index < 0;
	}
	u64 index_from_pointer(void const* p) const { 
		return __scast(u64, (__rcast(byte const*, p) - m_buffer) ); 
	}
	void common_init(u64 amountOfElements);
};


} // namespace detail


template<u32 objectSizeInBytes, bool userManagedMemoryPointer = false> class Pool {};


template<u32 objectSizeInBytes> class Pool<objectSizeInBytes, false> : public detail::CommonPoolDef<objectSizeInBytes>
{
public:
	void create(u64 amountOfElements);
	void destroy();

private:
	using NodeType = typename detail::CommonPoolDef<objectSizeInBytes>::Node;
};


template<u32 objectSizeInBytes> class Pool<objectSizeInBytes, true> : public detail::CommonPoolDef<objectSizeInBytes>
{
public:
	void create(
		void*  __aligned_allocated_memory,
		u64 amountOfElements
	);
	void destroy();

private:
	using NodeType = typename detail::CommonPoolDef<objectSizeInBytes>::Node;
};


#endif
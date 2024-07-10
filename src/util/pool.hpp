#ifndef __UTIL_POOL_ALLOCATOR__
#define __UTIL_POOL_ALLOCATOR__
#include "base.hpp"


namespace detail {


template<typename T> struct CommonPoolDef
{
public:
	T* allocate();
	void free(T* ptr);

	size_t allocate_index();
	void free_index(size_t idx);


	size_t availableBlocks() const { return m_freeBlk;   		}
	size_t size()    	     const { return m_elemCount; 		}
	size_t bytes() 			 const { return size() * sizeof(T); }	
	void   print() 			 const;

protected:
	
	struct Node {
		i64   index;
		Node* next;
	};
	
	Node* m_freelist; /* Book-keeping -> Each node points to a T object in memory. */
	Node* m_available;
	T*    m_buffer;   /* The actual memory allocated. */
	u64   m_elemCount;
	u64   m_freeBlk;


	bool occupied(size_t idx) { 
		return m_freelist[idx].index < 0;
	}
	size_t index_from_pointer(T const* p) { 
		return __scast(size_t, (p - m_buffer) ); 
	}
	void common_init(size_t amountOfElements);
};


} // namespace detail


template<typename T, bool userManagedMemoryPointer> class Pool {};


template<typename T> class Pool<T, false> : public detail::CommonPoolDef<T>
{
public:
	void create(size_t amountOfElements);
	void destroy();

private:
	using NodeType = typename detail::CommonPoolDef<T>::Node;
};


template<typename T> class Pool<T, true> : public detail::CommonPoolDef<T>
{
public:
	void create(
		void*  __aligned_allocated_memory,
		size_t amountOfElements
	);
	void destroy();

private:
	using NodeType = typename detail::CommonPoolDef<T>::Node;
};




#endif
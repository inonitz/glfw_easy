#pragma once
#include "allocator.hpp"
#include <functional>
#include <vector>




/* 
	Hash table with Separate Chaining, 
	will try to use as many buckets as possible 

    Note: When objects are small, they should be coupled with their nodes into 1 
    structure, meaning Node {
        Key,
        Value
    }[]

    instead of Node {
        Key,
        Value*
    }[]
    
    The common factor in this is that nodes are allocated in tightly-packed arrays for each bucket.
    The difference in this case would be that values are allocated in a different array, unrelated to the 
    Node Array.
    

*/
template<typename Key, typename Value, bool reHashForMaxNodes = false> struct Hashtable 
{
public:
    using HashFunction    = std::function<u64(Key)>;
    using CompareFunction = std::function<bool(Key, Key)>; /* MUST return 1 if keys match! */

private:
    static constexpr f32 cm_growth_factor = 1.5f;
    static constexpr f32 cm_load_factor   = 0.70f;
    /* 
        Nodes allocated are close in memory, 
        so this shouldn't be such a big problem. 
        still, we want to minimize the length of each linked list. 
        Need more time to think about this parameter and its usefulness.
    */
    static constexpr u32 cm_max_nodes_per_bucket = 4;


    struct alignsz(32) Node {
        Key    k;
        Value* v;
        Node*  next;
    };


    struct alignsz(32) Bucket {
        Node* list;
        Node* last;
        size_t length;


        Bucket() : list{nullptr}, last{nullptr}, length{0} {}

        Node* begin() const { return list; }
        Node* end()   const { return last; }
        void append(Node* already_allocated_and_init)
        {
            /* could probably write this without the if statement, check on this code again later. */
            if(likely(length == 0)) /* Edge-case: Node is first in the list => begin = last. */
            {
                list = last = already_allocated_and_init;
                last->next = nullptr;
            } else {
                /* Im calling list->next when the bucket hasn't initialized it. this is nullptr deref, therefore segfault*/
                last->next = already_allocated_and_init;
                last       = already_allocated_and_init;
                already_allocated_and_init->next = nullptr;
            }
            ++length;
            return;
        }
    };


    using NodeManager  = StaticPoolAllocator<Node>;
    using ValueManager = StaticPoolAllocator<Value>;

    NodeManager  nodes; 
    ValueManager values;
    std::vector<Bucket> buckets; /* Hashtable Capacity = maxAmountOfNodes; Hashtable Size = sum(buckets[i].size) */
	f32                 m_tableSize;
    std::pair<u32, u32> m_maxBucketLength;
    size_t              m_rehashed;

    HashFunction    hash_function;
    CompareFunction compare_function;



    __force_inline f32 loadFactor() const { return m_tableSize / __scast(f32, buckets.size()); }
    __force_inline bool tableShouldGrow() const {
        return (
            loadFactor() > cm_load_factor 
            ||
            ( reHashForMaxNodes && (m_maxBucketLength.first > cm_max_nodes_per_bucket) )
        );
    }

    /* Iterates Linearly through the bucket until the correct key is found. */
    Node* findInBucket(size_t idx, const_ref<Key> k) {
        Node* begin = nullptr;
        bool  found = false;

        /* last->next is nullptr anyways, we don't need to use end(). */
        for(begin = buckets[idx].begin(); begin != nullptr && !found; begin = begin->next) {
            found = compare_function(begin->k, k);
        }
        return begin; /* if !found, begin will reach nullptr. if we found, begin will be the correct ptr. */
    }


    bool deleteFromBucket(size_t index, const_ref<Key> key)
    {
        bool  found = false;
        Node* begin = buckets[index].begin();
        Node* tmp   = begin;

        if(begin == nullptr)
            return false;


        found = compare_function(begin->k, key);
        for(; begin->next != nullptr && !found; begin = begin->next) { /* Search for node using its previous [while node->next.k != key]*/
            found = compare_function(begin->next.k, key);
        }
        if(!found) 
            return false;
        

        if(begin == buckets[index].list) { /* we found it on the first compare. */
            buckets[index].list = tmp->next;
        }
        else {
            tmp = begin->next;
            begin->next = tmp->next;
            tmp->next = nullptr;
        }
        values.free(tmp->v);
        nodes.free(tmp);

        return true;
    }


    std::pair<u8, Value*> internal_insert(const_ref<Key> key, const_ref<Value> value);


    void rehash(size_t newAmountOfBuckets);

public:
    void overrideFunctors(const_ref<HashFunction> hasher, const_ref<CompareFunction> comparer) {
        hash_function    = hasher;
        compare_function = comparer;
        return;
    }


	void create(u32 maxAmountOfNodes, u32 amountOfBuckets);
	void destroy();

	Value* lookup(const_ref<Key> k);
	u8     insert(const_ref<Key> k, const_ref<Value> v); /* 2 = inserted new node. 1 = wrote to existing node. 0 = couldn't insert node. */
	bool   del(const_ref<Key> k);

    Value& operator[](const_ref<Key> key);



    size_t size()     const { return m_tableSize;  }
    size_t max_size() const { return nodes.size(); }
    size_t bucketCount() const { return buckets.size(); }
    f32  load_factor()     const { return loadFactor();   }
    f32  max_load_factor() const { return cm_load_factor; }
    void to_file(FILE* file=stdout);
};



template<typename Key, typename Value, bool rehashOnCondition> void Hashtable<Key, Value, rehashOnCondition>::rehash(size_t newSize)
{
    std::vector<Bucket> newBuckets; newBuckets.resize(newSize);
    Node* begin = nullptr;
    u64   index = 0;

    m_maxBucketLength = { 0, 0 };
    for(auto& bucket : buckets)
    {
        for(begin = bucket.begin(); begin != nullptr; begin = begin->next) {
            index = hash_function(begin->k) % newBuckets.size();
            newBuckets[index].append(begin);
        }
    }
    buckets.swap(newBuckets); /* newBuckets is the actual buckets we need. buckets (old) will be freed once out of scope. */
    ++m_rehashed;
    return;
}




template<typename Key, typename Value, bool rehashOnCondition> void Hashtable<Key, Value, rehashOnCondition>::create(
    u32 maxNodes,
    u32 bucketCount
) {
    nodes.create(maxNodes);
    values.create(maxNodes);
    buckets.resize(bucketCount);
    m_tableSize = 0.0f;
    m_maxBucketLength = { 0, 0 };
    m_rehashed = 0;
    return;
}


template<typename Key, typename Value, bool rehashOnCondition> void Hashtable<Key, Value, rehashOnCondition>::destroy()
{
    nodes.destroy();
    values.destroy();
    buckets.clear();
    return;
}


template<typename Key, typename Value, bool rehashOnCondition> Value* Hashtable<Key, Value, rehashOnCondition>::lookup(const_ref<Key> key)
{
    u64   index = hash_function(key) % buckets.size();
    Node* node  = findInBucket(index, key);
    debug_messagefmt("Looking For Key %7llu ... [b=%7llu] | %u\n", key, index, node != nullptr);
    return node == nullptr ? nullptr : node->v;
}


template<typename Key, typename Value, bool rehashOnCondition> std::pair<u8, Value*> Hashtable<Key, Value, rehashOnCondition>::internal_insert(
    const_ref<Key>   key,
    const_ref<Value> value
) {
    if(unlikely(m_tableSize == nodes.size())) {
        return { false, nullptr };
    }

    u64   index = hash_function(key) % buckets.size();
    Node* node  = findInBucket(index, key);


    /* key already exists, we'll update the value. */
    if(node != nullptr) {
        *node->v = value;
        return { 2u, node->v };
    }


    /* we didn't find the node in the appropriate bucket. we need to insert it. */
    node = nodes.allocate();
    node->k = key;
    node->v = values.allocate();
    buckets[index].append(node);
    ++m_tableSize;

    /* 
        In the case where we exceed max_nodes_per_bucket (ex. 4) OR the load factor is too high, 
        we'll increase the amount of buckets and rehash the Whole Hashtable. 
    */
    m_maxBucketLength.first  = (buckets[index].length > m_maxBucketLength.first ) ? buckets[index].length : m_maxBucketLength.first;
    m_maxBucketLength.second = (buckets[index].length > m_maxBucketLength.second) ? index                 : m_maxBucketLength.second;
    if(tableShouldGrow()) {
        f32 newSize = __scast(f32, buckets.size()) * cm_growth_factor;
        rehash(__scast(size_t, newSize));
    }
    return { true, node->v }; 
}




template<typename Key, typename Value, bool rehashOnCondition> u8 Hashtable<Key, Value, rehashOnCondition>::insert(
    const_ref<Key>   key,
    const_ref<Value> value
) {
    return internal_insert(key, value).first;
}


template<typename Key, typename Value, bool rehashOnCondition> bool Hashtable<Key, Value, rehashOnCondition>::del(const_ref<Key> key)
{
    bool t = deleteFromBucket(hash_function(key) % buckets.size(), key);
    m_tableSize -= t; /* if deletion occured, we'll subtract from tableSize. */
    return t;
}


template<typename Key, typename Value, bool rehashOnCondition> Value& Hashtable<Key, Value, rehashOnCondition>::operator[](const_ref<Key> key)
{
    auto pair = internal_insert(key, Value());
    ifcrashdbg(unlikely(pair.second == nullptr));
    return *pair.second;
}


template<typename Key, typename Value, bool rehashOnCondition> void Hashtable<Key, Value, rehashOnCondition>::to_file(FILE* file)
{
    size_t totalCollisons = 0, tableSize = __scast(size_t, m_tableSize);
    for(auto& bucket : buckets) {
        totalCollisons += bucket.length - (bucket.length > 1);
    }
    fprintf(file, "Hashtable:\n    %llu Buckets\n    %llu/%llu Nodes [%llu Allocated (%llu Bytes)]\n    %llu Collisions\n        %llu Re-hashes\n    Bucket %u Largest [%u Nodes]\n    Contents:\n",
        buckets.size(),
        tableSize,
        nodes.size(),
        tableSize,
        tableSize * sizeof(Node),
        totalCollisons,
        m_rehashed,
        m_maxBucketLength.second,
        m_maxBucketLength.first
    );
    Node* begin = nullptr;
    fprintf(file, "    [Bucket ID] [Nodes (Begin, End)]: [key, value] => [... , ...]\n");
    for(size_t id = 0; id < buckets.size(); ++id)
    {
        if(buckets[id].length == 0)
            // fprintf(file, "    [%llu] [Empty]\n", id);
            continue;
        else {
            fprintf(file, "    [%9llu] [%llu (%p, %p)]: ", 
                id, 
                buckets[id].length, 
                __scast(void*, buckets[id].list), 
                __scast(void*, buckets[id].last)
            );
            for(begin = buckets[id].begin(); begin != nullptr; begin = begin->next) {
                fprintf(file, "[%llu, %p] => ", begin->k, __scast(void*, begin->v));
            }
            fprintf(file, "||\n");
        }
    }
    return;
}
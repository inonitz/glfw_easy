// #include "texwork.hpp"
// #include "imgui_demo.hpp"
// #include "box2d_minimal.hpp"




// int main() {
// 	// return make_texture_resize_work();
// 	// return imgui_demo_main();
// 	// return box2d_minimal();
// }


#include <stdint.h>  // for uint16_t
#include <cstring>   // for size_t, memset
#include <memory>    // for allocator_traits<>::value_type
#include <string>    // for string, allocator
#include <vector>    // for vector, _Bit_reference, vector<>::reference




typedef uint16_t u16;
typedef uint8_t u8;


struct Reccurence
{
    std::vector<u16> indices;
    u16 count;
};


struct RepetitionRange
{
    u16 be[2];
};




class Solution {
public:
    static int lengthOfLongestSubstring(std::string s) {
        std::vector<Reccurence> buckets{94};
        std::vector<Reccurence> repBuckets;
        std::vector<RepetitionRange> splice;


        /* 
            Sort All Recurrences into a 'hash table' of sorts
        */
        for(size_t i = 0; i < s.size(); ++i) 
        {
            ++buckets[ s[i] - 32 ].count; 
              buckets[ s[i] - 32 ].indices.push_back(i);
        }

        /* Keep All buckets with size > 1 */
        for(size_t i = 0; i < buckets.size(); ++i) {
            if(buckets[i].count > 1) 
                repBuckets.push_back(buckets[i]);
        }


        /* 
            Find all Repeating Blocks of Characters.
        */
        u16 prevIndex, occurCount, c;
        bool inRange;
        for(size_t i = 0; i < repBuckets.size(); ++i) {
            auto& cBucket = repBuckets[i].indices;


            prevIndex  = cBucket[0];
            occurCount = 0;
            c = 0;
            inRange = c < repBuckets[i].count;
            for(; inRange; ) {
                /* 
                    Find sequences of adjacent-repeating characters, and 'compress' them into a range.
                */
                while(inRange && prevIndex + 1 == cBucket[c]) {
                    ++c;
                    ++prevIndex;
                    ++occurCount;
                    inRange = c < repBuckets[i].count;
                }

                /* if Actually a sequence */
                if(occurCount > 0) {
                    splice.push_back(RepetitionRange{  { static_cast<u16>(prevIndex - occurCount), prevIndex }  });
                }

                /* goto next character to find new sequence of repeatitive chars. */
                occurCount = 0;
                prevIndex = inRange ? cBucket[c] : 0;
                ++c;
                inRange = c < repBuckets[i].count;
            }
        }


        std::vector<u8> activeChars(s.size(), true);
        for(size_t i = 0; i < splice.size(); ++i) {
            memset(&activeChars[ splice[i].be[0] ], false, splice[i].be[1] - splice[i].be[0] + 1);
        }

        u16 maxLen = 0;
        c = 0;
        occurCount = 0;
        inRange = c < s.size();
        for(; inRange; ) {
            if(activeChars[c]) {
                ++c;
                ++occurCount;
                inRange = c < s.size();
            } else {
                maxLen = (occurCount > maxLen) ? occurCount : maxLen;
            }
        }


        return maxLen;
    }
};



int main()
{
    return Solution::lengthOfLongestSubstring("pwwwkew");
}
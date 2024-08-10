#include "util/hash.hpp"
#include "util/random.hpp"
#include "util/measure.hpp"
#include "util/flat_hash.hpp"
#include "util/vec2.hpp"




#define __MEASURE(timer_structure, func) \
	timer_structure.tick(); \
	func; \
	timer_structure.tock(); \
\


using namespace util;



template<u8 type> struct GridCell {
	f32 p;
	f32 v;
	math::vec2f ux;
	math::vec2f uy;
	math::vec2f uz;
};




static Timer<> 		  	 		   	     timer{};
static flat_hash<u64, GridCell<0>, Hash> table;

/* For Insert */
static size_t randCellCount = 256;
static std::vector<GridCell<0>> randCells(randCellCount);

/* stdout not big enough. */
static FILE* bench_data = fopen("benchmarker.txt", "w+");

/* lambda function for printing */
static auto printTableElement = [](size_t key, GridCell<0>) -> std::string {
	std::string out{22, ' '};
	std::snprintf(out.data(), out.size(), "{ k=%3llu }", key);
	return out;
};




int test_hashtables()
{
	table.create(192, 0);


	static std::array<f32, 5> tmp;
	for(auto& e : randCells) 
	{
		tmp = {
			random32f(),
			random32f(),
			random32f(),
			random32f(),
			random32f(),
		};
		e = {  tmp[0], tmp[1], 
			math::vec2f(tmp[2]),
			math::vec2f(tmp[3]),
			math::vec2f(tmp[4])
		};
	}


	/* Test Insertion. */
	u8 failed   = 0;
	u32 newSize = table.buckets();
	for(size_t i = 0; i < randCellCount; ++i)
	{
		__MEASURE(timer, failed = table.insert(i, randCells[i]));
		fprintf(bench_data, "Insert [%3llu=%s] [key=%3llu, value_ptr=%p] Took %lluns\n", i, table.statusToString(0, failed), i, (void*)&randCells[i], timer.duration().count());

		i -= failed > 0; /* we repeat the test at i if it didn't succeed. */
		while(failed > 0) { /* meaning NOT successful. */ 
			newSize = (newSize * 3) / 2;
			markfmt("newSize computed=%u\n", newSize);

			__MEASURE(timer, { failed = table.rehash(newSize); }); 
			fprintf(bench_data, "Insert [%3llu=%s re-hash Took %lluns]\n", i, table.statusToString(3, failed), timer.duration().count());

			failed = (failed == 0);
		}
	}
	table.to_file(printTableElement, bench_data);


	/* Test Lookups. */
	GridCell<0>* val;
	size_t k = 0;
	for(size_t i = 0; i < randCellCount; ++i)
	{
		k = random32u() % randCellCount + (random32u() & 0xFF);
		__MEASURE(timer, val = table.lookup(k));
		fprintf(bench_data, "Lookup   [%3llu=%s] [key=%3llu, value_ptr=%p] Took %lluns\n", i, table.statusToString(1, val == nullptr), k, (void*)&randCells[k], timer.duration().count());
	}


	/* Make sure inserts were successful. */
	k = 0;
	for(size_t i = 0; i < randCellCount; ++i)
	{
		k += table.lookup(i) != nullptr;
	}
	markfmt("found %llu/%llu inserted elements.\n", k, randCellCount);


	/* Test Deletion. */
	for(size_t i = 0; i < randCellCount; ++i)
	{
		k = random32u() % randCellCount + (random32u() & 0xFF);
		__MEASURE(timer, failed = table.del(k));
		// table.printIterator();
		fprintf(bench_data, "Deletion [%3llu=%s] [key=%3llu, value_ptr=%p] Took %lluns\n", i, table.statusToString(2, failed), k, (void*)&randCells[k], timer.duration().count());
	}


	/* Test Iterator */
	for(auto& elem : table) {
		fprintf(bench_data, "Iterator At [val=%3f, value_ptr=%p, idx=%llu]\n", elem.p, (void*)&elem, &elem - table.data());
	}
	__MEASURE(timer, { /* Measure busy loop */
		for(auto& elem : table) {
			(void(elem));
			__asm__ volatile("");
		}
	});
	fprintf(bench_data, "Iterator [length=%llu] Took %lluns\n", table.size(), timer.duration().count());


	/* Test [index, value] Iterator */
	for(auto& [value, index]: table.asMapIterator()) {
		fprintf(bench_data, "Iterator At [value_ptr=%p, idx=%llu]\n", (void*)value, index);
	}
	__MEASURE(timer, { /* Measure busy loop */
		for(auto& [value, index]: table.asMapIterator()) {
			(void(value));
			(void(index));
			__asm__ volatile("");
		}
	});
	fprintf(bench_data, "Iterator [length=%llu] Took %lluns\n", table.size(), timer.duration().count());



	table.to_file(printTableElement, bench_data);
	fclose(bench_data);
	return 0;
}

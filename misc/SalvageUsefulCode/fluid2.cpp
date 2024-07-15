#include "fluid2.hpp"
#include "flat_hash.hpp"
#include <algorithm>
#include <numeric>




#define CELL_TYPE_SHIFT   ((u8)6)
#define CELL_LAYER_MASK   ((u8)0b00111111)
#define CELL_TYPE_MASK    ((u8)0b11000000)
#define CELL_TYPE_INVALID ((u8)0b00 << CELL_TYPE_SHIFT) 
#define CELL_TYPE_AIR     ((u8)0b01 << CELL_TYPE_SHIFT) 
#define CELL_TYPE_FLUID   ((u8)0b10 << CELL_TYPE_SHIFT) 
#define CELL_TYPE_SOLID   ((u8)0b11 << CELL_TYPE_SHIFT)


struct Cell {
	Vec4 	     u_p; /* ux uy uz p */
	GridPosition x;  /* Calculate key from position P in grid. */
	/* 
		layer: 0->5 (6 bits) [0b00 = Invalid, 0b01 = subStep0 ]
		type : 6->7 (2 bits) [0b00 = Invalid, 0b01 = Air, 0b10 = Fluid, 0b11 = Solid]
	*/
	u8 			 bitFields;


	u8 layer() const { return bitFields & 0b00111111; }
	u8 type()  const { return bitFields & 0b11000000; }
};


struct Marker {
	GridPosition pos;
};




struct Simulation2D
{
	const math::vec3u bounds{500u, 500u, 500u};
	f32 dx;
	f32 k_cfl;
	f32 uMax;
	f32 dt;

	flat_hash<u32, Cell> cells;
	std::vector<Marker>  markers;


	bool inBounds(GridPosition& pos) {
		return (pos.x < bounds.x) && (pos.y < bounds.y) && (pos.z < bounds.z);
	}


	bool inBounds(math::vec3i& pos) {
		return (__scast(u32, pos.x) < bounds.x) && (__scast(u32, pos.y) < bounds.y) && (__scast(u32, pos.z) < bounds.z);
	}


	u32 toIndexKey(GridPosition const& pos) {
		/* Transform The 3d Coordinate of a point to a Key */
		/*
			I(x, y, z, B) = z + B_z * (y + B_y * x),
			where 
				[x, y, z] -> pos,
				B         -> bounds
			
			ΔI_z = Δz
			ΔI_y = B_z * Δy
			ΔI_x = B_z * B_y * Δx
		*/
		return pos.z + bounds.z * (pos.y + bounds.y * pos.x);
	}

	
	/* Source: https://math.stackexchange.com/questions/19765/calculating-coordinates-from-a-flattened-3d-array-when-you-know-the-size-index */
	void toGridPosition(u32 indexKey, GridPosition& set) {
		/* 
			Z index increases fastest, (Z is Width,  or Row)
			Y index after that, 	   (Y is Height, or Column)
			X index last 			   (X is Depth)
		*/
		u32 kRC, ColumnRowBounds = bounds.y * bounds.z;
		
		set.x = indexKey / ColumnRowBounds;
		kRC = set.x * ColumnRowBounds;

		set.y = (indexKey - kRC) / bounds.z;
		set.z = indexKey - kRC - (set.y * bounds.z);
		return;
	}


	void interpolate_rk2()
	{

	}


	void initialize(u32 boundX, u32 boundY, u32 boundZ, f32 h = 1e-3, f32 subSteps = 8);



private: /* Update Grid Begin */

	void resetCells()
	{
		for(auto& val : cells) {
			val.bitFields &= CELL_TYPE_MASK; /* Reset layer to =0, meaning -1, meaning invalid. */
		}
		return;
	}


	void updateFluidCells()
	{
		bool shouldInsert = true;
		u8   status = 0;
		u64  k = 0;
		Cell toInsert, c = {
			{ 0.0f, 0.0f, 0.0f, 0.0f },
			{ 0, 0, 0 },
			0b000000 | CELL_TYPE_FLUID
		};
		Cell* found = nullptr, *tmp = nullptr;

		
		
		/* [NOTE]:
			Instead of using ternary (or IF statement) make sure the 
			hash_table has a key for which a default value can be acquired from.
			That way we can just multiply pointers using conditionals
			for choosing memory accesses - and that way, we avoid if statements.
		*/


		/* Update Cells that are already marked by the markers. */
		for(auto& p : markers)
		{
			k 	  = toIndexKey(p.pos);
			found = cells.lookup(k);
			/* Use condition to determine correct memory location to read from. */
			status = (found == nullptr);
			tmp = status ? &c : found; /* CONDITIONAL_SET_PTR(tmp, &c, found == nullptr); */


			toInsert.x   = p.pos;
			toInsert.u_p = tmp->u_p;
			shouldInsert = 				   (   status && inBounds(p.pos)  		         );
			shouldInsert = shouldInsert || (  !status && found->type() != CELL_TYPE_SOLID);
			if(shouldInsert) 
			{
				status = cells.insert(k, toInsert); /* Need to check for status incase of error. */
				if(unlikely( status != FLAT_HASH_INSERT_SUCCESS )) {
					cells.rehash(); 
				}
			}
		}
	}


	void createFluid_AirBufferZone()
	{
		std::vector<GridPosition> liquidOrAir;
		bool shouldVisit, shouldUpdate;
		
		std::array<Cell*, 6>     NeighbourPtr  = { nullptr };
		std::array<i32, 6>       NeighbourKeys = { 0 };
		const std::array<i32, 4> NKeys = {
			 __scast(i32, bounds.z),
			-__scast(i32, bounds.z),
			 __scast(i32, bounds.z * bounds.y),
			-__scast(i32, bounds.z * bounds.y),
		};
		const std::array<math::vec3i, 6> delta = {
			math::vec3i{  0,  0, +1 },
			math::vec3i{  0,  0, -1 },
			math::vec3i{  0, +1,  0 },
			math::vec3i{  0, -1,  0 },
			math::vec3i{ +1,  0,  0 },
			math::vec3i{ -1,  0,  0 },
		};
		math::vec3i position, position2;
		Cell tmp = {
			{},
			{},
			0 | CELL_TYPE_AIR
		};


		for(u32 i = 1; i < std::max( 2u, __scast( u32, ceilf(k_cfl) ) ); ++i)
		{
			/* Collect All Relevant (Fluid/Air) Cells */
			for(auto& cell : cells) 
			{
				shouldVisit = cell.type() != CELL_TYPE_SOLID && cell.type() != CELL_TYPE_INVALID;
				shouldVisit = shouldVisit && cell.layer() == (i - 1);
				
				if(shouldVisit) liquidOrAir.push_back(cell.x);
			}


			/*
				Effectively, this works out to:
				for every neighbour n of C: 
					if n == nullptr:
						n->type = AIR; { i.e default }
						IF OutOfSimulationBounds(n): n->type = SOLID
						insert n to table
					else
						IF n exists in hashtable AND n->type = AIR OR FLUID:
							n->bitFields = tmp.bitFields [n->layer = i, n->type = AIR]
						insert n to table (because n already exists, it just updates the value in the table)
			*/
			for(auto& pos : liquidOrAir) 
			{
				i32 key = toIndexKey(pos);
				NeighbourKeys = {
					key + 1, 		/* +z neighbour */
					key - 1, 		/* -z neighbour */
					key + NKeys[0], /* +y neighbour */
					key + NKeys[1], /* -y neighbour */
					key + NKeys[2], /* +x neighbour */
					key + NKeys[3]  /* -x neighbour */
				};
				NeighbourPtr = {
					cells.lookup(NeighbourKeys[0]),
					cells.lookup(NeighbourKeys[1]),
					cells.lookup(NeighbourKeys[2]),
					cells.lookup(NeighbourKeys[3]),
					cells.lookup(NeighbourKeys[4]),
					cells.lookup(NeighbourKeys[5]) 
				};

				position = pos;
				tmp.x = pos;
				tmp.bitFields = i | CELL_TYPE_AIR;
				for(u32 i = 0; i < NeighbourKeys.size(); ++i) {
					shouldUpdate    = (NeighbourPtr[i] == nullptr);
					NeighbourPtr[i] = shouldUpdate ? &tmp : NeighbourPtr[i]; /* CONDITIONAL_SET_PTR(NeighbourPtr[i], &tmp, NeighbourPtr[i] == nullptr); */
					
					position2 = position + delta[i];
					NeighbourPtr[i]->bitFields += shouldUpdate * (inBounds(position2) << (CELL_TYPE_SHIFT + 1));
					NeighbourPtr[i]->bitFields = !shouldUpdate && ( 
							NeighbourPtr[i]->bitFields == CELL_TYPE_FLUID 
							|| 
							NeighbourPtr[i]->bitFields == CELL_TYPE_AIR
						) 
						? tmp.bitFields : NeighbourPtr[i]->bitFields;
					/*
						Alternative Code Without Ternary:
						CONDITIONAL_SET(NeighbourPtr[i]->bitFields, tmp.bitFields, 
							!shouldUpdate && ( 
								NeighbourPtr[i]->bitFields == CELL_TYPE_FLUID 
								|| 
								NeighbourPtr[i]->bitFields == CELL_TYPE_AIR
							) 
						);
					*/
					cells.insert(__scast(u64, NeighbourKeys[i]), *NeighbourPtr[i]);
				}
			}


		}
		return;
	}


	void cleanupCells()
	{
		std::vector<u64> collection;
		for(auto& cell : cells) {
			if ( cell.layer() == 0 ) collection.push_back(toIndexKey(cell.x));
		}
		for(auto& deletionIndex : collection) {
			cells.del(deletionIndex);
		}
		return;
	}


	void updateGrid()
	{
		resetCells();
		updateFluidCells();
		createFluid_AirBufferZone();
		cleanupCells();
		return;
	}


private:  /* Update Grid End   */


	void extractNeighbours(math::vec3u const& pos, std::array<math::vec3f, 8>& extracted)
	{
		const std::array<u32, 8> nbrKey = {
			toIndexKey({0, 0, 0}), toIndexKey({0, 0, 1}), toIndexKey({0, 1, 0}), toIndexKey({0, 1, 1}),
			toIndexKey({1, 0, 0}), toIndexKey({1, 0, 1}), toIndexKey({1, 1, 0}), toIndexKey({1, 1, 1}) 
		};
		u32 indexKey = toIndexKey(pos);
		static Cell defaultCell{}; /* this will have to do for now as I don't know how to treat boundaries properly YET. */

		static alignsz(32) std::array<Cell*, 8> VelPtrs = {
			cells.lookup(indexKey + nbrKey[0]), cells.lookup(indexKey + nbrKey[1]), cells.lookup(indexKey + nbrKey[2]), cells.lookup(indexKey + nbrKey[3]),
			cells.lookup(indexKey + nbrKey[4]), cells.lookup(indexKey + nbrKey[5]), cells.lookup(indexKey + nbrKey[6]), cells.lookup(indexKey + nbrKey[7]),
		};


		// static alignsz(32) std::array<Cell*, 8> defPtrs{ &defaultCell };
		// __m256i velptrVec = _mm256_set1_epi32(indexKey);
		// __m256i nbrkeyVec = _mm256_load_epi32(nbrKey.begin());
		// velptrVec = _mm256_add_epi32(velptrVec, nbrkeyVec);
		// _mm256_maskstore_ps((f32*)defPtrs.begin(), *(__m256i*)VelPtrs.begin(), VelPtrs);


		VelPtrs[0] = (VelPtrs[0] == nullptr) ? &defaultCell : VelPtrs[0];
		VelPtrs[1] = (VelPtrs[1] == nullptr) ? &defaultCell : VelPtrs[1];
		VelPtrs[2] = (VelPtrs[2] == nullptr) ? &defaultCell : VelPtrs[2];
		VelPtrs[3] = (VelPtrs[3] == nullptr) ? &defaultCell : VelPtrs[3];
		VelPtrs[4] = (VelPtrs[4] == nullptr) ? &defaultCell : VelPtrs[4];
		VelPtrs[5] = (VelPtrs[5] == nullptr) ? &defaultCell : VelPtrs[5];
		VelPtrs[6] = (VelPtrs[6] == nullptr) ? &defaultCell : VelPtrs[6];
		VelPtrs[7] = (VelPtrs[7] == nullptr) ? &defaultCell : VelPtrs[7];

		for(size_t i = 0; i < 8; ++i) {
			extracted[i] = VelPtrs[i]->u_p.xmm;
		}
		return;
	}


	void lerpedValue(__unused math::vec3f const& pos)
	{

	}
	

	// void velocityAt(math::vec3f const& pos, math::vec3f& out)
	// {
	// 	alignsz(64) std::array<math::vec3f, 8> Velocities;
	// 	alignsz(32) std::array<f32, 8> 		   VelocityI[3];
	// 	math::vec3u gridPos[3];
	// 	math::vec3f lerpWeights[3];


	// 	lerpWeights[0] = _mm_floor_ps(pos.xmm);
	// 	gridPos[0] 	= cvtu32(lerpWeights);

	// 	extractNeighbours(gridPos, Velocities);
	// 	for(size_t i = 0; i < 8; ++i) { VelocityI[0][i] = Velocities[i].x; }
	// 	for(size_t i = 0; i < 8; ++i) { VelocityI[1][i] = Velocities[i].y; }
	// 	for(size_t i = 0; i < 8; ++i) { VelocityI[2][i] = Velocities[i].z; }

	// 	lerpWeights = pos - lerpWeights;
	// 	out.x = TriLerpSimd(VelocityI[0], lerpWeights);
	// 	out.y = TriLerpSimd(VelocityI[1], lerpWeights);
	// 	out.z = TriLerpSimd(VelocityI[2], lerpWeights);
	// 	return;
	// }

	f32 TriLerpCellVelocities(__unused Vec3 const& pos)
	{
		return 1.0f;
	}


	void velocityAt(Vec3 const& pos, Vec3& out)
	{
		Vec3 normpos   = pos * (1.0f / dx);
		Vec3 normposmh = normpos - Vec3{0.5f};
		Vec3 blend[3];

		blend[0] = _mm_blend_ps(normpos.xmm, normposmh.xmm, 0b0110);
		blend[1] = _mm_blend_ps(normpos.xmm, normposmh.xmm, 0b1010);
		blend[2] = _mm_blend_ps(normpos.xmm, normposmh.xmm, 0b1100);
		out = {
			TriLerpCellVelocities(blend[0]),
			TriLerpCellVelocities(blend[1]),
			TriLerpCellVelocities(blend[2])
		};
		return;
	}


	void advect(GridPosition const& pos)
	{
		Vec3 v, posf0 = cvtf32(pos), posf1 = posf0, t{ dt * 0.5f };
		
		velocityAt(posf0, v);
		posf0 += dt * v;
		velocityAt(posf0, v);
		
		t *= 2.0f;
		posf1 += t * v; 
		return;
	}



public:


	void update()
	{
		dt = k_cfl * dx / uMax;


		updateGrid();
		return;
	}


};


void advect(Simulation2D& )
{

}


void applyForces();
void applyViscosity();
void project();


void extrapolate();

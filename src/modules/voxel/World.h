/**
 * @file
 */

#pragma once

#include "core/GLM.h"
#include "core/Common.h"
#include "polyvox/Mesh.h"
#include "polyvox/PagedVolume.h"
#include "polyvox/Raycast.h"
#include "voxel/Constants.h"
#include <memory>
#include <vector>
#include <atomic>
#include <list>

#include "WorldPager.h"
#include "WorldContext.h"
#include "io/Filesystem.h"
#include "BiomeManager.h"
#include "core/ConcurrentQueue.h"
#include "core/ThreadPool.h"
#include "core/Var.h"
#include "core/Random.h"
#include "core/Log.h"
#include <unordered_set>

namespace voxel {

struct ChunkMeshes {
	static constexpr bool MAY_GET_RESIZED = true;
	ChunkMeshes(int opaqueVertices, int opaqueIndices, int waterVertices, int waterIndices) :
			opaqueMesh(opaqueVertices, opaqueIndices, MAY_GET_RESIZED), waterMesh(waterVertices, waterIndices, MAY_GET_RESIZED) {
	}

	inline const glm::ivec3& translation() const {
		return opaqueMesh.getOffset();
	}

	Mesh opaqueMesh;
	Mesh waterMesh;

	inline bool operator<(const ChunkMeshes& rhs) const {
		return glm::all(glm::lessThan(translation(), rhs.translation()));
	}
};

typedef std::unordered_set<glm::ivec3, std::hash<glm::ivec3> > PositionSet;

class World {
public:
	enum Result {
		COMPLETED, ///< If the ray passed through the volume without being interupted
		INTERUPTED, ///< If the ray was interupted while travelling
		FAILED
	};

	World();
	~World();

	void setContext(const WorldContext& ctx) {
		_ctx = ctx;
	}

	// if clientData is true, additional data that is only useful for rendering is generated
	void setClientData(bool clientData) {
		_clientData = clientData;
	}

	/**
	 * @return true if the ray hit something - false if not. If true is returned, the position is set to @c hit and
	 * the @c Voxel that was hit is stored in @c voxel
	 * @param[out] hit If the ray hits a voxel, this is the position of the hit
	 * @param[out] voxel The voxel that was hit
	 */
	bool raycast(const glm::vec3& start, const glm::vec3& direction, float maxDistance, glm::ivec3& hit, Voxel& voxel) const;

	/**
	 * @return true if the ray hit something - false if not.
	 * @note The callback has a parameter of @c const PagedVolume::Sampler& and returns a boolean. If the callback returns false,
	 * the ray is interrupted. Only if the callback returned false at some point in time, this function will return @c true.
	 */
	template<typename Callback>
	inline bool raycast(const glm::vec3& start, const glm::vec3& direction, float maxDistance, Callback&& callback) const {
		const RaycastResults::RaycastResult result = raycastWithDirection(_volumeData, start, direction * maxDistance, std::forward<Callback>(callback));
		return result == RaycastResults::Interupted;
	}

	bool init(const std::string& luaParameters, const std::string& luaBiomes, uint32_t volumeMemoryMegaBytes = 512, uint16_t chunkSideLength = 256);
	void shutdown();
	void reset();
	bool isReset() const;

	bool findPath(const glm::ivec3& start, const glm::ivec3& end, std::list<glm::ivec3>& listResult);

	template<typename VoxelTypeChecker>
	int findFloor(int x, int z, VoxelTypeChecker&& check) const {
		const glm::vec3 start = glm::vec3(x, MAX_HEIGHT, z);
		const glm::vec3& direction = glm::down;
		const float distance = (float)MAX_HEIGHT;
		int y = NO_FLOOR_FOUND;
		raycast(start, direction, distance, [&] (const PagedVolume::Sampler& sampler) {
			if (check(sampler.getVoxel().getMaterial())) {
				y = sampler.getPosition().y;
				return false;
			}
			return true;
		});
		return y;
	}

	VoxelType getMaterial(int x, int y, int z) const;

	BiomeManager& getBiomeManager();
	const BiomeManager& getBiomeManager() const;

	void setVoxel(const glm::ivec3& pos, const voxel::Voxel& voxel);

	/**
	 * @brief Returns a random position inside the boundaries of the world (on the surface)
	 */
	glm::ivec3 randomPos() const;

	/**
	 * @brief Cuts the given world coordinate down to mesh tile vectors
	 */
	inline glm::ivec3 getMeshPos(const glm::ivec3& pos) const {
		const float size = getMeshSize();
		const int x = glm::floor(pos.x / size);
		const int y = glm::floor(pos.y / size);
		const int z = glm::floor(pos.z / size);
		return glm::ivec3(x * size, y * size, z * size);
	}

	/**
	 * @brief Cuts the given world coordinate down to chunk tile vectors
	 */
	inline glm::ivec3 getChunkPos(const glm::ivec3& pos) const {
		const float size = getChunkSize();
		const int x = glm::floor(pos.x / size);
		const int y = glm::floor(pos.y / size);
		const int z = glm::floor(pos.z / size);
		return glm::ivec3(x, y, z);
	}

	/**
	 * @brief We need to pop the mesh extractor queue to find out if there are new and ready to use meshes for us
	 */
	inline bool pop(ChunkMeshes& item) {
		return _meshQueue.pop(item);
	}

	void stats(int& meshes, int& extracted, int& pending) const;

	/**
	 * @brief If you don't need an extracted mesh anymore, make sure to allow the reextraction at a later time.
	 * @param[in] pos A World vector that is automatically converted into a mesh tile vector
	 * @return @c true if the given position was already extracted, @c false if not.
	 */
	bool allowReExtraction(const glm::ivec3& pos);

	/**
	 * @brief Performs async mesh extraction. You need to call @c pop in order to see if some extraction is ready.
	 *
	 * @param[in] pos A World vector that is automatically converted into a mesh tile vector
	 * @note This will not allow to reschedule an extraction for the same area until @c allowReExtraction was called.
	 */
	bool scheduleMeshExtraction(const glm::ivec3& pos);

	void onFrame(long dt);

	const core::Random& random() const { return _random; }

	inline long seed() const { return _seed; }

	void setSeed(long seed) {
		Log::info("Seed is: %li", seed);
		_seed = seed;
		_random.setSeed(seed);
		_pager.setSeed(seed);
		_pager.setNoiseOffset(glm::vec2(_random.randomf(-10000.0f, 10000.0f), _random.randomf(-10000.0f, 10000.0f)));
	}

	inline bool isCreated() const {
		return _seed != 0;
	}

	inline void setPersist(bool persist) {
		_pager.setPersist(persist);
	}

	int getChunkSize() const;
	PagedVolume::ChunkPtr getChunk(const glm::ivec3& pos) const;
	int getMeshSize() const;

private:
	void cleanupFutures();
	Region getChunkRegion(const glm::ivec3& pos) const;
	Region getMeshRegion(const glm::ivec3& pos) const;
	Region getRegion(const glm::ivec3& pos, int size) const;

	WorldPager _pager;
	PagedVolume *_volumeData = nullptr;
	BiomeManager _biomeManager;
	WorldContext _ctx;
	mutable std::mt19937 _engine;
	long _seed = 0l;
	bool _clientData = false;

	core::ThreadPool _threadPool;
	core::ConcurrentQueue<ChunkMeshes> _meshQueue;
	// fast lookup for positions that are already extracted and available in the _meshData vector
	PositionSet _meshesExtracted;
	core::VarPtr _meshSize;
	core::Random _random;
	std::vector<std::future<void> > _futures;
	std::atomic_bool _cancelThreads { false };
};

inline Region World::getChunkRegion(const glm::ivec3& pos) const {
	const int size = getChunkSize();
	return getRegion(pos, size);
}

inline Region World::getMeshRegion(const glm::ivec3& pos) const {
	const int size = getMeshSize();
	return getRegion(pos, size);
}

inline int World::getChunkSize() const {
	return _volumeData->getChunkSideLength();
}

inline PagedVolume::ChunkPtr World::getChunk(const glm::ivec3& pos) const {
	return _volumeData->getChunk(pos);
}

inline int World::getMeshSize() const {
	return _meshSize->intVal();
}

inline VoxelType World::getMaterial(int x, int y, int z) const {
	const Voxel& voxel = _volumeData->getVoxel(x, y, z);
	return voxel.getMaterial();
}

inline BiomeManager& World::getBiomeManager() {
	return _biomeManager;
}

inline const BiomeManager& World::getBiomeManager() const {
	return _biomeManager;
}

typedef std::shared_ptr<World> WorldPtr;

}

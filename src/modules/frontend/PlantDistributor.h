#pragma once

#include "core/Common.h"
#include <vector>

namespace voxel {
class World;
typedef std::shared_ptr<World> WorldPtr;
}

namespace frontend {

extern void distributePlants(const voxel::WorldPtr& world, int amount, const glm::ivec3& pos, std::vector<glm::vec3>& translations);

}
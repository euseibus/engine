#pragma once

#include "core/Common.h"
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#define IM_VEC2_CLASS_EXTRA                                                   \
        ImVec2(const glm::ivec2& f) { x = f.x; y = f.y; }                     \
        operator glm::ivec2() const { return glm::ivec2(x,y); }

#define IM_VEC4_CLASS_EXTRA                                                   \
        ImVec4(const glm::ivec4& f) { x = f.x; y = f.y; z = f.z; w = f.w; }   \
        operator glm::ivec4() const { return glm::ivec4(x,y,z,w); }

#define IM_ASSERT(_EXPR) core_assert(_EXPR)

#include <dearimgui/imgui.h>
#define IMGUI_DEFINE_PLACEMENT_NEW
#include <dearimgui/imgui_internal.h>
#include <utility>

template<class T, class ... Args>
T* imguiAlloc(Args&&... args) {
	T* instance = (T*) ImGui::MemAlloc(sizeof(T));
	IM_PLACEMENT_NEW(instance) T(std::forward<Args>(args)...);
	return instance;
}

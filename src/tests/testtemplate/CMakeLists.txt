project(testtemplate)
set(SRCS
	TestTemplate.h TestTemplate.cpp
)
engine_add_executable(TARGET ${PROJECT_NAME} SRCS ${SRCS} WINDOWED)
engine_target_link_libraries(TARGET ${PROJECT_NAME} DEPENDENCIES testcore)
#generate_shaders(${PROJECT_NAME} color)

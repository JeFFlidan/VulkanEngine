add_compile_definitions(RENDER_CORE_EXPORT)

set (LIBRARY_NAME render_core)

file(GLOB RENDER_CORE_FILES
	${DIR_ENGINE_SRC}/render_core/*/*.cpp
	${DIR_ENGINE_SRC}/render_core/*/*.h
	${DIR_ENGINE_SRC}/render_core/*/*/*.cpp
	${DIR_ENGINE_SRC}/render_core/*/*/*.h
)

add_library(${LIBRARY_NAME} SHARED ${RENDER_CORE_FILES})
set_target_properties(${LIBRARY_NAME} PROPERTIES OUTPUT_NAME ${LIBRARY_NAME})

if (GLSL_SUPPORT)
	add_compile_definitions(GLSL_SUPPORT)
endif()

target_link_libraries(${LIBRARY_NAME} engine_core)

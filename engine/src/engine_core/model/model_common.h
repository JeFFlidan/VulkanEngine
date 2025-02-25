#pragma once

#include "engine_core/uuid.h"
#include "rhi/resources.h"
#include "core/math_base.h"
#include <vector>
#include <string>

namespace ad_astris::ecore::model
{
	// Describes vertex with position and tex coord
	struct VertexF32PC
	{
		XMFLOAT3 position;
		XMFLOAT2 texCoord;
	};

	// Describes vertex with position, normal, tangent and tex coord
	struct VertexF32PNTC
	{
		XMFLOAT3 position;
		XMFLOAT3 normal;
		XMFLOAT3 tangent;
		XMFLOAT2 texCoord;
	};

	enum class VertexFormat
	{
		UNKNOWN = 0,
		F32_PNTC,
		F32_PC
	};

	struct ModelBounds
	{
		XMFLOAT3 origin;
		float radius;
		XMFLOAT3 extents;
	};
	
	struct StaticModelInfo
	{
		// Data for operations under hood
		UUID uuid;
		uint64_t vertexBufferSize{ 0 };
		uint64_t indexBufferSize{ 0 };
		ModelBounds bounds;
		VertexFormat vertexFormat{ VertexFormat::UNKNOWN };
		std::string originalFile;
		std::vector<std::string> materialsName;		// Materials name from model file
	};

	class Utils
	{
		public:
			static std::string get_str_vertex_format(VertexFormat format);
			static VertexFormat get_enum_vertex_format(std::string format);
			static std::string pack_static_model_info(StaticModelInfo* info);
			static StaticModelInfo unpack_static_model_info(std::string& strMetaData);
			static ModelBounds calculate_model_bounds(VertexF32PNTC* vertices, uint64_t count);
			static void calculate_tangent(VertexF32PNTC* vertices);
			static void setup_f32_pntc_format_description(
				std::vector<rhi::VertexBindingDescription>& bindingDescriptions,
				std::vector<rhi::VertexAttributeDescription>& attributeDescriptions);
			// Must be used with deferred lighting pass, postprocessing, etc.
			static void setup_f32_pc_format_description(
				std::vector<rhi::VertexBindingDescription>& bindingDescriptions,
				std::vector<rhi::VertexAttributeDescription>& attributeDescriptions);
	};
}
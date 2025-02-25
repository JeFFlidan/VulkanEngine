﻿#pragma once

#include <algorithm>
#include <cmath>
#if defined(_MSC_VER)
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXCollision.h>
#else
#include <math/DirectXMath.h>
#include <math/DirectXPackedVector.h>
#include <math/DirectXCollision.h>
#endif

using namespace DirectX;
using namespace PackedVector;

namespace ad_astris::math
{
	static constexpr float PI = XM_PI;
	static constexpr XMFLOAT4X4 IDENTITY_MATRIX = XMFLOAT4X4 {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	inline float length(XMFLOAT2 const& vec)
	{
		return std::sqrt(vec.x * vec.x + vec.y * vec.y);
	}

	inline float length(XMFLOAT3 const& vec)
	{
		return std::sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
	}

	inline float length(XMFLOAT4 const& vec)
	{
		return std::sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z + vec.w * vec.w);
	}

	// Calculates distance between 3D vectors
	inline float distance(XMVECTOR const& vec1, XMVECTOR const& vec2)
	{
		return XMVectorGetX(XMVector3Length(XMVectorSubtract(vec1, vec2)));
	}

	// Calculates squared distance between 3D vectors
	inline float distance_squared(XMVECTOR const& vec1, XMVECTOR const& vec2)
	{
		return XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(vec1, vec2)));
	}

	inline float distance_estimated(XMVECTOR const& vec1, XMVECTOR const& vec2)
	{
		return XMVectorGetX(XMVector3LengthEst(XMVectorSubtract(vec1, vec2)));
	}

	inline float distance(XMFLOAT2 const& vec1, XMFLOAT2 const& vec2)
	{
		return std::sqrt(
			(vec2.x - vec1.x) * (vec2.x - vec1.x) + 
			(vec2.y - vec1.y) * (vec2.y - vec1.y));
	}

	inline float distance_squared(XMFLOAT2 const& vec1, XMFLOAT2 const& vec2)
	{
		return (vec2.x - vec1.x) * (vec2.x - vec1.x) + (vec2.y - vec1.y) * (vec2.y - vec1.y);
	}

	inline float distance_estimated(XMFLOAT2 const& vec1, XMFLOAT2 const& vec2)
	{
		return XMVectorGetX(XMVector2LengthEst(XMLoadFloat2(&vec2) - XMLoadFloat2(&vec1)));
	}

	inline float distance(XMFLOAT3 const& vec1, XMFLOAT3 const& vec2)
	{
		return std::sqrt(
			(vec2.x - vec1.x) * (vec2.x - vec1.x) + 
			(vec2.y - vec1.y) * (vec2.y - vec1.y) +
			(vec2.z - vec1.z) * (vec2.z - vec1.z));
	}

	inline float distance_squared(XMFLOAT3 const& vec1, XMFLOAT3 const& vec2)
	{
		return (vec2.x - vec1.x) * (vec2.x - vec1.x) + 
			(vec2.y - vec1.y) * (vec2.y - vec1.y) +
			(vec2.z - vec1.z) * (vec2.z - vec1.z);
	}

	inline float distance_estimated(XMFLOAT3 const& vec1, XMFLOAT3 const& vec2)
	{
		return XMVectorGetX(XMVector3LengthEst(XMLoadFloat3(&vec2) - XMLoadFloat3(&vec1)));
	}
	
	inline float distance(XMFLOAT4 const& vec1, XMFLOAT4 const& vec2)
	{
		return std::sqrt(
			(vec2.x - vec1.x) * (vec2.x - vec1.x) + 
			(vec2.y - vec1.y) * (vec2.y - vec1.y) +
			(vec2.z - vec1.z) * (vec2.z - vec1.z) +
			(vec2.w - vec1.w) * (vec2.w - vec1.w));
	}

	inline float distance_squared(XMFLOAT4 const& vec1, XMFLOAT4 const& vec2)
	{
		return (vec2.x - vec1.x) * (vec2.x - vec1.x) + 
			(vec2.y - vec1.y) * (vec2.y - vec1.y) +
			(vec2.z - vec1.z) * (vec2.z - vec1.z) +
			(vec2.w - vec1.w) * (vec2.w - vec1.w);
	}

	inline float distance_estimated(XMFLOAT4 const& vec1, XMFLOAT4 const& vec2)
	{
		return XMVectorGetX(XMVector4LengthEst(XMLoadFloat4(&vec2) - XMLoadFloat4(&vec1))); 
	}

	inline float dot(XMFLOAT2 const& vec1, XMFLOAT2 const& vec2)
	{
		return vec1.x * vec2.x + vec1.y * vec2.y;
	}

	inline float dot(XMFLOAT3 const& vec1, XMFLOAT3 const& vec2)
	{
		return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z;
	}

	inline float dot(XMFLOAT4 const& vec1, XMFLOAT4 const& vec2)
	{
		return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z + vec1.w * vec2.w;
	}

	inline XMFLOAT3 to_euler_angles(XMVECTOR quaternion)
	{
		XMQuaternionNormalize(quaternion);
		XMFLOAT4 q;
		XMStoreFloat4(&q, quaternion);

		XMFLOAT3 output;
		float sinrCosp = 2 * (q.w * q.x + q.y * q.z);
		float cosrCosp = 1 - 2 * (q.x * q.x + q.y * q.y);
		output.x = std::atan2(sinrCosp, cosrCosp);

		float sinp = std::sqrt(1 + 2 * (q.w * q.y - q.x * q.z));
		float cosp = std::sqrt(1 - 2 * (q.w * q.y - q.x * q.z));
		output.y = 2 * std::atan2(sinp, cosp) - PI / 2;

		float sinyCosp = 2 * (q.w * q.z + q.x * q.y);
		float cosyCosp = 1 - 2 * (q.y * q.y + q.z * q.z);
		output.z = std::atan2(sinyCosp, cosyCosp);

		return output;
	}

	inline uint32_t get_mip_levels(uint32_t width, uint32_t height)
	{
		return uint32_t(std::floor(std::log2(std::max(width, height)))) + 1;
	}

	inline uint32_t previous_pow2(uint32_t val)
	{
		uint32_t r = 1;
		while (r * 2 < val)
			r *= 2;
		return r;
	}

	inline uint32_t saturate(float val)
	{
		return std::min(std::max(val, 0.0f), 1.0f);
	}

	inline uint32_t compress_color(const XMFLOAT3& color)
	{
		uint32_t result = 0;
		result |= (uint32_t)((uint8_t)(saturate(color.x) * 255.0f) << 0);
		result |= (uint32_t)((uint8_t)(saturate(color.y) * 255.0f) << 8);
		result |= (uint32_t)((uint8_t)(saturate(color.z) * 255.0f) << 16);
		return result;
	}

	inline uint32_t compress_color(const XMFLOAT4& color)
	{
		uint32_t result = 0;
		result |= (uint32_t)((uint8_t)(saturate(color.x) * 255.0f) << 0);
		result |= (uint32_t)((uint8_t)(saturate(color.y) * 255.0f) << 8);
		result |= (uint32_t)((uint8_t)(saturate(color.z) * 255.0f) << 16);
		result |= (uint32_t)((uint8_t)(saturate(color.w) * 255.0f) << 24);
		return result;
	}

	inline XMFLOAT2 max(const XMFLOAT2& vec1, const XMFLOAT2& vec2)
	{
		return XMFLOAT2(std::max(vec1.x, vec2.x), std::max(vec1.y, vec2.y));
	}

	inline XMFLOAT3 max(const XMFLOAT3& vec1, const XMFLOAT3& vec2)
	{
		return XMFLOAT3(std::max(vec1.x, vec2.x), std::max(vec1.y, vec2.y), std::max(vec1.z, vec2.z));
	}

	inline XMFLOAT4 max(const XMFLOAT4& vec1, const XMFLOAT4& vec2)
	{
		return XMFLOAT4(std::max(vec1.x, vec2.x), std::max(vec1.y, vec2.y), std::max(vec1.z, vec2.z), std::max(vec1.w, vec2.w));
	}

	inline XMFLOAT2 min(const XMFLOAT2& vec1, const XMFLOAT2& vec2)
	{
		return XMFLOAT2(std::min(vec1.x, vec2.x), std::min(vec1.y, vec2.y));
	}

	inline XMFLOAT3 min(const XMFLOAT3& vec1, const XMFLOAT3& vec2)
	{
		return XMFLOAT3(std::min(vec1.x, vec2.x), std::min(vec1.y, vec2.y), std::min(vec1.z, vec2.z));
	}

	inline XMFLOAT4 min(const XMFLOAT4& vec1, const XMFLOAT4& vec2)
	{
		return XMFLOAT4(std::min(vec1.x, vec2.x), std::min(vec1.y, vec2.y), std::min(vec1.z, vec2.z), std::min(vec1.w, vec2.w));
	}

	inline XMVECTOR closest_point_on_line_segment(const XMVECTOR& a, const XMVECTOR& b, const XMVECTOR& point)
	{
		XMVECTOR ab = XMVectorSubtract(a, b);
		XMVECTOR d = XMVectorDivide(XMVector3Dot(XMVectorSubtract(point, a), ab), XMVector3Dot(ab, ab));
		return XMVectorAdd(a, XMVectorMultiply(XMVectorSaturate(d), ab));
	}
}

#pragma once
#include <stdexcept>

#include "Camera.h"
#include "DeferredRenderSystem.h"

namespace cve
{

	// from: https://stackoverflow.com/a/57595105
	template <typename T, typename... Rest>
	void hashCombine(std::size_t& seed, const T& v, const Rest&... rest) 
	{
		seed ^= std::hash<T>{}(v)+0x9e3779b9 + (seed << 6) + (seed >> 2);
		(hashCombine(seed, rest), ...);
	};

	inline glm::mat4 CalculateLightsViewProj(Camera& camera, Light& light)
	{
		if (light.type != LightType::Directional)
			throw std::runtime_error("cannot calculate lightProjView matrix for point light");

		using glm::mat4;
		using glm::vec3;
		using glm::vec4;
		using glm::normalize;
		using glm::length;
		using glm::inverse;
		using glm::lookAt;
		using glm::ortho;

		// 1) Inverse of camera VP
		mat4 invCam = inverse(camera.GetProjectionMatrix() * camera.GetViewMatrix());

		// 2) World-space frustum corners (NDC cube ? world)
		std::array<vec4, 8> frustumCorners;
		int idx = 0;
		for (int x = 0; x < 2; ++x)
			for (int y = 0; y < 2; ++y)
				for (int z = 0; z < 2; ++z) {
					vec4 ndc = vec4(
						x == 0 ? -1.0f : 1.0f,
						y == 0 ? -1.0f : 1.0f,
						z == 0 ? -1.0f : 1.0f,
						1.0f
					);
					vec4 world = invCam * ndc;
					frustumCorners[idx++] = world / world.w;
				}

		// 3) Compute center of those corners
		vec3 center{ 0.0f };
		for (auto& c : frustumCorners)
			center += vec3(c);
		center /= float(frustumCorners.size());

		// 4) Treat light.position as a *direction* and normalize
		vec3 lightDir = normalize(light.position);

		// 5) Compute enclosing sphere radius (optional texel?snapping via ceil)
		float radius = 0.0f;
		for (auto& c : frustumCorners)
			radius = std::max(radius, length(vec3(c) - center));
		radius = std::ceil(radius * 16.0f) / 16.0f;

		// 6) Build light-space view matrix
		vec3 lightPos = center - lightDir * radius;
		mat4 lightView = lookAt(lightPos, center, vec3(0.0f, 1.0f, 0.0f));

		// 7) Transform corners into light space ? find AABB
		float minX = std::numeric_limits<float>::infinity();
		float maxX = -std::numeric_limits<float>::infinity();
		float minY = std::numeric_limits<float>::infinity();
		float maxY = -std::numeric_limits<float>::infinity();
		float minZ = std::numeric_limits<float>::infinity();
		float maxZ = -std::numeric_limits<float>::infinity();

		for (auto& c : frustumCorners) {
			vec4 lc = lightView * c;
			minX = std::min(minX, lc.x);
			maxX = std::max(maxX, lc.x);
			minY = std::min(minY, lc.y);
			maxY = std::max(maxY, lc.y);
			minZ = std::min(minZ, lc.z);
			maxZ = std::max(maxZ, lc.z);
		}

		// 8) Build orthographic projection that tightly fits the frustum
		mat4 lightProj = ortho(minX, maxX, minY, maxY, minZ, maxZ);

		// 9) Return light-space ViewProj
		mat4 lightViewProj = lightProj * lightView;

		return lightViewProj;
	}


}

#pragma once
#include <vector>

inline constexpr unsigned int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> m_ValidationLayers = { "VK_LAYER_KHRONOS_validation" };
#ifdef _DEBUG
inline constexpr bool enableValidationLayers = true;
#else
inline constexpr bool enableValidationLayers = false;
#endif


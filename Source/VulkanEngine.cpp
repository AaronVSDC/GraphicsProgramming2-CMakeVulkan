#include "VulkanEngine.hpp"

namespace cve {

void VulkanEngine::run()
{
    using clock = std::chrono::high_resolution_clock;
    auto lastTime = clock::now();
    int frames = 0;

    while (!glfwWindowShouldClose(m_InitVulkan.GetWindow()))
    {
        auto currentTime = clock::now();
        frames++;

        float duration = std::chrono::duration<float>(currentTime - lastTime).count();
        if (duration >= 1.0f)
        {
            std::cout << "\rFPS: " << frames << std::flush;

            frames = 0;
            lastTime = currentTime;
        }

        glfwPollEvents();
        m_InitVulkan.drawFrame();
    }

    vkDeviceWaitIdle(m_InitVulkan.getDevice());
}


}
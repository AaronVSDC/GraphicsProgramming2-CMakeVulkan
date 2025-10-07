# Vulkan Renderer

## Project Overview
This project was part of my **"Graphics Programming 2"** course in **"Digital Arts and Entertainment"**.  
We made a 3D renderer using the Vulkan API.  

My project supports the following Vulkan-specific techniques or concepts:

- Images and Buffers, including texture loading  
- Depth Buffering  
- Interactive camera with frame-independent controls  
- Deferred Rendering with Depth Prepass  
- Physically Based Lighting using Image-Based Lighting with HDR images, Cook-Torrance BRDF materials (using textures), Point Light, and Directional Light  
- Post Processing chain with Tone Mapping  

---

# Showcase Renders

## Sponza Scene
![Sponza](ReadMe/Sponza.PNG)

## Flight Helmet Scene
![Flight Helmet](ReadMe/FlightHelmet.png)

## Deferred Rendering Showcase
Depth Prepass → Position → MetalRough → Normal → Albedo  
![Deferred Rendering](ReadMe/DefferedRendering.png)

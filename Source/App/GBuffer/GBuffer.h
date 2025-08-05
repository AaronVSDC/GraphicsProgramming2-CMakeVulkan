#pragma once
#include "Device.h"
#include "Texture.h"


namespace cve
{
	class GBuffer final
	{
	public:
		// G-buffer attachment formats
		static constexpr VkFormat POS_FORMAT = VK_FORMAT_R16G16B16A16_SFLOAT; 
		static constexpr VkFormat NORM_FORMAT = VK_FORMAT_R8G8B8A8_UNORM;   
		static constexpr VkFormat ALBEDO_FORMAT = VK_FORMAT_R8G8B8A8_SRGB; // USE THIS FORMAT FOR OTHER TEXTURES PLEASE
		static constexpr VkFormat METALROUGH_FORMAT = VK_FORMAT_R8G8B8A8_SRGB; //r metal, g roughness
		static constexpr VkFormat OCCLUSION_FORMAT = VK_FORMAT_R8G8B8A8_SRGB;



		static constexpr VkFormat DEPTH_FORMAT = VK_FORMAT_D32_SFLOAT;

		void create(Device& device, uint32_t width, uint32_t height);
		void cleanup();

		VkImageView getPositionView()   const { return m_PositionImage->getImageView();  };
		VkImage getPositionImage() const { return m_PositionImage->getImage();  }
		VkImageView getNormalView()     const { return  m_NormalImage->getImageView();  };
		VkImage getNormalImage() const { return m_NormalImage->getImage();  }
		VkImageView getAlbedoSpecView() const { return  m_AlbedoImage->getImageView();  };
		VkImage getAlbedoSpecImage() const { return m_AlbedoImage->getImage();  }
		VkImageView getDepthView()      const { return  m_DepthImage->getImageView();  };
		VkImage getDepthImage() const { return m_DepthImage->getImage();  }

		VkImageView getMetalRoughView()   const { return m_MetalRoughImage->getImageView(); }
		VkImage     getMetalRoughImage()  const { return m_MetalRoughImage->getImage(); }
		VkSampler   getMetalRoughSampler()const { return m_MetalRoughImage->getSampler(); }

		VkImageView getOcclusionView()   const { return m_OcclusionImage->getImageView(); }
		VkImage     getOcclusionImage()  const { return m_OcclusionImage->getImage(); }
		VkSampler   getOcclusionSampler()const { return m_OcclusionImage->getSampler(); }

		uint32_t getWidth() const { return m_Width;  }
		uint32_t getHeight() const { return m_Height;  }

		VkSampler getPositionSampler()   const { return m_PositionImage->getSampler(); }
		VkSampler getNormalSampler()     const { return m_NormalImage->getSampler(); } 
		VkSampler getAlbedoSpecSampler() const { return m_AlbedoImage->getSampler(); }

		VkImageLayout m_PositionLayout;
		VkImageLayout m_NormalLayout;
		VkImageLayout m_AlbedoLayout;
		VkImageLayout m_DepthLayout;
		VkImageLayout m_MetalRoughLayout; 
		VkImageLayout m_OcclusionLayout; 
	private:

		std::unique_ptr<Texture> m_PositionImage;
		std::unique_ptr<Texture> m_NormalImage;
		std::unique_ptr<Texture> m_AlbedoImage;
		std::unique_ptr<Texture> m_DepthImage;
		std::unique_ptr<Texture> m_MetalRoughImage;
		std::unique_ptr<Texture> m_OcclusionImage;


		uint32_t m_Width, m_Height; 


	};
}

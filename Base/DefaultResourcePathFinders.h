#pragma once

#include "SpehsEngine/Core/ResourcePathFinder.h"
#include "SpehsEngine/Graphics/Renderer.h"


static const std::string ASSET_PATH = "data/assets/";


class ShaderPathFinder : public se::ResourcePathFinder
{
public:

	std::string getPath(const std::string_view _resource) const override
	{
		const se::graphics::RendererBackend rendererBackend = se::graphics::Renderer::getRendererBackend();

		switch (rendererBackend)
		{
		case se::graphics::RendererBackend::Auto:
		case se::graphics::RendererBackend::Direct3D12:
		case se::graphics::RendererBackend::Gnm:
		case se::graphics::RendererBackend::Nvn:		break;

		case se::graphics::RendererBackend::Direct3D9:	return ASSET_PATH + "shader/dx9/" + _resource + ".bin";
		case se::graphics::RendererBackend::Direct3D11: return ASSET_PATH + "shader/dx11/" + _resource + ".bin";
		case se::graphics::RendererBackend::Metal:		return ASSET_PATH + "shader/metal/" + _resource + ".bin";
		case se::graphics::RendererBackend::OpenGLES:
		case se::graphics::RendererBackend::OpenGL:		return ASSET_PATH + "shader/glsl/" + _resource + ".bin";
		case se::graphics::RendererBackend::Vulkan:		return ASSET_PATH + "shader/spirv/" + _resource + ".bin";
		}

		se_assert_m(false, "Unknown RendererBackend: " + std::to_string((int)rendererBackend));
		return "";
	}
};


class TexturePathFinder : public se::ResourcePathFinder
{
public:

	std::string getPath(const std::string_view _resource) const override
	{
		return ASSET_PATH + "texture/" + _resource;
	}
};


class FontPathFinder : public se::ResourcePathFinder
{
public:

	std::string getPath(const std::string_view _resource) const override
	{
		return ASSET_PATH + "font/" + _resource;
	}
};


class ModelPathFinder : public se::ResourcePathFinder
{
public:

	std::string getPath(const std::string_view _resource) const override
	{
		return ASSET_PATH + "model/" + _resource;
	}
};


class AudioPathFinder : public se::ResourcePathFinder
{
public:

	std::string getPath(const std::string_view _resource) const override
	{
		return ASSET_PATH + "audio/" + _resource;
	}
};

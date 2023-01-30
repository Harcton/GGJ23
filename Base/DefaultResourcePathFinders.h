#pragma once

#include "SpehsEngine/Core/ResourcePathFinder.h"
#include "SpehsEngine/Core/SE_Assert.h"
#include "SpehsEngine/Core/StringViewUtilityFunctions.h"
#include "SpehsEngine/Graphics/Renderer.h"
#include "SpehsEngine/Graphics/Types.h"
#include <string>


static const std::string DATA_PATH = "data/";


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

		case se::graphics::RendererBackend::Direct3D9:	return DATA_PATH + "shaders/dx9/" + _resource;
		case se::graphics::RendererBackend::Direct3D11: return DATA_PATH + "shaders/dx11/" + _resource;
		case se::graphics::RendererBackend::Metal:		return DATA_PATH + "shaders/metal/" + _resource;
		case se::graphics::RendererBackend::OpenGLES:
		case se::graphics::RendererBackend::OpenGL:		return DATA_PATH + "shaders/glsl/" + _resource;
		case se::graphics::RendererBackend::Vulkan:		return DATA_PATH + "shaders/spirv/" + _resource;
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
		return DATA_PATH + "textures/" + _resource;
	}
};


class FontPathFinder : public se::ResourcePathFinder
{
public:

	std::string getPath(const std::string_view _resource) const override
	{
		return DATA_PATH + "fonts/" + _resource;
	}
};


class ModelPathFinder : public se::ResourcePathFinder
{
public:

	std::string getPath(const std::string_view _resource) const override
	{
		return DATA_PATH + "models/" + _resource;
	}
};


class AudioPathFinder : public se::ResourcePathFinder
{
public:

	std::string getPath(const std::string_view _resource) const override
	{
		return DATA_PATH + "audio/" + _resource;
	}
};

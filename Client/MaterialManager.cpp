#include "stdafx.h"
#include "Client/MaterialManager.h"

#include "SpehsEngine/Graphics/ShaderManager.h"
#include "SpehsEngine/Graphics/TextureManager.h"
#include "SpehsEngine/Graphics/Uniform.h"
#include "Base/DemoContext.h"


using namespace se::graphics;

MaterialManager::MaterialManager(DemoContext& _context)
	: context(_context)
{
	//////////
	// Load shaders
	context.shaderManager.create("frog", "vs_frog", "fs_frog");

	//////////
	// Create some common textures
	{
		TextureModes genModes;
		genModes.sampleMin = genModes.sampleMag = genModes.sampleMip = TextureSamplingMode::Point;
		TextureInput textureInput;
		textureInput.width = textureInput.height = 1;
		textureInput.data = { 128, 128, 255, 255 };
		context.textureManager.create("flat_normal", textureInput, genModes);
	}
	{
		TextureModes genModes;
		genModes.sampleMin = genModes.sampleMag = genModes.sampleMip = TextureSamplingMode::Point;
		TextureInput textureInput;
		textureInput.format = TextureInput::Format::R8;
		textureInput.width = textureInput.height = 1;
		textureInput.data = { 255 };
		context.textureManager.create("roughness_1", textureInput, genModes);
		textureInput.data = { 0 };
		context.textureManager.create("roughness_0", textureInput, genModes);
	}
}

void MaterialManager::setMaterial(const std::string& _name, std::shared_ptr<Material> _material)
{
	materials[_name] = _material;
}
std::shared_ptr<Material> MaterialManager::getMaterial(const std::string& _name)
{
	auto it = materials.find(_name);
	if (it != materials.end())
	{
		return it->second;
	}
	return nullptr;
}
std::shared_ptr<Material> MaterialManager::createMaterial(MaterialType _type)
{
	switch (_type)
	{
		case MaterialType::DemoFrog:
			{
				std::shared_ptr<Material> material = std::make_shared<Material>();
				material->setShader(context.shaderManager.find("phong"), ShaderVariant::Default);
				material->setShader(context.shaderManager.find("frog"), ShaderVariant::Skinned);
				material->setTexture(nullptr, "s_texColor", PhongTextureType::Color);
				material->setTexture(nullptr, "s_texNormal", PhongTextureType::Normal);
				material->setTexture(nullptr, "s_texRoughness", PhongTextureType::Roughness);
				material->setLit(true);
				material->setUniformContainer(std::make_shared<PhongAttributes>(), "PhongAttributes");
				return material;
			}
	}
	return nullptr;
}
std::shared_ptr<Material> MaterialManager::createMaterial(DefaultMaterialType _type)
{
	return se::graphics::createMaterial(_type, context.shaderManager);
}

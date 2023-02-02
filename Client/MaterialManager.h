#pragma once

#include "SpehsEngine/Graphics/DefaultMaterials.h"


namespace se::graphics
{
	class Material;
}

struct DemoContext;


enum class MaterialType
{
	DemoFrog,
};

class MaterialManager
{
public:
	MaterialManager(DemoContext& _context);

	void setMaterial(const std::string& _name, std::shared_ptr<se::graphics::Material> _material);
	std::shared_ptr<se::graphics::Material> getMaterial(const std::string& _name);
	std::shared_ptr<se::graphics::Material> createMaterial(MaterialType _type);
	std::shared_ptr<se::graphics::Material> createMaterial(se::graphics::DefaultMaterialType _type);

private:

	DemoContext& context;
	std::unordered_map<std::string, std::shared_ptr<se::graphics::Material>> materials;
};

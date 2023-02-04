#include "stdafx.h"
#include "Base/UserSettings.h"

#include "SpehsEngine/Core/File/File.h"


const uint32_t currentVersion = 0;

UserSettings::UserSettings(const std::string_view& _windowName)
	: filepath("user_settings_" + _windowName + ".bin")
{
	read();
}

bool UserSettings::write()
{
	se::Archive archive;
	const uint32_t version = currentVersion;
	se_write_to_archive(archive, version);
#define WRITE(p_Type, p_Name, p_DefaultValue) archive.write(#p_Name, get##p_Name());
	FOR_EACH_USER_SETTING(WRITE)
#undef WRITE
		se::WriteBuffer writeBuffer;
	archive.write(writeBuffer);
	se::File file;
	writeBuffer.swap(file.data);
	file.path = filepath;
	if (se::writeFile(file))
	{
		changedSinceSave = false;
		return true;
	}
	else
	{
		return false;
	}
}

bool UserSettings::read()
{
	se::File file;
	if (se::readFile(file, std::string(filepath)))
	{
		se::ReadBuffer readBuffer(file.data.data(), file.data.size());
		se::Archive archive;
		if (archive.read(readBuffer))
		{
			uint32_t version = 0;
			se_read_from_archive(archive, version);
#define READ(p_Type, p_Name, p_DefaultValue) \
			{ \
				p_Type value = p_DefaultValue; \
				if (archive.read(#p_Name, value)) \
				{ \
					set##p_Name(value); \
				} \
			}
			FOR_EACH_USER_SETTING(READ)
#undef READ
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

void UserSettings::update()
{
	if (hasChangesSinceLastSave())
	{
		write();
	}
}

// Automated setting name generation
std::string formatOptionName(const std::string_view optionName)
{
	std::string result;
	result.reserve(optionName.length());
	bool wasUpper = false;
	for (size_t c = 0; c < optionName.length(); c++)
	{
		const bool isUpper = std::isupper(optionName[c]);
		const bool hasNext = c + 1 < optionName.length();
		const bool nextIsUpper = hasNext ? std::isupper(optionName[c + 1]) : false;
		if (c > 0)
		{
			if ((isUpper && !wasUpper) || (wasUpper && isUpper && !nextIsUpper))
			{
				result += " ";
			}
		}
		result += optionName[c];
		wasUpper = isUpper;
	}
	return result;
}
#define SETTING_NAME(p_Type, p_Name, p_DefaultValue) const std::string UserSettings::p_Name::name = se::variableNameToDisplay(#p_Name, true);
FOR_EACH_USER_SETTING(SETTING_NAME)
#undef SETTING_NAME

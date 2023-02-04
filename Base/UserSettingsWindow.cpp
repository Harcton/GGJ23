#include "stdafx.h"
#include "Base/UserSettingsWindow.h"

#include "Base/UserSettings.h"
#include "SpehsEngine/Graphics/Window.h"
#include "SpehsEngine/Graphics/Renderer.h"
#include "SpehsEngine/ImGUi/Utility/ImGuiUtility.h"
#include "SpehsEngine/ImGui/Utility/BackendWrapper.h"


const uint32_t currentVersion = 0;

UserSettingsWindow::UserSettingsWindow(DemoContext& _demoContext)
	: context(_demoContext)
{
	context.imguiBackend.connectToPreRenderSignal(scopedConnections.add(),
		[this]()
		{
			update();
		});
}

void UserSettingsWindow::update()
{
	if (!windowOpen)
	{
		return;
	}

	if (displayModes.empty())
	{
		const int displayIndex = context.mainWindow.getDisplayIndex();
		std::vector<se::graphics::DisplayMode> modes = context.renderer.getDisplayModes(displayIndex);
		for (const se::graphics::DisplayMode& mode : modes)
		{
			bool add = true;
			for (const DisplayMode& displayMode : displayModes)
			{
				if (mode.width == displayMode.width && mode.height == displayMode.height)
				{
					add = false;
					break;
				}
			}
			if (add)
			{
				if (mode.width == context.userSettings.getResolution().x && mode.height == context.userSettings.getResolution().y)
				{
					displayModeIndex = displayModes.size();
				}
				displayModes.push_back(DisplayMode());
				displayModes.back().width = mode.width;
				displayModes.back().height = mode.height;
				displayModes.back().string = se::formatString("[%i, %i]", mode.width, mode.height);
			}
		}
	}

	if (ImGui::BeginCentered("User settings", &windowOpen))
	{
		static std::vector<DisplayMode>* localDisplayModes = nullptr;
		localDisplayModes = &displayModes;
		struct DisplayModeToString
		{
			static bool toString(void* data, int n, const char** out_str)
			{
				if (localDisplayModes && n < localDisplayModes->size())
				{
					*out_str = (*localDisplayModes)[n].string.c_str();
					return true;
				}
				else
				{
					return false;
				}
			}
		};

		int currentDisplayModeIndex = int(displayModeIndex);
		if (ImGui::Combo("Screen resolution", &currentDisplayModeIndex, &DisplayModeToString::toString, nullptr, int(displayModes.size())))
		{
			displayModeIndex = size_t(currentDisplayModeIndex);
			const DisplayMode& displayMode = displayModes[displayModeIndex];
			context.userSettings.setResolution(glm::ivec2(displayMode.width, displayMode.height));
		}
#define RENDER(p_Type, p_Name, p_DefaultValue) \
		{ \
			p_Type value = context.userSettings.get##p_Name(); \
			if (ImGui::InputT(context.userSettings.get##p_Name##OptionName(), value)) \
			{ \
				context.userSettings.set##p_Name(value); \
			} \
		}
		FOR_EACH_USER_SETTING(RENDER)
#undef RENDER
	}
	ImGui::End();
}
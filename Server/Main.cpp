#include "stdafx.h"

#include "SpehsEngine/Audio/AudioLib.h"
#include "SpehsEngine/Core/CoreLib.h"
#include "SpehsEngine/Math/MathLib.h"
#include "SpehsEngine/Net/NetLib.h"
#include "SpehsEngine/Input/InputLib.h"
#include "SpehsEngine/Physics/PhysicsLib.h"
#include "SpehsEngine/GUI/GUILib.h"
#include "SpehsEngine/Debug/DebugLib.h"
#include "Base/DemoContext.h"


int main()
{
	se::CoreLib core;
	se::NetLib net(core);
	se::MathLib math(core);
	se::PhysicsLib physics(math);
	se::AudioLib audio;
	se::InputLib input;
	se::GUILib gui;
	se::debug::DebugLib debug(gui);

	DemoContext demoContext("Server");

	demoContext.mainWindow.setBorderless(false);
	demoContext.mainWindow.setWidth(1280);
	demoContext.mainWindow.setHeight(720);
	demoContext.mainWindow.setCenteredX();
	demoContext.mainWindow.setCenteredY();
	demoContext.mainWindow.show();

	const se::time::Time minFrameTime = se::time::fromSeconds(1.0f / float(60.0f));
	while (true)
	{
		SE_SCOPE_PROFILER("Frame");
		const se::time::ScopedFrameLimiter frameLimiter(minFrameTime);

		if (!demoContext.update())
		{
			break;
		}

		if (ImGui::Begin("Server"))
		{
		}
		ImGui::End();

		demoContext.render();
	}

	return 0;
}

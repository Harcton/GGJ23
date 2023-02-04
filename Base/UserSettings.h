#pragma once

#define FOR_EACH_USER_SETTING(p_What) \
/*p_What(p_Type, p_Name, p_Default)*/ \
p_What(glm::ivec2, Resolution, glm::ivec2(1280, 720)) \
p_What(bool, SkipLobby, false) \
p_What(bool, EnableDebugOverviewWindow, false) \
p_What(uint8_t, DefaultTargetClientCount, 2) \


class UserSettings
{
#define SETTING(p_Type, p_Name, p_DefaultValue) \
public: \
	const p_Type& get##p_Name() const { return setting##p_Name.value; } \
	const std::string& get##p_Name##OptionName() const { return setting##p_Name.name; } \
	const p_Type get##p_Name##Default() const { return p_DefaultValue; } \
	void set##p_Name(const p_Type& newValue) \
	{ \
		if (!(setting##p_Name.value == newValue)) \
		{ \
			changedSinceSave = true; \
			const p_Type oldValue(std::move(setting##p_Name.value)); \
			setting##p_Name.value = newValue; \
			setting##p_Name.changedSignal(oldValue, newValue); \
		} \
	} \
	void connectTo##p_Name##ChangedSignal(boost::signals2::scoped_connection& scopedConnection, const boost::function<void(const p_Type&, const p_Type&)>& callback, const bool callImmediately = false) \
	{ \
		scopedConnection = setting##p_Name.changedSignal.connect(callback); \
		if (callImmediately) \
		{ \
			callback(setting##p_Name.value, setting##p_Name.value); \
		} \
	} \
private: \
	struct p_Name \
	{ \
		boost::signals2::signal<void(const p_Type&, const p_Type&)> changedSignal; \
		p_Type value = p_DefaultValue; \
		static const std::string name; \
	} setting##p_Name;
	FOR_EACH_USER_SETTING(SETTING)
#undef SETTING

public:

	UserSettings(const std::string_view& _windowName);

	bool write();
	bool read();

	void update();

	bool hasChangesSinceLastSave() const { return changedSinceSave; }

	void signalAllSettingsAsChanged()
	{
#define SETTING_SIGNAL(p_Type, p_Name, p_DefaultValue) setting##p_Name.changedSignal(setting##p_Name.value, setting##p_Name.value);
		FOR_EACH_USER_SETTING(SETTING_SIGNAL)
#undef SETTING_SIGNAL
	}

private:

	const std::string filepath;
	bool changedSinceSave = false;
	bool windowOpen = true;
};

#pragma once
#include "State.h"
#include "Settings.h"

namespace NightLightLibrary
{
	class NightLight
	{
	public:
		NightLight();
		~NightLight() noexcept;
		static const bool isSupported(const bool checkEnabled = false);

		const bool didStatusChange() const noexcept;

		NightLight& disable() noexcept;
		NightLight& enable() noexcept;
		const bool isEnabled() const noexcept;

		NightLight& pause() noexcept;
		NightLight& resume();
		const bool isRunning() const;

		const bool isUsable() const noexcept;
		NightLight& disableSystemUI() noexcept;

		NightLight& useSunSchedule() noexcept;
		NightLight& useManualSchedule() noexcept;
		const bool isOnSunSchedule() const noexcept;

		NightLight& setStartTime(const Time& t);
		NightLight& setEndTime(const Time& t);
		Time getStartTime() const noexcept;
		Time getEndTime() const noexcept;
		const bool isWithinTimeRange() const;

		const int16_t getColorTemperature() const;
		const int16_t getDayColorTemperature() const noexcept;
		const int16_t getNightColorTemperature() const noexcept;
		NightLight& setNightColorTemperature(const int16_t ct);
		// attempts to emulate color temperature transition
		const int16_t getSmoothenedColorTemperature() const;
		const ULONGLONG getSmootheningDuration() const noexcept;

		const bool isAdjustingColorTemperature() const noexcept;
		const bool wasAdjustingColorTemperature() const noexcept;

		NightLight& save(const bool dontTrigger = true);
		NightLight& load(const bool ignoreStatusChange = false);
		NightLight& backup();
		NightLight& restore();

		NightLight& startWatching(const std::function<void(NightLight&)>& callback = [](NightLight&) noexcept {});
		NightLight& stopWatching() noexcept;
		NightLight& pauseWatching() noexcept;
		NightLight& resumeWatching() noexcept;

	private:		
		Settings	_settings;
		State		_state;
		Settings	_backupSettings;
		State		_backupState;

		bool        _statusChanged{ false };
		ULONGLONG	_lastStatusChangeTime{ 0 };
		bool		_settingsChanged{ false };
		bool		_adjustingColorTemperatureChanged{ false };

		NightLight& _loadSettings(const bool ignoreStatusChange = false);
		NightLight& _loadState(const bool ignoreStatusChange = false);
	}; // class NightLight
} // namespace NightLightLibrary

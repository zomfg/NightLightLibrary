#pragma once
#include "State.h"
#include "Settings.h"

namespace NightLightLibrary
{
	class NightLight
	{
	public:
		/*
		static NightLight& instance() {
			static NightLight instance;
			return instance;
		};
		*/
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
		const bool wasManuallyTriggered() const noexcept;

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

		// attempts to emulate color temperature transition
		const int16_t getSmoothenedColorTemperature() const;
		const int16_t getColorTemperature() const;
		const int16_t getDayColorTemperature() const noexcept;
		const int16_t getNightColorTemperature() const noexcept;
		NightLight& setNightColorTemperature(const int16_t ct);

		NightLight& save();
		NightLight& load(const bool ignoreStatusChange = false);
		NightLight& backup();
		NightLight& restore();

		NightLight& startWatching(const std::function<void(NightLight&)>& callback = [](NightLight&) noexcept {});
		NightLight& stopWatching() noexcept;

	private:		
		Settings	_settings;
		State		_state;
		Settings	_backupSettings;
		State		_backupState;

		bool        _statusChanged{ false };
		ULONGLONG	_lastStatusChangeTime{ 0 };

		std::unique_ptr<Registry::Watcher> _watcher;
	}; // class NightLight
} // namespace NightLightLibrary

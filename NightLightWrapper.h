#pragma once
#include <functional>
namespace NightLightLibrary
{
	class NightLight;

	class NightLightWrapper
	{
	public:
		NightLightWrapper();
		~NightLightWrapper();

		static const bool isSupported(const bool checkEnabled = false);

		const bool didStatusChange() const noexcept;

		NightLightWrapper& disable() noexcept;
		NightLightWrapper& enable() noexcept;
		const bool isEnabled() const noexcept;

		NightLightWrapper& pause() noexcept;
		NightLightWrapper& resume();
		const bool isRunning() const;

		const bool isUsable() const noexcept;
		NightLightWrapper& disableSystemUI() noexcept;

		NightLightWrapper& useSunSchedule() noexcept;
		NightLightWrapper& useManualSchedule() noexcept;
		const bool isOnSunSchedule() const noexcept;

		NightLightWrapper& setStartTime(const int8_t hours, const int8_t minutes);
		NightLightWrapper& setEndTime(const int8_t hours, const int8_t minutes);
		NightLightWrapper& getStartTime(int8_t& hours, int8_t& minutes) noexcept;
		NightLightWrapper& getEndTime(int8_t& hours, int8_t& minutes) noexcept;
		const bool isWithinTimeRange() const;

		const int16_t getColorTemperature() const;
		const int16_t getDayColorTemperature() const noexcept;
		const int16_t getNightColorTemperature() const noexcept;
		NightLightWrapper& setNightColorTemperature(const int16_t ct);
		// attempts to emulate color temperature transition
		const int16_t getSmoothenedColorTemperature() const;

		const bool isAdjustingColorTemperature() const noexcept;
		const bool wasAdjustingColorTemperature() const noexcept;

		NightLightWrapper& save(const bool dontTrigger = true);
		NightLightWrapper& load(const bool ignoreStatusChange = false);
		NightLightWrapper& backup();
		NightLightWrapper& restore();

		NightLightWrapper& startWatching(const std::function<void(NightLightWrapper&)>& callback = [](NightLightWrapper&) noexcept {});
		NightLightWrapper& stopWatching() noexcept;
		NightLightWrapper& pauseWatching() noexcept;
		NightLightWrapper& resumeWatching() noexcept;
	private:
		std::unique_ptr<NightLight> _nl;
	}; // class NightLightWrapper
} // namespace NightLightLibrary


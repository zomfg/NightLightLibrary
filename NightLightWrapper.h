#pragma once

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
		const bool wasManuallyTriggered() const noexcept;

		const bool isUsable() const noexcept;
		NightLightWrapper& disableSystemUI() noexcept;

		NightLightWrapper& useSunSchedule() noexcept;
		NightLightWrapper& useManualSchedule() noexcept;
		const bool isOnSunSchedule() const noexcept;

		const bool isWithinTimeRange() const;

		// attempts to emulate color temperature transition
		const int16_t getSmoothenedColorTemperature() const;
		const int16_t getColorTemperature() const;
		const int16_t getDayColorTemperature() const noexcept;
		const int16_t getNightColorTemperature() const noexcept;
		NightLightWrapper& setNightColorTemperature(const int16_t ct);

		NightLightWrapper& save();
		NightLightWrapper& load(const bool ignoreStatusChange = false);
		NightLightWrapper& backup();
		NightLightWrapper& restore();

		NightLightWrapper& startWatching(const std::function<void(NightLightWrapper&)>& callback = [](NightLightWrapper&) noexcept {});
		NightLightWrapper& stopWatching() noexcept;

	private:
		std::unique_ptr<NightLight> _nl;
	}; // class NightLightWrapper
} // namespace NightLightLibrary


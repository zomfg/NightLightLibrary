#pragma once
#include "nightlight_schema_types.h"
#include "Registry.h"

namespace NightLightLibrary
{
	struct Time : public _Time, public Registry::Bond<Time>
	{
		static Time now();
		static const bool isWithinRange(const Time& start, Time end, Time t) noexcept;

		const bool isWithinRange(const Time& start, const Time& end) const noexcept;

		Time& operator=(const Time& newtime);

		Time& setHours(const int8_t h);
		Time& setMinutes(const int8_t m);

		const uint16_t toMinutes() const noexcept;

		Time& _reset() noexcept override;
	}; // struct Time

	struct Settings : public _Settings<Time>, public Registry::Record<Settings>, public Registry::Bond<Settings>
	{
		static const LPCSTR getRegistryKey();

		Time getStartTime() const noexcept;
		Settings& setStartTime(const Time& time);
		Time getEndTime() const noexcept;
		Settings& setEndTime(const Time& time);

		const bool isEnabled() const noexcept;
		Settings& setEnabled(const bool enabled) noexcept;

		const bool isOnSunSchedule() const noexcept;
		Settings& setOnSunSchedule(const bool onSunSchedule) noexcept;

		const int16_t getDayColorTemperature() const noexcept;
		const int16_t getNightColorTemperature() const noexcept;
		Settings& setNightColorTemperature(const int16_t ctemp);
		const bool isPreviewing() const noexcept;

		//static const bool load(Settings&);
		Settings& save() override;

		Settings& _reset() noexcept override;
	}; // struct Settings
}; // namespace NightLightLibrary
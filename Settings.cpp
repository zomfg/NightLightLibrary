#include "stdafx.h"
#include "Settings.h"

namespace NightLightLibrary
{
	const LPCSTR Settings::getRegistryKey() {
		static const std::string key = Schema::metadata.attributes.find(Registry::Name::SubkeyAttribute)->second;
		return key.c_str();
	}
	Time Settings::getStartTime() const noexcept
	{
		return (isOnSunSchedule() ? sunScheduleStartTime : manualScheduleStartTime);
	}

	Time Settings::getEndTime() const noexcept
	{
		return (isOnSunSchedule() ? sunScheduleEndTime : manualScheduleEndTime);
	}

	Settings& Settings::setStartTime(const Time& time)
	{
		manualScheduleStartTime = time;
		_dirty = true;
		return *this;
	}

	Settings& Settings::setEndTime(const Time& time)
	{
		manualScheduleEndTime = time;
		_dirty = true;
		return *this;
	}

	const bool Settings::isEnabled() const noexcept
	{
		return enabled;
	}

	Settings& Settings::setEnabled(const bool e) noexcept
	{
		enabled = e;
		_dirty = true;
		return *this;
	}

	const bool Settings::isOnSunSchedule() const noexcept
	{
		return onSunSchedule;
	}

	Settings& Settings::setOnSunSchedule(const bool s) noexcept
	{
		onSunSchedule = s;
		_dirty = true;
		return *this;
	}

	const int16_t Settings::getNightColorTemperature() const noexcept
	{
		return colorTemperature;
	}

	const int16_t Settings::getDayColorTemperature() const noexcept
	{
		return static_cast<int16_t>(Schema::var::colorTemperature::metadata.default_value.int_value);
	}

	Settings& Settings::setNightColorTemperature(const int16_t ctemp)
	{
		static const int16_t minTemp = ::boost::lexical_cast<int16_t>(Schema::var::colorTemperature::metadata.attributes.find("Min")->second);
		static const int16_t maxTemp = ::boost::lexical_cast<int16_t>(Schema::var::colorTemperature::metadata.attributes.find("Max")->second);
		colorTemperature = std::min(std::max(ctemp, minTemp), maxTemp);
		_dirty = true;
		return *this;
	}

	Settings& Settings::save()
	{
		Record::save(*this);
		return *this;
	}
	
	Settings& Settings::_reset() noexcept
	{
		enabled = (Schema::var::enabled::metadata.default_value.uint_value == 1);
		
		onSunSchedule = (Schema::var::onSunSchedule::metadata.default_value.uint_value == 1);
		
		colorTemperature = static_cast<int16_t>(Schema::var::colorTemperature::metadata.default_value.int_value);
		
		manualScheduleStartTime._reset();
		manualScheduleEndTime._reset();

		sunScheduleStartTime._reset();
		sunScheduleEndTime._reset();
		_dirty = true;
		return *this;
	}




	Time& Time::setHours(const int8_t h)
	{
		static const int8_t minH = static_cast<int8_t>(::boost::lexical_cast<int16_t>(Schema::var::hours::metadata.attributes.find("Min")->second));
		static const int8_t maxH = static_cast<int8_t>(::boost::lexical_cast<int16_t>(Schema::var::hours::metadata.attributes.find("Max")->second));
		hours = std::min(std::max(h, minH), maxH);
		return *this;
	}

	Time& Time::setMinutes(const int8_t m)
	{
		static const int8_t minM = static_cast<int8_t>(::boost::lexical_cast<int16_t>(Schema::var::minutes::metadata.attributes.find("Min")->second));
		static const int8_t maxM = static_cast<int8_t>(::boost::lexical_cast<int16_t>(Schema::var::minutes::metadata.attributes.find("Max")->second));
		minutes = std::min(std::max(m, minM), maxM);
		return *this;
	}

	const uint16_t Time::toMinutes() const noexcept
	{
		return (hours * 60 + minutes);
	}

	Time& Time::operator=(const Time& newtime)
	{
		if (this != &newtime)
			setHours(newtime.hours).setMinutes(newtime.minutes);
		return *this;
	}

	Time& Time::_reset() noexcept
	{
		hours = static_cast<int8_t>(Schema::var::hours::metadata.default_value.int_value);
		minutes = static_cast<int8_t>(Schema::var::minutes::metadata.default_value.int_value);
		return *this;
	}

	Time Time::now()
	{
		SYSTEMTIME now;
		GetLocalTime(&now);
		Time tnow;
		tnow.setHours(static_cast<int8_t>(now.wHour)).setMinutes(static_cast<int8_t>(now.wMinute));
		return tnow;
	}

	const bool Time::isWithinRange(const Time& start, Time end, Time t) noexcept
	{
		if (start.hours > end.hours || (start.hours == end.hours && start.minutes > end.minutes))
			end.hours += 24;
		if (start.hours > t.hours || (start.hours == t.hours && start.minutes > t.hours))
			t.hours += 24;
		const uint16_t tm = t.toMinutes();
		return (tm >= start.toMinutes() && tm <= end.toMinutes());
	}

	const bool Time::isWithinRange(const Time& start, const Time& end) const noexcept
	{
		return isWithinRange(start, end, *this);
	}
}
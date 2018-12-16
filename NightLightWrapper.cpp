#include "stdafx.h"
#include "NightLightWrapper.h"
#include "NightLight.h"

namespace NightLightLibrary
{
#define NL_NONCHAINABLE_WRAPPER(_return_, _name_, _decoration_)\
_return_ NightLightWrapper::_name_() _decoration_ {\
	return _nl->_name_(); \
}

#define NL_CHAINABLE_WRAPPER(_name_, _arg_type_,_arg_, _decoration_)\
NightLightWrapper& NightLightWrapper::_name_(_arg_type_ _arg_) _decoration_ {\
	_nl->_name_(_arg_); \
	return *this; \
}

#define NL_SET_TIME_WRAPPER(_name_)\
NightLightWrapper& NightLightWrapper::set##_name_##Time(const int8_t hours, const int8_t minutes){\
	Time t; \
	t.setHours(hours); \
	t.setMinutes(minutes); \
	_nl->set##_name_##Time(t); \
	return *this; \
}

#define NL_GET_TIME_WRAPPER(_name_)\
NightLightWrapper& NightLightWrapper::get##_name_##Time(int8_t& hours, int8_t& minutes) noexcept {\
	Time t = _nl->get##_name_##Time(); \
	hours = t.hours; \
	minutes = t.minutes; \
	return *this; \
}

	NightLightWrapper::NightLightWrapper() : _nl(std::make_unique<NightLight>()) {}
	NightLightWrapper::~NightLightWrapper() = default;

	const bool NightLightWrapper::isSupported(const bool checkEnabled)
	{
		return NightLight::isSupported(checkEnabled);
	}

	NL_NONCHAINABLE_WRAPPER(const bool, didStatusChange, const noexcept);

	NL_CHAINABLE_WRAPPER(disable,,, noexcept);
	NL_CHAINABLE_WRAPPER(enable,,, noexcept);

	NL_NONCHAINABLE_WRAPPER(const bool, isEnabled, const noexcept);

	NL_CHAINABLE_WRAPPER(pause,,, noexcept);
	NL_CHAINABLE_WRAPPER(resume,,, );

	NL_NONCHAINABLE_WRAPPER(const bool, isRunning, const);
	NL_NONCHAINABLE_WRAPPER(const bool, isUsable, const noexcept);

	NL_CHAINABLE_WRAPPER(disableSystemUI,,, noexcept);
	NL_CHAINABLE_WRAPPER(useSunSchedule,,, noexcept);
	NL_CHAINABLE_WRAPPER(useManualSchedule,,, noexcept);

	NL_NONCHAINABLE_WRAPPER(const bool, isOnSunSchedule, const noexcept);

	NL_SET_TIME_WRAPPER(Start);
	NL_SET_TIME_WRAPPER(End);

	NL_GET_TIME_WRAPPER(Start);
	NL_GET_TIME_WRAPPER(End);

	NL_NONCHAINABLE_WRAPPER(const bool, isWithinTimeRange, const);
	NL_NONCHAINABLE_WRAPPER(const int16_t, getSmoothenedColorTemperature, const);
	NL_NONCHAINABLE_WRAPPER(const int16_t, getColorTemperature, const);
	NL_NONCHAINABLE_WRAPPER(const int16_t, getDayColorTemperature, const noexcept);
	NL_NONCHAINABLE_WRAPPER(const int16_t, getNightColorTemperature, const noexcept);
	NL_NONCHAINABLE_WRAPPER(const bool, isAdjustingColorTemperature, const noexcept);
	NL_NONCHAINABLE_WRAPPER(const bool, wasAdjustingColorTemperature, const noexcept);

	NL_CHAINABLE_WRAPPER(setNightColorTemperature, const int16_t, ct, );

	NL_CHAINABLE_WRAPPER(save, const bool, dontTrigger, );
	NL_CHAINABLE_WRAPPER(load, const bool, ignoreStatusChange, );

	NL_CHAINABLE_WRAPPER(backup,,, );
	NL_CHAINABLE_WRAPPER(restore,,, );

	NightLightWrapper& NightLightWrapper::startWatching(const std::function<void(NightLightWrapper&)>& callback)
	{
		_nl->startWatching([&, callback](NightLight&) { callback(*this); });
		return *this;
	}
	NL_CHAINABLE_WRAPPER(stopWatching,,, noexcept);
	NL_CHAINABLE_WRAPPER(pauseWatching,,, noexcept);
	NL_CHAINABLE_WRAPPER(resumeWatching,,, noexcept);
}
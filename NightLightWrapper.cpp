#include "stdafx.h"
#include "NightLight.h"
#include "NightLightWrapper.h"

namespace NightLightLibrary
{

	NightLightWrapper::NightLightWrapper() : _nl(std::make_unique<NightLight>())
	{
	}

	NightLightWrapper::~NightLightWrapper()
	{
	}

	const bool NightLightWrapper::isSupported(const bool checkEnabled)
	{
		return NightLight::isSupported(checkEnabled);
	}

	const bool NightLightWrapper::didStatusChange() const noexcept
	{
		return _nl->didStatusChange();
	}

	NightLightWrapper& NightLightWrapper::disable() noexcept
	{
		_nl->disable();
		return *this;
	}

	NightLightWrapper& NightLightWrapper::enable() noexcept
	{
		_nl->enable();
		return *this;
	}

	const bool NightLightWrapper::isEnabled() const noexcept
	{
		return _nl->isEnabled();
	}

	NightLightWrapper& NightLightWrapper::pause() noexcept
	{
		_nl->pause();
		return *this;
	}

	NightLightWrapper& NightLightWrapper::resume()
	{
		_nl->resume();
		return *this;
	}

	const bool NightLightWrapper::isRunning() const
	{
		return _nl->isRunning();
	}

	const bool NightLightWrapper::wasManuallyTriggered() const noexcept
	{
		return _nl->wasManuallyTriggered();
	}

	const bool NightLightWrapper::isUsable() const noexcept
	{
		return _nl->isUsable();
	}

	NightLightWrapper& NightLightWrapper::disableSystemUI() noexcept
	{
		_nl->disableSystemUI();
		return *this;
	}

	NightLightWrapper& NightLightWrapper::useSunSchedule() noexcept
	{
		_nl->useSunSchedule();
		return *this;
	}

	NightLightWrapper& NightLightWrapper::useManualSchedule() noexcept
	{
		_nl->useManualSchedule();
		return *this;
	}

	const bool NightLightWrapper::isOnSunSchedule() const noexcept
	{
		return _nl->isOnSunSchedule();
	}

	const bool NightLightWrapper::isWithinTimeRange() const
	{
		return _nl->isWithinTimeRange();
	}

	const int16_t NightLightWrapper::getSmoothenedColorTemperature() const
	{
		return _nl->getSmoothenedColorTemperature();
	}

	const int16_t NightLightWrapper::getColorTemperature() const
	{
		return _nl->getColorTemperature();
	}

	const int16_t NightLightWrapper::getDayColorTemperature() const noexcept
	{
		return _nl->getDayColorTemperature();
	}

	const int16_t NightLightWrapper::getNightColorTemperature() const noexcept
	{
		return _nl->getNightColorTemperature();
	}

	NightLightWrapper& NightLightWrapper::setNightColorTemperature(const int16_t ct)
	{
		_nl->setNightColorTemperature(ct);
		return *this;
	}

	NightLightWrapper& NightLightWrapper::save()
	{
		_nl->save();
		return *this;
	}

	NightLightWrapper& NightLightWrapper::load(const bool ignoreStatusChange)
	{
		_nl->load(ignoreStatusChange);
		return *this;
	}

	NightLightWrapper& NightLightWrapper::backup()
	{
		_nl->backup();
		return *this;
	}

	NightLightWrapper& NightLightWrapper::restore()
	{
		_nl->restore();
		return *this;
	}

	NightLightWrapper& NightLightWrapper::startWatching(const std::function<void(NightLightWrapper&)>& callback)
	{
		_nl->startWatching([&, callback](NightLight&) { callback(*this); });
		return *this;
	}

	NightLightWrapper& NightLightWrapper::stopWatching() noexcept
	{
		_nl->stopWatching();
		return *this;
	}

}
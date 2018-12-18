#include "stdafx.h"
#include "NightLight.h"

namespace NightLightLibrary
{
	enum class SmootheningDuration : ULONGLONG // in ms
	{
		Long = 120'000, // for auto switching on/off
		Short = 2'000,  // for manual switching
		None = 0 // for while moving ct slider
	};



#pragma region NightLight

	NightLight::NightLight()
	{
		backup();
		load(true);
	}
	NightLight::~NightLight() noexcept
	{
		stopWatching();
	}

	const bool NightLight::isSupported(const bool checkEnabled)
	{
		Settings settings;
		State state;
		const bool supported = Settings::load(settings) && State::load(state);
		if (!supported)
			return false;
		if (checkEnabled)
			return (settings.isEnabled() || state.isRunning() || settings.isAdjustingColorTemperature()/*unsure*/);
		return true;
	}

	const bool NightLight::didStatusChange() const noexcept
	{
		return _statusChanged;
	}

	NightLight& NightLight::resume()
	{
		_state.resume();
		return *this;
	}

	NightLight& NightLight::pause() noexcept
	{
		_state.pause();
		return *this;
	}

	const bool NightLight::isRunning() const
	{
		return _state.isRunning();
	}

	const bool NightLight::isUsable() const noexcept
	{
		return _state.isUsable();
	}

	NightLight& NightLight::useSunSchedule() noexcept
	{
		_settings.setOnSunSchedule(true);
		return *this;
	}

	NightLight& NightLight::useManualSchedule() noexcept
	{
		_settings.setOnSunSchedule(false);
		return *this;
	}

	const bool NightLight::isOnSunSchedule() const noexcept
	{
		return _settings.isOnSunSchedule();
	}

	NightLight& NightLight::enable() noexcept
	{
		_settings.setEnabled(true);
		// replicating behavior: not scheduled => scheduled + within range = running
		//if (isWithinTimeRange())
			//resume();
		return *this;
	}

	NightLight& NightLight::disable() noexcept
	{
		_settings.setEnabled(false);
		return *this;
	}

	const bool NightLight::isEnabled() const noexcept
	{
		return _settings.isEnabled();
	}

	NightLight& NightLight::disableSystemUI() noexcept
	{
		_state.setUsable(false);
		return *this;
	}

	NightLight& NightLight::setStartTime(const Time& t)
	{
		_settings.setStartTime(t);
		useManualSchedule();
		return *this;
	}

	NightLight& NightLight::setEndTime(const Time& t)
	{
		_settings.setEndTime(t);
		useManualSchedule();
		return *this;
	}

	Time NightLight::getStartTime() const noexcept
	{
		return _settings.getStartTime();
	}

	Time NightLight::getEndTime() const noexcept
	{
		return _settings.getEndTime();
	}

	const bool NightLight::isWithinTimeRange() const
	{
		return Time::now().isWithinRange(getStartTime(), getEndTime());
	}

	const int16_t NightLight::getColorTemperature() const
	{
		return isRunning() || isAdjustingColorTemperature() ? getNightColorTemperature() : getDayColorTemperature();
	}

	const int16_t NightLight::getDayColorTemperature() const noexcept
	{
		return _settings.getDayColorTemperature();
	}

	const int16_t NightLight::getNightColorTemperature() const noexcept
	{
		return _settings.getNightColorTemperature();
	}

	NightLight& NightLight::setNightColorTemperature(const int16_t ct)
	{
		_settings.setNightColorTemperature(ct);
		return *this;
	}

	const ULONGLONG NightLight::getSmootheningDuration() const noexcept
	{
		if (_statusChanged && (_settingsChanged || _state.wasManuallyTriggered()))
			return static_cast<ULONGLONG>(SmootheningDuration::Short);
		if (_statusChanged && _state.wasManuallyTriggered() == false)
			return static_cast<ULONGLONG>(SmootheningDuration::Long);
		return static_cast<ULONGLONG>(SmootheningDuration::None);
	}

	const int16_t NightLight::getSmoothenedColorTemperature() const
	{
		const ULONGLONG duration = getSmootheningDuration();
		if (duration == 0)
			return getColorTemperature();
		const ULONGLONG timeSinceStatusChange = ::GetTickCount64() - _lastStatusChangeTime; // ms
		if (timeSinceStatusChange >= duration)
			return getColorTemperature();

		const int16_t from = !isRunning() ? getNightColorTemperature() : getDayColorTemperature();
		const int16_t to = isRunning() ? getNightColorTemperature() : getDayColorTemperature();
		return static_cast<int16_t>(from + (to - from) * (timeSinceStatusChange / static_cast<double>(duration)));
	}

	const bool NightLight::isAdjustingColorTemperature() const noexcept
	{
		return _settings.isAdjustingColorTemperature();
	}

	const bool NightLight::wasAdjustingColorTemperature() const noexcept
	{
		return _adjustingColorTemperatureChanged;
	}

	NightLight& NightLight::save(const bool dontTrigger)
	{
		if (dontTrigger)
			pauseWatching();
		_settings.save();
		_state.save(/*isEnabled() && isWithinTimeRange()*/); // TODO: double check logic
		if (dontTrigger)
			resumeWatching();
		return *this;
	}

	NightLight& NightLight::load(const bool ignoreStatusChange)
	{
		_loadSettings(ignoreStatusChange);
		_loadState(ignoreStatusChange);
		return *this;
	}

	NightLight& NightLight::_loadState(const bool ignoreStatusChange)
	{
		_statusChanged = false;
		const bool previousStatus = isRunning();
		if (State::load(_state) && ignoreStatusChange == false) {
			_statusChanged = (previousStatus != isRunning());
			if (_statusChanged) {
				_lastStatusChangeTime = ::GetTickCount64();

				_adjustingColorTemperatureChanged = false;

				//if (_state.wasManuallyTriggered() == false)
					//_settingsChanged = false;
			}
		}
		return *this;
	}

	NightLight& NightLight::_loadSettings(const bool ignoreStatusChange)
	{
		_settingsChanged = false;
		const bool previousAdjusting = isAdjustingColorTemperature();
		Settings previous;
		previous.swap(_settings);
		if (Settings::load(_settings) && ignoreStatusChange == false) {
			_settingsChanged = (previous != _settings);

			if (_settingsChanged)
				_statusChanged = false;
		}
		_adjustingColorTemperatureChanged = (previousAdjusting != isAdjustingColorTemperature() && _settingsChanged);
		return *this;
	}

	NightLight& NightLight::backup()
	{
		// tag backups as dirty so the restore() will save() only after a successful load()
		_backupSettings._dirty = Settings::load(_backupSettings);
		_backupState._dirty = State::load(_backupState);
		return *this;
	}

	NightLight& NightLight::restore()
	{
		_backupSettings.save();
		_backupState.save();
		return load();
	}

	NightLight& NightLight::startWatching(const std::function<void(NightLight&)>& callback)
	{
		_state.startWatching([&, callback]() {
			_loadState();
			callback(*this);
		});
		_settings.startWatching([&, callback]() {
			_loadSettings();
			callback(*this);
		});
		return *this;
	}

	NightLight& NightLight::stopWatching() noexcept
	{
		_state.stopWatching();
		_settings.stopWatching();
		return *this;
	}

	NightLight& NightLight::pauseWatching() noexcept
	{
		_state.pauseWatching();
		_settings.pauseWatching();
		return *this;
	}

	NightLight& NightLight::resumeWatching() noexcept
	{
		_state.resumeWatching();
		_settings.resumeWatching();
		return *this;
	}

#pragma endregion NightLight

} // namespace NightLightLibrary

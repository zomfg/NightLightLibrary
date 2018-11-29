#include "stdafx.h"
#include "NightLight.h"

namespace NightLightLibrary
{
	enum class SmootheningDuration : ULONGLONG // in ms
	{
		Long = 120'000, // for auto switching on/off
		Short = 2'000  // for manual switching
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
			return (settings.isEnabled() || state.isRunning());
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

	const bool NightLight::wasManuallyTriggered() const noexcept
	{
		return _state.wasManuallyTriggered();
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
		return isRunning() ? getNightColorTemperature() : getDayColorTemperature();
	}

	const int16_t NightLight::getSmoothenedColorTemperature() const
	{
		const ULONGLONG duration = static_cast<ULONGLONG>(wasManuallyTriggered() ? SmootheningDuration::Short : SmootheningDuration::Long);
		const ULONGLONG timeSinceStatusChange = ::GetTickCount64() - _lastStatusChangeTime; // ms
		if (timeSinceStatusChange >= duration)
			return getColorTemperature();

		const int16_t from = !isRunning() ? getNightColorTemperature() : getDayColorTemperature();
		const int16_t to = isRunning() ? getNightColorTemperature() : getDayColorTemperature();
		return static_cast<int16_t>(from + (to - from) * (timeSinceStatusChange / static_cast<double>(duration)));
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

	NightLight& NightLight::save()
	{
		_settings.save();
		_state.save(/*isEnabled() && isWithinTimeRange()*/); // TODO: double check logic
		return *this;
	}

	NightLight& NightLight::load(const bool ignoreStatusChange)
	{
		const bool previousStatus = isRunning();
		Settings::load(_settings);
		State::load(_state);
		if (ignoreStatusChange == false) {
			_statusChanged = (previousStatus != isRunning());
			if (_statusChanged)
				_lastStatusChangeTime = ::GetTickCount64();
		}
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
		if (_watcher)
			_watcher->stop();
		else
			_watcher = std::make_unique<Registry::Watcher>();

		const std::vector<LPCSTR> subKeys{
			_state.getRegistryKey(),
			_settings.getRegistryKey()
		};
		const std::function<void()> wrapper = [&, callback]() {
			load(); // refresh data on change
			callback(*this);
		};
		_watcher->start(subKeys, wrapper);
		return *this;
	}

	NightLight& NightLight::stopWatching() noexcept
	{
		_watcher.reset();
		return *this;
	}

#pragma endregion NightLight

} // namespace NightLightLibrary
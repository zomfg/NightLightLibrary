#include "stdafx.h"
#include "NightLightWrapper.h"
#include "State.h"
#include "Settings.h"

namespace NightLightLibrary
{
	enum class SmootheningDuration : ULONGLONG // in ms
	{
		Long = 120'000, // for auto switching on/off
		Short = 2'000,  // for manual switching
		None = 0 // for while moving ct slider
	};

	// maximum gap in ms between settings change and state change
	// during which the status change will be considered as a manual change
	constexpr ULONGLONG SettingsEnducedStatusChangePeriod = 100; // ms

#pragma region NightLight
	class NightLightWrapper::NightLight
	{
	public:
		NightLight()
		{
			backup();
			load(true);
		}
		~NightLight() noexcept
		{
			stopWatching();
		}

		static const bool isSupported(const bool checkEnabled = false)
		{
			Settings settings;
			State state;
			const bool supported = Settings::load(settings) && State::load(state);
			if (!supported)
				return false;
			if (checkEnabled)
				return (settings.isEnabled() || state.isRunning() || settings.isPreviewing()/*unsure*/);
			return true;
		}

		const bool didStatusChange() const noexcept
		{
			return _statusChanged;
		}

		NightLight& resume()
		{
			_state.resume();
			return *this;
		}

		NightLight& pause() noexcept
		{
			_state.pause();
			return *this;
		}

		const bool isRunning() const
		{
			return _state.isRunning();
		}

		const bool isUsable() const noexcept
		{
			return _state.isUsable();
		}

		NightLight& useSunSchedule() noexcept
		{
			_settings.setOnSunSchedule(true);
			return *this;
		}

		NightLight& useManualSchedule() noexcept
		{
			_settings.setOnSunSchedule(false);
			return *this;
		}

		const bool isOnSunSchedule() const noexcept
		{
			return _settings.isOnSunSchedule();
		}

		NightLight& enable() noexcept
		{
			_settings.setEnabled(true);
			// replicating behavior: not scheduled => scheduled + within range = running
			//if (isWithinTimeRange())
				//resume();
			return *this;
		}

		NightLight& disable() noexcept
		{
			_settings.setEnabled(false);
			return *this;
		}

		const bool isEnabled() const noexcept
		{
			return _settings.isEnabled();
		}

		NightLight& disableSystemUI() noexcept
		{
			_state.setUsable(false);
			return *this;
		}

		NightLight& setStartTime(const Time& t)
		{
			_settings.setStartTime(t);
			useManualSchedule();
			return *this;
		}

		NightLight& setEndTime(const Time& t)
		{
			_settings.setEndTime(t);
			useManualSchedule();
			return *this;
		}

		Time getStartTime() const noexcept
		{
			return _settings.getStartTime();
		}

		Time getEndTime() const noexcept
		{
			return _settings.getEndTime();
		}

		const bool isWithinTimeRange() const
		{
			return Time::now().isWithinRange(getStartTime(), getEndTime());
		}

		const int16_t getColorTemperature() const
		{
			return isRunning() || isPreviewing() ? getNightColorTemperature() : getDayColorTemperature();
		}

		const int16_t getDayColorTemperature() const noexcept
		{
			return _settings.getDayColorTemperature();
		}

		const int16_t getNightColorTemperature() const noexcept
		{
			return _settings.getNightColorTemperature();
		}

		NightLight& setNightColorTemperature(const int16_t ct)
		{
			_settings.setNightColorTemperature(ct);
			return *this;
		}

		const ULONGLONG getSmootheningDuration() const noexcept
		{
			if (_statusChanged && (_settingsChanged || _state.wasManuallyTriggered()))
				return static_cast<ULONGLONG>(SmootheningDuration::Short);
			if (_statusChanged && _state.wasManuallyTriggered() == false)
				return static_cast<ULONGLONG>(SmootheningDuration::Long);
			return static_cast<ULONGLONG>(SmootheningDuration::None);
		}

		const int16_t getSmoothenedColorTemperature() const
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

		const bool isPreviewing() const noexcept
		{
			return _settings.isPreviewing();
		}

		const bool wasPreviewing() const noexcept
		{
			return _previewingChanged;
		}

		NightLight& save(const bool dontTrigger = true)
		{
			if (dontTrigger)
				pauseWatching();
			_settings.save();
			_state.save(/*isEnabled() && isWithinTimeRange()*/); // TODO: double check logic
			if (dontTrigger)
				resumeWatching();
			return *this;
		}

		NightLight& load(const bool ignoreStatusChange = false)
		{
			_loadSettings(ignoreStatusChange);
			_loadState(ignoreStatusChange);
			return *this;
		}

		NightLight& backup()
		{
			// tag backups as dirty so the restore() will save() only after a successful load()
			_backupSettings._dirty = Settings::load(_backupSettings);
			_backupState._dirty = State::load(_backupState);
			return *this;
		}

		NightLight& restore()
		{
			_backupSettings.save();
			_backupState.save();
			return load();
		}

		NightLight& startWatching(const std::function<void(NightLight&)>& callback = [](NightLight&) noexcept {})
		{
			_state.startWatching([&, callback]() {
				_loadState();
#ifdef _DEBUG
				// delay state callback to allow settings callback to print to console without mixing both outputs
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
#endif // _DEBUG
				callback(*this);
				});
			_settings.startWatching([&, callback]() {
				_loadSettings();
				callback(*this);
				});
			return *this;
		}

		NightLight& stopWatching() noexcept
		{
			_state.stopWatching();
			_settings.stopWatching();
			return *this;
		}

		NightLight& pauseWatching() noexcept
		{
			_state.pauseWatching();
			_settings.pauseWatching();
			return *this;
		}

		NightLight& resumeWatching() noexcept
		{
			_state.resumeWatching();
			_settings.resumeWatching();
			return *this;
		}

	private:
		Settings	_settings;
		State		_state;
		Settings	_backupSettings;
		State		_backupState;

		std::atomic<bool>        _statusChanged{ false };
		std::atomic<ULONGLONG>	_lastStatusChangeTime{ 0 };

		std::atomic<bool>		_settingsChanged{ false };
		std::atomic<ULONGLONG>	_lastSettingsChangeTime{ 0 };

		std::atomic<bool>		_previewingChanged{ false };

		NightLight& _loadState(const bool ignoreStatusChange = false)
		{
			_statusChanged = false;
			const bool previousStatus = isRunning();
			if (State::load(_state) && ignoreStatusChange == false) {
				_statusChanged = (previousStatus != isRunning());
				if (_statusChanged) {
					_lastStatusChangeTime = ::GetTickCount64();

					_previewingChanged = false;

					// when settings were changed far enough in the past
					// it means status change was not caused by direct settings change
					// and settings flag should be reset
					if (_lastStatusChangeTime > _lastSettingsChangeTime + SettingsEnducedStatusChangePeriod)
						_settingsChanged = false;
				}
			}
			return *this;
		}

		NightLight& _loadSettings(const bool ignoreStatusChange = false)
		{
			const bool previouPreviewing = isPreviewing();
			Settings previous;
			previous.swap(_settings);
			if (Settings::load(_settings) && ignoreStatusChange == false) {
				ULONGLONG now = ::GetTickCount64();
				if (now > _lastSettingsChangeTime + SettingsEnducedStatusChangePeriod)
					_settingsChanged = (previous != _settings);

				if (_settingsChanged) {
					_lastSettingsChangeTime = now;
					_statusChanged = false;
				}
			}
			_previewingChanged = (previouPreviewing != isPreviewing() && _settingsChanged);
			return *this;
		}
	}; // class NightLightWrapper::NightLight

#pragma endregion NightLight


#pragma region NightLightWrapper

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
	NL_NONCHAINABLE_WRAPPER(const bool, isPreviewing, const noexcept);
	NL_NONCHAINABLE_WRAPPER(const bool, wasPreviewing, const noexcept);

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

#pragma endregion NightLightWrapper
} // namespace NightLightLibrary
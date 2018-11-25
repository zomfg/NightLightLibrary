#include "stdafx.h"
#include "State.h"

namespace NightLightLibrary
{
	const LPCSTR State::getRegistryKey() { 
		static const std::string key = Schema::metadata.attributes.find(Registry::Name::SubkeyAttribute)->second;
		return key.c_str();
	}

	const bool State::wasManuallyTriggered() const noexcept
	{
		return trigger == TriggerType::Manual;
	}

	State& State::resume()
	{
		status = Status::Running;
		_dirty = true;
		return *this;
	}

	State& State::pause() noexcept
	{
		status.set_nothing();
		_dirty = true;
		return *this;
	}

	const bool State::isRunning() const
	{
		return status == Status::Running && isUsable();
	}

	const bool State::isUsable() const noexcept
	{
		return usable;
	}

	State& State::setUsable(const bool u) noexcept
	{
		usable = u;
		_dirty = true;
		return *this;
	}

	State& State::_reset()
	{
		if (Schema::var::status::metadata.default_value.nothing)
			status.set_nothing();
		else
			status = static_cast<Status>(Schema::var::status::metadata.default_value.int_value);

		trigger = static_cast<TriggerType>(Schema::var::trigger::metadata.default_value.int_value);

		changedOn = Schema::var::changedOn::metadata.default_value.uint_value;
		usable = (Schema::var::usable::metadata.default_value.uint_value == 1);
		_dirty = true;
		return *this;
	}

	State& State::save(/*const bool starsAligned*/)
	{
		if (_dirty == false)
			return *this;
		GetSystemTimeAsFileTime((LPFILETIME)&changedOn);
		//if (isRunning() || (starsAligned && isUsable()))
		trigger = isUsable() ? TriggerType::Manual : TriggerType::Automatic;
		Record::save(*this);
		return *this;
	}
}
#pragma once
#include "nightlight_schema_types.h"
#include "Registry.h"

namespace NightLightLibrary {
	struct State : public _State, public Registry::Record<State>, public Registry::Bond<State>
	{
		static const LPCSTR getRegistryKey();
		
		const bool wasManuallyTriggered() const noexcept;
		
		const bool isRunning() const;
		State& pause() noexcept;
		State& resume();
		
		State& setUsable(const bool usable) noexcept;
		const bool isUsable() const noexcept;

		State& save(/*const bool starsAligned*/) override;

		State& _reset() override;
	};
}

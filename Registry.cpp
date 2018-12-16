#include "stdafx.h"
#include "Registry.h"

namespace NightLightLibrary
{
	namespace Registry
	{
		class Exception : public std::exception
		{
		public:
			Exception(const char* const m = "") noexcept : message(m) {}
			const char* what() const noexcept override { return message; }
		private:
			const char* message;
		};


#pragma region Watcher

		Watcher::~Watcher()
		{
			stop();
		}

		const bool Watcher::isWatching() const noexcept
		{
			return _watching;
		}

		void Watcher::setWatching(const bool w) noexcept
		{
			_watching = w;
		}

		void Watcher::start(const std::vector<LPCSTR>& subKeys, const std::function<void(LPCSTR)>& callback)
		{
			if (isWatching() || subKeys.size() < 1)
				return;
			setWatching(true);
			std::thread(&Watcher::watchLoop, this, subKeys, callback).detach();
		}

		const bool Watcher::isPaused() const noexcept
		{
			return _paused;
		}

		void Watcher::setPaused(const bool p) noexcept
		{
			_paused = p;
		}

		void Watcher::pause() noexcept
		{
			setPaused(true);
		}

		void Watcher::resume() noexcept
		{
			setPaused(false);
		}

		void Watcher::stop() noexcept
		{
			if (!isWatching())
				return;
			setWatching(false);
			_lastBreathEvent = ::CreateEventA(NULL, TRUE, FALSE, NULL);
			if (_cancelEvent != NULL)
				::SetEvent(_cancelEvent);
			::WaitForSingleObject(_lastBreathEvent, INFINITE);
			::CloseHandle(_lastBreathEvent);
			_lastBreathEvent = NULL;
		}

		const int Watcher::watch(std::vector<HANDLE>& events, std::vector<HKEY>& keys)
		{
			// https://docs.microsoft.com/en-us/windows/desktop/sync/waiting-for-multiple-objects
			// https://docs.microsoft.com/en-us/windows/desktop/api/winreg/nf-winreg-regnotifychangekeyvalue
			int changedKeyIndex = -1;
			for (size_t idx = 0; idx < keys.size(); idx++) {
				constexpr DWORD dwFilter = REG_NOTIFY_CHANGE_LAST_SET | // reports even when value unchanged
					//REG_NOTIFY_CHANGE_NAME |
					//REG_NOTIFY_CHANGE_ATTRIBUTES |
					//REG_NOTIFY_CHANGE_SECURITY |
					REG_NOTIFY_THREAD_AGNOSTIC;
				// Watch the registry key for a change of value.
				LSTATUS lErrorCode = ::RegNotifyChangeKeyValue(
					keys[idx],
					TRUE,
					dwFilter,
					events[idx],
					TRUE); // async
				if (lErrorCode != ERROR_SUCCESS)
					throw Exception("Error in RegNotifyChangeKeyValue");
			}

			const DWORD triggeredEventIdx = ::WaitForMultipleObjects(
				static_cast<DWORD>(events.size()),  // number of objects in array
				events.data(),      // array of objects
				FALSE,       // wait for any object
				INFINITE);   // wait forever
			const size_t cancelEventIdx = WAIT_OBJECT_0 + events.size() - 1;
			if (triggeredEventIdx >= WAIT_OBJECT_0 && triggeredEventIdx < cancelEventIdx) { // one of the regkeys was changed
				changedKeyIndex = triggeredEventIdx - WAIT_OBJECT_0;
#ifdef _DEBUG
				std::cout << "Key changed : [" << changedKeyIndex << "]" << std::endl;
#endif // _DEBUG
				// delay events reset and callback() to buffer triggers in quick successions
				//std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
			else if (triggeredEventIdx == cancelEventIdx) // cancel event triggered
				setWatching(false); // break the loop
			else
				throw Exception("Wait event error");

			for (HANDLE& event : events)
				::ResetEvent(event);
			return changedKeyIndex;
		}

		void Watcher::watchLoop(const std::vector<LPCSTR>& subKeys, const std::function<void(LPCSTR)>& callback)
		{
			std::vector<HANDLE> events;
			std::vector<HKEY>   keys;
			try
			{
				for (const LPCSTR& subKey : subKeys) {
					HKEY key;
					LSTATUS lErrorCode = ::RegOpenKeyExA(HKEY_CURRENT_USER, subKey, 0, KEY_NOTIFY, &key);
					if (lErrorCode != ERROR_SUCCESS)
						throw Exception("Error in RegOpenKeyExA");
					keys.push_back(key);

					HANDLE event = ::CreateEventA(NULL, TRUE, FALSE, NULL);
					if (event == NULL)
						throw Exception("Error in CreateEvent");
					events.push_back(event);
				}
				_cancelEvent = ::CreateEventA(NULL, TRUE, FALSE, NULL);
				if (_cancelEvent == NULL)
					throw Exception("Error in CreateEvent");
				events.push_back(_cancelEvent); // make cancel event last in the list

				while (isWatching()) {
					const int subkeyIndex = watch(events, keys);
					if (!isPaused() && subkeyIndex > -1 && subkeyIndex < subKeys.size())
						callback(subKeys[subkeyIndex]);
				}
			}
			catch (const Exception& e)
			{
#ifdef _DEBUG
				std::cout << e.what() << " (" << ::GetLastError() << ")." << std::endl;
#else // _DEBUG
				UNREFERENCED_PARAMETER(e);
#endif // _DEBUG
			}
			for (HKEY& key : keys)
				::CloseHandle(key);

			for (HANDLE& event : events) // close all events, including "cancel"
				::CloseHandle(event);
			_cancelEvent = NULL;

			if (_lastBreathEvent != NULL)
				::SetEvent(_lastBreathEvent);
		}

#pragma endregion Watcher

	} // namespace Registry
} // namespace NightLightLibrary
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

		void Watcher::start(const std::vector<LPCSTR>& subKeys, const std::function<void()>& callback)
		{
			if (isWatching() || subKeys.size() < 1)
				return;
			setWatching(true);
			std::thread(&Watcher::watchLoop, this, subKeys, callback).detach();
		}

		void Watcher::stop() noexcept
		{
			if (!isWatching())
				return;
			setWatching(false);
			_lastBreathEvent = ::CreateEventA(NULL, TRUE, FALSE, "Watcher::lastBreathEvent");
			if (_cancelEvent != NULL)
				::SetEvent(_cancelEvent);
			::WaitForSingleObject(_lastBreathEvent, INFINITE);
			::CloseHandle(_lastBreathEvent);
			_lastBreathEvent = NULL;
		}

		const bool Watcher::watch(std::vector<HANDLE>& events, std::vector<HKEY>& keys)
		{
			// https://docs.microsoft.com/en-us/windows/desktop/sync/waiting-for-multiple-objects
			// https://docs.microsoft.com/en-us/windows/desktop/api/winreg/nf-winreg-regnotifychangekeyvalue
			bool registryChanged = false;
			for (size_t idx = 0; idx < keys.size(); idx++) {
				constexpr DWORD dwFilter = REG_NOTIFY_CHANGE_NAME |
					REG_NOTIFY_CHANGE_ATTRIBUTES |
					REG_NOTIFY_CHANGE_LAST_SET |
					REG_NOTIFY_CHANGE_SECURITY;
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
			if (triggeredEventIdx >= WAIT_OBJECT_0 && triggeredEventIdx < cancelEventIdx) // one of the regkeys was changed
				registryChanged = true;
			else if (triggeredEventIdx == cancelEventIdx) // cancel event triggered
				setWatching(false); // break the loop
			else
				throw Exception("Wait event error");

			for (HANDLE& event : events)
				::ResetEvent(event);
			return registryChanged;
		}

		void Watcher::watchLoop(const std::vector<LPCSTR>& subKeys, const std::function<void()>& callback)
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

					HANDLE event = ::CreateEventA(NULL, TRUE, FALSE, "Watcher::subkey");
					if (event == NULL)
						throw Exception("Error in CreateEvent");
					events.push_back(event);
				}
				_cancelEvent = ::CreateEventA(NULL, TRUE, FALSE, "Watcher::cancelEvent");
				if (_cancelEvent == NULL)
					throw Exception("Error in CreateEvent");
				events.push_back(_cancelEvent); // make cancel event last in the list

				while (isWatching())
					if (watch(events, keys))
						callback();
			}
			catch (const Exception& e)
			{
#ifdef _DEBUG
				std::cout << e.what() << " (" << ::GetLastError() << ")." << std::endl;
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
	}
}
#pragma once
#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#include <Windows.h>
#undef VC_EXTRALEAN
#else
#include <Windows.h>
#endif // VC_EXTRALEAN
#include <bond/core/bond.h>
#include <bond/stream/input_buffer.h>
#ifdef _DEBUG
#include <iomanip>
#endif

namespace NightLightLibrary
{
	namespace Registry
	{
		class Watcher
		{
		public:
			Watcher() {};
			~Watcher();
			Watcher(const Watcher&) = delete;
			Watcher& operator=(const Watcher&) = delete;
			const bool isWatching() const noexcept;
			void start(const std::vector<LPCSTR>& subKeys, const std::function<void(LPCSTR)>& callback);
			void stop() noexcept;
			const bool isPaused() const noexcept;
			void pause() noexcept;
			void resume() noexcept;
		private:
			std::atomic<HANDLE> _cancelEvent{ NULL };
			std::atomic<HANDLE> _lastBreathEvent{ NULL };
			std::atomic<bool>   _watching{ false };
			std::atomic<bool>   _paused{ false };

			void setWatching(const bool watching) noexcept;
			void setPaused(const bool paused) noexcept;
			void watchLoop(const std::vector<LPCSTR>& subKeys, const std::function<void(LPCSTR)>& callback);
			const int watch(std::vector<HANDLE>& events, std::vector<HKEY>& keys);
		}; // class Watcher

		struct Header
		{
			uint32_t	h1{ 0 };
			FILETIME	filetime{ 0, 0 };
			uint32_t	h2{ 0 };
		}; // struct Header

		// bond data
		struct Metadata
		{
			int16_t     protocol{ ::bond::ProtocolType::COMPACT_PROTOCOL };
			int16_t     version{ ::bond::v1 };
		}; // struct Metadata

		template<typename T> struct Bond
		{
			// needs to be implemented for re-using objects with bond
			// should reset bond schema properties to default values
			virtual T& _reset() = 0;
		protected:
			~Bond() {};
		}; // struct Bond

		namespace Name
		{
			constexpr LPCSTR SubkeyAttribute = "RegistrySubkey";
			constexpr LPCSTR Value = "Data";
		} // namespace Name

		template<typename T> struct Record
		{
			constexpr static const LPCSTR	getRegistryValueName() noexcept { return Registry::Name::Value; };
			constexpr static const LPCSTR	getRegistryKey() { assert(false);  return ""; };
			Header			_header;
			Metadata		_metadata;
			bool            _dirty{ false };

			static const bool load(T& obj) {
				if (!Registry::load(obj))
					return false;
				obj._dirty = false;
				return true;
			}
			static const bool save(T& obj) { 
				if (obj._dirty)
					obj._dirty = Registry::save(obj);
				return !obj._dirty;
			}
			virtual T& save() = 0;

			void startWatching(const std::function<void()>& callback = []() noexcept {})
			{
				if (_watcher)
					_watcher->stop();
				else
					_watcher = std::make_unique<Watcher>();

				const std::vector<LPCSTR> subKeys{ T::getRegistryKey() };
				const std::function<void(LPCSTR)> wrapper = [&, callback](LPCSTR) {
					callback();
				};
				_watcher->start(subKeys, wrapper);
			}

			void stopWatching() noexcept	{ _watcher.reset(); }
			void pauseWatching() noexcept	{ if (_watcher) _watcher->pause(); }
			void resumeWatching() noexcept	{ if (_watcher) _watcher->resume(); }

		protected:
			~Record() {};
		private:
			std::unique_ptr<Watcher> _watcher;
		}; // struct Record


#ifdef _DEBUG
		inline void printData(const uint8_t* data, uint32_t size)
		{
			const uint8_t* const end = data + size;
			std::cout << "REG DATA:" << std::hex;
			while (data < end)
				std::cout << " " << std::setfill('0') << std::setw(2) << (int)*(data++);
			std::cout << std::dec << " END" << std::endl;
		}
#endif

		inline const DWORD getValueSize(const LPCSTR& regSubkey, const LPCSTR& regValueName) noexcept
		{
			DWORD dataSize = 0;
			DWORD type = REG_BINARY;
			const LSTATUS s = ::RegGetValueA(
				HKEY_CURRENT_USER,
				regSubkey,
				regValueName,
				RRF_RT_REG_BINARY,
				&type,
				nullptr,
				&dataSize
			);
			if (s == ERROR_SUCCESS)
				return dataSize;
			return 0;
		} // getValueSize()

		template<typename T> inline void unmarshal(const ::bond::InputBuffer& buffer, T& obj)
		{
			// Unmarshal reads protocol version information from input stream and uses
			// appropriate protocol reader to deserialize data.
			bond::Unmarshal(buffer, obj);
		} // unmarshal()

		template<typename T> const bool load(T& obj)
		{
			static_assert(std::is_base_of<Record<T>, T>::value, "must be a Registry::Record");
			DWORD dataSize = getValueSize(T::getRegistryKey(), T::getRegistryValueName());

			if (dataSize == 0 || dataSize < sizeof(obj._header) + sizeof(obj._metadata))
				return false;

			std::vector<uint8_t> data(dataSize);

			DWORD types = REG_BINARY;
			const LSTATUS s = ::RegGetValueA(
				HKEY_CURRENT_USER,
				T::getRegistryKey(),
				T::getRegistryValueName(),
				RRF_RT_REG_BINARY,
				&types,
				data.data(),
				&dataSize
			);
			if (s != ERROR_SUCCESS)
				return false;
#ifdef _DEBUG
			printData((uint8_t*)(data.data()), dataSize);
#endif
			try
			{
				::bond::InputBuffer input = ::bond::InputBuffer(&data[0], dataSize);
				// copy header for saving back to registry
				input.Read(&(obj._header), sizeof(obj._header));
				
				// copy metadata "manually" because InputBuffer cannot rewind in c++
				memcpy(&(obj._metadata), &data[sizeof(obj._header)], sizeof(obj._metadata));
				unmarshal(input, obj._reset());
			}
			catch (const std::exception& e)
			{
#ifdef _DEBUG
				std::cout << "bond read fail: " << e.what() << std::endl;
#else // _DEBUG
				UNREFERENCED_PARAMETER(e);
#endif // _DEBUG
				return false;
			}
			return true;
		} // load()

		template <typename T> const bool save(T& obj)
		{
			static_assert(std::is_base_of<Record<T>, T>::value, "must be a Registry::Record");

			::bond::OutputBuffer output;

			// restore the original header with updated time
			GetSystemTimeAsFileTime(&(obj._header.filetime));

			try
			{
				output.Write(obj._header);

				switch (obj._metadata.protocol)
				{
#ifdef BOND_COMPACT_BINARY_PROTOCOL
				case ::bond::ProtocolType::COMPACT_PROTOCOL:
				{
					::bond::CompactBinaryWriter<::bond::OutputBuffer> writer(output, obj._metadata.version);
					::bond::Marshal(obj, writer);
				}
				break;
#endif
#ifdef BOND_FAST_BINARY_PROTOCOL
				case ::bond::ProtocolType::FAST_PROTOCOL:
				{
					::bond::FastBinaryWriter<::bond::OutputBuffer> writer(output);
					::bond::Marshal(obj, writer);
				}
				break;
#endif
#ifdef BOND_SIMPLE_BINARY_PROTOCOL
				case ::bond::ProtocolType::SIMPLE_PROTOCOL:
				{
					::bond::SimpleBinaryWriter<::bond::OutputBuffer> writer(output, obj._metadata.version);
					::bond::Marshal(obj, writer);
				}
				break;
#endif
				default:
					return false;
				}
			}
			catch (const std::exception& e)
			{
#ifdef _DEBUG
				std::cout << "bond write fail: " << e.what() << std::endl;
#else // _DEBUG
				UNREFERENCED_PARAMETER(e);
#endif // _DEBUG
				return false;
			}
			
			const LSTATUS s = ::RegSetKeyValueA(
				HKEY_CURRENT_USER,
				T::getRegistryKey(),
				T::getRegistryValueName(),
				REG_BINARY,
				output.GetBuffer().data(),
				output.GetBuffer().size()
			);

#ifdef _DEBUG
			printData((uint8_t*)(output.GetBuffer().data()), output.GetBuffer().size());
#endif // _DEBUG

			return (s == ERROR_SUCCESS);
		} // save()
	} // namespace Registry
}; // namespace NightLightLibrary

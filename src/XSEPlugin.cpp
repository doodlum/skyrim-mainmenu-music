#include <nlohmann/json.hpp>
using json = nlohmann::json;

std::string gMainMenuTheme;

struct Hooks
{
	struct MainMenu_MusicCopyBuffer
	{
		static INT64 thunk(char* Destination, rsize_t SizeInBytes, char*, char* a4)
		{
			json          JSONSettings;
			std::ifstream i(L"Data\\SKSE\\Plugins\\RandomMainMenuMusic.json");
			i >> JSONSettings;
			std::vector<std::string> filenames = JSONSettings["filenames"];
			srand((unsigned int)time(NULL));
			gMainMenuTheme = filenames.at(rand() % filenames.size());
			if (gMainMenuTheme.length() > 264)
				logger::critical("filename {} was too large!", gMainMenuTheme);
			return func(Destination, SizeInBytes, const_cast<char*>(gMainMenuTheme.c_str()), a4);
		}
		static inline REL::Relocation<decltype(thunk)> func;
	};

	static void Install()
	{
		stl::write_thunk_call<MainMenu_MusicCopyBuffer>(REL::RelocationID(51328, 52185).address() + REL::Relocate(0xAE, 0x141));
	}
};


void Init()
{
	Hooks::Install();
}

void InitializeLog()
{
#ifndef NDEBUG
	auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
	auto path = logger::log_directory();
	if (!path) {
		util::report_and_fail("Failed to find standard logging directory"sv);
	}

	*path /= fmt::format("{}.log"sv, Plugin::NAME);
	auto       sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

#ifndef NDEBUG
	const auto level = spdlog::level::trace;
#else
	const auto level = spdlog::level::info;
#endif

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));
	log->set_level(level);
	log->flush_on(level);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%l] %v"s);
}

EXTERN_C [[maybe_unused]] __declspec(dllexport) bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
#ifndef NDEBUG
	while (!IsDebuggerPresent()) {};
#endif

	InitializeLog();

	logger::info("Loaded plugin");

	SKSE::Init(a_skse);

	Init();

	return true;
}

EXTERN_C [[maybe_unused]] __declspec(dllexport) constinit auto SKSEPlugin_Version = []() noexcept {
	SKSE::PluginVersionData v;
	v.PluginName("PluginName");
	v.PluginVersion({ 1, 1, 0, 0 });
	v.UsesAddressLibrary(true);
	v.HasNoStructUse(true);
	return v;
}();

EXTERN_C [[maybe_unused]] __declspec(dllexport) bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface*, SKSE::PluginInfo* pluginInfo)
{
	pluginInfo->name = SKSEPlugin_Version.pluginName;
	pluginInfo->infoVersion = SKSE::PluginInfo::kVersion;
	pluginInfo->version = SKSEPlugin_Version.pluginVersion;
	return true;
}

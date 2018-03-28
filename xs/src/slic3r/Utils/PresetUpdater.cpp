#include "PresetUpdater.hpp"

#include <iostream>  // XXX
#include <thread>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>

#include <wx/app.h>
#include <wx/event.h>

#include "libslic3r/Utils.hpp"
#include "slic3r/GUI/GUI.hpp"
// #include "slic3r/GUI/AppConfig.hpp"
#include "slic3r/GUI/PresetBundle.hpp"
#include "slic3r/Utils/Http.hpp"

namespace fs = boost::filesystem;


namespace Slic3r {


// TODO: proper URL
static const std::string SLIC3R_VERSION_URL = "https://gist.githubusercontent.com/vojtechkral/4d8fd4a3b8699a01ec892c264178461c/raw/e9187c3e15ceaf1a90f29b7c43cf3ccc746140f0/slic3rPE.version";
enum {
	SLIC3R_VERSION_BODY_MAX = 256,
};

struct PresetUpdater::priv
{
	// AppConfig *app_config;
	// PresetBundle *bundle;
	int version_online_event;
	fs::path cache_path;
	bool cancel;
	std::thread thread;

	priv(int event);

	// void get_slic3r_version(int event) const;
	void download(const std::set<VendorProfile> vendors) const;
};

PresetUpdater::priv::priv(int event) :
	version_online_event(event),
	cache_path(fs::path(Slic3r::data_dir()) / "cache"),
	cancel(false)
{}

// void PresetUpdater::priv::get_slic3r_version(int event) const
// {
// 	// TODO
// }

void PresetUpdater::priv::download(const std::set<VendorProfile> vendors) const
{
	std::cerr << "PresetUpdater::priv::download()" << std::endl;

	// Download current Slic3r version
	Http::get(SLIC3R_VERSION_URL)
		.size_limit(SLIC3R_VERSION_BODY_MAX)
		.on_progress([this](Http::Progress, bool &cancel) {
			cancel = this->cancel;
		})
		.on_complete([&](std::string body, unsigned http_status) {
			boost::trim(body);
			std::cerr << "Got version: " << http_status << ", body: \"" << body << '"' << std::endl;
			wxCommandEvent* evt = new wxCommandEvent(version_online_event);
			evt->SetString(body);
			GUI::get_app()->QueueEvent(evt);
		})
		.perform_sync();

	// Donwload vendor preset bundles
	// std::cerr << "Bundle vendors: " << bundle->vendors.size() << std::endl;
	std::cerr << "Bundle vendors: " << vendors.size() << std::endl;
	// for (const auto &vendor : bundle->vendors) {
	for (const auto &vendor : vendors) {
		std::cerr << "vendor: " << vendor.name << std::endl;
		std::cerr << "  URL: " << vendor.config_update_url << std::endl;

		if (cancel) { return; }

		// TODO: Proper caching

		auto target_path = cache_path / vendor.id;
		target_path += ".ini";
		std::cerr << "target_path: " << target_path << std::endl;

		Http::get(vendor.config_update_url)
			.on_progress([this](Http::Progress, bool &cancel) {
				cancel = this->cancel;
			})
			.on_complete([&](std::string body, unsigned http_status) {
				std::cerr << "Got ini: " << http_status << ", body: " << body.size() << std::endl;
				fs::fstream file(target_path, std::ios::out | std::ios::binary | std::ios::trunc);
				file.write(body.c_str(), body.size());
			})
			.perform_sync();
	}
}

// PresetUpdater::PresetUpdater(AppConfig *app_config, PresetBundle *preset_bundle) :
PresetUpdater::PresetUpdater(int version_online_event) :
	p(new priv(version_online_event))
{
	// p->app_config = app_config;
	// p->bundle = preset_bundle;
}


// Public

PresetUpdater::~PresetUpdater()
{
	if (p && p->thread.joinable()) {
		p->cancel = true;
		p->thread.join();
	}
}

// void PresetUpdater::download(PresetBundle *preset_bundle)
// {
// 	std::cerr << "PresetUpdater::download()" << std::endl;

// 	auto self = std::make_shared<PresetUpdater>(preset_bundle);
// 	auto thread = std::thread([self](){
// 		self->p->download();
// 	});
// 	self->p->thread = std::move(thread);
// }

// void PresetUpdater::get_slic3r_version(int event)
// {
// 	std::cerr << "PresetUpdater::get_slic3r_version()" << std::endl;

// 	p->thread = std::move(std::thread([this, event]() {
// 		this->p->get_slic3r_version(event);
// 	}));
// }

void PresetUpdater::download(PresetBundle *preset_bundle)
{
	std::cerr << "PresetUpdater::download()" << std::endl;

	// Copy the whole vendors data for use in the background thread
	// Unfortunatelly as of C++11, it needs to be copied again
	// into the closure (but perhaps the compiler can elide this).
	std::set<VendorProfile> vendors = preset_bundle->vendors;

	p->thread = std::move(std::thread([this, vendors]() {
		this->p->download(std::move(vendors));
	}));
}


// TODO: remove
namespace Utils {

void preset_update_check()
{
	std::cerr << "preset_update_check()" << std::endl;

	// TODO:
	// 1. Get a version tag or the whole bundle from the web
	// 2. Store into temporary location (?)
	// 3. ???
	// 4. Profit!
}

}

}

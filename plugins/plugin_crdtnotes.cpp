#include <solanaceae/plugin/solana_plugin_v1.h>

#include <solanaceae/crdtnotes/crdtnotes.hpp>
#include <solanaceae/crdtnotes/crdtnotes_sync.hpp>

#include <entt/entt.hpp>
#include <entt/fwd.hpp>

#include <memory>
#include <iostream>

static std::unique_ptr<CRDTNotes> g_crdtn = nullptr;
static std::unique_ptr<CRDTNotesSync> g_crdtns = nullptr;

constexpr const char* plugin_name = "CRDTNotes";

extern "C" {

SOLANA_PLUGIN_EXPORT const char* solana_plugin_get_name(void) {
	return plugin_name;
}

SOLANA_PLUGIN_EXPORT uint32_t solana_plugin_get_version(void) {
	return SOLANA_PLUGIN_VERSION;
}

SOLANA_PLUGIN_EXPORT uint32_t solana_plugin_start(struct SolanaAPI* solana_api) {
	std::cout << "PLUGIN " << plugin_name << " START()\n";

	if (solana_api == nullptr) {
		return 1;
	}

	try {
		auto* cs = PLUG_RESOLVE_INSTANCE(ContactStore4I);

		// static store, could be anywhere tho
		// construct with fetched dependencies
		g_crdtn = std::make_unique<CRDTNotes>();
		g_crdtns = std::make_unique<CRDTNotesSync>(*g_crdtn, *cs);

		// register types
		PLUG_PROVIDE_INSTANCE(CRDTNotesSync, plugin_name, g_crdtns.get());
		PLUG_PROVIDE_INSTANCE(CRDTNotesEventI, plugin_name, g_crdtns.get());
	} catch (const ResolveException& e) {
		std::cerr << "PLUGIN " << plugin_name << " " << e.what << "\n";
		return 2;
	}

	return 0;
}

SOLANA_PLUGIN_EXPORT void solana_plugin_stop(void) {
	std::cout << "PLUGIN " << plugin_name << " STOP()\n";

	g_crdtn.reset();
	g_crdtns.reset();
}

SOLANA_PLUGIN_EXPORT float solana_plugin_tick(float delta) {
	return g_crdtns->iterate(delta);
}

} // extern C


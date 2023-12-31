#include <solanaceae/plugin/solana_plugin_v1.h>

#include <solanaceae/crdtnotes/crdtnotes.hpp>
#include <solanaceae/crdtnotes/crdtnotes_sync.hpp>
//#include <solanaceae/util/config_model.hpp>

#include <memory>
#include <iostream>

#define RESOLVE_INSTANCE(x) static_cast<x*>(solana_api->resolveInstance(#x))
#define PROVIDE_INSTANCE(x, p, v) solana_api->provideInstance(#x, p, static_cast<x*>(v))

static std::unique_ptr<CRDTNotes> g_crdtn = nullptr;
static std::unique_ptr<CRDTNotesSync> g_crdtns = nullptr;

extern "C" {

SOLANA_PLUGIN_EXPORT const char* solana_plugin_get_name(void) {
	return "CRDTNotes";
}

SOLANA_PLUGIN_EXPORT uint32_t solana_plugin_get_version(void) {
	return SOLANA_PLUGIN_VERSION;
}

SOLANA_PLUGIN_EXPORT uint32_t solana_plugin_start(struct SolanaAPI* solana_api) {
	std::cout << "PLUGIN CRDTN START()\n";

	if (solana_api == nullptr) {
		return 1;
	}

	//ConfigModelI* conf = nullptr;
	Contact3Registry* cr = nullptr;

	{ // make sure required types are loaded
		//conf = RESOLVE_INSTANCE(ConfigModelI);
		cr = RESOLVE_INSTANCE(Contact3Registry);

		//if (conf == nullptr) {
			//std::cerr << "PLUGIN CRDTN missing ConfigModelI\n";
			//return 2;
		//}

		if (cr == nullptr) {
			std::cerr << "PLUGIN CRDTNTS missing Contact3Registry\n";
			return 2;
		}
	}

	// static store, could be anywhere tho
	// construct with fetched dependencies
	g_crdtn = std::make_unique<CRDTNotes>();
	g_crdtns = std::make_unique<CRDTNotesSync>(*g_crdtn, *cr);

	// register types
	PROVIDE_INSTANCE(CRDTNotesSync, "CRDTNotes", g_crdtns.get());
	PROVIDE_INSTANCE(CRDTNotesEventI, "CRDTNotes", g_crdtns.get());

	return 0;
}

SOLANA_PLUGIN_EXPORT void solana_plugin_stop(void) {
	std::cout << "PLUGIN CRDTN STOP()\n";

	g_crdtn.reset();
	g_crdtns.reset();
}

SOLANA_PLUGIN_EXPORT void solana_plugin_tick(float delta) {
	(void)delta;
	//std::cout << "PLUGIN CRDTN TICK()\n";
	g_crdtns->iterate(delta);
}

} // extern C


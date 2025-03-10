#include <solanaceae/plugin/solana_plugin_v1.h>

#include <solanaceae/contact/contact_store_i.hpp>

#include <solanaceae/crdtnotes/crdtnotes.hpp>
#include <solanaceae/crdtnotes/crdtnotes_sync.hpp>
#include <solanaceae/crdtnotes_toxsync/crdtnotes_toxsync.hpp>
#include <solanaceae/toxcore/tox_interface.hpp>
#include <solanaceae/toxcore/tox_event_interface.hpp>
#include <solanaceae/tox_contacts/tox_contact_model2.hpp>

#include <entt/entt.hpp>
#include <entt/fwd.hpp>

#include <memory>
#include <iostream>

static std::unique_ptr<CRDTNotesToxSync> g_crdtn_ts = nullptr;

constexpr const char* plugin_name = "CRDTNotesToxSync";

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
		auto* notes_sync = PLUG_RESOLVE_INSTANCE(CRDTNotesEventI);
		auto* cs = PLUG_RESOLVE_INSTANCE(ContactStore4I);
		auto* t = PLUG_RESOLVE_INSTANCE(ToxI);
		auto* tep = PLUG_RESOLVE_INSTANCE(ToxEventProviderI);
		auto* tcm = PLUG_RESOLVE_INSTANCE(ToxContactModel2);

		// static store, could be anywhere tho
		// construct with fetched dependencies
		g_crdtn_ts = std::make_unique<CRDTNotesToxSync>(*notes_sync, *cs, *t, *tep, *tcm);

		// register types
		PLUG_PROVIDE_INSTANCE(CRDTNotesToxSync, plugin_name, g_crdtn_ts.get());
	} catch (const ResolveException& e) {
		std::cerr << "PLUGIN " << plugin_name << " " << e.what << "\n";
		return 2;
	}

	return 0;
}

SOLANA_PLUGIN_EXPORT void solana_plugin_stop(void) {
	std::cout << "PLUGIN " << plugin_name << " STOP()\n";

	g_crdtn_ts.reset();
}

SOLANA_PLUGIN_EXPORT float solana_plugin_tick(float delta) {
	return g_crdtn_ts->iterate(delta);
}

} // extern C


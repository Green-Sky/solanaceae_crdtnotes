#include <solanaceae/plugin/solana_plugin_v1.h>

#include <solanaceae/crdtnotes/crdtnotes.hpp>
#include <solanaceae/crdtnotes/crdtnotes_sync.hpp>
#include <solanaceae/crdtnotes_toxsync/crdtnotes_toxsync.hpp>
//#include <solanaceae/util/config_model.hpp>
#include <solanaceae/contact/contact_model3.hpp>
#include <solanaceae/toxcore/tox_interface.hpp>
#include <solanaceae/toxcore/tox_event_interface.hpp>
#include <solanaceae/tox_contacts/tox_contact_model2.hpp>

#include <memory>
#include <iostream>

#define RESOLVE_INSTANCE(x) static_cast<x*>(solana_api->resolveInstance(#x))
#define PROVIDE_INSTANCE(x, p, v) solana_api->provideInstance(#x, p, static_cast<x*>(v))

static std::unique_ptr<CRDTNotesToxSync> g_crdtn_ts = nullptr;

extern "C" {

SOLANA_PLUGIN_EXPORT const char* solana_plugin_get_name(void) {
	return "CRDTNotesToxSync";
}

SOLANA_PLUGIN_EXPORT uint32_t solana_plugin_get_version(void) {
	return SOLANA_PLUGIN_VERSION;
}

SOLANA_PLUGIN_EXPORT uint32_t solana_plugin_start(struct SolanaAPI* solana_api) {
	std::cout << "PLUGIN CRDTNTS START()\n";

	if (solana_api == nullptr) {
		return 1;
	}

	//ConfigModelI* conf = nullptr;
	CRDTNotesEventI* notes_sync = nullptr;
	Contact3Registry* cr = nullptr;
	ToxI* t = nullptr;
	ToxEventProviderI* tep = nullptr;
	ToxContactModel2* tcm = nullptr;

	{ // make sure required types are loaded
		//conf = RESOLVE_INSTANCE(ConfigModelI);
		notes_sync = RESOLVE_INSTANCE(CRDTNotesEventI);
		cr = RESOLVE_INSTANCE(Contact3Registry);
		t = RESOLVE_INSTANCE(ToxI);
		tep = RESOLVE_INSTANCE(ToxEventProviderI);
		tcm = RESOLVE_INSTANCE(ToxContactModel2);

		//if (conf == nullptr) {
			//std::cerr << "PLUGIN CRDTN missing ConfigModelI\n";
			//return 2;
		//}

		if (notes_sync == nullptr) {
			std::cerr << "PLUGIN CRDTNTS missing CRDTNotesEventI\n";
			return 2;
		}

		if (cr == nullptr) {
			std::cerr << "PLUGIN CRDTNTS missing Contact3Registry\n";
			return 2;
		}

		if (t == nullptr) {
			std::cerr << "PLUGIN CRDTNTS missing ToxI\n";
			return 2;
		}

		if (tep == nullptr) {
			std::cerr << "PLUGIN CRDTNTS missing ToxEventProviderI\n";
			return 2;
		}

		if (tcm == nullptr) {
			std::cerr << "PLUGIN CRDTNTS missing ToxContactModel2\n";
			return 2;
		}
	}

	// static store, could be anywhere tho
	// construct with fetched dependencies
	g_crdtn_ts = std::make_unique<CRDTNotesToxSync>(*notes_sync, *cr, *t, *tep, *tcm);

	// register types
	PROVIDE_INSTANCE(CRDTNotesToxSync, "CRDTNotesToxSync", g_crdtn_ts.get());

	return 0;
}

SOLANA_PLUGIN_EXPORT void solana_plugin_stop(void) {
	std::cout << "PLUGIN CRDTNTS STOP()\n";

	g_crdtn_ts.reset();
}

SOLANA_PLUGIN_EXPORT float solana_plugin_tick(float delta) {
	return g_crdtn_ts->iterate(delta);
}

} // extern C


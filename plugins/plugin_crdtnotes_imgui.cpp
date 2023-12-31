#include <solanaceae/plugin/solana_plugin_v1.h>

#include <solanaceae/crdtnotes_imgui/crdtnotes_imgui.hpp>
//#include <solanaceae/util/config_model.hpp>
#include <imgui.h>

#include <memory>
#include <limits>
#include <iostream>

#define RESOLVE_INSTANCE(x) static_cast<x*>(solana_api->resolveInstance(#x))
#define PROVIDE_INSTANCE(x, p, v) solana_api->provideInstance(#x, p, static_cast<x*>(v))

static std::unique_ptr<CRDTNotesImGui> g_crdtn_imgui = nullptr;

extern "C" {

SOLANA_PLUGIN_EXPORT const char* solana_plugin_get_name(void) {
	return "CRDTNIMGUIotesImGui";
}

SOLANA_PLUGIN_EXPORT uint32_t solana_plugin_get_version(void) {
	return SOLANA_PLUGIN_VERSION;
}

SOLANA_PLUGIN_EXPORT uint32_t solana_plugin_start(struct SolanaAPI* solana_api) {
	std::cout << "PLUGIN CRDTNIMGUI START()\n";

	if (solana_api == nullptr) {
		return 1;
	}

	//ConfigModelI* conf = nullptr;
	CRDTNotesSync* crdtns = nullptr;
	Contact3Registry* cr = nullptr;
	ImGuiContext* imguic = nullptr;

	{ // make sure required types are loaded
		//conf = RESOLVE_INSTANCE(ConfigModelI);
		crdtns = RESOLVE_INSTANCE(CRDTNotesSync);
		cr = RESOLVE_INSTANCE(Contact3Registry);
		imguic = RESOLVE_INSTANCE(ImGuiContext);

		//if (conf == nullptr) {
			//std::cerr << "PLUGIN CRDTNIMGUI missing ConfigModelI\n";
			//return 2;
		//}

		if (crdtns == nullptr) {
			std::cerr << "PLUGIN CRDTNIMGUI missing CRDTNotesSync\n";
			return 2;
		}

		if (cr == nullptr) {
			std::cerr << "PLUGIN CRDTNIMGUI missing Contact3Registry\n";
			return 2;
		}

		if (imguic == nullptr) {
			std::cerr << "PLUGIN CRDTNIMGUI missing ImGuiContext\n";
			return 2;
		}
	}

	ImGui::SetCurrentContext(imguic);

	// static store, could be anywhere tho
	// construct with fetched dependencies
	g_crdtn_imgui = std::make_unique<CRDTNotesImGui>(*crdtns, *cr);

	// register types
	PROVIDE_INSTANCE(CRDTNotesImGui, "CRDTNotesImGui", g_crdtn_imgui.get());

	return 0;
}

SOLANA_PLUGIN_EXPORT void solana_plugin_stop(void) {
	std::cout << "PLUGIN CRDTNIMGUI STOP()\n";

	g_crdtn_imgui.reset();
}

SOLANA_PLUGIN_EXPORT float solana_plugin_tick(float delta) {
	return std::numeric_limits<float>::max();
}

SOLANA_PLUGIN_EXPORT float solana_plugin_render(float delta) {
	return g_crdtn_imgui->render();
}

} // extern C


#include <solanaceae/plugin/solana_plugin_v1.h>

#include <solanaceae/crdtnotes_imgui/crdtnotes_imgui.hpp>
#include <imgui.h>

#include <entt/entt.hpp>
#include <entt/fwd.hpp>

#include <memory>
#include <limits>
#include <iostream>

static std::unique_ptr<CRDTNotesImGui> g_crdtn_imgui = nullptr;

constexpr const char* plugin_name = "CRDTNotesImGui";

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
		auto* crdtns = PLUG_RESOLVE_INSTANCE(CRDTNotesSync);
		auto* cr = PLUG_RESOLVE_INSTANCE_VERSIONED(Contact3Registry, "1");
		auto* imguic = PLUG_RESOLVE_INSTANCE_VERSIONED(ImGuiContext, ImGui::GetVersion());
		auto* imguimemaf = PLUG_RESOLVE_INSTANCE_VERSIONED(ImGuiMemAllocFunc, ImGui::GetVersion());
		auto* imguimemff = PLUG_RESOLVE_INSTANCE_VERSIONED(ImGuiMemFreeFunc, ImGui::GetVersion());
		// meh
		auto* imguimemud = plug_resolveInstanceOptional<void*>(solana_api, "ImGuiMemUserData", ImGui::GetVersion());

		ImGui::SetCurrentContext(imguic);
		ImGui::SetAllocatorFunctions(imguimemaf, imguimemff, imguimemud);

		// static store, could be anywhere tho
		// construct with fetched dependencies
		g_crdtn_imgui = std::make_unique<CRDTNotesImGui>(*crdtns, *cr);

		// register types
		PLUG_PROVIDE_INSTANCE(CRDTNotesImGui, plugin_name, g_crdtn_imgui.get());
	} catch (const ResolveException& e) {
		std::cerr << "PLUGIN " << plugin_name << " " << e.what << "\n";
		return 2;
	}

	return 0;
}

SOLANA_PLUGIN_EXPORT void solana_plugin_stop(void) {
	std::cout << "PLUGIN " << plugin_name << " STOP()\n";

	g_crdtn_imgui.reset();
}

SOLANA_PLUGIN_EXPORT float solana_plugin_tick(float delta) {
	return std::numeric_limits<float>::max();
}

SOLANA_PLUGIN_EXPORT float solana_plugin_render(float delta) {
	return g_crdtn_imgui->render();
}

} // extern C


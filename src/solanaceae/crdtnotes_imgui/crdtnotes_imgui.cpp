#include "./crdtnotes_imgui.hpp"

#include <solanaceae/contact/components.hpp>

#include <cstdint>
#include <vector>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include <iostream>

namespace detail {
	uint8_t nib_from_hex(char c) {
		assert((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'));

		if (c >= '0' && c <= '9') {
			return static_cast<uint8_t>(c) - '0';
		} else if (c >= 'a' && c <= 'f') {
			return (static_cast<uint8_t>(c) - 'a') + 10u;
		} else {
			return 0u;
		}
	}
	char nib_to_hex(uint8_t c) {
		assert((c & 0xf0) == 0x00);

		if (/*c >= 0x00 &&*/ c <= 0x09) {
			return static_cast<char>(c) + '0';
		} else if (c >= 0x0a && c <= 0x0f) {
			return (static_cast<char>(c) + 'a') - 10u;
		} else {
			return 0u;
		}
	}

	template<typename Container>
	std::string to_hex(const Container& data) {
		std::string res;
		for (const uint8_t it : data) {
			res += nib_to_hex(it << 4);
			res += nib_to_hex(it & 0x0f);
		}
		return res;
	}
} // detail


CRDTNotesImGui::CRDTNotesImGui(CRDTNotes& notes, Contact3Registry& cr) : _notes(notes), _cr(cr) {
}

float CRDTNotesImGui::render(void) {
	if (_show_global_list) {
		if (ImGui::Begin("CRDTNotes")) {
			if (ImGui::Button("Create New Doc")) {
				ImGui::OpenPopup("create new doc contact");
			}

			if (ImGui::BeginPopup("create new doc contact")) {
				for (const auto& c : _cr.view<Contact::Components::TagBig>()) {
					if (renderContactListContactSmall(c, false)) {
						const auto& self = _cr.get<Contact::Components::Self>(c).self;

						_notes.addDoc(
							// tox id (id from self)
							{},
							// random 32bytes?
							{}
						);

						// and open the doc
					}
				}
				ImGui::EndPopup();
			}


			ImGui::SeparatorText("Global list");
			const auto doclist = _notes.getDocList();
			for (const auto& docid : doclist) {
				const auto docid_str = detail::to_hex(docid);
				//ImGui::TextUnformatted(docid_str.c_str());
				if (ImGui::Selectable(docid_str.c_str())) {
					// open in editor
					_open_docs.emplace(docid);
				}
			}
		}
		ImGui::End();
	}

	{
		std::vector<CRDTNotes::DocID> to_remove;
		for (const auto& docid : _open_docs) {
			const std::string docid_str = "Doc " + detail::to_hex(docid);
			bool open = true;

			if (ImGui::Begin(docid_str.c_str(), &open)) {
				renderDoc(docid);
			}
			ImGui::End();

			if (!open) {
				to_remove.push_back(docid);
			}
		}
		for (const auto& docid : to_remove) {
			_open_docs.erase(docid);
		}
	}

	return 1.f;
}

bool CRDTNotesImGui::renderContactListContactSmall(const Contact3 c, const bool selected) const {
	std::string label;

	label += (_cr.all_of<Contact::Components::Name>(c) ? _cr.get<Contact::Components::Name>(c).name.c_str() : "<unk>");
	label += "###";
	label += std::to_string(entt::to_integral(c));

	return ImGui::Selectable(label.c_str(), selected);
}

bool CRDTNotesImGui::renderDoc(const CRDTNotes::DocID& doc_id) {
	auto* doc = _notes.getDoc(doc_id);
	if (doc == nullptr) {
		return false;
	}

	auto text = doc->getText();
	if (renderDocText(text)) {
		auto op_vec = doc->merge(text);
		std::cout << "doc changed " << op_vec.size() << " ops generated\n";
		// ... uh?
		return true;
	}

	return false;
}

bool CRDTNotesImGui::renderDocText(std::string& text) const {
	// TODO: replace with text editor (zep) or visualize stuff??
	return ImGui::InputTextMultiline("##doc", &text, {-1,-1}, ImGuiInputTextFlags_AllowTabInput);
}


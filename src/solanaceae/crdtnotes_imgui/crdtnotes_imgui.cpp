#include "./crdtnotes_imgui.hpp"

#include <solanaceae/contact/contact_store_i.hpp>
#include <solanaceae/contact/components.hpp>

#include <cstdint>
#include <vector>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include <iostream>
#include <cassert>

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
			res += nib_to_hex(it >> 4);
			res += nib_to_hex(it & 0x0f);
		}
		return res;
	}
} // detail


std::unordered_set<CRDTNotes::DocWriteLock>::iterator CRDTNotesImGui::findLock(const CRDTNotes::DocID& doc_id) {
	auto it = _held_locks.begin();
	for (; it != _held_locks.end() && it->id != doc_id; it++) {}
	return it;
}

CRDTNotesImGui::CRDTNotesImGui(CRDTNotes& notes, CRDTNotesSync& notes_sync, ContactStore4I& cs) : _notes(notes), _notes_sync(notes_sync), _cs(cs) {
}

float CRDTNotesImGui::render(void) {
	if (_show_global_list) {
		if (ImGui::Begin("CRDTNotes")) {
			if (ImGui::Button("Create New Doc")) {
				ImGui::OpenPopup("create new doc contact");
			}

			if (ImGui::BeginPopup("create new doc contact")) {
				for (const auto& c : _cs.registry().view<Contact::Components::TagBig>()) {
					if (renderContactListContactSmall(c, false)) {
						//const auto& self = _cr.get<Contact::Components::Self>(c).self;
						//assert(_cr.all_of<Contact::Components::ID>(self));
						//const auto& self_id = _cr.get<Contact::Components::ID>(self);
						//assert(!self_id.data.empty());

						//CRDTNotes::CRDTAgent self_agent_id;

						//// at most agent size, up to self id size
						//for (size_t i = 0; i < self_agent_id.size() && i < self_id.data.size(); i++) {
							//self_agent_id.at(i) = self_id.data.at(i);
						//}

						//_notes.addDoc(
							//// tox id (id from self)
							//self_agent_id
						//);
						_notes_sync.addNewDoc(_cs.contactHandle(c), false);

						//// and open the doc
					}
				}
				ImGui::EndPopup();
			}


			ImGui::SeparatorText("Global list");
			const auto doclist = _notes_sync.getDocList();
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

			ImGui::SetNextWindowSize({200, 200}, ImGuiCond_Appearing);
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

	return 2.f;
}

bool CRDTNotesImGui::renderContactListContactSmall(const Contact4 c, const bool selected) const {
	std::string label;

	const auto& cr = _cs.registry();

	label += (cr.all_of<Contact::Components::Name>(c) ? cr.get<Contact::Components::Name>(c).name.c_str() : "<unk>");
	label += "###";
	label += std::to_string(entt::to_integral(c));

	return ImGui::Selectable(label.c_str(), selected);
}

bool CRDTNotesImGui::renderDoc(const CRDTNotes::DocID& doc_id) {
	auto* doc = _notes_sync.getDoc(doc_id);
	if (doc == nullptr) {
		return false;
	}

	auto lock_it = findLock(doc_id);
	bool self_held = lock_it != _held_locks.end();
	const bool foreign_held = !self_held && _notes.isWriteLocked(doc_id);

	auto text = doc->getText();
	ImGui::InputTextMultiline(
		"##doc",
		&text,
		{-1,-1},
			ImGuiInputTextFlags_AllowTabInput |
			(foreign_held ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_None) |
			ImGuiInputTextFlags_CallbackAlways
		//cb,
		//&text
	);
	if (!foreign_held && !self_held && (ImGui::IsItemActive() || ImGui::IsItemEdited())) {
		// TODO: check
		_held_locks.emplace(_notes.writeLockAquire(doc_id).value());
		self_held = true;
		std::cout << "!!!! imgui lock aquired\n";
	} else if (!foreign_held && self_held && !(ImGui::IsItemActive() || ImGui::IsItemEdited())) {
		// release lock
		_held_locks.erase(lock_it);
		std::cout << "!!!! imgui lock released\n";
	}

	if (self_held && ImGui::IsItemEdited()) {
		_notes_sync.merge(doc_id, text);
		return true;
	}

	return false;
}

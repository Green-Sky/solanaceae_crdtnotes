#include "./crdtnotes_imgui.hpp"

#include <cstdint>

#include <imgui.h>

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


CRDTNotesImGui::CRDTNotesImGui(CRDTNotes& notes) : _notes(notes) {
}

float CRDTNotesImGui::render(void) {
	if (_show_global_list) {
		if (ImGui::Begin("CRDTNotes - Global list")) {
			const auto doclist = _notes.getDocList();
			for (const auto& docid : doclist) {
				const auto docid_str = detail::to_hex(docid);
				ImGui::TextUnformatted(docid_str.c_str());
			}
		}
		ImGui::End();
	}

	return 1.f;
}


#pragma once

#include <green_crdt/v3/text_document.hpp>

#include <array>
#include <cstdint>
#include <functional>
#include <unordered_map>

using ID32 = std::array<uint8_t, 32>;

template<>
struct std::hash<ID32> {
	std::size_t operator()(ID32 const& s) const noexcept {
		static_assert(sizeof(size_t) == 8);
		// TODO: maybe shuffle the indices a bit
		return
			(static_cast<size_t>(s[0]) << 8*0) |
			(static_cast<size_t>(s[1]) << 8*1) |
			(static_cast<size_t>(s[2]) << 8*2) |
			(static_cast<size_t>(s[3]) << 8*3) |
			(static_cast<size_t>(s[4]) << 8*4) |
			(static_cast<size_t>(s[5]) << 8*5) |
			(static_cast<size_t>(s[6]) << 8*6) |
			(static_cast<size_t>(s[7]) << 8*7)
		;
	}
};

class CRDTNotes {
	public:
		using CRDTAgent = ID32;
		using DocID = ID32;
		using Doc = GreenCRDT::V3::TextDocument<CRDTAgent>;

	private:
		// TODO: add metadata to docs
		std::unordered_map<DocID, Doc> _docs;

	public:

		// config?
		CRDTNotes(void);
		~CRDTNotes(void);

		std::vector<DocID> getDocList(void);

		const Doc* getDoc(const DocID& id) const;
		Doc* getDoc(const DocID& id);

		Doc* addDoc(const CRDTAgent& self_agent, const DocID& doc);
};


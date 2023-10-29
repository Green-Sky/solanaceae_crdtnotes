#include "./crdtnotes.hpp"

CRDTNotes::CRDTNotes(void) {
}

CRDTNotes::~CRDTNotes(void) {
}

std::vector<CRDTNotes::DocID> CRDTNotes::getDocList(void) {
	std::vector<CRDTNotes::DocID> list;
	for (const auto& [id, doc] : _docs) {
		list.push_back(id);
	}
	return list;
}

const CRDTNotes::Doc* CRDTNotes::getDoc(const DocID& id) const {
	auto res = _docs.find(id);
	return res != _docs.cend() ? &(res->second) : nullptr;
}

CRDTNotes::Doc* CRDTNotes::getDoc(const DocID& id) {
	auto res = _docs.find(id);
	return res != _docs.cend() ? &(res->second) : nullptr;
}

CRDTNotes::Doc* CRDTNotes::addDoc(const CRDTAgent& self_agent, const DocID& id) {
	if (_docs.count(id)) {
		// error exists
		// noop?
		return nullptr;
	}

	// create and set local_actor
	auto& doc = _docs[id];
	doc.local_actor = self_agent;
	return &doc;
}


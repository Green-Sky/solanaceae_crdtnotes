#include "./crdtnotes.hpp"

CRDTNotes::CRDTNotes(void) {
	_rng.seed(std::random_device{}());
	_rng.discard(707);
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

CRDTNotes::Doc* CRDTNotes::addDoc(const CRDTAgent& self_agent) {
	DocID new_id;
	for (auto& it : new_id) {
		// TODO: this discards alot
		it = static_cast<uint8_t>(_rng());
	}

	return addDoc(self_agent, new_id);
}


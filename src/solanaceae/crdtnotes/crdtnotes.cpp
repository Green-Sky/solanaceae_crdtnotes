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

void CRDTNotes::writeLockRelease(const DocID& id) {
	assert(_doc_write_locks.count(id) > 0);
	_doc_write_locks.erase(id);
}

bool CRDTNotes::isWriteLocked(const DocID& id) const {
	return _doc_write_locks.count(id);
}

std::optional<CRDTNotes::DocWriteLock> CRDTNotes::writeLockAquire(const DocID& id) {
	if (_doc_write_locks.count(id)) {
		return std::nullopt; // replace with exception instead?
	}

	_doc_write_locks.emplace(id);
	return DocWriteLock{*this, id};
}

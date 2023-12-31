#include "./crdtnotes_sync.hpp"

#include <solanaceae/contact/components.hpp>

#include <cstdint>
#include <vector>
#include <iostream>

static ID32 id_from_vec(const std::vector<uint8_t>& vec) {
	ID32 new_id;
	for (size_t i = 0; i < new_id.size() && i < vec.size(); i++) {
		new_id.at(i) = vec.at(i);
	}

	return new_id;
}

CRDTNotesSync::CRDTNotesSync(CRDTNotes& notes, Contact3Registry& cr) : _notes(notes), _cr(cr)  {
	_rng.seed(std::random_device{}());
	_rng.discard(707);
}

CRDTNotesSync::~CRDTNotesSync(void) {
}

float CRDTNotesSync::iterate(float time_delta) {
	return 1.f;
}

CRDTNotes::Doc* CRDTNotesSync::getDoc(const CRDTNotes::DocID& doc_id) {
	return _notes.getDoc(doc_id);
}

std::optional<CRDTNotes::DocID> CRDTNotesSync::addNewDoc(Contact3Handle c, bool secret) {
	if (!static_cast<bool>(c)) {
		std::cerr << "CRDTNS error: invalid contact\n";
		return std::nullopt;
	}

	const auto& self = c.get<Contact::Components::Self>().self;
	assert(_cr.all_of<Contact::Components::ID>(self));
	const auto& self_id = _cr.get<Contact::Components::ID>(self);
	assert(!self_id.data.empty());

	CRDTNotes::CRDTAgent self_agent_id = id_from_vec(self_id.data);

	CRDTNotes::DocID new_id;
	{ // generate new random id
		for (auto& it : new_id) {
			// TODO: this discards alot
			it = static_cast<uint8_t>(_rng());
		}
	}

	const auto* doc_ptr = _notes.addDoc(
		// tox id (id from self)
		self_agent_id,
		new_id // docid
	);

	if (doc_ptr == nullptr) {
		return std::nullopt;
	}

	if (!secret) {
		_docs_contacts[new_id].emplace(c);
	}

	return new_id;
}

bool CRDTNotesSync::addDoc(const CRDTNotes::DocID& doc_id, Contact3Handle c) {
	if (!static_cast<bool>(c)) {
		std::cerr << "CRDTNS error: invalid contact\n";
		return false;
	}

	const auto& self = c.get<Contact::Components::Self>().self;
	assert(_cr.all_of<Contact::Components::ID>(self));
	const auto& self_id = _cr.get<Contact::Components::ID>(self);
	assert(!self_id.data.empty());

	CRDTNotes::CRDTAgent self_agent_id = id_from_vec(self_id.data);

	// preexisting does not overwrite self!!!
	const auto* doc_ptr = _notes.addDoc(self_agent_id, doc_id);

	_docs_contacts[doc_id].emplace(c);

	return doc_ptr != nullptr;
}

std::vector<CRDTNotes::DocID> CRDTNotesSync::getDocList(void) {
	return _notes.getDocList();
}

std::vector<CRDTNotes::DocID> CRDTNotesSync::getDocList(Contact3Handle c) {
	std::vector<CRDTNotes::DocID> list;

	Contact3Handle parent;
	if (c.all_of<Contact::Components::Parent>()) {
		parent = Contact3Handle{*c.registry(), c.get<Contact::Components::Parent>().parent};
	}

	for (const auto& [k, v] : _docs_contacts) {
		if (v.count(c)) {
			list.push_back(k);
			continue; // avoid dups
		}

		if (v.count(parent)) {
			list.push_back(k);
		}
	}

	return list;
}

void CRDTNotesSync::merge(const CRDTNotes::DocID& doc_id, std::string_view new_text) {
	auto* doc_ptr = _notes.getDoc(doc_id);
	if (doc_ptr == nullptr) {
		std::cerr << "CRDTNS error: tried to merge into unknown doc\n";
		return;
	}

	auto op_vec = doc_ptr->merge(new_text);
	std::cout << "doc changed " << op_vec.size() << " ops generated\n";

	// USE OPS
}

void CRDTNotesSync::onCRDTNSyncEvent(Events::NGCEXT_crdtns_gossip&& e) {
	addDoc(e.doc_id, e.c);
}

void CRDTNotesSync::onCRDTNSyncEvent(Events::NGCEXT_crdtns_gossip_frontier&& e) {
}

void CRDTNotesSync::onCRDTNSyncEvent(Events::NGCEXT_crdtns_fetch_complete_frontier&& e) {
}

void CRDTNotesSync::onCRDTNSyncEvent(Events::NGCEXT_crdtns_fetch_op_range&& e) {
}

void CRDTNotesSync::onCRDTNSyncEvent(Events::NGCEXT_crdtns_ops&& e) {
}


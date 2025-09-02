#include "./crdtnotes_sync.hpp"

#include <solanaceae/contact/contact_store_i.hpp>
#include <solanaceae/crdtnotes/crdtnotes_contact_sync_model.hpp>

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

CRDTNotesSync::CRDTNotesSync(CRDTNotes& notes, ContactStore4I& cs) : _notes(notes), _cs(cs)  {
	_rng.seed(std::random_device{}());
	_rng.discard(707);
}

CRDTNotesSync::~CRDTNotesSync(void) {
}

float CRDTNotesSync::iterate(float time_delta) {
	if (!_gossip_queue.empty()) {
		// TODO: set is sorted by id, not by order added
		// only one per iterate *should* be enough
		const auto it = _gossip_queue.cbegin();
		if (_docs_contacts.count(*it)) {
			for (const auto& c : _docs_contacts.at(*it)) {
				if (!c.all_of<CRDTNotesContactSyncModelI*>()) {

					// TODO: this is a fallback, remove
					if (c.all_of<Contact::Components::ParentOf>()) {
						for (const auto child : c.get<Contact::Components::ParentOf>().subs) {
							if (c.registry()->all_of<Contact::Components::TagSelfStrong>(child)) {
								continue;
							}
							if (!c.registry()->all_of<CRDTNotesContactSyncModelI*>(child)) {
								std::cerr << "CRDTNotesSync: error, fallback failed\n";
								continue;
							}

							c.registry()->get<CRDTNotesContactSyncModelI*>(child)->SendGossip({*c.registry(), child}, *it);
						}
					}
					continue; // skip, not impl
				}

				c.get<CRDTNotesContactSyncModelI*>()->SendGossip(c, *it);
			}
		}
		_gossip_queue.erase(it);
	}

	if (!_fetch_frontier_queue.empty()) {
		// TODO: set is sorted by id, not by order added
		// only one per iterate *should* be enough
		const auto it = _fetch_frontier_queue.cbegin();
		if (_docs_contacts.count(*it)) {
			for (const auto& c : _docs_contacts.at(*it)) {
				if (!c.all_of<CRDTNotesContactSyncModelI*>()) {

					// TODO: this is a fallback, remove
					if (c.all_of<Contact::Components::ParentOf>()) {
						for (const auto child : c.get<Contact::Components::ParentOf>().subs) {
							if (c.registry()->all_of<Contact::Components::TagSelfStrong>(child)) {
								continue;
							}
							if (!c.registry()->all_of<CRDTNotesContactSyncModelI*>(child)) {
								std::cerr << "CRDTNotesSync: error, fallback failed\n";
								continue;
							}

							c.registry()->get<CRDTNotesContactSyncModelI*>(child)->SendFetchCompleteFrontier({*c.registry(), child}, *it);
						}
					}
					continue; // skip, not impl
				}

				c.get<CRDTNotesContactSyncModelI*>()->SendFetchCompleteFrontier(c, *it);
			}
		}
		_fetch_frontier_queue.erase(it);
	}

	return 2.f;
}

CRDTNotes::Doc* CRDTNotesSync::getDoc(const CRDTNotes::DocID& doc_id) {
	return _notes.getDoc(doc_id);
}

std::optional<CRDTNotes::DocID> CRDTNotesSync::addNewDoc(ContactHandle4 c, bool secret) {
	if (!static_cast<bool>(c)) {
		std::cerr << "CRDTNS error: invalid contact\n";
		return std::nullopt;
	}

	const auto& cr = _cs.registry();

	const auto self = c.get<Contact::Components::Self>().self;
	assert(cr.all_of<Contact::Components::ID>(self));
	const auto& self_id = cr.get<Contact::Components::ID>(self);
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

	_gossip_queue.emplace(new_id);

	return new_id;
}

bool CRDTNotesSync::addDoc(const CRDTNotes::DocID& doc_id, ContactHandle4 c) {
	if (!static_cast<bool>(c)) {
		std::cerr << "CRDTNS error: invalid contact\n";
		return false;
	}

	const auto& cr = _cs.registry();

	const auto& self = c.get<Contact::Components::Self>().self;
	assert(cr.all_of<Contact::Components::ID>(self));
	const auto& self_id = cr.get<Contact::Components::ID>(self);
	assert(!self_id.data.empty());

	CRDTNotes::CRDTAgent self_agent_id = id_from_vec(self_id.data);

	// preexisting does not overwrite self!!!
	const auto* doc_ptr = _notes.addDoc(self_agent_id, doc_id);

	_docs_contacts[doc_id].emplace(c);

	if (doc_ptr != nullptr) {
		// new for us
		_gossip_queue.emplace(doc_id);
		_fetch_frontier_queue.emplace(doc_id);
	}

	return doc_ptr != nullptr;
}

std::vector<CRDTNotes::DocID> CRDTNotesSync::getDocList(void) {
	return _notes.getDocList();
}

std::vector<CRDTNotes::DocID> CRDTNotesSync::getDocList(ContactHandle4 c) {
	std::vector<CRDTNotes::DocID> list;

	ContactHandle4 parent;
	if (c.all_of<Contact::Components::Parent>()) {
		parent = ContactHandle4{*c.registry(), c.get<Contact::Components::Parent>().parent};
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


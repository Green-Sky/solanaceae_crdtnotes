#include "./crdtnotes_sync.hpp"

#include <solanaceae/contact/contact_store_i.hpp>
#include <solanaceae/crdtnotes/crdtnotes_contact_sync_model.hpp>

#include <solanaceae/contact/components.hpp>

#include <entt/container/dense_set.hpp>

#include <cstdint>
#include <vector>
#include <iostream>

namespace Components {

// attached to contact
struct OpSendQueue {
	std::map<CRDTNotes::DocID, std::vector<CRDTNotes::Doc::Op>> ops;
	// HACK: limit to 5 ops per packet for now
	// TODO: ft based alternative for >5 ops
};

} // Components

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
	for (auto doc_it = _docs_incoming_ops.begin(); doc_it != _docs_incoming_ops.end();) {
		if (_notes.isWriteLocked(doc_it->first)) {
			doc_it++;
			continue;
		}

		auto lock_opt = _notes.writeLockAquire(doc_it->first);
		assert(lock_opt);

		auto* doc_ptr = getDoc(doc_it->first);
		// TODO: record every applied op and throw event, so eg gui can react better
		// , or better yet, edit events in string space (imgui can consume them)
		doc_ptr->apply(doc_it->second);

		std::cout << "CRDTNotesSync: applied " << doc_it->second.size() << " ops\n";

		doc_it = _docs_incoming_ops.erase(doc_it);
	}

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

	bool sending_ops {false};
	{ // send ops in queue
		std::vector<Contact4> empty_queue;
		for (const auto& [c, op_comp, sync_model] : _cs.registry().view<Components::OpSendQueue, CRDTNotesContactSyncModelI*>().each()) {
			// HACK: one pkg with up to 5 ops per tick per peer
			//for (const auto& [doc_id, op_vec] : op_comp.ops) {
			for (auto it = op_comp.ops.begin(); it != op_comp.ops.end();) {
				if (it->second.empty()) {
					it = op_comp.ops.erase(it);
					continue;
				} else if (it->second.size() <= 5) {
					std::cout << "sending " << it->second.size() << " ops\n";
					sync_model->SendOps(_cs.contactHandle(c), it->first, it->second);
					it = op_comp.ops.erase(it);
					//sending_ops = true;
				} else {
					std::vector<CRDTNotes::Doc::Op> tmp_ops {it->second.cbegin(), it->second.cbegin()+5};
					assert(tmp_ops.size() == 5);
					sync_model->SendOps(_cs.contactHandle(c), it->first, tmp_ops);
					it->second.erase(it->second.cbegin(), it->second.cbegin()+5);
					sending_ops = true;
				}

				break; // single update only
			}

			if (op_comp.ops.empty()) {
				empty_queue.push_back(c);
			}
		}
		_cs.registry().remove<Components::OpSendQueue>(empty_queue.cbegin(), empty_queue.cend());
	}

	return sending_ops ? 0.05f : 2.f;
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

	// attach OpSendQueue to every contact
	// needs to be placed at the contact with the sync model
	entt::dense_set<Contact4> handled_contacts;
	for (auto c : _docs_contacts.at(doc_id)) {
		if (handled_contacts.contains(c)) {
			continue;
		}

		if (!c.all_of<CRDTNotesContactSyncModelI*>()) {

			// TODO: this is a fallback, remove
			if (c.all_of<Contact::Components::ParentOf>()) {
				for (const auto child : c.get<Contact::Components::ParentOf>().subs) {
					if (handled_contacts.contains(child)) {
						continue;
					}
					if (c.registry()->all_of<Contact::Components::TagSelfStrong>(child)) {
						continue;
					}
					if (!c.registry()->all_of<CRDTNotesContactSyncModelI*>(child)) {
						std::cerr << "CRDTNotesSync error: fallback failed\n";
						continue;
					}

					auto& op_queue = c.registry()->get_or_emplace<Components::OpSendQueue>(child).ops[doc_id];
					if (op_queue.empty()) {
						op_queue = op_vec;
					} else {
						op_queue.insert(op_queue.cend(), op_vec.cbegin(), op_vec.cend());
					}
					handled_contacts.emplace(child);
				}
			}
			continue; // skip, not impl
		}

		auto& op_queue = c.get_or_emplace<Components::OpSendQueue>().ops[doc_id];
		if (op_queue.empty()) {
			op_queue = op_vec;
		} else {
			op_queue.insert(op_queue.cend(), op_vec.cbegin(), op_vec.cend());
		}
		handled_contacts.emplace(c);
	}
}

void CRDTNotesSync::onCRDTNSyncEvent(Events::NGCEXT_crdtns_gossip&& e) {
	addDoc(e.doc_id, e.c);
}

void CRDTNotesSync::onCRDTNSyncEvent(Events::NGCEXT_crdtns_gossip_frontier&& e) {
}

void CRDTNotesSync::onCRDTNSyncEvent(Events::NGCEXT_crdtns_fetch_complete_frontier&& e) {
}

void CRDTNotesSync::onCRDTNSyncEvent(Events::NGCEXT_crdtns_fetch_add_range&& e) {
}

void CRDTNotesSync::onCRDTNSyncEvent(Events::NGCEXT_crdtns_fetch_del&& e) {
}

void CRDTNotesSync::onCRDTNSyncEvent(Events::NGCEXT_crdtns_ops&& e) {
	addDoc(e.doc_id, e.c);

	if (e.ops.empty()) {
		std::cerr << "CRDTNotesSync warning: got empty ops event/pkg\n";
		return;
	}

	// TODO: deduplicate ops ?
	auto lock_opt = _notes.writeLockAquire(e.doc_id);
	if (lock_opt) {
		// TODO: perms n stuff
		// TODO: check if seq missing
		auto* doc_ptr = getDoc(e.doc_id);
		// TODO: record every applied op and throw event, so eg gui can react better
		// , or better yet, edit events in string space (imgui can consume them)
		doc_ptr->apply(e.ops);

		// TODO: check if new frontier
	} else {
		auto& op_in_vec = _docs_incoming_ops[e.doc_id];
		if (op_in_vec.empty()) {
			op_in_vec = e.ops;
		} else {
			op_in_vec.insert(op_in_vec.cend(), e.ops.cbegin(), e.ops.cend());
		}
	}
}


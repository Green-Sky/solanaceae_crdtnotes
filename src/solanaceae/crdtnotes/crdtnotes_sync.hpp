#pragma once

#include "./crdtnotes.hpp"

#include <solanaceae/contact/fwd.hpp>

#include <entt/entity/registry.hpp>
#include <entt/entity/handle.hpp>

#include <set>
#include <random>
#include <unordered_map>
#include <map>

// fwd
struct CRDTNotesContactSyncModelI;

namespace Events {

	// - DocID
	struct NGCEXT_crdtns_gossip {
		ContactHandle4 c;
		CRDTNotes::DocID doc_id;
	};

	// - DocID
	// - array [
	//   - AgentID
	//   - seq (frontier)
	//   - del_num
	// - ]
	struct NGCEXT_crdtns_gossip_frontier {
		ContactHandle4 c;
		CRDTNotes::DocID doc_id;
		std::vector<CRDTNotes::Frontier> frontier;
	};

	// - DocID
	struct NGCEXT_crdtns_fetch_complete_frontier {
		ContactHandle4 c;
		CRDTNotes::DocID doc_id;
	};

	// - DocID
	// - AgentID
	// - seq_from
	// - seq_to
	struct NGCEXT_crdtns_fetch_add_range {
		ContactHandle4 c;
		CRDTNotes::DocID doc_id;
		CRDTNotes::CRDTAgent agent;
		uint64_t seq_from;
		uint64_t seq_to;
	};

	// - DocID
	// - AgentID
	struct NGCEXT_crdtns_fetch_del {
		ContactHandle4 c;
		CRDTNotes::DocID doc_id;
		CRDTNotes::CRDTAgent agent;
	};

	// - DocID
	// - array [
	//   - op
	// - ]
	struct NGCEXT_crdtns_ops {
		ContactHandle4 c;
		CRDTNotes::DocID doc_id;
		std::vector<CRDTNotes::Doc::Op> ops;
	};

	// TODO: curser
	// - DocID
	// - AgentID
	// - CRDTNotes::Doc::ListType::ListID parent_left

} // Events

// this is different than other "i might not handle this" event interfaces
struct CRDTNotesEventI {
	virtual ~CRDTNotesEventI(void) {}

	virtual void onCRDTNSyncEvent(Events::NGCEXT_crdtns_gossip&& e) = 0;
	virtual void onCRDTNSyncEvent(Events::NGCEXT_crdtns_gossip_frontier&& e) = 0;
	virtual void onCRDTNSyncEvent(Events::NGCEXT_crdtns_fetch_complete_frontier&& e) = 0;
	virtual void onCRDTNSyncEvent(Events::NGCEXT_crdtns_fetch_add_range&& e) = 0;
	virtual void onCRDTNSyncEvent(Events::NGCEXT_crdtns_fetch_del&& e) = 0;
	virtual void onCRDTNSyncEvent(Events::NGCEXT_crdtns_ops&& e) = 0;
};

// high level sync logic
// gets called on incoming packets
// calls CRDTNotesContactSyncModelI on contacts
class CRDTNotesSync final : public CRDTNotesEventI {
	// pull inside????
	CRDTNotes& _notes;
	ContactStore4I& _cs;

	std::default_random_engine _rng;

	std::unordered_map<CRDTNotes::DocID, std::set<ContactHandle4>> _docs_contacts;
	struct Peer {
		// global frontier
		// what we know the peer knows(/gossiped) about
		std::unordered_map<decltype(CRDTNotes::Frontier::agent), decltype(CRDTNotes::Frontier::seq)> other_frontier;
	};
	std::unordered_map<CRDTNotes::DocID, std::map<ContactHandle4, Peer>> _docs_peers;

	// queue of unapplied ops, kept here until write lock can be aquired
	std::unordered_map<CRDTNotes::DocID, std::vector<CRDTNotes::Doc::Op>> _docs_incoming_ops;

	// if a doc is eg new, it is added here
	std::set<CRDTNotes::DocID> _gossip_queue; // TODO: no
	std::set<CRDTNotes::DocID> _fetch_frontier_queue;

	public:
		CRDTNotesSync(CRDTNotes& notes, ContactStore4I& cs);

		~CRDTNotesSync(void);

		// called from main thread periodically
		float iterate(float time_delta);

	public: // CRDTNotes api
		CRDTNotes::Doc* getDoc(const CRDTNotes::DocID& doc_id);

		// adds a doc and assosiates contact (and self)
		// if secret, only self is added (and thats why contact is needed)
		std::optional<CRDTNotes::DocID> addNewDoc(ContactHandle4 c, bool secret = false);

		// adds a doc by id to a contact
		// (for gossip or manual add)
		bool addDoc(const CRDTNotes::DocID& doc_id, ContactHandle4 c);

		std::vector<CRDTNotes::DocID> getDocList(void);
		std::vector<CRDTNotes::DocID> getDocList(ContactHandle4 c);

		void merge(const CRDTNotes::DocID& doc_id, std::string_view new_text);

	public:
		void onCRDTNSyncEvent(Events::NGCEXT_crdtns_gossip&& e) override;
		void onCRDTNSyncEvent(Events::NGCEXT_crdtns_gossip_frontier&& e) override;
		void onCRDTNSyncEvent(Events::NGCEXT_crdtns_fetch_complete_frontier&& e) override;
		void onCRDTNSyncEvent(Events::NGCEXT_crdtns_fetch_add_range&& e) override;
		void onCRDTNSyncEvent(Events::NGCEXT_crdtns_fetch_del&& e) override;
		void onCRDTNSyncEvent(Events::NGCEXT_crdtns_ops&& e) override;
};


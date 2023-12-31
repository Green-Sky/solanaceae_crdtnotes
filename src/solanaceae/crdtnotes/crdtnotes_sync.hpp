#pragma once

#include "./crdtnotes.hpp"

#include <solanaceae/contact/contact_model3.hpp>

#include <set>
#include <random>

// fwd
struct CRDTNotesContactSyncModelI;

namespace Events {

	// - DocID
	struct NGCEXT_crdtns_gossip {
		Contact3Handle c;
		CRDTNotes::DocID doc_id;
	};

	// - DocID
	// - array [
	//   - AgentID
	//   - seq (frontier)
	// - ]
	struct NGCEXT_crdtns_gossip_frontier {
		Contact3Handle c;
		CRDTNotes::DocID doc_id;
		std::vector<CRDTNotes::Frontier> selected_frontier;
	};

	// - DocID
	struct NGCEXT_crdtns_fetch_complete_frontier {
		Contact3Handle c;
		CRDTNotes::DocID doc_id;
	};

	// - DocID
	// - AgentID
	// - seq_from
	// - seq_to
	struct NGCEXT_crdtns_fetch_op_range {
		Contact3Handle c;
		CRDTNotes::DocID doc_id;
		CRDTNotes::CRDTAgent agent;
		uint64_t seq_from;
		uint64_t seq_to;
	};

	// - DocID
	// - array [
	//   - op
	// - ]
	struct NGCEXT_crdtns_ops {
		Contact3Handle c;
		CRDTNotes::DocID doc_id;
		std::vector<CRDTNotes::Doc::Op> ops;
	};

} // Events

// this is different than other "i might not handle this" event interfaces
struct CRDTNotesEventI {
	virtual ~CRDTNotesEventI(void) {}

	virtual void onCRDTNSyncEvent(Events::NGCEXT_crdtns_gossip&& e) = 0;
	virtual void onCRDTNSyncEvent(Events::NGCEXT_crdtns_gossip_frontier&& e) = 0;
	virtual void onCRDTNSyncEvent(Events::NGCEXT_crdtns_fetch_complete_frontier&& e) = 0;
	virtual void onCRDTNSyncEvent(Events::NGCEXT_crdtns_fetch_op_range&& e) = 0;
	virtual void onCRDTNSyncEvent(Events::NGCEXT_crdtns_ops&& e) = 0;
};

// high level sync logic
// gets called on incoming packets
// calls CRDTNotesContactSyncModelI on contacts
class CRDTNotesSync final : public CRDTNotesEventI {
	// pull inside????
	CRDTNotes& _notes;
	Contact3Registry& _cr;

	std::default_random_engine _rng;

	std::unordered_map<CRDTNotes::DocID, std::set<Contact3Handle>> _docs_contacts;

	// if a doc is eg new, it is added here
	std::set<CRDTNotes::DocID> _gossip_queue;
	std::set<CRDTNotes::DocID> _fetch_frontier_queue;

	public:
		CRDTNotesSync(CRDTNotes& notes, Contact3Registry& cr);

		~CRDTNotesSync(void);

		// called from main thread periodically
		float iterate(float time_delta);

	public: // CRDTNotes api
		CRDTNotes::Doc* getDoc(const CRDTNotes::DocID& doc_id);

		// adds a doc and assosiates contact (and self)
		// if secret, only self is added (and thats why contact is needed)
		std::optional<CRDTNotes::DocID> addNewDoc(Contact3Handle c, bool secret = false);

		// adds a doc by id to a contact
		// (for gossip or manual add)
		bool addDoc(const CRDTNotes::DocID& doc_id, Contact3Handle c);

		std::vector<CRDTNotes::DocID> getDocList(void);
		std::vector<CRDTNotes::DocID> getDocList(Contact3Handle c);

		void merge(const CRDTNotes::DocID& doc_id, std::string_view new_text);

	public:
		void onCRDTNSyncEvent(Events::NGCEXT_crdtns_gossip&& e) override;
		void onCRDTNSyncEvent(Events::NGCEXT_crdtns_gossip_frontier&& e) override;
		void onCRDTNSyncEvent(Events::NGCEXT_crdtns_fetch_complete_frontier&& e) override;
		void onCRDTNSyncEvent(Events::NGCEXT_crdtns_fetch_op_range&& e) override;
		void onCRDTNSyncEvent(Events::NGCEXT_crdtns_ops&& e) override;
};


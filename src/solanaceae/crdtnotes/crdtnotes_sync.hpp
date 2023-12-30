#pragma once

#include "./crdtnotes.hpp"
#include <solanaceae/contact/contact_model3.hpp>

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

	public:
		void onCRDTNSyncEvent(Events::NGCEXT_crdtns_gossip&& e) override;
		void onCRDTNSyncEvent(Events::NGCEXT_crdtns_gossip_frontier&& e) override;
		void onCRDTNSyncEvent(Events::NGCEXT_crdtns_fetch_complete_frontier&& e) override;
		void onCRDTNSyncEvent(Events::NGCEXT_crdtns_fetch_op_range&& e) override;
		void onCRDTNSyncEvent(Events::NGCEXT_crdtns_ops&& e) override;
};


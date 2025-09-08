#pragma once

#include "./crdtnotes.hpp"

#include <solanaceae/contact/fwd.hpp>

// send api
struct CRDTNotesContactSyncModelI {
	virtual ~CRDTNotesContactSyncModelI(void) {}

	// gossip
	public:
		// notify of doc existing
		virtual void SendGossip(
			ContactHandle4 c,
			const CRDTNotes::DocID& doc_id
		) = 0;

		virtual void SendGossip(
			ContactHandle4 c,
			const CRDTNotes::DocID& doc_id,
			const std::vector<CRDTNotes::Frontier>& frontier
		) = 0;

	// fetch
	public:
		// causes the other peer to send gossip with all known frontiers (on cool down)
		virtual void SendFetchCompleteFrontier(
			ContactHandle4 c,
			const CRDTNotes::DocID& doc_id
		) = 0;

		// action range request
		virtual void SendFetchAddRange(
			ContactHandle4 c,
			const CRDTNotes::DocID& doc_id,
			const CRDTNotes::CRDTAgent& agent,
			const uint64_t seq_from,
			const uint64_t seq_to
		) = 0;

		virtual void SendFetchDel(
			ContactHandle4 c,
			const CRDTNotes::DocID& doc_id,
			const CRDTNotes::CRDTAgent& agent
		) = 0;

	public: // ops response
		virtual void SendOps(
			ContactHandle4 c,
			const CRDTNotes::DocID& doc_id,
			// TODO: optimize this
			const std::vector<CRDTNotes::Doc::Op>&
		) = 0;
};


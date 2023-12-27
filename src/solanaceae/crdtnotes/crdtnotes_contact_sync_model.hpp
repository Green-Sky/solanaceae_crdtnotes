#pragma once

#include "./crdtnotes.hpp"

#include <solanaceae/contact/contact_model3.hpp>

// send api
struct CRDTNotesContactSyncModelI {
	virtual ~CRDTNotesContactSyncModelI(void) {}

	// gossip
	public:
		// notify of doc existing
		virtual void SendGossip(
			Contact3Handle c,
			const CRDTNotes::DocID& doc_id
		) = 0;

		virtual void SendGossip(
			Contact3Handle c,
			const CRDTNotes::DocID& doc_id,
			const std::vector<CRDTNotes::Frontier>& selected_frontier
		) = 0;

	// fetch
	public:
		// causes the other peer to send gossip with all known frontiers (on cool down)
		virtual void SendFetchCompleteFrontier(
			Contact3Handle c,
			const CRDTNotes::DocID& doc_id
		) = 0;

		// action range request
		virtual void SendFetchOps(
			Contact3Handle c,
			const CRDTNotes::DocID& doc_id,
			const CRDTNotes::CRDTAgent& agent,
			const uint64_t seq_from,
			const uint64_t seq_to
		) = 0;

	public: // ops response
		virtual void SendOps(
			Contact3Handle c,
			const CRDTNotes::DocID& doc_id,
			// TODO: optimize this
			const std::vector<CRDTNotes::Doc::Op>&
		) = 0;
};


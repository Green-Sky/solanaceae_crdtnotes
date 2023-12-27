#pragma once

#include <solanaceae/crdtnotes/crdtnotes.hpp>
#include <solanaceae/crdtnotes/crdtnotes_contact_sync_model.hpp>
#include <solanaceae/contact/contact_model3.hpp>
#include <solanaceae/toxcore/tox_event_interface.hpp>

// fwd
struct ToxI;
struct ToxEventProviderI;

// implements CRDTNotesContactSyncModelI and attaches itself to tox contacts
class CRDTNotesToxSync : public CRDTNotesContactSyncModelI, public ToxEventI {
	CRDTNotes& _notes;
	Contact3Registry& _cr;
	ToxI& _t;
	ToxEventProviderI& _tep;

	public:
		CRDTNotesToxSync(
			CRDTNotes& notes,
			Contact3Registry& cr,
			ToxI& t,
			ToxEventProviderI& tep
		);
		~CRDTNotesToxSync(void);

		float iterate(float time_delta);

	public: // sync api
		void SendGossip(
			Contact3Handle c,
			const CRDTNotes::DocID& doc_id
		) override;

		void SendGossip(
			Contact3Handle c,
			const CRDTNotes::DocID& doc_id,
			const std::vector<CRDTNotes::Frontier>& selected_frontier
		) override;

		void SendFetchCompleteFrontier(
			Contact3Handle c,
			const CRDTNotes::DocID& doc_id
		) override;

		void SendFetchOps(
			Contact3Handle c,
			const CRDTNotes::DocID& doc_id,
			const CRDTNotes::CRDTAgent& agent,
			const uint64_t seq_from,
			const uint64_t seq_to
		) override;

		void SendOps(
			Contact3Handle c,
			const CRDTNotes::DocID& doc_id,
			const std::vector<CRDTNotes::Doc::Op>&
		) override;
};


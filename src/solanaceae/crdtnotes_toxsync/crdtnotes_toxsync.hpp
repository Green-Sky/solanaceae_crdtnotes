#pragma once

#include <solanaceae/crdtnotes/crdtnotes_sync.hpp>
#include <solanaceae/crdtnotes/crdtnotes_contact_sync_model.hpp>
#include <solanaceae/contact/fwd.hpp>
#include <solanaceae/toxcore/tox_event_interface.hpp>
#include <solanaceae/tox_contacts/tox_contact_model2.hpp>

// fwd
struct ToxI;
struct ToxEventProviderI;

// implements CRDTNotesContactSyncModelI and attaches itself to tox contacts
class CRDTNotesToxSync : public CRDTNotesContactSyncModelI, public ToxEventI {
	CRDTNotesEventI& _notes_sync;
	ContactStore4I& _cs;
	ToxI& _t;
	ToxEventProviderI::SubscriptionReference _tep_sr;
	ToxContactModel2& _tcm;

	public:
		CRDTNotesToxSync(
			CRDTNotesEventI& notes_sync,
			ContactStore4I& cs,
			ToxI& t,
			ToxEventProviderI& tep,
			ToxContactModel2& tcm
		);
		~CRDTNotesToxSync(void);

		float iterate(float time_delta);

	public: // sync api
		void SendGossip(
			ContactHandle4 c,
			const CRDTNotes::DocID& doc_id
		) override;

		void SendGossip(
			ContactHandle4 c,
			const CRDTNotes::DocID& doc_id,
			const std::vector<CRDTNotes::Frontier>& selected_frontier
		) override;

		void SendFetchCompleteFrontier(
			ContactHandle4 c,
			const CRDTNotes::DocID& doc_id
		) override;

		void SendFetchOps(
			ContactHandle4 c,
			const CRDTNotes::DocID& doc_id,
			const CRDTNotes::CRDTAgent& agent,
			const uint64_t seq_from,
			const uint64_t seq_to
		) override;

		void SendOps(
			ContactHandle4 c,
			const CRDTNotes::DocID& doc_id,
			const std::vector<CRDTNotes::Doc::Op>&
		) override;

	private:
		bool parse_crdtn_gossip(
			ContactHandle4 c,
			const uint8_t* data, size_t data_size,
			bool _private
		);
		bool parse_crdtn_gossip_frontier(
			ContactHandle4 c,
			const uint8_t* data, size_t data_size,
			bool _private
		);
		bool parse_crdtn_fetch_complete_frontier(
			ContactHandle4 c,
			const uint8_t* data, size_t data_size,
			bool _private
		);
		bool parse_crdtn_fetch_op_range(
			ContactHandle4 c,
			const uint8_t* data, size_t data_size,
			bool _private
		);
		bool parse_crdtn_ops(
			ContactHandle4 c,
			const uint8_t* data, size_t data_size,
			bool _private
		);

		bool handlePacket(
			const uint32_t group_number,
			const uint32_t peer_number,
			const uint8_t* data,
			const size_t data_size,
			const bool _private
		);

	protected: // tox events
		bool onToxEvent(const Tox_Event_Group_Peer_Join* e) override;
		bool onToxEvent(const Tox_Event_Group_Custom_Packet* e) override;
		bool onToxEvent(const Tox_Event_Group_Custom_Private_Packet* e) override;
};


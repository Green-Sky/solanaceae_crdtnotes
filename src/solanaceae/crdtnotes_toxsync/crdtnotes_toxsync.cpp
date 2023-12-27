#include "./crdtnotes_toxsync.hpp"

#include <solanaceae/toxcore/tox_interface.hpp>

#include <solanaceae/tox_contacts/components.hpp>

enum class NGCEXT_Event : uint8_t {
	// - DocID
	CRDTN_GOSSIP = 0x80 | 0x10,

	// - DocID
	// - array [
	//   - AgentID
	//   - seq (frontier)
	// - ]
	CRDTN_GOSSIP_FRONTIER,

	// - DocID
	CRDTN_FETCH_COMPLETE_FRONTIER,

	// - DocID
	// - AgentID
	// - seq_from
	// - seq_to
	CRDTN_FETCH_OP_RANGE,

	// - DocID
	// - array [
	//   - seq
	//   - action date
	// - ]
	CRDTN_OPS,
};


CRDTNotesToxSync::CRDTNotesToxSync(
	CRDTNotes& notes,
	Contact3Registry& cr,
	ToxI& t,
	ToxEventProviderI& tep
) : _notes(notes), _cr(cr), _t(t), _tep(tep) {
}

CRDTNotesToxSync::~CRDTNotesToxSync(void) {
}

float CRDTNotesToxSync::iterate(float time_delta) {
	return 1.f; // TODO: 1sec for now, needs better logic
}

void CRDTNotesToxSync::SendGossip(
	Contact3Handle c,
	const CRDTNotes::DocID& doc_id
) {
	if (!c.all_of<Contact::Components::ToxGroupPeerEphemeral>()) {
		return;
	}

	std::vector<uint8_t> pkg;

	pkg.push_back(static_cast<uint8_t>(NGCEXT_Event::CRDTN_GOSSIP));

	for (const uint8_t v : doc_id) {
		pkg.push_back(v);
	}

	// send
	const auto& gp = c.get<Contact::Components::ToxGroupPeerEphemeral>();
	_t.toxGroupSendCustomPrivatePacket(
		gp.group_number, gp.peer_number,
		true,
		pkg
	);
}

void CRDTNotesToxSync::SendGossip(
	Contact3Handle c,
	const CRDTNotes::DocID& doc_id,
	const std::vector<CRDTNotes::Frontier>& selected_frontier
) {
	if (!c.all_of<Contact::Components::ToxGroupPeerEphemeral>()) {
		return;
	}

	std::vector<uint8_t> pkg;

	pkg.push_back(static_cast<uint8_t>(NGCEXT_Event::CRDTN_GOSSIP_FRONTIER));
	// 1

	for (const uint8_t v : doc_id) {
		pkg.push_back(v);
	}
	// +32

	for (const auto& [f_id, f_seq] : selected_frontier) {
		for (const uint8_t v : f_id) {
			pkg.push_back(v);
		}
		// +32

		for (size_t i = 0; i < sizeof(f_seq); i++) {
			pkg.push_back((f_seq >> i*8) & 0xff);
		}
		// +8
	}
	// +40

	// send
	const auto& gp = c.get<Contact::Components::ToxGroupPeerEphemeral>();
	_t.toxGroupSendCustomPrivatePacket(
		gp.group_number, gp.peer_number,
		true,
		pkg
	);
}

void CRDTNotesToxSync::SendFetchCompleteFrontier(
	Contact3Handle c,
	const CRDTNotes::DocID& doc_id
) {
	if (!c.all_of<Contact::Components::ToxGroupPeerEphemeral>()) {
		return;
	}

	std::vector<uint8_t> pkg;

	pkg.push_back(static_cast<uint8_t>(NGCEXT_Event::CRDTN_FETCH_COMPLETE_FRONTIER));

	for (const uint8_t v : doc_id) {
		pkg.push_back(v);
	}

	// send
	const auto& gp = c.get<Contact::Components::ToxGroupPeerEphemeral>();
	_t.toxGroupSendCustomPrivatePacket(
		gp.group_number, gp.peer_number,
		true,
		pkg
	);
}

void CRDTNotesToxSync::SendFetchOps(
	Contact3Handle c,
	const CRDTNotes::DocID& doc_id,
	const CRDTNotes::CRDTAgent& agent,
	const uint64_t seq_from,
	const uint64_t seq_to
) {
	if (!c.all_of<Contact::Components::ToxGroupPeerEphemeral>()) {
		return;
	}

	std::vector<uint8_t> pkg;

	pkg.push_back(static_cast<uint8_t>(NGCEXT_Event::CRDTN_FETCH_OP_RANGE));

	for (const uint8_t v : doc_id) {
		pkg.push_back(v);
	}

	for (const uint8_t v : agent) {
		pkg.push_back(v);
	}

	for (size_t i = 0; i < sizeof(seq_from); i++) {
		pkg.push_back((seq_from >> i*8) & 0xff);
	}
	// +8

	for (size_t i = 0; i < sizeof(seq_to); i++) {
		pkg.push_back((seq_to >> i*8) & 0xff);
	}
	// +8


	// send
	const auto& gp = c.get<Contact::Components::ToxGroupPeerEphemeral>();
	_t.toxGroupSendCustomPrivatePacket(
		gp.group_number, gp.peer_number,
		true,
		pkg
	);
}

void CRDTNotesToxSync::SendOps(
	Contact3Handle c,
	const CRDTNotes::DocID& doc_id,
	const std::vector<CRDTNotes::Doc::Op>& ops
) {
	// ideally this is a file transfer/stream

	if (!c.all_of<Contact::Components::ToxGroupPeerEphemeral>()) {
		return;
	}

	std::vector<uint8_t> pkg;

	pkg.push_back(static_cast<uint8_t>(NGCEXT_Event::CRDTN_OPS));

	for (const uint8_t v : doc_id) {
		pkg.push_back(v);
	}

	// this is very inefficent
	// a full add op is 124bytes like this
	for (const auto& op : ops) {
		if(std::holds_alternative<CRDTNotes::Doc::OpAdd>(op)) {
			const auto& add_op = std::get<CRDTNotes::Doc::OpAdd>(op);
			pkg.push_back(0x00); // wasteful 1 byte for 1 bit

			for (const uint8_t v : add_op.id.id) {
				pkg.push_back(v);
			}

			for (size_t i = 0; i < sizeof(add_op.id.seq); i++) {
				pkg.push_back((add_op.id.seq >> i*8) & 0xff);
			}
			pkg.push_back(add_op.value); // what we actually care for

			if (add_op.parent_left.has_value()) {
				// exists
				pkg.push_back(0x01); // wasteful 1 byte for 1 bit
				for (const uint8_t v : add_op.parent_left.value().id) {
					pkg.push_back(v);
				}
				for (size_t i = 0; i < sizeof(add_op.parent_left.value().seq); i++) {
					pkg.push_back((add_op.parent_left.value().seq >> i*8) & 0xff);
				}
			} else {
				pkg.push_back(0x00); // wasteful 1 byte for 1 bit
			}

			if (add_op.parent_right.has_value()) {
				// exists
				pkg.push_back(0x01); // wasteful 1 byte for 1 bit
				for (const uint8_t v : add_op.parent_right.value().id) {
					pkg.push_back(v);
				}
				for (size_t i = 0; i < sizeof(add_op.parent_right.value().seq); i++) {
					pkg.push_back((add_op.parent_right.value().seq >> i*8) & 0xff);
				}
			} else {
				pkg.push_back(0x00); // wasteful 1 byte for 1 bit
			}
		} else if (std::holds_alternative<CRDTNotes::Doc::OpDel>(op)) {
			const auto& del_op = std::get<CRDTNotes::Doc::OpDel>(op);
			pkg.push_back(0x01); // wasteful 1 byte for 1 bit
			for (const uint8_t v : del_op.id.id) {
				pkg.push_back(v);
			}
			for (size_t i = 0; i < sizeof(del_op.id.seq); i++) {
				pkg.push_back((del_op.id.seq >> i*8) & 0xff);
			}
		}
	}

	// send
	const auto& gp = c.get<Contact::Components::ToxGroupPeerEphemeral>();
	_t.toxGroupSendCustomPrivatePacket(
		gp.group_number, gp.peer_number,
		true,
		pkg
	);
}


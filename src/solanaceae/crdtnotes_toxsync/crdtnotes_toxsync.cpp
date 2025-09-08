#include "./crdtnotes_toxsync.hpp"

#include <solanaceae/contact/contact_store_i.hpp>
#include <solanaceae/toxcore/tox_interface.hpp>

#include <solanaceae/tox_contacts/components.hpp>

#include <iostream>

// TODO: really need a good way to manage/negotiate packet ids
enum class NGCEXT_Event : uint8_t {
	// - DocID
	CRDTN_GOSSIP = 0x80 | 0x10,

	// - DocID
	// - array [
	//   - AgentID
	//   - seq (frontier)
	//   - del_num
	// - ]
	CRDTN_GOSSIP_FRONTIER,

	// - DocID
	CRDTN_FETCH_COMPLETE_FRONTIER,

	// - DocID
	// - AgentID
	// - seq_from
	// - seq_to
	CRDTN_FETCH_ADD_RANGE,

	// - DocID
	// - AgentID
	CRDTN_FETCH_DEL,

	// - DocID
	// - array [
	//   - seq
	//   - action date
	// - ]
	CRDTN_OPS,
};

#define _DATA_HAVE(x, error) if ((data_size - curser) < (x)) { error }

CRDTNotesToxSync::CRDTNotesToxSync(
	CRDTNotesEventI& notes_sync,
	ContactStore4I& cs,
	ToxI& t,
	ToxEventProviderI& tep,
	ToxContactModel2& tcm
) : _notes_sync(notes_sync), _cs(cs), _t(t), _tep_sr(tep.newSubRef(this)), _tcm(tcm) {
	// TODO: non groups

	// should be called for every peer (except self)
	// we hook here to inject ourself as contact sync model
	_tep_sr
		.subscribe(Tox_Event_Type::TOX_EVENT_GROUP_PEER_JOIN)

		.subscribe(Tox_Event_Type::TOX_EVENT_GROUP_CUSTOM_PACKET)
		.subscribe(Tox_Event_Type::TOX_EVENT_GROUP_CUSTOM_PRIVATE_PACKET)
	;
}

CRDTNotesToxSync::~CRDTNotesToxSync(void) {
	// TODO: find a better way to remove dangling pointers
	std::vector<Contact4> to_remove_self;
	_cs.registry().view<CRDTNotesContactSyncModelI*>().each([&to_remove_self, this](Contact4 c, const auto* csm) {
		if (this == csm) {
			to_remove_self.push_back(c);
		}
	});
	_cs.registry().remove<CRDTNotesContactSyncModelI*>(to_remove_self.cbegin(), to_remove_self.cend());
}

float CRDTNotesToxSync::iterate(float time_delta) {
	// TODO: do i actually need this?, logic is somewhere else and this is reactive only
	return 1.f; // TODO: 1sec for now, needs better logic
}

void CRDTNotesToxSync::SendGossip(
	ContactHandle4 c,
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
	ContactHandle4 c,
	const CRDTNotes::DocID& doc_id,
	const std::vector<CRDTNotes::Frontier>& frontier
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

	for (const auto& [f_id, f_seq, del_num] : frontier) {
		for (const uint8_t v : f_id) {
			pkg.push_back(v);
		}
		// +32

		for (size_t i = 0; i < sizeof(f_seq); i++) {
			pkg.push_back((f_seq >> i*8) & 0xff);
		}
		// +8

		for (size_t i = 0; i < sizeof(del_num); i++) {
			pkg.push_back((del_num >> i*8) & 0xff);
		}
		// +8
	}
	// +48

	// send
	const auto& gp = c.get<Contact::Components::ToxGroupPeerEphemeral>();
	_t.toxGroupSendCustomPrivatePacket(
		gp.group_number, gp.peer_number,
		true,
		pkg
	);
}

void CRDTNotesToxSync::SendFetchCompleteFrontier(
	ContactHandle4 c,
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

void CRDTNotesToxSync::SendFetchAddRange(
	ContactHandle4 c,
	const CRDTNotes::DocID& doc_id,
	const CRDTNotes::CRDTAgent& agent,
	const uint64_t seq_from,
	const uint64_t seq_to
) {
	if (!c.all_of<Contact::Components::ToxGroupPeerEphemeral>()) {
		return;
	}

	std::vector<uint8_t> pkg;

	pkg.push_back(static_cast<uint8_t>(NGCEXT_Event::CRDTN_FETCH_ADD_RANGE));

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

void CRDTNotesToxSync::SendFetchDel(
	ContactHandle4 c,
	const CRDTNotes::DocID& doc_id,
	const CRDTNotes::CRDTAgent& agent
) {
	if (!c.all_of<Contact::Components::ToxGroupPeerEphemeral>()) {
		return;
	}

	std::vector<uint8_t> pkg;

	pkg.push_back(static_cast<uint8_t>(NGCEXT_Event::CRDTN_FETCH_DEL));

	for (const uint8_t v : doc_id) {
		pkg.push_back(v);
	}

	for (const uint8_t v : agent) {
		pkg.push_back(v);
	}

	const auto& gp = c.get<Contact::Components::ToxGroupPeerEphemeral>();
	_t.toxGroupSendCustomPrivatePacket(
		gp.group_number, gp.peer_number,
		true,
		pkg
	);
}

void CRDTNotesToxSync::SendOps(
	ContactHandle4 c,
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
	// a full del op is 41bytes
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

bool CRDTNotesToxSync::parse_crdtn_gossip(
	ContactHandle4 c,
	const uint8_t* data, size_t data_size,
	bool // dont care private
) {
	Events::NGCEXT_crdtns_gossip e;
	e.c = c;

	size_t curser = 0;

	_DATA_HAVE(e.doc_id.size() * sizeof(decltype(e.doc_id)::value_type), std::cerr << "NGCEXT: packet too small, missing doc_id\n"; return false;)
	for (size_t i = 0; i < e.doc_id.size(); i++, curser++) {
		e.doc_id[i] = data[curser];
	}

	//return dispatch(
		//NGCEXT_Event::FT1_REQUEST,
		//e
	//);

	std::cout << "CRDTN gossip parsed\n";
	_notes_sync.onCRDTNSyncEvent(std::move(e));
	return true;
}

bool CRDTNotesToxSync::parse_crdtn_gossip_frontier(
	ContactHandle4 c,
	const uint8_t* data, size_t data_size,
	bool // dont care private
) {
	Events::NGCEXT_crdtns_gossip_frontier e;
	e.c = c;

	size_t curser = 0;

	_DATA_HAVE(e.doc_id.size() * sizeof(decltype(e.doc_id)::value_type), std::cerr << "NGCEXT: packet too small, missing doc_id\n"; return false;)
	for (size_t i = 0; i < e.doc_id.size(); i++, curser++) {
		e.doc_id[i] = data[curser];
	}

	// expect remaining size to be a multiple of agentid+seq

	while (curser < data_size) {
		CRDTNotes::Frontier new_f;
		_DATA_HAVE(new_f.agent.size() * sizeof(CRDTNotes::CRDTAgent::value_type) + sizeof(new_f.seq) + sizeof(new_f.del_num), std::cerr << "NGCEXT: packet malformed, not enough data for frontier\n"; return false;)

		for (size_t i = 0; i < new_f.agent.size(); i++, curser++) {
			new_f.agent[i] = data[curser];
		}

		new_f.seq = 0;
		for (size_t i = 0; i < sizeof(new_f.seq); i++, curser++) {
			new_f.seq |= uint64_t(data[curser]) << i*8;
		}

		new_f.del_num = 0;
		for (size_t i = 0; i < sizeof(new_f.del_num); i++, curser++) {
			new_f.del_num |= uint64_t(data[curser]) << i*8;
		}

		e.frontier.emplace_back(std::move(new_f));
	}

	std::cout << "CRDTN gossip_frontier parsed\n";
	_notes_sync.onCRDTNSyncEvent(std::move(e));
	return true;
}

bool CRDTNotesToxSync::parse_crdtn_fetch_complete_frontier(
	ContactHandle4 c,
	const uint8_t* data, size_t data_size,
	bool // dont care private
) {
	Events::NGCEXT_crdtns_fetch_complete_frontier e;
	e.c = c;

	size_t curser = 0;

	_DATA_HAVE(e.doc_id.size() * sizeof(decltype(e.doc_id)::value_type), std::cerr << "NGCEXT: packet too small, missing doc_id\n"; return false;)
	for (size_t i = 0; i < e.doc_id.size(); i++, curser++) {
		e.doc_id[i] = data[curser];
	}

	std::cout << "CRDTN fetch_complete_frontier parsed\n";
	_notes_sync.onCRDTNSyncEvent(std::move(e));
	return true;
}

bool CRDTNotesToxSync::parse_crdtn_fetch_add_range(
	ContactHandle4 c,
	const uint8_t* data, size_t data_size,
	bool // dont care private
) {
	Events::NGCEXT_crdtns_fetch_add_range e;
	e.c = c;

	size_t curser = 0;

	_DATA_HAVE(e.doc_id.size() * sizeof(decltype(e.doc_id)::value_type), std::cerr << "NGCEXT: packet too small, missing doc_id\n"; return false;)
	for (size_t i = 0; i < e.doc_id.size(); i++, curser++) {
		e.doc_id[i] = data[curser];
	}

	_DATA_HAVE(e.agent.size() * sizeof(decltype(e.agent)::value_type), std::cerr << "NGCEXT: packet too small, missing agent\n"; return false;)
	for (size_t i = 0; i < e.agent.size(); i++, curser++) {
		e.agent[i] = data[curser];
	}

	_DATA_HAVE(sizeof(e.seq_from), std::cerr << "NGCEXT: packet too small, missing seq_from\n"; return false;)
	e.seq_from = 0;
	for (size_t i = 0; i < sizeof(e.seq_from); i++, curser++) {
		e.seq_from |= uint64_t(data[curser]) << i*8;
	}

	_DATA_HAVE(sizeof(e.seq_to), std::cerr << "NGCEXT: packet too small, missing seq_to\n"; return false;)
	e.seq_to = 0;
	for (size_t i = 0; i < sizeof(e.seq_to); i++, curser++) {
		e.seq_to |= uint64_t(data[curser]) << i*8;
	}

	std::cout << "CRDTN fetch_add_range parsed\n";
	_notes_sync.onCRDTNSyncEvent(std::move(e));
	return true;
}

bool CRDTNotesToxSync::parse_crdtn_fetch_del(
	ContactHandle4 c,
	const uint8_t* data, size_t data_size,
	bool // dont care private
) {
	Events::NGCEXT_crdtns_fetch_del e;
	e.c = c;

	size_t curser = 0;

	_DATA_HAVE(e.doc_id.size() * sizeof(decltype(e.doc_id)::value_type), std::cerr << "NGCEXT: packet too small, missing doc_id\n"; return false;)
	for (size_t i = 0; i < e.doc_id.size(); i++, curser++) {
		e.doc_id[i] = data[curser];
	}

	_DATA_HAVE(e.agent.size() * sizeof(decltype(e.agent)::value_type), std::cerr << "NGCEXT: packet too small, missing agent\n"; return false;)
	for (size_t i = 0; i < e.agent.size(); i++, curser++) {
		e.agent[i] = data[curser];
	}

	std::cout << "CRDTN fetch_del parsed\n";
	_notes_sync.onCRDTNSyncEvent(std::move(e));
	return true;
}

bool CRDTNotesToxSync::parse_crdtn_ops(
	ContactHandle4 c,
	const uint8_t* data, size_t data_size,
	bool // dont care private
) {
	Events::NGCEXT_crdtns_ops e;
	e.c = c;

	size_t curser = 0;

	_DATA_HAVE(e.doc_id.size() * sizeof(decltype(e.doc_id)::value_type), std::cerr << "NGCEXT: packet too small, missing doc_id\n"; return false;)
	for (size_t i = 0; i < e.doc_id.size(); i++, curser++) {
		e.doc_id[i] = data[curser];
	}

	// an op is atleast 1+32+8 (41) (del)
	while (curser < data_size) {
		_DATA_HAVE(1, std::cerr << "NGCEXT: packet too small, missing optype\n"; return false;)
		const uint8_t op_type = data[curser++];
		if (op_type == 0x00) { // op add
			CRDTNotes::Doc::OpAdd new_op;

			_DATA_HAVE(new_op.id.id.size() * sizeof(decltype(new_op.id.id)::value_type), std::cerr << "NGCEXT: packet malformed, agent for op to small\n"; return false;)
			for (size_t i = 0; i < new_op.id.id.size(); i++, curser++) {
				new_op.id.id[i] = data[curser];
			}

			_DATA_HAVE(sizeof(new_op.id.seq), std::cerr << "NGCEXT: packet malformed, missing seq for op\n"; return false;)
			new_op.id.seq = 0;
			for (size_t i = 0; i < sizeof(new_op.id.seq); i++, curser++) {
				new_op.id.seq |= uint64_t(data[curser]) << i*8;
			}

			_DATA_HAVE(sizeof(new_op.value), std::cerr << "NGCEXT: packet malformed, missing value for op\n"; return false;)
			static_assert(sizeof(new_op.value) == 1);
			new_op.value = data[curser++];

			_DATA_HAVE(1, std::cerr << "NGCEXT: packet too small, missing has_parent_left\n"; return false;)
			const uint8_t has_parent_left = data[curser++];
			if (has_parent_left == 0x01) { // TODO: test for other values
				new_op.parent_left.emplace();
				_DATA_HAVE(new_op.parent_left.value().id.size() * sizeof(decltype(new_op.parent_left.value().id)::value_type), std::cerr << "NGCEXT: packet malformed, agent for parent left for op to small\n"; return false;)
				for (size_t i = 0; i < new_op.parent_left.value().id.size(); i++, curser++) {
					new_op.parent_left.value().id[i] = data[curser];
				}

				_DATA_HAVE(sizeof(new_op.parent_left.value().seq), std::cerr << "NGCEXT: packet malformed, missing seq for parent_left for op\n"; return false;)
				new_op.parent_left.value().seq = 0;
				for (size_t i = 0; i < sizeof(new_op.parent_left.value().seq); i++, curser++) {
					new_op.parent_left.value().seq |= uint64_t(data[curser]) << i*8;
				}
			}

			_DATA_HAVE(1, std::cerr << "NGCEXT: packet too small, missing has_parent_right\n"; return false;)
			const uint8_t has_parent_right = data[curser++];
			if (has_parent_right == 0x01) { // TODO: test for other values
				new_op.parent_right.emplace();
				_DATA_HAVE(new_op.parent_right.value().id.size() * sizeof(decltype(new_op.parent_right.value().id)::value_type), std::cerr << "NGCEXT: packet malformed, agent for parent_right for op to small\n"; return false;)
				for (size_t i = 0; i < new_op.parent_right.value().id.size(); i++, curser++) {
					new_op.parent_right.value().id[i] = data[curser];
				}

				_DATA_HAVE(sizeof(new_op.parent_right.value().seq), std::cerr << "NGCEXT: packet malformed, missing seq for parent_right for op\n"; return false;)
				new_op.parent_right.value().seq = 0;
				for (size_t i = 0; i < sizeof(new_op.parent_right.value().seq); i++, curser++) {
					new_op.parent_right.value().seq |= uint64_t(data[curser]) << i*8;
				}
			}

			e.ops.emplace_back(std::move(new_op));
		} else if (op_type == 0x01) { // op del
			CRDTNotes::Doc::OpDel new_op;

			_DATA_HAVE(new_op.id.id.size() * sizeof(decltype(new_op.id.id)::value_type), std::cerr << "NGCEXT: packet malformed, agent for op to small\n"; return false;)
			for (size_t i = 0; i < new_op.id.id.size(); i++, curser++) {
				new_op.id.id[i] = data[curser];
			}

			_DATA_HAVE(sizeof(new_op.id.seq), std::cerr << "NGCEXT: packet malformed, missing seq for op\n"; return false;)
			new_op.id.seq = 0;
			for (size_t i = 0; i < sizeof(new_op.id.seq); i++, curser++) {
				new_op.id.seq |= uint64_t(data[curser]) << i*8;
			}

			e.ops.emplace_back(std::move(new_op));
		} else {
			std::cerr << "NGCEXT: packet malformed, unknown optype " << (int)op_type << "\n";
			return false;
		}
	}

	std::cout << "CRDTN ops parsed (count:" << e.ops.size() << ")\n";
	_notes_sync.onCRDTNSyncEvent(std::move(e));
	return true;
}

bool CRDTNotesToxSync::handlePacket(
	const uint32_t group_number,
	const uint32_t peer_number,
	const uint8_t* data,
	const size_t data_size,
	const bool _private
) {
	if (data_size < 1) {
		return false; // waht
	}

	// tcm id to c
	auto c = _tcm.getContactGroupPeer(group_number, peer_number);
	if (!c.valid()) {
		// HUH?
		return false;
	}

	NGCEXT_Event pkg_type = static_cast<NGCEXT_Event>(data[0]);

	switch (pkg_type) {
		case NGCEXT_Event::CRDTN_GOSSIP:
			return parse_crdtn_gossip(c, data+1, data_size-1, _private);
		case NGCEXT_Event::CRDTN_GOSSIP_FRONTIER:
			return parse_crdtn_gossip_frontier(c, data+1, data_size-1, _private);
		case NGCEXT_Event::CRDTN_FETCH_COMPLETE_FRONTIER:
			return parse_crdtn_fetch_complete_frontier(c, data+1, data_size-1, _private);
		case NGCEXT_Event::CRDTN_FETCH_ADD_RANGE:
			return parse_crdtn_fetch_add_range(c, data+1, data_size-1, _private);
		case NGCEXT_Event::CRDTN_FETCH_DEL:
			return parse_crdtn_fetch_del(c, data+1, data_size-1, _private);
		case NGCEXT_Event::CRDTN_OPS:
			return parse_crdtn_ops(c, data+1, data_size-1, _private);
		default:
			return false;
	}

	return false;
}

bool CRDTNotesToxSync::onToxEvent(const Tox_Event_Group_Peer_Join* e) {
	// TODO: replace with actual contact events
	const auto group_number = tox_event_group_peer_join_get_group_number(e);
	const auto peer_number = tox_event_group_peer_join_get_peer_id(e);

	// tcm id to c
	auto c = _tcm.getContactGroupPeer(group_number, peer_number);
	if (!c.valid()) {
		// HUH?
		return false;
	}

	// TODO: find a better way
	c.emplace_or_replace<CRDTNotesContactSyncModelI*>(this);

	return false;
}

bool CRDTNotesToxSync::onToxEvent(const Tox_Event_Group_Custom_Packet* e) {
	const auto group_number = tox_event_group_custom_packet_get_group_number(e);
	const auto peer_number = tox_event_group_custom_packet_get_peer_id(e);
	const uint8_t* data = tox_event_group_custom_packet_get_data(e);
	const auto data_length = tox_event_group_custom_packet_get_data_length(e);
	return handlePacket(group_number, peer_number, data, data_length, false);
}

bool CRDTNotesToxSync::onToxEvent(const Tox_Event_Group_Custom_Private_Packet* e) {
	const auto group_number = tox_event_group_custom_private_packet_get_group_number(e);
	const auto peer_number = tox_event_group_custom_private_packet_get_peer_id(e);
	const uint8_t* data = tox_event_group_custom_private_packet_get_data(e);
	const auto data_length = tox_event_group_custom_private_packet_get_data_length(e);
	return handlePacket(group_number, peer_number, data, data_length, true);
}

#undef _DATA_HAVE


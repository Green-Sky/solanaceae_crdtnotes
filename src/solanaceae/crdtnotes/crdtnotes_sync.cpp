#include "./crdtnotes_sync.hpp"

void CRDTNotesSync::onCRDTNSyncEvent(Events::NGCEXT_crdtns_gossip&& e) {
}

void CRDTNotesSync::onCRDTNSyncEvent(Events::NGCEXT_crdtns_gossip_frontier&& e) {
}

void CRDTNotesSync::onCRDTNSyncEvent(Events::NGCEXT_crdtns_fetch_complete_frontier&& e) {
}

void CRDTNotesSync::onCRDTNSyncEvent(Events::NGCEXT_crdtns_fetch_op_range&& e) {
}

void CRDTNotesSync::onCRDTNSyncEvent(Events::NGCEXT_crdtns_ops&& e) {
}


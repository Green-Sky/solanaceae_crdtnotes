#pragma once

#include <solanaceae/crdtnotes/crdtnotes_sync.hpp>
#include <solanaceae/contact/fwd.hpp>

#include <set>
#include <unordered_set>

class CRDTNotesImGui {
	CRDTNotes& _notes;
	CRDTNotesSync& _notes_sync;
	ContactStore4I& _cs;

	bool _show_global_list {true};

	std::set<CRDTNotes::DocID> _open_docs;
	std::unordered_set<CRDTNotes::DocWriteLock> _held_locks;

	std::unordered_set<CRDTNotes::DocWriteLock>::iterator findLock(const CRDTNotes::DocID& doc_id);

	public:
		CRDTNotesImGui(CRDTNotes& notes, CRDTNotesSync& notes_sync, ContactStore4I& cs);

		float render(void);

		bool renderContactListContactSmall(const Contact4 c, const bool selected) const;

		bool renderDoc(const CRDTNotes::DocID& doc_id);
};


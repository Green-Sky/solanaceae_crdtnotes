#pragma once

#include <solanaceae/crdtnotes/crdtnotes_sync.hpp>
#include <solanaceae/contact/fwd.hpp>

#include <set>

class CRDTNotesImGui {
	CRDTNotes& _notes;
	CRDTNotesSync& _notes_sync;
	ContactStore4I& _cs;

	bool _show_global_list {true};

	std::set<CRDTNotes::DocID> _open_docs;

	public:
		CRDTNotesImGui(CRDTNotes& notes, CRDTNotesSync& notes_sync, ContactStore4I& cs);

		float render(void);

		bool renderContactListContactSmall(const Contact4 c, const bool selected) const;

		bool renderDoc(const CRDTNotes::DocID& doc_id);
		bool renderDocText(std::string& text) const;
};


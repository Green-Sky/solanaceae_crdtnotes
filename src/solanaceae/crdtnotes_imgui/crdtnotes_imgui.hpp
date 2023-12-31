#pragma once

#include <solanaceae/crdtnotes/crdtnotes_sync.hpp>
#include <solanaceae/contact/contact_model3.hpp>

#include <set>

class CRDTNotesImGui {
	CRDTNotesSync& _notes_sync;
	Contact3Registry& _cr;

	bool _show_global_list {true};

	std::set<CRDTNotes::DocID> _open_docs;

	public:
		CRDTNotesImGui(CRDTNotesSync& notes_sync, Contact3Registry& cr);

		float render(void);

		bool renderContactListContactSmall(const Contact3 c, const bool selected) const;

		bool renderDoc(const CRDTNotes::DocID& doc_id);
		bool renderDocText(std::string& text) const;
};


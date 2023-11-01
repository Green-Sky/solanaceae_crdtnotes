#pragma once

#include <solanaceae/crdtnotes/crdtnotes.hpp>

class CRDTNotesImGui {
	CRDTNotes& _notes;

	bool _show_global_list {true};

	public:
		CRDTNotesImGui(CRDTNotes& notes);

		float render(void);
};


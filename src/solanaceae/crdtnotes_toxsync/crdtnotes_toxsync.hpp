#pragma once

#include <solanaceae/crdtnotes/crdtnotes.hpp>
#include <solanaceae/contact/contact_model3.hpp>

class CRDTNotesToxSync {
	CRDTNotes& _notes;
	Contact3Registry& _cr;

	public:
		CRDTNotesToxSync(CRDTNotes& notes, Contact3Registry& cr);

		float iterate(void);
};


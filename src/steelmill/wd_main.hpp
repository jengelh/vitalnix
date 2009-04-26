#ifndef STEELMILL_WD_MAIN_HPP
#define STEELMILL_WD_MAIN_HPP

#include <wx/wxprec.h>
#ifndef WD_PRECOMP
#	include <wx/wx.h>
#endif
#include "steelmill/xu_common.hpp"

class WD_MainMenu : public wxFrame {
    public: /* functions */
	WD_MainMenu(const char *, const wxSize & = wxDSIZE,
	    const wxPoint & = wxDPOS);
	~WD_MainMenu(void);

    private: /* functions */
	void About(wxCommandEvent &);
	void Add_Single(wxCommandEvent &);
	void Configure(wxCommandEvent &);
	void FixUUID(wxCommandEvent &);
	void Exit(wxCommandEvent &);
	void Help(wxCommandEvent &);
	void Pwl_Format(wxCommandEvent &);
	void Show_Console(wxCommandEvent &);
	void Sync(wxCommandEvent &);
	void View_Groups(wxCommandEvent &);
	void View_Users(wxCommandEvent &);
	void lpcadm(wxCommandEvent &);

    private: /* variables */
	wxFlexGridSizer *generate_menu(void);

	DECLARE_EVENT_TABLE();
	DECLARE_NO_COPY_CLASS(WD_MainMenu);
};

#endif /* STEELMILL_WD_MAIN_HPP */

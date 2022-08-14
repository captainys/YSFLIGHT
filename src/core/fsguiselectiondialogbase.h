#ifndef fsguiselectiondialogbase_IS_INCLUDED
#define fsguiselectiondialogbase_IS_INCLUDED
/* { */

#include "fsguiinfltdlg.h"

class FsGuiDialogWithFieldAndAircraft : public FsGuiInFlightDialog
{
public:
	YsScenery *scn;
	YSBOOL stpLoaded;
	YsVec3 stpPos;
	YsAtt3 stpAtt;

	YSBOOL useMapPreview;

	FsGuiDialogWithFieldAndAircraft();
	~FsGuiDialogWithFieldAndAircraft();

	void ReloadField(FsWorld *world,const char fieldName[]);
	void ReloadStartPos(FsWorld *world,const char fieldName[],const char stpName[]);
private:
	void MakeMapVisibleDistanceAllInfinity(void);

public:
	void DrawAirplane(FsWorld *world,const char typeName[],const YsAtt3 &att,YSSIZE_T nWpnCfg,const int *wpnCfg,YSBOOL smallMode) const;
	void DrawField(FsWorld *world,YSBOOL drawStp,YSBOOL smallMode,YSBOOL drawCursor,const YsVec3 &cursorPos) const;

	/*! ResetFieldListBySearchKeyword populates the field list.  It preserves the current selection.  It also calls FieldSelectionChanged
	    if the selection changes because of the search keyword.
	*/
	void ResetFieldListBySearchKeyword(FsWorld *world,FsGuiListBox *fieldList,FsGuiTextBox *fieldSearch,YSBOOL forRacingMode);
	virtual void FieldSelectionChanged(void);  // Maybe called from inside ResetFieldListBySearchKeyword();

	/*! PopulateFieldList populates the fieldList, but does not call FieldSelectionChanged, nor re-select what was selected before.
	    It is called from ResetFieldListBySearchKeyword.
	    Also can be used for initializing the field selector.
	    If no keyword is needed, keyword can be "" or nullptr.
	*/
	void PopulateFieldList(FsWorld *world,FsGuiListBox *fieldList,const char keyword[],YSBOOL forRacingMode);
};


/* } */
#endif

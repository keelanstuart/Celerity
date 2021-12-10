/*
	Copyright © 2013-2020, Keelan Stuart (hereafter referenced as AUTHOR). All Rights Reserved.
	Permission to use, copy, modify, and distribute this software is hereby granted, without fee and without a signed licensing agreement,
	provided that the above copyright notice appears in all copies, modifications, and distributions.
	Furthermore, AUTHOR assumes no responsibility for any damages caused either directly or indirectly by the use of this software, nor vouches for
	any fitness of purpose of this software.
	All other copyrighted material contained herein is noted and rights attributed to individual copyright holders.
	
	For inquiries, contact: keelanstuart@gmail.com
*/

#include "wtfpropertygridctrl.h"

#pragma once


// CPropertyGrid

class CPropertyGrid : public CWTFPropertyGridCtrl
{
	DECLARE_DYNAMIC(CPropertyGrid)

public:
	CPropertyGrid();
	virtual ~CPropertyGrid();

	typedef const TCHAR *(*PROPERTY_DESCRIPTION_CB)(props::FOURCHARCODE property_id);
	typedef const TCHAR *(*FILE_FILTER_CB)(props::FOURCHARCODE property_id);

	void SetActiveProperties(props::IPropertySet *props, PROPERTY_DESCRIPTION_CB prop_desc = nullptr, FILE_FILTER_CB file_filter = nullptr, bool reset = true);

	CWTFPropertyGridProperty *FindItemByName(const TCHAR *name, CWTFPropertyGridProperty *top = nullptr);

	virtual void OnClickButton(CPoint point);

	bool IsLocked() { return m_bLocked; }

protected:
	props::IPropertySet *m_Props;
	bool m_bLocked;

	DECLARE_MESSAGE_MAP()

public:
	virtual void OnPropertyChanged(CWTFPropertyGridProperty* pProp);

};



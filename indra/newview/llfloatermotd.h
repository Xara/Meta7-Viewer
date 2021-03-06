/** 
 * @file llfloaterlagmeter.h
 * @brief The "Lag-o-Meter" floater used to tell users what is causing lag.
 *
 * $LicenseInfo:firstyear=2007&license=viewergpl$
 * 
 * Copyright (c) 2007-2009, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlifegrid.net/programs/open_source/licensing/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at
 * http://secondlifegrid.net/programs/open_source/licensing/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 */

#ifndef LLFLOATERMOTD_H
#define LLFLOATERMOTD_H

#include "llfloater.h"

class LLFloaterMOTD : public LLFloater, public LLFloaterSingleton<LLFloaterMOTD>
{
	friend class LLUISingleton<LLFloaterMOTD, VisibilityPolicy<LLFloater> >;
	
public:
	/*virtual*/ void draw();
	virtual BOOL postBuild();
	static void onBtnOK( void* userdata );

private:
	LLFloaterMOTD(const LLSD& key);
	/*virtual*/ ~LLFloaterMOTD();

	LLTextBox * mMOTDText;
	LLStringUtil::format_map_t mStringArgs;
};

#endif

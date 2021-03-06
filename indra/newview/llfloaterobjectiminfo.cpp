/** 
 * @file llfloaterobjectiminfo.cpp
 * @brief A floater with information about an object that sent an IM.
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

#include "llviewerprecompiledheaders.h"

#include "llfloaterobjectiminfo.h"

#include "llagentdata.h"
#include "llcachename.h"
#include "llcommandhandler.h"
#include "llfloater.h"
#include "llfloateravatarinfo.h"
#include "llfloatergroupinfo.h"
#include "llfloatermute.h"
#include "llmutelist.h"
#include "llsdutil.h"
#include "lluictrlfactory.h"
#include "llurldispatcher.h"
#include "llviewercontrol.h"
#include "lllandmark.h"
#include "llagent.h"
#include "llworldmap.h"
#include "llregionhandle.h"

// [RLVa:KB] - Version: 1.23.4 | Alternate: Meta7-370 | Checked: 2009-07-04 (RLVa-1.0.0a) | Added: RLVa-0.2.0g
#include "rlvhandler.h"
// [/RLVa:KB]

class LLFloaterObjectIMInfo;

class LLFloaterObjectIMInfoRegionHandleCallback : public LLRegionHandleCallback
{
private:
	bool mCallbackActive;
	LLFloaterObjectIMInfo *mFloater;

public:
	LLFloaterObjectIMInfoRegionHandleCallback(LLFloaterObjectIMInfo *floater) { mCallbackActive = TRUE; mFloater = floater; }
	virtual ~LLFloaterObjectIMInfoRegionHandleCallback() {}
	virtual bool dataReady(
		const LLUUID& region_id,
		const U64& region_handle);
	void detach() {
		mFloater = NULL;
		if(!mCallbackActive) delete this;
		//A true callback or event handler would support removing from the queue
		//as this is not possible we'll keep this object alife till dataReady is
		//called. Which may never happen and can lead into a memory leak.
	}
};

////////////////////////////////////////////////////////////////////////////
// LLFloaterObjectIMInfo
class LLFloaterObjectIMInfo : public LLFloater, public LLFloaterSingleton<LLFloaterObjectIMInfo>
{
public:
	LLFloaterObjectIMInfo(const LLSD& sd);
	virtual ~LLFloaterObjectIMInfo();

	BOOL postBuild(void);

	void update(const LLUUID& id, const std::string& name, const std::string& m7url, const LLUUID& owner, bool owner_is_group, const LLUUID& region_id, const std::string& localpart);

	void draw();

	// UI Handlers
	static void onClickMap(void* data);
	static void onClickOwner(void* data);
	static void onClickMute(void* data);

	static void nameCallback(const LLUUID& id, const std::string& first, const std::string& last, BOOL is_group, void* data);

	void regionhandle(const U64& region_handle);

private:
	LLUUID mObjectID;
	std::string mObjectName;
	std::string mM7URL;
	LLUUID mOwnerID;
	std::string mOwnerName;
	bool mOwnerIsGroup;
	U64 mRegionHandle;
	std::string mRegionName;
	bool lookingforRegion;
	std::string mPosPart;
	LLFloaterObjectIMInfoRegionHandleCallback *mCallback;
};

bool LLFloaterObjectIMInfoRegionHandleCallback::dataReady(const LLUUID &region_id, const U64 &region_handle)
{
	llinfos << "Resolved region_id " << region_id.asString() << " to handle " << region_handle << llendl;
	if(mFloater) {
		mFloater->regionhandle(region_handle);
		mCallbackActive = FALSE;
	} else {
		delete this;
	}
	return true;
}

void LLFloaterObjectIMInfo::regionhandle(const U64& region_handle)
{
	mRegionHandle = region_handle;
	if(LLWorldMap::getInstance()->simNameFromPosGlobal(from_region_handle(region_handle), mRegionName)) {
		lookingforRegion = FALSE;
		mM7URL = mRegionName + mPosPart;
		childSetVisible("Unknown_M7URL",false);
		childSetVisible("M7URL",true);
		childSetText("M7URL",mM7URL);
	} else {
		U32 global_x;
		U32 global_y;
		from_region_handle(region_handle, &global_x, &global_y);
		U16 grid_x = (U16)(global_x / REGION_WIDTH_UNITS);
		U16 grid_y = (U16)(global_y / REGION_WIDTH_UNITS);
		lookingforRegion = TRUE;
		llinfos << "region name for (" << grid_x << ", " << grid_y << ") not cached, requesting" << llendl;
		LLWorldMap::getInstance()->sendMapBlockRequest(grid_x, grid_y, grid_x, grid_y, true);
	}
}

void LLFloaterObjectIMInfo::draw()
{
	if(lookingforRegion) {
		if(LLWorldMap::getInstance()->simNameFromPosGlobal(from_region_handle(mRegionHandle), mRegionName)) {
			lookingforRegion = FALSE;
			mM7URL = mRegionName + mPosPart;
			childSetVisible("Unknown_M7URL",false);
			childSetVisible("M7URL",true);
			childSetText("M7URL",mM7URL);
		}
	}
	LLFloater::draw();
}

LLFloaterObjectIMInfo::~LLFloaterObjectIMInfo()
{
	if(mCallback) mCallback->detach();
}

LLFloaterObjectIMInfo::LLFloaterObjectIMInfo(const LLSD& seed)
: mObjectID(), mObjectName(), mM7URL(), mOwnerID(), mOwnerName(), mOwnerIsGroup(false), lookingforRegion(false), mRegionName()
{
	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_object_im_info.xml");
	
	if (getRect().mLeft == 0 
		&& getRect().mBottom == 0)
	{
		center();
	}

	mCallback = NULL;
}

BOOL LLFloaterObjectIMInfo::postBuild(void)
{
	childSetAction("Mute",onClickMute,this);
	childSetActionTextbox("OwnerName",onClickOwner, this);
	childSetActionTextbox("M7URL",onClickMap, this);

	return true;
}

void LLFloaterObjectIMInfo::update(const LLUUID& object_id, const std::string& name, const std::string& m7url, const LLUUID& owner_id, bool owner_is_group, const LLUUID &region_id, const std::string& localpart)
{
	// When talking to an old region we won't have a m7url.
	// The object id isn't really the object id either but we don't use it so who cares.
//	bool have_m7url = !m7url.empty();
// [RLVa:KB] - Version: 1.23.4 | Checked: 2009-07-04 (RLVa-1.0.0a) | Added: RLVa-0.2.0g
	bool have_m7url = (!m7url.empty()) && (!gRlvHandler.hasBehaviour(RLV_BHVR_SHOWLOC));
// [/RLVa:KB]
	childSetVisible("Unknown_M7URL",!have_m7url);
	childSetVisible("M7URL",have_m7url);

	childSetText("ObjectName",name);
	childSetText("M7URL",m7url);
	childSetText("OwnerName",std::string(""));

//	bool my_object = (owner_id == gAgentID);
// [RLVa:KB] - Version: 1.23.4 | Checked: 2009-07-08 (RLVa-1.0.0e) | Added: RLVa-0.2.0g
	bool my_object = (owner_id == gAgentID) || ((gRlvHandler.hasBehaviour(RLV_BHVR_SHOWNAMES)) && (gRlvHandler.isAgentNearby(owner_id)));
// [/RLVa:KB]
	childSetEnabled("Mute",!my_object);
	
	mObjectID = object_id;
	mObjectName = name;
	mM7URL = m7url;
	mOwnerID = owner_id;
	mOwnerIsGroup = owner_is_group;

	std::string tempname = name;

	size_t found = tempname.find(" ");
	while(found != std::string::npos)
	{
		tempname.replace(found, 1, "");
		found = tempname.find(" ");
	}

	if (tempname.length() < 1)
	{
		tempname = "no name";
//		chat.mFromName = name;
		childSetText("ObjectName",tempname);
	}

	if (gCacheName) gCacheName->get(owner_id,owner_is_group,nameCallback,this);

	// If we do not have m7url, try resolving using the region_id and localpos
	// (but don't resolve if there was a m7url but RLVa ate it because the avie isn't supposed to be able to see where they are -- Kitty)
	//if(!have_m7url) {
	if (m7url.empty()) {
		mPosPart = localpart;
		mCallback = new LLFloaterObjectIMInfoRegionHandleCallback(this);
		LLLandmark::requestRegionHandle(gMessageSystem, gAgent.getRegionHost(), region_id, mCallback);
		llinfos << "URL had no m7url, resolving region_id " << region_id.asString() << llendl;
	}
}

//static 
void LLFloaterObjectIMInfo::onClickMap(void* data)
{
	LLFloaterObjectIMInfo* self = (LLFloaterObjectIMInfo*)data;

	std::ostringstream link;
	link << "meta7://" << self->mM7URL;
	LLURLDispatcher::dispatch(link.str(), NULL, true);
}

//static 
void LLFloaterObjectIMInfo::onClickOwner(void* data)
{
	LLFloaterObjectIMInfo* self = (LLFloaterObjectIMInfo*)data;
	if (self->mOwnerIsGroup)
	{
		LLFloaterGroupInfo::showFromUUID(self->mOwnerID);
	}
//	else
// [RLVa:KB] - Version: 1.23.4 | Checked: 2009-07-08 (RLVa-1.0.0e) | Added: RLVa-0.2.0g
	else if ( (!gRlvHandler.hasBehaviour(RLV_BHVR_SHOWNAMES)) || (!gRlvHandler.isAgentNearby(self->mOwnerID)) )
// [/RLVa:KB]
	{
		LLFloaterAvatarInfo::showFromObject(self->mOwnerID);
	}
}

//static 
void LLFloaterObjectIMInfo::onClickMute(void* data)
{
	LLFloaterObjectIMInfo* self = (LLFloaterObjectIMInfo*)data;

	LLMute::EType mute_type = (self->mOwnerIsGroup) ? LLMute::GROUP : LLMute::AGENT;
// [RLVa:KB] - Version: 1.23.4 | Checked: 2009-07-08 (RLVa-1.0.0e) | Added: RLVa-0.2.0g
	if ( (LLMute::GROUP != mute_type) && (gRlvHandler.hasBehaviour(RLV_BHVR_SHOWNAMES)) && (gRlvHandler.isAgentNearby(self->mOwnerID)) )
	{
		return;
	}
// [/RLVa:KB]

	LLMute mute(self->mOwnerID, self->mOwnerName, mute_type);
	LLMuteList::getInstance()->add(mute);
	LLFloaterMute::showInstance();
	self->close();
}

//static 
void LLFloaterObjectIMInfo::nameCallback(const LLUUID& id, const std::string& first, const std::string& last, BOOL is_group, void* data)
{
	LLFloaterObjectIMInfo* self = (LLFloaterObjectIMInfo*)data;
	self->mOwnerName = first;
	if (!last.empty())
	{
		self->mOwnerName += " " + last;
	}

// [RLVa:KB] - Version: 1.23.4 | Checked: 2009-07-08 (RLVa-1.0.0e) | Added: RLVa-0.2.0g
	if ( (!is_group) && (gRlvHandler.hasBehaviour(RLV_BHVR_SHOWNAMES)) && (gRlvHandler.isAgentNearby(id)) )
	{
		self->mOwnerName = gRlvHandler.getAnonym(self->mOwnerName);
	}
// [/RLVa:KB]

	self->childSetText("OwnerName",self->mOwnerName);
}

////////////////////////////////////////////////////////////////////////////
// LLObjectIMInfo
void LLObjectIMInfo::show(const LLUUID &object_id, const std::string &name, const std::string &location, const LLUUID &owner_id, bool owner_is_group, const LLUUID &region_id, const std::string& localpart)
{
	LLFloaterObjectIMInfo* im_info_floater = LLFloaterObjectIMInfo::showInstance();
	im_info_floater->update(object_id,name,location,owner_id,owner_is_group,region_id,localpart);
}

////////////////////////////////////////////////////////////////////////////
// LLObjectIMInfoHandler
class LLObjectIMInfoHandler : public LLCommandHandler
{
public:
	LLObjectIMInfoHandler() : LLCommandHandler("objectim", true) { }

	bool handle(const LLSD& tokens, const LLSD& query_map, LLWebBrowserCtrl *);
};

// Creating the object registers with the dispatcher.
LLObjectIMInfoHandler gObjectIMHandler;

// ex. meta7:///app/objectim/9426adfc-9c17-8765-5f09-fdf19957d003?owner=a112d245-9095-4e9c-ace4-ffa31717f934&groupowned=true&m7url=ahern/123/123/123&name=Object
bool LLObjectIMInfoHandler::handle(const LLSD &tokens, const LLSD &query_map, LLWebBrowserCtrl *brws)
{
	LLUUID task_id = tokens[0].asUUID();
	std::string name = query_map["name"].asString();
	std::string m7url = query_map["m7url"].asString();
	LLUUID owner = query_map["owner"].asUUID();
	bool group_owned = query_map.has("groupowned");
	LLUUID region_id = query_map["regionid"].asUUID();
	std::string localpart = query_map["localpos"].asString();
	
	LLObjectIMInfo::show(task_id,name,m7url,owner,group_owned,region_id,localpart);

	return true;
}

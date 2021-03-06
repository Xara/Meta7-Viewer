// Copyright (c)2009 Thomas Shikami
// 
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
// 
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#ifndef TS_TSSTUFF_H
#define TS_TSSTUFF_H

#include "llfloater.h"
#include "llthread.h"
#include "lllineeditor.h"
#include "tslua.h"

#include <queue>

class TSLuaThread;

class TSStuff
{
public:
	TSStuff();
	~TSStuff();
	static void init();
	static TSStuff *getInstance();
	static void idle(void *data);
	static void cleanupClass();
	TSLuaThread *getThread() { return mThread; }
	void resetLua();

private:
	static TSStuff *sInstance;
	TSLuaThread *mThread;
	void doInit();
	void doCleanup();
};

class TSLuaAction
{
public:
	virtual ~TSLuaAction();
	virtual void run();
};

class TSLuaThread : public LLThread
{
public:
	TSLuaThread();
	~TSLuaThread();

public:
	void addCommand(const char *s); // Main Thrad
	void runActions(); // Main Thread
	void addAction(TSLuaAction *action); // Lua Thread
	TSLuaAction *getActionDelete(); // Lua Thread
	void syncAction(TSLuaAction *action); // Lua Thread
	void abort(); // Main Thread
	bool isAborted() { return mAborted; }

protected:
	bool runCondition();
	void run();

private:
	TSLua *mLua;
	typedef std::queue<TSLuaAction*> action_queue_t;
	action_queue_t mActionQueue;
	action_queue_t mActionDeleteQueue;
	TSLuaAction *mSyncAction;
	typedef std::queue<char*> command_queue_t;
	command_queue_t mCommandQueue;
	bool mExecuting;
	bool mAborted;
};

class LLFloaterLuaConsole : public LLFloater
{
public:
	LLFloaterLuaConsole();
	~LLFloaterLuaConsole();

	/*virtual*/ BOOL postBuild();
	/*virtual*/ void draw();
	/*virtual*/ void onClose(bool app_quitting = false);

	static LLFloaterLuaConsole* getInstance();
	static void show(void*);

	static bool isVisible() { if(sInstance) return (sInstance->getVisible() != FALSE); return false; }
	static void toggle(void*);

private:
	static void onClickSend(void*);
	static void onClickClear(void*);
	static void onClickAbort(void*);
	static void onClickReset(void*);

private:
	static LLFloaterLuaConsole *sInstance;
};

#endif

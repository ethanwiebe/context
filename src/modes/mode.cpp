#include "mode.h"

#include <assert.h>
#include <string.h>

// setting up mode names
#define MODE_DEF(i,name,x) #name,
const char* modeNameArray[] = {
	#include "modelist.h"
};
#undef MODE_DEF

// forward declare all mode classes
#define MODE_DEF(i,name,funcName) extern ModeBase* funcName(ContextEditor*);
#include "modelist.h"
#undef MODE_DEF

ModeIndex ModeNameToIndex(const char* name){
	constexpr ModeIndex count = (s32)(sizeof(modeNameArray)/sizeof(modeNameArray[0]));
	ModeIndex i = 0;
	while (i<count){
		if (strncmp(name,modeNameArray[i],16)==0)
			return i;
		++i;
	}
	
	return MODE_NOT_FOUND;
}

const char* ModeIndexToName(ModeIndex index){
	return modeNameArray[index];
}

// mode creation
#define MODE_DEF(index,name,funcName) case index: return funcName(ctx);
ModeBase* CreateMode(ModeIndex i,ContextEditor* ctx){
	switch(i){
		#include "modelist.h"
	}
	
	return nullptr;
}
#undef MODE_DEF


#include "mode.h"

#include <assert.h>
#include <string.h>

#define MODE_DEF(name,x,y) #name,
const char* modeNameArray[] = {
	#include "modelist.h"
};
#undef MODE_DEF

#define MODE_DEF(name,opArray,arraySize) extern ModeOption opArray[arraySize];
#include "modelist.h"
#undef MODE_DEF

struct ModeOptionEntry {
	ModeOption* modeOption;
	size_t size;
};

#define MODE_DEF(name,opArray,arraySize) {opArray,arraySize},
const ModeOptionEntry modeOptionArray[] = {
	#include "modelist.h"
};
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

const ModeOption* ModeIndexToOptionArray(ModeIndex i,size_t& size){
	size = modeOptionArray[i].size;
	return modeOptionArray[i].modeOption;
}


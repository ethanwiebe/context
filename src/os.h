#pragma once

#include "core.h"

#include "textbuffer.h"

class OSInterface {

public:
	virtual bool PathExists(std::string_view) const = 0;
	virtual bool PathIsFile(std::string_view) const = 0;
	virtual bool FileIsReadable(std::string_view) const = 0;
	virtual bool FileIsWritable(std::string_view) const = 0; // note: this creates the file 
															 // if it does not exist
	virtual s64 GetModifyTime(std::string_view) const {return 0;};
	
	virtual void AutocompletePath(std::string&) const {};

	virtual bool ReadFileIntoTextBuffer(std::string_view,Ref<TextBuffer>) const = 0;
	virtual bool WriteTextBufferIntoFile(std::string_view,Ref<TextBuffer>) const = 0;
};

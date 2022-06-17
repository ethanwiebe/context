#pragma once

#include "core.h"

#include "os.h"

inline void FixWindowsPath(std::string& path){
	size_t pos = path.find('\\');
	while (pos!=std::string::npos){
		path[pos] = '/';
		pos = path.find('\\');
	}
}

class WindowsOSImpl : public OSInterface {
public:
	bool PathExists(std::string_view) const override;
	bool PathIsFile(std::string_view) const override;
	bool FileIsReadable(std::string_view) const override;
	bool FileIsWritable(std::string_view) const override;
//	s64 GetModifyTime(std::string_view) const override;
	
//	void AutocompletePath(std::string&) const override;

	bool ReadFileIntoTextBuffer(std::string_view,Ref<TextBuffer>) const override;
	bool WriteTextBufferIntoFile(std::string_view,Ref<TextBuffer>) const override;
};
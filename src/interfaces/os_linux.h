#pragma once

#include "core.h"

#include "os.h"
#include "../logger.h"

#include <sys/stat.h>
#include <pwd.h>
#include <string_view>
#include <fstream>
#include <cstdio>

class LinuxOSImpl : public OSInterface {
public:
	bool PathExists(std::string_view) const override;
	bool PathIsFile(std::string_view) const override;
	bool FileIsReadable(std::string_view) const override;
	bool FileIsWritable(std::string_view) const override;
	
	void ListDir(const std::string&,std::vector<std::string>&) const override;
	
	s64 GetModifyTime(std::string_view) const override;
	
	bool PathsAreSame(std::string_view,std::string_view) const override;

	bool ReadFileIntoTextBuffer(std::string_view,Ref<TextBuffer>) const override;
	bool WriteTextBufferIntoFile(std::string_view,Ref<TextBuffer>) const override;
	
	std::string GetHomePath() const override;
};

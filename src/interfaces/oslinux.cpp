#include "oslinux.h"

bool LinuxOSImpl::PathExists(std::string_view path) const {
	struct stat buf;
	return stat(path.data(),&buf)==0;
}

bool LinuxOSImpl::PathIsFile(std::string_view path) const {
	struct stat buf;
	if (stat(path.data(),&buf)!=0)
		return false;

	return !S_ISDIR(buf.st_mode);
}

bool LinuxOSImpl::FileIsReadable(std::string_view path) const {
	std::ifstream file(path.data());
	return file.good();
}

bool LinuxOSImpl::FileIsWritable(std::string_view path) const {
	bool good;
	bool exists = PathExists(path);
	{
		std::ofstream file(path.data(),std::ios::out|std::ios::app);
		good = file.good();
	}
	if (!exists)
		std::remove(path.data());

	return good;
}

s64 LinuxOSImpl::GetModifyTime(std::string_view path) const {
	struct stat64 buf;
	if (stat64(path.data(),&buf)!=0)
		return 0;
	
	return buf.st_mtim.tv_sec;
}


bool LinuxOSImpl::ReadFileIntoTextBuffer(std::string_view path,Ref<TextBuffer> textBuffer) const {
	textBuffer->clear();

	std::fstream file(path.data(),std::ios::in);
	if (!file.good()) return false;

	std::string line;
	s32 index = 0;
	auto it = textBuffer->begin();
	while (std::getline(file,line)){
		logger << "line: " << index++ << line << "\n";
		textBuffer->InsertLine(it,line);
	}
	
	return true;
}

bool LinuxOSImpl::WriteTextBufferIntoFile(std::string_view path,Ref<TextBuffer> textBuffer) const {
	std::fstream file(path.data(),std::ios::out);
	if (!file.good()) return false;

	for (const auto& line : *textBuffer){
		file << line << "\n";
	}

	return true;
}

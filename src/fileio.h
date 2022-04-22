#pragma once

#include "core.h"

#include "textfile.h"

#include <fstream>
#include <string_view>


class FileManager {
public:
	static Ref<TextFile> ReadFile(const std::string& path);

	
};

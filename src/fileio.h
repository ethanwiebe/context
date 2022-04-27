#pragma once

#include "core.h"

#include "textbuffer.h"

#include <fstream>
#include <string_view>


class FileManager {
public:
	static Ref<TextBuffer> ReadFile(const std::string& path);

	
};

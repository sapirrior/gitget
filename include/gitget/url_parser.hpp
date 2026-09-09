#pragma once

#include "gitget/arguments.hpp"
#include <string>

namespace gitget::url_parser {

bool parse_git_url(const std::string& input_url, Arguments& args);

} // namespace gitget::url_parser

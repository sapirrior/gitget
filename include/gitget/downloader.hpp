#pragma once

#include "gitget/arguments.hpp"
#include <string>

namespace gitget {

void download_from_url(const std::string& url,
                       const std::string& provider_display_name,
                       const Arguments& args);

} // namespace gitget

#pragma once

#include <optional>
#include <string>

namespace gitget {

enum class ProviderType {
    Auto,
    GitHub,
    GitLab,
    Bitbucket,
    Codeberg,
    Raw
};

struct Arguments {
    std::string repo;
    std::string path;
    std::string branch{"main"};
    std::string url;
    std::optional<std::string> output;
    std::optional<std::string> token;
    ProviderType provider{ProviderType::Auto};
    std::string custom_provider_name;
};

} // namespace gitget

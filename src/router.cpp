#include "gitget/router.hpp"
#include "gitget/providers/github.hpp"
#include "gitget/providers/gitlab.hpp"
#include "gitget/providers/bitbucket.hpp"
#include "gitget/providers/codeberg.hpp"
#include "gitget/providers/raw.hpp"
#include "gitget/error.hpp"

namespace gitget::router {

void route(const Arguments& args) {
    switch (args.provider) {
        case ProviderType::Auto:
            if (!args.url.empty() || args.repo.rfind("http://", 0) == 0 || args.repo.rfind("https://", 0) == 0) {
                providers::raw::download_file(args);
            } else {
                providers::github::download_file(args);
            }
            break;
        case ProviderType::Raw:
            providers::raw::download_file(args);
            break;
        case ProviderType::GitHub:
            providers::github::download_file(args);
            break;
        case ProviderType::GitLab:
            providers::gitlab::download_file(args);
            break;
        case ProviderType::Bitbucket:
            providers::bitbucket::download_file(args);
            break;
        case ProviderType::Codeberg:
            providers::codeberg::download_file(args);
            break;
        default:
            throw ArgumentError("Unsupported provider requested: " + args.custom_provider_name);
    }
}

} // namespace gitget::router

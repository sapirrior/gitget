#include "gitget/providers/gitlab.hpp"
#include "gitget/downloader.hpp"
#include "gitget/error.hpp"

namespace gitget::providers::gitlab {

void download_file(const Arguments& args) {
    if (args.repo.empty()) {
        throw ArgumentError("Missing required flag: --repo <group/project>");
    }
    if (args.repo.find('/') == std::string::npos) {
        throw ArgumentError("Invalid GitLab project path: '" + args.repo + "'. Expected '<group/project>'.");
    }
    if (args.path.empty()) {
        throw ArgumentError("Missing required flag: --path <file/path>");
    }
    if (args.branch.empty()) {
        throw ArgumentError("Branch name cannot be empty");
    }

    const std::string url = "https://gitlab.com/" +
                            args.repo + "/-/raw/" +
                            args.branch + "/" +
                            args.path;

    download_from_url(url, "GitLab", args);
}

} // namespace gitget::providers::gitlab

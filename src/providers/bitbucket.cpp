#include "gitget/providers/bitbucket.hpp"
#include "gitget/downloader.hpp"
#include "gitget/error.hpp"

namespace gitget::providers::bitbucket {

void download_file(const Arguments& args) {
    if (args.repo.empty()) {
        throw ArgumentError("Missing required flag: --repo <workspace/repo_slug>");
    }
    if (args.repo.find('/') == std::string::npos) {
        throw ArgumentError("Invalid Bitbucket repository: '" + args.repo + "'. Expected '<workspace/repo_slug>'.");
    }
    if (args.path.empty()) {
        throw ArgumentError("Missing required flag: --path <file/path>");
    }
    if (args.branch.empty()) {
        throw ArgumentError("Branch name cannot be empty");
    }

    const std::string url = "https://bitbucket.org/" +
                            args.repo + "/raw/" +
                            args.branch + "/" +
                            args.path;

    download_from_url(url, "Bitbucket", args);
}

} // namespace gitget::providers::bitbucket

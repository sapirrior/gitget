#include "gitget/providers/github.hpp"
#include "gitget/downloader.hpp"
#include "gitget/error.hpp"

namespace gitget::providers::github {

void download_file(const Arguments& args) {
    if (args.repo.empty()) {
        throw ArgumentError("Missing required flag: --repo <owner/repo>");
    }
    if (args.repo.find('/') == std::string::npos) {
        throw ArgumentError("Invalid GitHub repository format: '" + args.repo + "'. Expected '<owner>/<repo>' (e.g. torvalds/linux).");
    }
    if (args.path.empty()) {
        throw ArgumentError("Missing required flag: --path <file/path>");
    }
    if (args.branch.empty()) {
        throw ArgumentError("Branch name cannot be empty");
    }

    const std::string url = "https://raw.githubusercontent.com/" +
                            args.repo + "/" +
                            args.branch + "/" +
                            args.path;

    download_from_url(url, "GitHub", args);
}

} // namespace gitget::providers::github

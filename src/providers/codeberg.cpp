#include "gitget/providers/codeberg.hpp"
#include "gitget/downloader.hpp"
#include "gitget/error.hpp"

namespace gitget::providers::codeberg {

void download_file(const Arguments& args) {
    if (args.repo.empty()) {
        throw ArgumentError("Missing required flag: --repo <owner/repo>");
    }
    if (args.repo.find('/') == std::string::npos) {
        throw ArgumentError("Invalid Codeberg repository: '" + args.repo + "'. Expected '<owner>/<repo>'.");
    }
    if (args.path.empty()) {
        throw ArgumentError("Missing required flag: --path <file/path>");
    }
    if (args.branch.empty()) {
        throw ArgumentError("Branch name cannot be empty");
    }

    const std::string url = "https://codeberg.org/" +
                            args.repo + "/raw/branch/" +
                            args.branch + "/" +
                            args.path;

    download_from_url(url, "Codeberg", args);
}

} // namespace gitget::providers::codeberg

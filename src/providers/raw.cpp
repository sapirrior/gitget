#include "gitget/providers/raw.hpp"
#include "gitget/downloader.hpp"
#include "gitget/error.hpp"

namespace gitget::providers::raw {

void download_file(const Arguments& args) {
    std::string download_url = args.url;

    if (download_url.empty()) {
        // Fallback: if url wasn't passed directly, build from repo/branch/path if available
        if (args.repo.rfind("http://", 0) == 0 || args.repo.rfind("https://", 0) == 0) {
            download_url = args.repo;
            if (!args.path.empty()) {
                if (download_url.back() != '/' && args.path.front() != '/') {
                    download_url += '/';
                }
                download_url += args.path;
            }
        } else {
            throw ArgumentError("Raw provider requires a full URL (via --url / -u or http(s) URL in --repo / positional argument)");
        }
    }

    download_from_url(download_url, "Raw", args);
}

} // namespace gitget::providers::raw

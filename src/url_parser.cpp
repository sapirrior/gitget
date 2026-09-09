#include "gitget/url_parser.hpp"

#include <regex>
#include <string>

namespace gitget::url_parser {

bool parse_git_url(const std::string& input_url, Arguments& args) {
    // 1. GitHub web blob URL:
    // https://github.com/{owner}/{repo}/blob/{branch}/{path}
    // or raw:
    // https://raw.githubusercontent.com/{owner}/{repo}/{branch}/{path}
    static const std::regex gh_blob_re(
        R"(^https?://github\.com/([^/]+)/([^/]+)/blob/([^/]+)/(.+)$)",
        std::regex::optimize
    );
    static const std::regex gh_raw_re(
        R"(^https?://raw\.githubusercontent\.com/([^/]+)/([^/]+)/([^/]+)/(.+)$)",
        std::regex::optimize
    );

    std::smatch match;
    if (std::regex_match(input_url, match, gh_blob_re) || std::regex_match(input_url, match, gh_raw_re)) {
        args.provider = ProviderType::GitHub;
        args.repo = match[1].str() + "/" + match[2].str();
        args.branch = match[3].str();
        args.path = match[4].str();
        return true;
    }

    // 2. GitLab web blob URL:
    // https://gitlab.com/{owner}/{repo}/-/blob/{branch}/{path}
    // or raw:
    // https://gitlab.com/{owner}/{repo}/-/raw/{branch}/{path}
    static const std::regex gl_blob_re(
        R"(^https?://gitlab\.com/(.+)/-/(?:blob|raw)/([^/]+)/(.+)$)",
        std::regex::optimize
    );
    if (std::regex_match(input_url, match, gl_blob_re)) {
        args.provider = ProviderType::GitLab;
        args.repo = match[1].str();
        args.branch = match[2].str();
        args.path = match[3].str();
        return true;
    }

    // 3. Bitbucket web src or raw URL:
    // https://bitbucket.org/{owner}/{repo}/src/{branch}/{path}
    // or raw:
    // https://bitbucket.org/{owner}/{repo}/raw/{branch}/{path}
    static const std::regex bb_re(
        R"(^https?://bitbucket\.org/([^/]+)/([^/]+)/(?:src|raw)/([^/]+)/(.+)$)",
        std::regex::optimize
    );
    if (std::regex_match(input_url, match, bb_re)) {
        args.provider = ProviderType::Bitbucket;
        args.repo = match[1].str() + "/" + match[2].str();
        args.branch = match[3].str();
        args.path = match[4].str();
        return true;
    }

    // 4. Codeberg / Forgejo / Gitea src or raw URL:
    // https://codeberg.org/{owner}/{repo}/src/branch/{branch}/{path}
    // or raw:
    // https://codeberg.org/{owner}/{repo}/raw/branch/{branch}/{path}
    static const std::regex cb_re(
        R"(^https?://codeberg\.org/([^/]+)/([^/]+)/(?:src|raw)/branch/([^/]+)/(.+)$)",
        std::regex::optimize
    );
    if (std::regex_match(input_url, match, cb_re)) {
        args.provider = ProviderType::Codeberg;
        args.repo = match[1].str() + "/" + match[2].str();
        args.branch = match[3].str();
        args.path = match[4].str();
        return true;
    }

    return false;
}

} // namespace gitget::url_parser

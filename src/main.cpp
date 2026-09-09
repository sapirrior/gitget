#include "gitget/arguments.hpp"
#include "gitget/error.hpp"
#include "gitget/router.hpp"
#include "gitget/url_parser.hpp"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

namespace {

std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return s;
}

gitget::ProviderType parse_provider(const std::string& input) {
    std::string p = to_lower(input);
    if (p == "github" || p == "gh") {
        return gitget::ProviderType::GitHub;
    } else if (p == "gitlab" || p == "gl") {
        return gitget::ProviderType::GitLab;
    } else if (p == "bitbucket" || p == "bb") {
        return gitget::ProviderType::Bitbucket;
    } else if (p == "codeberg" || p == "cb") {
        return gitget::ProviderType::Codeberg;
    } else if (p == "raw" || p == "direct" || p == "custom") {
        return gitget::ProviderType::Raw;
    }
    throw gitget::ArgumentError("Unknown provider '" + input + "'. Supported providers: github, gitlab, bitbucket, codeberg, raw.");
}

void print_usage(const char* prog_name) {
    std::cout << "Usage: " << prog_name << " -r <owner/repo> -p <file/path> [options]\n"
              << "       " << prog_name << " <git-url> [options]\n\n"
              << "Options:\n"
              << "  -r, --repo <owner/repo>      Target repository [required unless URL is given]\n"
              << "  -p, --path <file/path>       File path within repository\n"
              << "  -u, --url <url>              Direct raw or web Git file URL\n"
              << "  -t, --token <token>          Authentication token for private repositories\n"
              << "  -P, --provider <name>        Git provider: github, gitlab, bitbucket, codeberg, raw (default: auto)\n"
              << "  -b, --branch <branch>        Branch or tag name (default: main)\n"
              << "  -o, --output <file>          Destination file path (default: filename from path/url)\n"
              << "  -h, --help                   Show this help message\n\n"
              << "Environment Variables:\n"
              << "  GITHUB_TOKEN / GH_TOKEN      Token for private GitHub repos\n"
              << "  GITLAB_TOKEN / GL_TOKEN      Token for private GitLab repos\n"
              << "  BITBUCKET_TOKEN / BB_TOKEN   Token for private Bitbucket repos\n"
              << "  CODEBERG_TOKEN / CB_TOKEN    Token for private Codeberg repos\n";
}

void print_hint(const char* prog_name) {
    std::cerr << "Try '" << prog_name << " --help' for more information.\n";
}

bool is_flag(const std::string& str) {
    return str.rfind("-", 0) == 0 && str.size() > 1;
}

bool is_http_url(const std::string& str) {
    return str.rfind("http://", 0) == 0 || str.rfind("https://", 0) == 0;
}

} // namespace

int main(int argc, char* argv[]) {
    const char* prog_name = (argc > 0 && argv[0]) ? argv[0] : "gitget";
    std::vector<std::string> raw_args(argv, argv + argc);
    gitget::Arguments args;

    size_t index = 1;
    while (index < raw_args.size()) {
        const std::string& arg = raw_args[index];
        if (arg == "--repo" || arg == "-r") {
            if (index + 1 >= raw_args.size() || is_flag(raw_args[index + 1])) {
                std::cerr << "error: flag '" << arg << "' requires an argument (<owner/repo>)\n";
                print_hint(prog_name);
                return 1;
            }
            args.repo = raw_args[index + 1];
            index += 2;
        } else if (arg == "--path" || arg == "-p") {
            if (index + 1 >= raw_args.size() || is_flag(raw_args[index + 1])) {
                std::cerr << "error: flag '" << arg << "' requires an argument (<file/path>)\n";
                print_hint(prog_name);
                return 1;
            }
            args.path = raw_args[index + 1];
            index += 2;
        } else if (arg == "--url" || arg == "-u") {
            if (index + 1 >= raw_args.size() || is_flag(raw_args[index + 1])) {
                std::cerr << "error: flag '" << arg << "' requires an argument (<url>)\n";
                print_hint(prog_name);
                return 1;
            }
            args.url = raw_args[index + 1];
            index += 2;
        } else if (arg == "--token" || arg == "-t") {
            if (index + 1 >= raw_args.size() || is_flag(raw_args[index + 1])) {
                std::cerr << "error: flag '" << arg << "' requires an argument (<token>)\n";
                print_hint(prog_name);
                return 1;
            }
            args.token = raw_args[index + 1];
            index += 2;
        } else if (arg == "--provider" || arg == "-P") {
            if (index + 1 >= raw_args.size() || is_flag(raw_args[index + 1])) {
                std::cerr << "error: flag '" << arg << "' requires an argument (<provider>)\n";
                print_hint(prog_name);
                return 1;
            }
            try {
                args.provider = parse_provider(raw_args[index + 1]);
                args.custom_provider_name = raw_args[index + 1];
            } catch (const gitget::ArgumentError& ex) {
                std::cerr << "error: " << ex.what() << "\n";
                print_hint(prog_name);
                return 1;
            }
            index += 2;
        } else if (arg == "--branch" || arg == "-b") {
            if (index + 1 >= raw_args.size() || is_flag(raw_args[index + 1])) {
                std::cerr << "error: flag '" << arg << "' requires an argument (<branch>)\n";
                print_hint(prog_name);
                return 1;
            }
            args.branch = raw_args[index + 1];
            index += 2;
        } else if (arg == "--output" || arg == "-o") {
            if (index + 1 >= raw_args.size() || is_flag(raw_args[index + 1])) {
                std::cerr << "error: flag '" << arg << "' requires an argument (<file>)\n";
                print_hint(prog_name);
                return 1;
            }
            args.output = raw_args[index + 1];
            index += 2;
        } else if (arg == "--help" || arg == "-h") {
            print_usage(prog_name);
            return 0;
        } else if (is_http_url(arg) && args.url.empty() && args.repo.empty()) {
            args.url = arg;
            index++;
        } else {
            std::cerr << "error: unrecognized option '" << arg << "'\n";
            print_hint(prog_name);
            return 1;
        }
    }

    // Auto-parse web or raw git URLs into provider, repo, branch, path
    if (!args.url.empty()) {
        std::string original_url = args.url;
        if (gitget::url_parser::parse_git_url(original_url, args)) {
            // URL parsed successfully into structured provider arguments
            args.url.clear();
        }
    }

    try {
        gitget::router::route(args);
    } catch (const gitget::ArgumentError& ex) {
        std::cerr << "error: " << ex.what() << "\n";
        print_hint(prog_name);
        return 1;
    } catch (const gitget::HttpError& ex) {
        std::cerr << "error: " << ex.what() << "\n";
        return 2;
    } catch (const gitget::NetworkError& ex) {
        std::cerr << "error: " << ex.what() << "\n";
        return 3;
    } catch (const gitget::FileSystemError& ex) {
        std::cerr << "error: " << ex.what() << "\n";
        return 4;
    } catch (const std::exception& ex) {
        std::cerr << "error: " << ex.what() << "\n";
        return 5;
    }

    return 0;
}

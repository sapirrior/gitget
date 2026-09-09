#include "gitget/downloader.hpp"
#include "gitget/error.hpp"

#include <curl/curl.h>

#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>

namespace gitget {

namespace {

size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total_size = size * nmemb;
    auto* buffer = static_cast<std::string*>(userp);
    try {
        buffer->append(static_cast<char*>(contents), total_size);
    } catch (...) {
        return 0;
    }
    return total_size;
}

struct CurlGlobalGuard {
    CurlGlobalGuard() {
        CURLcode res = curl_global_init(CURL_GLOBAL_DEFAULT);
        if (res != CURLE_OK) {
            throw NetworkError(std::string("curl_global_init failed: ") + curl_easy_strerror(res));
        }
    }
    ~CurlGlobalGuard() {
        curl_global_cleanup();
    }
    CurlGlobalGuard(const CurlGlobalGuard&) = delete;
    CurlGlobalGuard& operator=(const CurlGlobalGuard&) = delete;
};

struct CurlHandleDeleter {
    void operator()(CURL* curl) const noexcept {
        if (curl) {
            curl_easy_cleanup(curl);
        }
    }
};

struct CurlSlistDeleter {
    void operator()(curl_slist* list) const noexcept {
        if (list) {
            curl_slist_free_all(list);
        }
    }
};

using UniqueCurl = std::unique_ptr<CURL, CurlHandleDeleter>;
using UniqueSlist = std::unique_ptr<curl_slist, CurlSlistDeleter>;

std::string get_env_var(const char* name) {
    const char* val = std::getenv(name);
    return val ? std::string(val) : std::string();
}

std::string resolve_token(const Arguments& args) {
    if (args.token.has_value() && !args.token->empty()) {
        return *args.token;
    }
    switch (args.provider) {
        case ProviderType::GitHub: {
            std::string t = get_env_var("GITHUB_TOKEN");
            if (t.empty()) t = get_env_var("GH_TOKEN");
            return t;
        }
        case ProviderType::GitLab: {
            std::string t = get_env_var("GITLAB_TOKEN");
            if (t.empty()) t = get_env_var("GL_TOKEN");
            return t;
        }
        case ProviderType::Bitbucket: {
            std::string t = get_env_var("BITBUCKET_TOKEN");
            if (t.empty()) t = get_env_var("BB_TOKEN");
            return t;
        }
        case ProviderType::Codeberg: {
            std::string t = get_env_var("CODEBERG_TOKEN");
            if (t.empty()) t = get_env_var("CB_TOKEN");
            return t;
        }
        default:
            return get_env_var("GIT_TOKEN");
    }
}

std::string extract_filename_from_url(const std::string& url) {
    auto clean_url = url.substr(0, url.find_first_of("?#"));
    while (!clean_url.empty() && clean_url.back() == '/') {
        clean_url.pop_back();
    }
    auto slash = clean_url.find_last_of('/');
    if (slash != std::string::npos && slash + 1 < clean_url.size()) {
        return clean_url.substr(slash + 1);
    }
    return "download";
}

std::string format_http_error(long code, const std::string& provider, const std::string& target_desc) {
    std::ostringstream ss;
    ss << "HTTP " << code << " (" << provider << "): ";
    switch (code) {
        case 404:
            ss << "Target file not found. Verify url/path (" << target_desc << "). If this repository is private, provide an auth token via -t or environment variable.";
            break;
        case 403:
            ss << "Access forbidden. Check permissions or rate limits. If the repo is private, provide an auth token via -t or environment variable.";
            break;
        case 401:
            ss << "Unauthorized. Authentication failed or token is missing/expired.";
            break;
        case 500:
        case 502:
        case 503:
        case 504:
            ss << "Remote server error encountered.";
            break;
        default:
            ss << "Request failed.";
            break;
    }
    return ss.str();
}

} // namespace

void download_from_url(const std::string& url,
                       const std::string& provider_display_name,
                       const Arguments& args) {
    static const CurlGlobalGuard curl_guard;
    UniqueCurl curl(curl_easy_init());
    if (!curl) {
        throw NetworkError("Failed to initialize libcurl easy handle");
    }

    std::string response_buffer;
    char errbuf[CURL_ERROR_SIZE] = {0};

    UniqueSlist headers;
    std::string token = resolve_token(args);
    if (!token.empty()) {
        if (args.provider == ProviderType::GitLab) {
            // GitLab accepts PRIVATE-TOKEN: <token>
            headers.reset(curl_slist_append(headers.release(), ("PRIVATE-TOKEN: " + token).c_str()));
        } else {
            // GitHub, Codeberg, Bitbucket Bearer token
            headers.reset(curl_slist_append(headers.release(), ("Authorization: Bearer " + token).c_str()));
        }
    }

    if (headers) {
        curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, headers.get());
    }

    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &response_buffer);
    curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl.get(), CURLOPT_MAXREDIRS, 10L);
    curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl.get(), CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl.get(), CURLOPT_ERRORBUFFER, errbuf);
    curl_easy_setopt(curl.get(), CURLOPT_FAILONERROR, 0L);
    curl_easy_setopt(curl.get(), CURLOPT_USERAGENT, "gitget/0.1.0");

    CURLcode res = curl_easy_perform(curl.get());
    if (res != CURLE_OK) {
        size_t len = std::strlen(errbuf);
        std::string err_msg = (len > 0) ? errbuf : curl_easy_strerror(res);
        throw NetworkError("Network transfer failed: " + err_msg);
    }

    long http_code = 0;
    curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code < 200 || http_code >= 300) {
        std::string desc = !args.url.empty() ? args.url : (args.repo + " @ " + args.path);
        throw HttpError(http_code, format_http_error(http_code, provider_display_name, desc));
    }

    std::string output_path;
    if (args.output.has_value() && !args.output->empty()) {
        output_path = *args.output;
    } else if (!args.path.empty()) {
        auto slash_pos = args.path.find_last_of('/');
        if (slash_pos == std::string::npos) {
            output_path = args.path;
        } else {
            output_path = args.path.substr(slash_pos + 1);
            if (output_path.empty()) {
                output_path = extract_filename_from_url(url);
            }
        }
    } else {
        output_path = extract_filename_from_url(url);
    }

    std::error_code ec;
    std::filesystem::path dest(output_path);
    if (dest.has_parent_path()) {
        std::filesystem::create_directories(dest.parent_path(), ec);
        if (ec) {
            throw FileSystemError("Failed to create destination directories for '" + output_path + "': " + ec.message());
        }
    }

    std::ofstream out_file(dest, std::ios::binary);
    if (!out_file.is_open()) {
        throw FileSystemError("Failed to create/open destination file '" + output_path + "' for writing");
    }

    out_file.write(response_buffer.data(), static_cast<std::streamsize>(response_buffer.size()));
    if (!out_file.good()) {
        throw FileSystemError("Failed to write full contents to file '" + output_path + "'");
    }
}

} // namespace gitget

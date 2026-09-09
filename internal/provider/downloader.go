package provider

import (
	"errors"
	"fmt"
	"io"
	"net/http"
	"net/url"
	"os"
	"path/filepath"
	"strings"
	"time"

	"github.com/sapirrior/gitget/internal/app"
)

func ResolveToken(args *app.Arguments) string {
	if args.Token != "" {
		return args.Token
	}
	switch args.Provider {
	case app.ProviderGitHub:
		if t := os.Getenv("GITHUB_TOKEN"); t != "" {
			return t
		}
		return os.Getenv("GH_TOKEN")
	case app.ProviderGitLab:
		if t := os.Getenv("GITLAB_TOKEN"); t != "" {
			return t
		}
		return os.Getenv("GL_TOKEN")
	case app.ProviderBitbucket:
		if t := os.Getenv("BITBUCKET_TOKEN"); t != "" {
			return t
		}
		return os.Getenv("BB_TOKEN")
	case app.ProviderCodeberg:
		if t := os.Getenv("CODEBERG_TOKEN"); t != "" {
			return t
		}
		return os.Getenv("CB_TOKEN")
	default:
		return os.Getenv("GIT_TOKEN")
	}
}

func ExtractFilenameFromURL(rawURL string) string {
	parsed, err := url.Parse(rawURL)
	if err == nil && parsed.Path != "" {
		clean := strings.TrimRight(parsed.Path, "/")
		base := filepath.Base(clean)
		if base != "" && base != "." && base != "/" {
			return base
		}
	}
	return "download"
}

func BuildTargetURL(args *app.Arguments) (string, string, error) {
	switch args.Provider {
	case app.ProviderGitHub:
		if args.Repo == "" {
			return "", "", errors.New("missing required flag: -r, --repo <owner/repo>")
		}
		if !strings.Contains(args.Repo, "/") {
			return "", "", fmt.Errorf("invalid repository format: '%s'. Expected '<owner>/<repo>'", args.Repo)
		}
		if args.Path == "" {
			return "", "", errors.New("missing required flag: -p, --path <file/path>")
		}
		u := fmt.Sprintf("https://raw.githubusercontent.com/%s/%s/%s", args.Repo, args.Branch, args.Path)
		return u, "GitHub", nil

	case app.ProviderGitLab:
		if args.Repo == "" {
			return "", "", errors.New("missing required flag: -r, --repo <group/project>")
		}
		if args.Path == "" {
			return "", "", errors.New("missing required flag: -p, --path <file/path>")
		}
		u := fmt.Sprintf("https://gitlab.com/%s/-/raw/%s/%s", args.Repo, args.Branch, args.Path)
		return u, "GitLab", nil

	case app.ProviderBitbucket:
		if args.Repo == "" {
			return "", "", errors.New("missing required flag: -r, --repo <workspace/repo_slug>")
		}
		if args.Path == "" {
			return "", "", errors.New("missing required flag: -p, --path <file/path>")
		}
		u := fmt.Sprintf("https://bitbucket.org/%s/raw/%s/%s", args.Repo, args.Branch, args.Path)
		return u, "Bitbucket", nil

	case app.ProviderCodeberg:
		if args.Repo == "" {
			return "", "", errors.New("missing required flag: -r, --repo <owner/repo>")
		}
		if args.Path == "" {
			return "", "", errors.New("missing required flag: -p, --path <file/path>")
		}
		u := fmt.Sprintf("https://codeberg.org/%s/raw/branch/%s/%s", args.Repo, args.Branch, args.Path)
		return u, "Codeberg", nil

	case app.ProviderRaw:
		target := args.URL
		if target == "" {
			target = args.Repo
		}
		if target == "" {
			return "", "", errors.New("raw provider requires a direct URL via -u, --url or positional argument")
		}
		return target, "Raw", nil

	default:
		return "", "", fmt.Errorf("unsupported provider '%s'", args.Provider)
	}
}

func Download(args *app.Arguments) error {
	targetURL, providerName, err := BuildTargetURL(args)
	if err != nil {
		return err
	}

	client := &http.Client{
		Timeout: 45 * time.Second,
	}

	req, err := http.NewRequest("GET", targetURL, nil)
	if err != nil {
		return fmt.Errorf("failed to create HTTP request: %w", err)
	}

	req.Header.Set("User-Agent", "gitget/0.1.0")

	token := ResolveToken(args)
	if token != "" {
		if args.Provider == app.ProviderGitLab {
			req.Header.Set("PRIVATE-TOKEN", token)
		} else {
			req.Header.Set("Authorization", "Bearer "+token)
		}
	}

	resp, err := client.Do(req)
	if err != nil {
		return fmt.Errorf("network transfer failed: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode < 200 || resp.StatusCode >= 300 {
		switch resp.StatusCode {
		case 404:
			return fmt.Errorf("HTTP 404 (%s): File not found at '%s'. If private, provide token via -t or environment", providerName, targetURL)
		case 403:
			return fmt.Errorf("HTTP 403 (%s): Access forbidden (rate-limit or authentication required)", providerName)
		case 401:
			return fmt.Errorf("HTTP 401 (%s): Unauthorized. Invalid or missing token", providerName)
		default:
			return fmt.Errorf("HTTP %d (%s): Request failed", resp.StatusCode, providerName)
		}
	}

	outputPath := args.Output
	if outputPath == "" {
		if args.Path != "" {
			slash := strings.LastIndex(args.Path, "/")
			if slash == -1 {
				outputPath = args.Path
			} else {
				outputPath = args.Path[slash+1:]
			}
		}
		if outputPath == "" {
			outputPath = ExtractFilenameFromURL(targetURL)
		}
	}

	dir := filepath.Dir(outputPath)
	if dir != "" && dir != "." {
		if err := os.MkdirAll(dir, 0755); err != nil {
			return fmt.Errorf("failed to create destination directories: %w", err)
		}
	}

	outFile, err := os.Create(outputPath)
	if err != nil {
		return fmt.Errorf("failed to open output file '%s': %w", outputPath, err)
	}
	defer outFile.Close()

	if _, err := io.Copy(outFile, resp.Body); err != nil {
		return fmt.Errorf("failed to write content to '%s': %w", outputPath, err)
	}

	return nil
}

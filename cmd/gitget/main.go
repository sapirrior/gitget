package main

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"github.com/sapirrior/gitget/internal/app"
	"github.com/sapirrior/gitget/internal/parser"
	"github.com/sapirrior/gitget/internal/provider"
)

func parseProvider(p string) (app.ProviderType, error) {
	switch strings.ToLower(p) {
	case "github", "gh":
		return app.ProviderGitHub, nil
	case "gitlab", "gl":
		return app.ProviderGitLab, nil
	case "bitbucket", "bb":
		return app.ProviderBitbucket, nil
	case "codeberg", "cb":
		return app.ProviderCodeberg, nil
	case "raw", "direct":
		return app.ProviderRaw, nil
	default:
		return "", fmt.Errorf("unknown provider '%s'. Supported: github, gitlab, bitbucket, codeberg, raw", p)
	}
}

func printUsage(progName string) {
	fmt.Printf(`Usage: %s -r <owner/repo> -p <file/path> [options]
       %s <git-url> [options]

Options:
  -r, --repo <owner/repo>      Target repository [required unless URL is given]
  -p, --path <file/path>       File path within repository
  -u, --url <url>              Direct raw or web Git file URL
  -t, --token <token>          Authentication token for private repositories
  -P, --provider <name>        Git provider: github, gitlab, bitbucket, codeberg, raw (default: auto)
  -b, --branch <branch>        Branch or tag name (default: main)
  -o, --output <file>          Destination file path (default: filename from path/url)
  -h, --help                   Show this help message

Environment Variables:
  GITHUB_TOKEN / GH_TOKEN      Token for private GitHub repos
  GITLAB_TOKEN / GL_TOKEN      Token for private GitLab repos
  BITBUCKET_TOKEN / BB_TOKEN   Token for private Bitbucket repos
  CODEBERG_TOKEN / CB_TOKEN    Token for private Codeberg repos
`, progName, progName)
}

func printHint(progName string) {
	fmt.Fprintf(os.Stderr, "Try '%s --help' for more information.\n", progName)
}

func isFlag(s string) bool {
	return strings.HasPrefix(s, "-") && len(s) > 1
}

func isHTTPURL(s string) bool {
	return strings.HasPrefix(s, "http://") || strings.HasPrefix(s, "https://")
}

func main() {
	progName := "gitget"
	if len(os.Args) > 0 {
		progName = filepath.Base(os.Args[0])
	}

	args := &app.Arguments{
		Branch:   "main",
		Provider: app.ProviderAuto,
	}

	rawArgs := os.Args[1:]
	i := 0
	for i < len(rawArgs) {
		arg := rawArgs[i]
		switch {
		case arg == "-r" || arg == "--repo":
			if i+1 >= len(rawArgs) || isFlag(rawArgs[i+1]) {
				fmt.Fprintf(os.Stderr, "error: flag '%s' requires an argument (<owner/repo>)\n", arg)
				printHint(progName)
				os.Exit(1)
			}
			args.Repo = rawArgs[i+1]
			i += 2
		case arg == "-p" || arg == "--path":
			if i+1 >= len(rawArgs) || isFlag(rawArgs[i+1]) {
				fmt.Fprintf(os.Stderr, "error: flag '%s' requires an argument (<file/path>)\n", arg)
				printHint(progName)
				os.Exit(1)
			}
			args.Path = rawArgs[i+1]
			i += 2
		case arg == "-u" || arg == "--url":
			if i+1 >= len(rawArgs) || isFlag(rawArgs[i+1]) {
				fmt.Fprintf(os.Stderr, "error: flag '%s' requires an argument (<url>)\n", arg)
				printHint(progName)
				os.Exit(1)
			}
			args.URL = rawArgs[i+1]
			i += 2
		case arg == "-t" || arg == "--token":
			if i+1 >= len(rawArgs) || isFlag(rawArgs[i+1]) {
				fmt.Fprintf(os.Stderr, "error: flag '%s' requires an argument (<token>)\n", arg)
				printHint(progName)
				os.Exit(1)
			}
			args.Token = rawArgs[i+1]
			i += 2
		case arg == "-P" || arg == "--provider":
			if i+1 >= len(rawArgs) || isFlag(rawArgs[i+1]) {
				fmt.Fprintf(os.Stderr, "error: flag '%s' requires an argument (<provider>)\n", arg)
				printHint(progName)
				os.Exit(1)
			}
			p, err := parseProvider(rawArgs[i+1])
			if err != nil {
				fmt.Fprintf(os.Stderr, "error: %s\n", err)
				printHint(progName)
				os.Exit(1)
			}
			args.Provider = p
			i += 2
		case arg == "-b" || arg == "--branch":
			if i+1 >= len(rawArgs) || isFlag(rawArgs[i+1]) {
				fmt.Fprintf(os.Stderr, "error: flag '%s' requires an argument (<branch>)\n", arg)
				printHint(progName)
				os.Exit(1)
			}
			args.Branch = rawArgs[i+1]
			i += 2
		case arg == "-o" || arg == "--output":
			if i+1 >= len(rawArgs) || isFlag(rawArgs[i+1]) {
				fmt.Fprintf(os.Stderr, "error: flag '%s' requires an argument (<file>)\n", arg)
				printHint(progName)
				os.Exit(1)
			}
			args.Output = rawArgs[i+1]
			i += 2
		case arg == "-h" || arg == "--help":
			printUsage(progName)
			os.Exit(0)
		case isHTTPURL(arg) && args.URL == "" && args.Repo == "":
			args.URL = arg
			i++
		default:
			fmt.Fprintf(os.Stderr, "error: unrecognized option '%s'\n", arg)
			printHint(progName)
			os.Exit(1)
		}
	}

	if args.URL != "" {
		if !parser.ParseGitURL(args.URL, args) {
			args.Provider = app.ProviderRaw
		}
	} else if args.Provider == app.ProviderAuto {
		args.Provider = app.ProviderGitHub
	}

	if err := provider.Download(args); err != nil {
		fmt.Fprintf(os.Stderr, "error: %s\n", err)
		os.Exit(1)
	}
}

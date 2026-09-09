package parser

import (
	"regexp"

	"github.com/sapirrior/gitget/internal/app"
)

var (
	ghBlobRe = regexp.MustCompile(`^https?://github\.com/([^/]+)/([^/]+)/blob/([^/]+)/(.+)$`)
	ghRawRe  = regexp.MustCompile(`^https?://raw\.githubusercontent\.com/([^/]+)/([^/]+)/([^/]+)/(.+)$`)
	glBlobRe = regexp.MustCompile(`^https?://gitlab\.com/(.+)/-/(?:blob|raw)/([^/]+)/(.+)$`)
	bbBlobRe = regexp.MustCompile(`^https?://bitbucket\.org/([^/]+)/([^/]+)/(?:src|raw)/([^/]+)/(.+)$`)
	cbBlobRe = regexp.MustCompile(`^https?://codeberg\.org/([^/]+)/([^/]+)/(?:src|raw)/branch/([^/]+)/(.+)$`)
)

func ParseGitURL(rawURL string, args *app.Arguments) bool {
	if m := ghBlobRe.FindStringSubmatch(rawURL); len(m) == 5 {
		args.Provider = app.ProviderGitHub
		args.Repo = m[1] + "/" + m[2]
		args.Branch = m[3]
		args.Path = m[4]
		return true
	}
	if m := ghRawRe.FindStringSubmatch(rawURL); len(m) == 5 {
		args.Provider = app.ProviderGitHub
		args.Repo = m[1] + "/" + m[2]
		args.Branch = m[3]
		args.Path = m[4]
		return true
	}
	if m := glBlobRe.FindStringSubmatch(rawURL); len(m) == 4 {
		args.Provider = app.ProviderGitLab
		args.Repo = m[1]
		args.Branch = m[2]
		args.Path = m[3]
		return true
	}
	if m := bbBlobRe.FindStringSubmatch(rawURL); len(m) == 5 {
		args.Provider = app.ProviderBitbucket
		args.Repo = m[1] + "/" + m[2]
		args.Branch = m[3]
		args.Path = m[4]
		return true
	}
	if m := cbBlobRe.FindStringSubmatch(rawURL); len(m) == 5 {
		args.Provider = app.ProviderCodeberg
		args.Repo = m[1] + "/" + m[2]
		args.Branch = m[3]
		args.Path = m[4]
		return true
	}
	return false
}

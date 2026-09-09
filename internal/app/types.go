package app

type ProviderType string

const (
	ProviderAuto      ProviderType = "auto"
	ProviderGitHub    ProviderType = "github"
	ProviderGitLab    ProviderType = "gitlab"
	ProviderBitbucket ProviderType = "bitbucket"
	ProviderCodeberg  ProviderType = "codeberg"
	ProviderRaw       ProviderType = "raw"
)

type Arguments struct {
	Repo     string
	Path     string
	Branch   string
	URL      string
	Output   string
	Token    string
	Provider ProviderType
}

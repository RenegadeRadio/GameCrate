namespace GameCrate.Gui.Services;

public sealed class GameCrateResult
{
    public required int ExitCode { get; init; }
    public required string StandardOutput { get; init; }
    public required string StandardError { get; init; }

    public bool Success => ExitCode == 0;

    /// <summary>
    /// Best-effort combined CLI output for display when a command fails.
    /// </summary>
    public string FormatOutput(string? fallback = null)
    {
        string stderr = StandardError.Trim();
        string stdout = StandardOutput.Trim();

        if (!string.IsNullOrEmpty(stderr) && !string.IsNullOrEmpty(stdout))
        {
            return stderr + Environment.NewLine + Environment.NewLine + stdout;
        }

        if (!string.IsNullOrEmpty(stderr))
        {
            return stderr;
        }

        if (!string.IsNullOrEmpty(stdout))
        {
            return stdout;
        }

        return fallback ?? $"Command failed with exit code {ExitCode}.";
    }
}

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

        string? explained = ExplainExitCode(ExitCode);
        if (explained != null)
        {
            return explained;
        }

        return fallback ?? $"Command failed with exit code {ExitCode}.";
    }

    private static string? ExplainExitCode(int exitCode)
    {
        // NTSTATUS STATUS_DLL_NOT_FOUND (0xC0000135) — common when VC++ runtime is missing.
        if (exitCode == -1073741515 || exitCode == unchecked((int)0xC0000135U))
        {
            return "gamecrate.exe could not start (missing DLL).\n\n" +
                   "Install the Microsoft Visual C++ Redistributable (x64), or download " +
                   "the latest GameCrate release which bundles a standalone gamecrate.exe.\n\n" +
                   "https://aka.ms/vs/17/release/vc_redist.x64.exe";
        }

        return null;
    }
}

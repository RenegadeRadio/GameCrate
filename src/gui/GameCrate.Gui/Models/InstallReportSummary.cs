using System.Text.Json.Serialization;

namespace GameCrate.Gui.Models;

public sealed class InstallReportSummary
{
    [JsonPropertyName("profileId")]
    public string? ProfileId { get; init; }

    [JsonPropertyName("installerExitCode")]
    public int InstallerExitCode { get; init; }

    [JsonPropertyName("executable")]
    public string? Executable { get; init; }

    [JsonPropertyName("installedFileCount")]
    public int InstalledFileCount { get; init; }

    [JsonPropertyName("outsideWriteCount")]
    public int OutsideWriteCount { get; init; }

    [JsonPropertyName("suspiciousOutsideWriteCount")]
    public int SuspiciousOutsideWriteCount { get; init; }

    [JsonPropertyName("registryChangeCount")]
    public int RegistryChangeCount { get; init; }

    [JsonPropertyName("outsideRegistryChangeCount")]
    public int OutsideRegistryChangeCount { get; init; }

    [JsonPropertyName("suspiciousRegistryChangeCount")]
    public int SuspiciousRegistryChangeCount { get; init; }

    [JsonPropertyName("outsideWrites")]
    public List<string>? OutsideWrites { get; init; }

    [JsonPropertyName("suspiciousOutsideWrites")]
    public List<string>? SuspiciousOutsideWrites { get; init; }

    public string ToSummaryText()
    {
        var lines = new List<string>
        {
            $"Installer exit code: {InstallerExitCode}",
            $"Files installed (under footprint): {InstalledFileCount}",
            $"Outside writes: {OutsideWriteCount}" +
                (SuspiciousOutsideWriteCount > 0 ? $" ({SuspiciousOutsideWriteCount} suspicious)" : ""),
            $"Registry changes: {RegistryChangeCount}" +
                (SuspiciousRegistryChangeCount > 0 ? $" ({SuspiciousRegistryChangeCount} suspicious)" : ""),
        };

        if (!string.IsNullOrWhiteSpace(Executable))
        {
            lines.Add($"Detected executable: {Executable}");
        }
        else
        {
            lines.Add("Detected executable: (none — pick game.exe manually)");
        }

        if (OutsideWriteCount > 0 && OutsideWrites is { Count: > 0 })
        {
            lines.Add("");
            lines.Add("Outside writes (sample):");
            foreach (string path in OutsideWrites.Take(5))
            {
                lines.Add($"  • {path}");
            }

            if (OutsideWrites.Count > 5)
            {
                lines.Add($"  … and {OutsideWrites.Count - 5} more (open full JSON for list)");
            }
        }

        if (SuspiciousOutsideWriteCount > 0)
        {
            lines.Add("");
            lines.Add("Suspicious outside writes:");
            foreach (string path in SuspiciousOutsideWrites ?? [])
            {
                lines.Add($"  • {path}");
            }
        }

        return string.Join(Environment.NewLine, lines);
    }
}

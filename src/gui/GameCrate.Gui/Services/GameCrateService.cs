using System.Diagnostics;
using System.IO;
using System.Text.Json;
using GameCrate.Gui.Models;

namespace GameCrate.Gui.Services;

public sealed class GameCrateService
{
    private static readonly JsonSerializerOptions JsonOptions = new()
    {
        PropertyNameCaseInsensitive = true,
    };

    public string ExecutablePath { get; }

    public GameCrateService(string? executablePath = null)
    {
        ExecutablePath = executablePath ?? FindExecutable();
        if (!File.Exists(ExecutablePath))
        {
            throw new FileNotFoundException(
                "Could not find gamecrate.exe. Build the CLI or place it next to GameCrate.Gui.exe.",
                ExecutablePath);
        }
    }

    public static bool IsCliAvailable() => File.Exists(FindExecutable());

    public static string FindExecutable()
    {
        string baseDir = AppContext.BaseDirectory;
        string[] candidates =
        [
            Path.Combine(baseDir, "gamecrate.exe"),
            Path.Combine(baseDir, "..", "gamecrate.exe"),
            Path.Combine(baseDir, "..", "..", "build", "gamecrate.exe"),
            Path.Combine(baseDir, "..", "..", "..", "build", "gamecrate.exe"),
            Path.Combine(baseDir, "..", "..", "build", "Release", "gamecrate.exe"),
        ];

        foreach (string candidate in candidates)
        {
            string fullPath = Path.GetFullPath(candidate);
            if (File.Exists(fullPath))
            {
                return fullPath;
            }
        }

        return Path.Combine(baseDir, "gamecrate.exe");
    }

    public async Task<IReadOnlyList<GameProfile>> ListProfilesAsync(CancellationToken cancellationToken = default)
    {
        GameCrateResult result = await RunAsync(cancellationToken, "list-profiles", "--json").ConfigureAwait(false);
        if (!result.Success)
        {
            throw new InvalidOperationException(result.FormatOutput());
        }

        string json = result.StandardOutput.Trim();
        if (string.IsNullOrEmpty(json))
        {
            return [];
        }

        try
        {
            List<GameProfile>? profiles = JsonSerializer.Deserialize<List<GameProfile>>(json, JsonOptions);
            return profiles ?? [];
        }
        catch (JsonException ex)
        {
            string preview = json.Length > 200 ? json[..200] + "..." : json;
            throw new InvalidOperationException(
                "gamecrate.exe returned invalid JSON for list-profiles.\n\n" +
                $"Parser error: {ex.Message}\n\nOutput preview:\n{preview}",
                ex);
        }
    }

    public Task<GameCrateResult> LaunchAsync(string profileId, CancellationToken cancellationToken = default) =>
        RunAsync(cancellationToken, "launch", "--profile", profileId, "--no-wait");

    public Task<GameCrateResult> InstallAsync(
        string id,
        string name,
        string installDir,
        string installer,
        bool virtualizeAppData = true,
        bool allowNetwork = false,
        CancellationToken cancellationToken = default)
    {
        List<string> args =
        [
            "install",
            "--id", id,
            "--name", name,
            "--install-dir", installDir,
            "--installer", installer,
        ];

        if (!virtualizeAppData)
        {
            args.Add("--no-virtual-app-data");
        }

        if (allowNetwork)
        {
            args.Add("--network");
        }

        return RunAsync(cancellationToken, args.ToArray());
    }

    public Task<GameCrateResult> ShowInstallReportAsync(string profileId, CancellationToken cancellationToken = default) =>
        RunAsync(cancellationToken, "show-install-report", "--profile", profileId);

    public Task<GameCrateResult> DestroyProfileAsync(
        string profileId,
        bool wipeData,
        CancellationToken cancellationToken = default)
    {
        List<string> args = ["destroy-profile", "--profile", profileId];
        if (wipeData)
        {
            args.Add("--wipe-data");
        }

        return RunAsync(cancellationToken, args.ToArray());
    }

    private async Task<GameCrateResult> RunAsync(CancellationToken cancellationToken, params string[] args)
    {
        var startInfo = new ProcessStartInfo
        {
            FileName = ExecutablePath,
            UseShellExecute = false,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            CreateNoWindow = true,
        };

        foreach (string arg in args)
        {
            startInfo.ArgumentList.Add(arg);
        }

        using Process process = new() { StartInfo = startInfo };
        if (!process.Start())
        {
            throw new InvalidOperationException("Failed to start gamecrate.exe.");
        }

        Task<byte[]> stdoutTask = CliEncoding.ReadAllBytesAsync(process.StandardOutput.BaseStream, cancellationToken);
        Task<byte[]> stderrTask = CliEncoding.ReadAllBytesAsync(process.StandardError.BaseStream, cancellationToken);
        await process.WaitForExitAsync(cancellationToken).ConfigureAwait(false);

        return new GameCrateResult
        {
            ExitCode = process.ExitCode,
            StandardOutput = CliEncoding.Decode(await stdoutTask.ConfigureAwait(false)),
            StandardError = CliEncoding.Decode(await stderrTask.ConfigureAwait(false)),
        };
    }
}

namespace GameCrate.Gui.Services;

public sealed class GameCrateResult
{
    public required int ExitCode { get; init; }
    public required string StandardOutput { get; init; }
    public required string StandardError { get; init; }

    public bool Success => ExitCode == 0;
}

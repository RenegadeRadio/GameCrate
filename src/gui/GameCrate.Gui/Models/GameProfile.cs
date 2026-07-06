using System.Text.Json.Serialization;

namespace GameCrate.Gui.Models;

public sealed class GameProfile
{
    [JsonPropertyName("id")]
    public string Id { get; set; } = string.Empty;

    [JsonPropertyName("name")]
    public string Name { get; set; } = string.Empty;

    [JsonPropertyName("installDir")]
    public string InstallDir { get; set; } = string.Empty;

    [JsonPropertyName("executable")]
    public string Executable { get; set; } = string.Empty;

    [JsonPropertyName("saveDir")]
    public string SaveDir { get; set; } = string.Empty;

    [JsonPropertyName("network")]
    public bool Network { get; set; }

    [JsonPropertyName("virtualizeAppData")]
    public bool VirtualizeAppData { get; set; } = true;
}

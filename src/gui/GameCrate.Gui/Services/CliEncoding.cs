using System.IO;
using System.Text;

namespace GameCrate.Gui.Services;

internal static class CliEncoding
{
    public static async Task<byte[]> ReadAllBytesAsync(Stream stream, CancellationToken cancellationToken)
    {
        using MemoryStream memory = new();
        await stream.CopyToAsync(memory, cancellationToken).ConfigureAwait(false);
        return memory.ToArray();
    }

    public static string Decode(byte[] bytes)
    {
        if (bytes.Length == 0)
        {
            return string.Empty;
        }

        if (bytes.Length >= 3 && bytes[0] == 0xEF && bytes[1] == 0xBB && bytes[2] == 0xBF)
        {
            return Encoding.UTF8.GetString(bytes, 3, bytes.Length - 3);
        }

        if (bytes.Length >= 2 && bytes[0] == 0xFF && bytes[1] == 0xFE)
        {
            return Encoding.Unicode.GetString(bytes, 2, bytes.Length - 2);
        }

        if (LooksLikeUtf16Le(bytes))
        {
            return Encoding.Unicode.GetString(bytes);
        }

        return Encoding.UTF8.GetString(bytes);
    }

    private static bool LooksLikeUtf16Le(byte[] bytes)
    {
        int pairs = Math.Min(bytes.Length / 2, 8);
        if (pairs < 2)
        {
            return false;
        }

        int asciiPairs = 0;
        for (int i = 0; i < pairs; ++i)
        {
            byte low = bytes[i * 2];
            byte high = bytes[i * 2 + 1];
            if (high == 0 && low < 0x80)
            {
                asciiPairs++;
            }
        }

        return asciiPairs >= 2;
    }
}

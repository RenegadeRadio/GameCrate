using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Reflection;
using System.Text.Json;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using GameCrate.Gui.Models;
using GameCrate.Gui.Services;
using MessageBox = System.Windows.MessageBox;

namespace GameCrate.Gui;

public partial class MainWindow : Window
{
    private readonly GameCrateService _service = new();
    private readonly ObservableCollection<GameProfile> _profiles = new();

    public MainWindow()
    {
        InitializeComponent();
        ProfileList.ItemsSource = _profiles;
        VersionText.Text = $"v{GetAppVersion()}";
        Loaded += async (_, _) => await RefreshProfilesAsync();
    }

    private static string GetAppVersion() =>
        Assembly.GetExecutingAssembly().GetName().Version?.ToString(3) ?? "0.0.0";

    private void AboutButton_Click(object sender, RoutedEventArgs e)
    {
        MessageBox.Show(
            $"GameCrate v{GetAppVersion()}\n\n" +
            "Windows LPAC sandbox for standalone games.\n" +
            "Not for Steam/Epic/GOG — see docs\\SCOPE.md\n\n" +
            "https://github.com/RenegadeRadio/GameCrate",
            "About GameCrate",
            MessageBoxButton.OK,
            MessageBoxImage.Information);
    }

    public void OpenInstallDialog()
    {
        var dialog = new InstallWindow(_service) { Owner = this };
        if (dialog.ShowDialog() == true)
        {
            _ = RefreshProfilesAsync();
        }
    }

    private async Task RefreshProfilesAsync()
    {
        SetBusy(true, "Loading profiles...");
        try
        {
            var profiles = await _service.ListProfilesAsync();
            _profiles.Clear();
            foreach (var profile in profiles)
            {
                _profiles.Add(profile);
            }

            StatusText.Text = profiles.Count == 0
                ? "No sandboxed games yet. Click Install game to add one."
                : $"{profiles.Count} profile(s). Double-click or select and click Play.";
        }
        catch (Exception ex)
        {
            StatusText.Text = $"Error: {ex.Message}";
        }
        finally
        {
            SetBusy(false, null);
        }
    }

    private GameProfile? SelectedProfile =>
        ProfileList.SelectedItem as GameProfile;

    private void ProfileList_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        var selected = SelectedProfile != null;
        PlayButton.IsEnabled = selected;
        ReportButton.IsEnabled = selected;
        DestroyButton.IsEnabled = selected;
    }

    private async void ProfileList_MouseDoubleClick(object sender, MouseButtonEventArgs e)
    {
        await LaunchSelectedAsync();
    }

    private async void PlayButton_Click(object sender, RoutedEventArgs e)
    {
        await LaunchSelectedAsync();
    }

    private async Task LaunchSelectedAsync()
    {
        var profile = SelectedProfile;
        if (profile == null)
        {
            return;
        }

        SetBusy(true, $"Launching {profile.Name}...");
        try
        {
            var result = await _service.LaunchAsync(profile.Id);
            if (result.Success)
            {
                StatusText.Text = $"Launched {profile.Name}. Game runs in AppContainer sandbox.";
            }
            else
            {
                MessageBox.Show(result.FormatOutput(), "Launch failed", MessageBoxButton.OK, MessageBoxImage.Warning);
                StatusText.Text = "Launch failed.";
            }
        }
        catch (Exception ex)
        {
            MessageBox.Show(ex.Message, "Launch failed", MessageBoxButton.OK, MessageBoxImage.Error);
        }
        finally
        {
            SetBusy(false, null);
        }
    }

    private void InstallButton_Click(object sender, RoutedEventArgs e)
    {
        OpenInstallDialog();
    }

    private async void RefreshButton_Click(object sender, RoutedEventArgs e)
    {
        await RefreshProfilesAsync();
    }

    private async void ReportButton_Click(object sender, RoutedEventArgs e)
    {
        var profile = SelectedProfile;
        if (profile == null)
        {
            return;
        }

        SetBusy(true, "Loading install report...");
        try
        {
            var result = await _service.ShowInstallReportAsync(profile.Id);
            if (!result.Success)
            {
                MessageBox.Show(result.FormatOutput("No install report found."), "Install report", MessageBoxButton.OK, MessageBoxImage.Information);
                return;
            }

            string json = result.StandardOutput.Trim();
            InstallReportSummary? summary = null;
            try
            {
                summary = JsonSerializer.Deserialize<InstallReportSummary>(json);
            }
            catch (JsonException)
            {
                // Fall back to opening raw JSON below.
            }

            if (summary != null)
            {
                var viewFull = MessageBox.Show(
                    summary.ToSummaryText() + Environment.NewLine + Environment.NewLine +
                    "Open full install-report.json in your default editor?",
                    $"Install report — {profile.Name}",
                    MessageBoxButton.YesNo,
                    MessageBoxImage.Information);

                if (viewFull != MessageBoxResult.Yes)
                {
                    StatusText.Text = $"Install report summary shown for {profile.Name}.";
                    return;
                }
            }

            var tempPath = Path.Combine(Path.GetTempPath(), $"gamecrate-report-{profile.Id}.json");
            await File.WriteAllTextAsync(tempPath, json);
            Process.Start(new ProcessStartInfo(tempPath) { UseShellExecute = true });
            StatusText.Text = $"Opened install report for {profile.Name}.";
        }
        catch (Exception ex)
        {
            MessageBox.Show(ex.Message, "Install report", MessageBoxButton.OK, MessageBoxImage.Error);
        }
        finally
        {
            SetBusy(false, null);
        }
    }

    private async void DestroyButton_Click(object sender, RoutedEventArgs e)
    {
        var profile = SelectedProfile;
        if (profile == null)
        {
            return;
        }

        var confirm = MessageBox.Show(
            $"Remove profile \"{profile.Name}\" ({profile.Id})?\n\n" +
            "This deletes the profile, revokes sandbox ACLs, and wipes GameCrate data " +
            "under %LOCALAPPDATA%\\GameCrate\\<id>\\ (saves, virtual AppData, reports).\n\n" +
            "Game files in the install directory are not deleted.",
            "Remove profile",
            MessageBoxButton.YesNo,
            MessageBoxImage.Warning);

        if (confirm != MessageBoxResult.Yes)
        {
            return;
        }

        SetBusy(true, $"Removing {profile.Name}...");
        try
        {
            var result = await _service.DestroyProfileAsync(profile.Id, wipeData: true);
            if (result.Success)
            {
                await RefreshProfilesAsync();
            }
            else
            {
                MessageBox.Show(result.FormatOutput(), "Remove failed", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }
        catch (Exception ex)
        {
            MessageBox.Show(ex.Message, "Remove failed", MessageBoxButton.OK, MessageBoxImage.Error);
        }
        finally
        {
            SetBusy(false, null);
        }
    }

    private void SetBusy(bool busy, string? status)
    {
        InstallButton.IsEnabled = !busy;
        RefreshButton.IsEnabled = !busy;
        PlayButton.IsEnabled = !busy && SelectedProfile != null;
        ReportButton.IsEnabled = !busy && SelectedProfile != null;
        DestroyButton.IsEnabled = !busy && SelectedProfile != null;

        if (status != null)
        {
            StatusText.Text = status;
        }
    }
}

using System.IO;
using System.Text.RegularExpressions;
using System.Windows;
using GameCrate.Gui.Services;
using Microsoft.Win32;
using MessageBox = System.Windows.MessageBox;
using OpenFileDialog = Microsoft.Win32.OpenFileDialog;

namespace GameCrate.Gui;

public partial class InstallWindow : Window
{
    private readonly GameCrateService _service;
    private bool _installing;

    public InstallWindow(GameCrateService service)
    {
        _service = service;
        InitializeComponent();
    }

    private void BrowseInstallDir_Click(object sender, RoutedEventArgs e)
    {
        using var dialog = new System.Windows.Forms.FolderBrowserDialog
        {
            Description = "Choose an empty folder for the game install"
        };

        if (dialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
        {
            InstallDirBox.Text = dialog.SelectedPath;
        }
    }

    private void BrowseInstaller_Click(object sender, RoutedEventArgs e)
    {
        var dialog = new OpenFileDialog
        {
            Filter = "Installers (*.exe)|*.exe|All files (*.*)|*.*",
            Title = "Select installer"
        };

        if (dialog.ShowDialog() == true)
        {
            InstallerBox.Text = dialog.FileName;
            if (string.IsNullOrWhiteSpace(NameBox.Text))
            {
                NameBox.Text = Path.GetFileNameWithoutExtension(dialog.FileName);
            }

            if (string.IsNullOrWhiteSpace(IdBox.Text) && !string.IsNullOrWhiteSpace(NameBox.Text))
            {
                IdBox.Text = Slugify(NameBox.Text);
            }
        }
    }

    private static string Slugify(string value)
    {
        var slug = Regex.Replace(value.ToLowerInvariant(), @"[^a-z0-9]+", "-").Trim('-');
        return slug.Length > 0 ? slug : "game";
    }

    private async void InstallStartButton_Click(object sender, RoutedEventArgs e)
    {
        if (_installing)
        {
            return;
        }

        var name = NameBox.Text.Trim();
        var id = IdBox.Text.Trim();
        var installDir = InstallDirBox.Text.Trim();
        var installer = InstallerBox.Text.Trim();

        if (string.IsNullOrEmpty(name) || string.IsNullOrEmpty(id) ||
            string.IsNullOrEmpty(installDir) || string.IsNullOrEmpty(installer))
        {
            MessageBox.Show("Fill in all fields.", "Install", MessageBoxButton.OK, MessageBoxImage.Warning);
            return;
        }

        if (!File.Exists(installer))
        {
            MessageBox.Show("Installer file not found.", "Install", MessageBoxButton.OK, MessageBoxImage.Warning);
            return;
        }

        if (!Regex.IsMatch(id, @"^[a-z0-9][a-z0-9\-]*$"))
        {
            MessageBox.Show("Profile ID must start with a lowercase letter or number and contain only lowercase letters, numbers, and hyphens.",
                "Install", MessageBoxButton.OK, MessageBoxImage.Warning);
            return;
        }

        _installing = true;
        InstallStartButton.IsEnabled = false;
        CancelButton.IsEnabled = false;
        InstallProgress.Visibility = Visibility.Visible;
        InstallProgress.IsIndeterminate = true;
        InstallStatus.Text = "Running sandboxed installer — this may take several minutes...";

        try
        {
            var result = await _service.InstallAsync(
                id,
                name,
                installDir,
                installer,
                virtualizeAppData: VirtualAppDataCheck.IsChecked == true,
                allowNetwork: NetworkCheck.IsChecked == true);

            InstallProgress.IsIndeterminate = false;
            InstallProgress.Visibility = Visibility.Collapsed;

            if (result.Success)
            {
                InstallStatus.Text = "Install finished. Review the install report from the main window.";
                MessageBox.Show(
                    "Install completed.\n\nSelect the profile and click Install report to review outside writes.",
                    "Install complete",
                    MessageBoxButton.OK,
                    MessageBoxImage.Information);
                DialogResult = true;
                Close();
                return;
            }

            InstallStatus.Text = "Install failed.";
            MessageBox.Show(
                result.StandardError.Length > 0 ? result.StandardError : result.StandardOutput,
                "Install failed",
                MessageBoxButton.OK,
                MessageBoxImage.Error);
        }
        catch (Exception ex)
        {
            InstallStatus.Text = "Install failed.";
            MessageBox.Show(ex.Message, "Install failed", MessageBoxButton.OK, MessageBoxImage.Error);
        }
        finally
        {
            _installing = false;
            InstallStartButton.IsEnabled = true;
            CancelButton.IsEnabled = true;
            InstallProgress.Visibility = Visibility.Collapsed;
        }
    }

    private void CancelButton_Click(object sender, RoutedEventArgs e)
    {
        if (!_installing)
        {
            DialogResult = false;
            Close();
        }
    }
}

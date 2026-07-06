using System.Drawing;
using System.Windows;
using System.Windows.Forms;
using GameCrate.Gui.Services;
using Application = System.Windows.Application;
using MessageBox = System.Windows.MessageBox;

namespace GameCrate.Gui;

public partial class App : Application
{
    private NotifyIcon? _trayIcon;
    private MainWindow? _mainWindow;

    protected override void OnStartup(StartupEventArgs e)
    {
        base.OnStartup(e);

        if (!GameCrateService.IsCliAvailable())
        {
            MessageBox.Show(
                "gamecrate.exe was not found.\n\n" +
                "Place GameCrate.Gui.exe next to gamecrate.exe, or add gamecrate to your PATH.",
                "GameCrate",
                MessageBoxButton.OK,
                MessageBoxImage.Error);
            Shutdown();
            return;
        }

        _mainWindow = new MainWindow();
        _mainWindow.Closed += (_, _) => Shutdown();

        _trayIcon = new NotifyIcon
        {
            Icon = SystemIcons.Shield,
            Text = "GameCrate — sandboxed games",
            Visible = true
        };

        _trayIcon.DoubleClick += (_, _) => ShowMainWindow();
        _trayIcon.ContextMenuStrip = BuildTrayMenu();

        ShowMainWindow();
    }

    private ContextMenuStrip BuildTrayMenu()
    {
        var menu = new ContextMenuStrip();

        var open = new ToolStripMenuItem("Open GameCrate");
        open.Click += (_, _) => ShowMainWindow();
        menu.Items.Add(open);

        menu.Items.Add(new ToolStripSeparator());

        var install = new ToolStripMenuItem("Install game...");
        install.Click += (_, _) =>
        {
            ShowMainWindow();
            _mainWindow?.OpenInstallDialog();
        };
        menu.Items.Add(install);

        menu.Items.Add(new ToolStripSeparator());

        var exit = new ToolStripMenuItem("Exit");
        exit.Click += (_, _) => Shutdown();
        menu.Items.Add(exit);

        return menu;
    }

    private void ShowMainWindow()
    {
        if (_mainWindow == null)
        {
            return;
        }

        _mainWindow.Show();
        _mainWindow.WindowState = WindowState.Normal;
        _mainWindow.Activate();
    }

    protected override void OnExit(ExitEventArgs e)
    {
        if (_trayIcon != null)
        {
            _trayIcon.Visible = false;
            _trayIcon.Dispose();
        }

        base.OnExit(e);
    }
}

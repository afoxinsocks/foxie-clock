using System.Timers;
using Xamarin.Forms;

namespace FoxieClock
{
    public class ConnectPage : ContentPage
    {
        public Button connectButton;
        public Label welcomeLabel;

        public ConnectPage()
        {
            BackgroundColor = Color.FromHex("290f4c");

            Title = "Foxie Clock";

            BindingContext = new ConnectPageViewModel(this);


            var help = new ToolbarItem
            {
                Text = "Help",
            };
            help.SetBinding(ToolbarItem.CommandProperty, nameof(ConnectPageViewModel.HelpCommand));

            ToolbarItems.Add(help);

            welcomeLabel = new Label
            {
                Text = "Welcome! Tap the button below to connect to your clock.",
                FontSize = 25,
                HorizontalTextAlignment = TextAlignment.Center,
                TextColor = Color.White,
                Margin = new Thickness(10),
            };

            var foxieImage = new Image
            {
                Source = "Foxie"
            };
            if (Device.RuntimePlatform == Device.Android)
            {
                foxieImage.Source = "foxie_icon.png";
            }

            connectButton = new Button
            {
                Text = "Connect",
                FontSize = 30,
                CornerRadius = 25,
                TextColor = Color.Black,
                BackgroundColor = Color.FromHex("e4ac2a"),
                Margin = new Thickness(10)
            };
            connectButton.SetBinding(Button.CommandProperty, nameof(ConnectPageViewModel.ConnectCommand));

            var grid = new Grid
            {
                Margin = new Thickness(20, 40),

                ColumnDefinitions =
                {
                    new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) },
                },
                RowDefinitions =
                {
                    new RowDefinition { Height = new GridLength(1.5, GridUnitType.Star) },
                    new RowDefinition { Height = new GridLength(4, GridUnitType.Star) },
                    new RowDefinition { Height = new GridLength(1.25, GridUnitType.Star) },
                    new RowDefinition { Height = new GridLength(0.75, GridUnitType.Star) },
                }
            };

            grid.Children.Add(welcomeLabel, 0, 0);
            Grid.SetColumnSpan(welcomeLabel, 1);

            grid.Children.Add(foxieImage, 0, 1);
            Grid.SetColumnSpan(foxieImage, 1);

            grid.Children.Add(connectButton, 0, 2);
            Grid.SetColumnSpan(connectButton, 1);

            Content = grid;
        }
    }

    public class ConnectPageViewModel
    {
        ConnectPage Page;
        Timer ConnectingTimer;
        private BLEClock Clock;

        public ConnectPageViewModel(ConnectPage page)
        {
            Page = page;
            Clock = new BLEClock();

            HelpCommand = new Command(async () =>
            {
                await Application.Current.MainPage.Navigation.PushAsync(new ConnectHelpPage());
            });

            ConnectCommand = new Command(async () =>
            {
                if (!Clock.IsConnecting)
                {
                    ConnectingTimer.Start();
                }
                else
                {
                    // else cancel the connection attempt
                    ConnectingTimer.Stop();
                    await Clock.Disconnect();
                    Page.connectButton.Text = "Connect";
                    Page.welcomeLabel.Text = "Welcome! Tap the button below to connect to your clock.";
                }
            });

            ConnectingTimer = new Timer();
            ConnectingTimer.Enabled = false;
            ConnectingTimer.Interval = 100; // ms
            ConnectingTimer.AutoReset = false;
            ConnectingTimer.Elapsed += CheckConnectingTimer;

            ConnectingTimer.Start();
        }

        private async void BeginConnecting()
        {
            await Device.InvokeOnMainThreadAsync(() =>
            {
                Page.connectButton.Text = "Connecting...";
                Page.welcomeLabel.Text = "Attempting to connect to your clock...";
            });
            _ = Clock.Connect();
        }

        private async void CheckConnectingTimer(object sender, ElapsedEventArgs e)
        {
            if (Clock.Connected)
            {
                await Device.InvokeOnMainThreadAsync(async () =>
                {
                    Page.connectButton.Text = "Connected";
                    await Application.Current.MainPage.Navigation.PushAsync(new HomePage(Clock));
                    Page.connectButton.Text = "Connect";
                    Page.welcomeLabel.Text = "Welcome! Tap the button below to connect to your clock.";
                });
                return;
            }

            if (!Clock.Connected)
            {
                if (!Clock.IsConnecting)
                {
                    // restart trying to connect
                    BeginConnecting();
                }
                ConnectingTimer.Start();
            }
        }

        public Command ConnectCommand { get; }
        public Command HelpCommand { get; }
    }
}

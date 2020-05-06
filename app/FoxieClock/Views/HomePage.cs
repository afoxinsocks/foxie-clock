using System.Timers;
using Xamarin.Forms;

namespace FoxieClock
{
    public class HomePage : ContentPage
    {
        BLEClock Clock;
        HomePageViewModel ViewModel;
        public Slider ColorSlider;
        public Slider BrightnessSlider;

        public HomePage(BLEClock clock)
        {
            Clock = clock;

            BackgroundColor = Color.FromHex("290f4c");

            Title = "Home";

            ViewModel = new HomePageViewModel(clock, this);
            BindingContext = ViewModel;

            var help = new ToolbarItem
            {
                Text = "Help",
            };
            help.SetBinding(ToolbarItem.CommandProperty, nameof(HomePageViewModel.HelpCommand));

            ToolbarItems.Add(help);

            var foxieImage = new Image
            {
                Source = "Foxie"
            };
            if (Device.RuntimePlatform == Device.Android)
            {
                foxieImage.Source = "foxie_icon.png";
            }

            var animationButton = new Button
            {
                Text = "Animations",
                FontSize = 30,
                CornerRadius = 20,
                TextColor = Color.Black,
                BackgroundColor = Color.FromHex("e4ac2a"),
                Margin = new Thickness(5)
            };
            animationButton.SetBinding(Button.CommandProperty, nameof(HomePageViewModel.AnimationsCommand));

            var colorLabel = new Label
            {
                Text = "Color",
                FontSize = 18,
                TextColor = Color.White,
                HorizontalTextAlignment = TextAlignment.End,
                VerticalTextAlignment = TextAlignment.Center,
                Margin = new Thickness(3),
            };
            ColorSlider = new Slider
            {
                Minimum = 0,
                Maximum = 255,
            };
            ColorSlider.ValueChanged += (s, e) =>
            {
                ViewModel.ChangeColor();
            };

            var brightnessLabel = new Label
            {
                Text = "Brightness",
                FontSize = 18,
                TextColor = Color.White,
                HorizontalTextAlignment = TextAlignment.End,
                VerticalTextAlignment = TextAlignment.Center,
                Margin = new Thickness(3),
            };
            BrightnessSlider = new Slider
            {
                Minimum = 0,
                Maximum = 192,
            };
            BrightnessSlider.ValueChanged += (s, e) =>
            {
                ViewModel.ChangeBrightness();
            };

            var digitModeButton = new Button
            {
                Text = "Digit display mode",
                FontSize = 20,
                CornerRadius = 20,
                TextColor = Color.Black,
                BackgroundColor = Color.FromHex("e4832a"),
                Margin = new Thickness(5)
            };
            digitModeButton.SetBinding(Button.CommandProperty, nameof(HomePageViewModel.SetDigitDisplayModeCommand));

            var toggle24HButton = new Button
            {
                Text = "12/24H",
                FontSize = 18,
                CornerRadius = 20,
                TextColor = Color.Black,
                BackgroundColor = Color.FromHex("e4832a"),
                Margin = new Thickness(5)
            };
            toggle24HButton.SetBinding(Button.CommandProperty, nameof(HomePageViewModel.Set24hTimeMode));

            var blinkerButton = new Button
            {
                Text = "Blinkers",
                FontSize = 20,
                CornerRadius = 20,
                TextColor = Color.Black,
                BackgroundColor = Color.FromHex("e4832a"),
                Margin = new Thickness(5)
            };
            blinkerButton.SetBinding(Button.CommandProperty, nameof(HomePageViewModel.SetBlinkerMode));

            var setTimeButton = new Button
            {
                Text = "Set time",
                FontSize = 20,
                CornerRadius = 20,
                TextColor = Color.Black,
                BackgroundColor = Color.FromHex("e4832a"),
                Margin = new Thickness(5)
            };
            setTimeButton.SetBinding(Button.CommandProperty, nameof(HomePageViewModel.SetTimeCommand));

            var disconnectButton = new Button
            {
                Text = "Disconnect",
                FontSize = 20,
                CornerRadius = 15,
                TextColor = Color.Black,
                BackgroundColor = Color.LightGray,
                Margin = new Thickness(5)
            };
            disconnectButton.SetBinding(Button.CommandProperty, nameof(HomePageViewModel.DisconnectCommand));


            var grid = new Grid
            {
                Margin = new Thickness(30, 30),

                ColumnDefinitions =
                {
                    new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) },
                    new ColumnDefinition { Width = new GridLength(2, GridUnitType.Star) },
                },
                RowDefinitions =
                {
                    new RowDefinition { Height = new GridLength(0.1, GridUnitType.Star) },
                    new RowDefinition { Height = new GridLength(1, GridUnitType.Star) }, // animations

                    new RowDefinition { Height = new GridLength(0.25, GridUnitType.Star) }, // empty

                    new RowDefinition { Height = new GridLength(0.5, GridUnitType.Star) }, // colors
                    new RowDefinition { Height = new GridLength(0.5, GridUnitType.Star) }, // brightness

                    new RowDefinition { Height = new GridLength(0.25, GridUnitType.Star) }, // empty

                    new RowDefinition { Height = new GridLength(1, GridUnitType.Star) }, // digit display mode
                    new RowDefinition { Height = new GridLength(1, GridUnitType.Star) }, // toggle 24h mode
                    new RowDefinition { Height = new GridLength(1, GridUnitType.Star) }, // set time

                    new RowDefinition { Height = new GridLength(1, GridUnitType.Star) }, // foxie logo

                    new RowDefinition { Height = new GridLength(1, GridUnitType.Star) }, // disconnect
                }
            };


            grid.Children.Add(animationButton, 0, 1);
            Grid.SetColumnSpan(animationButton, 2);

            grid.Children.Add(colorLabel, 0, 3);
            grid.Children.Add(ColorSlider, 1, 3);

            grid.Children.Add(brightnessLabel, 0, 4);
            grid.Children.Add(BrightnessSlider, 1, 4);

            grid.Children.Add(digitModeButton, 0, 6);
            Grid.SetColumnSpan(digitModeButton, 2);
            grid.Children.Add(toggle24HButton, 0, 7);
            grid.Children.Add(blinkerButton, 1, 7);
            grid.Children.Add(setTimeButton, 0, 8);
            Grid.SetColumnSpan(setTimeButton, 2);

            grid.Children.Add(foxieImage, 0, 9);
            Grid.SetColumnSpan(foxieImage, 2);

            grid.Children.Add(disconnectButton, 0, 10);
            Grid.SetColumnSpan(disconnectButton, 2);

            Content = grid;
        }

        protected override void OnDisappearing()
        {
            ViewModel.OnDisappearing();
        }

    }

    public class HomePageViewModel
    {
        BLEClock Clock;
        HomePage Page;
        Timer ThrottleTimer;

        bool ChildWindowOpen = false;

        public HomePageViewModel(BLEClock clock, HomePage page)
        {
            Clock = clock;
            Page = page;

            // always try to set the time immediately upon page entry
            _ = Clock.SetTime();

            ThrottleTimer = new Timer();
            ThrottleTimer.AutoReset = false;
            ThrottleTimer.Enabled = false;
            ThrottleTimer.Interval = 100; // ms, don't do this too often

            HelpCommand = new Command(async () =>
            {
                ChildWindowOpen = true;
                await Application.Current.MainPage.Navigation.PushAsync(new HomeHelpPage());
                ChildWindowOpen = false;
            });

            AnimationsCommand = new Command(async () =>
            {
                ChildWindowOpen = true;
                await Application.Current.MainPage.Navigation.PushAsync(new AnimationPage(Clock));
                ChildWindowOpen = false;
            });

            SetDigitDisplayModeCommand = new Command(async () =>
            {
                ChildWindowOpen = true;
                await Application.Current.MainPage.Navigation.PushAsync(new DigitDisplaySelectionPage(Clock));
                ChildWindowOpen = false;
            });

            Set24hTimeMode = new Command(async () =>
            {
                await Clock.Toggle24hTime();
            });

            SetBlinkerMode = new Command(async () =>
            {
                await Clock.ToggleBlinkers();
            });

            SetTimeCommand = new Command(async () =>
            {
                await Clock.SetTime();
            });

            DisconnectCommand = new Command(async () =>
            {
                await Application.Current.MainPage.Navigation.PopAsync();
            });
        }

        public void ChangeColor()
        {
            StopTimer();
            ThrottleTimer.Elapsed += ChangeColorElapsed;
            ThrottleTimer.Start();
        }

        public void ChangeBrightness()
        {
            StopTimer();
            ThrottleTimer.Elapsed += ChangeBrightnessElapsed;
            ThrottleTimer.Start();
        }

        private void StopTimer()
        {
            ThrottleTimer.Stop();
            ThrottleTimer.Elapsed -= ChangeColorElapsed;
            ThrottleTimer.Elapsed -= ChangeBrightnessElapsed;
        }

        private async void ChangeColorElapsed(object sender, ElapsedEventArgs e)
        {
            await Device.InvokeOnMainThreadAsync(async () =>
            {
                await Clock.SetColorWheel((byte)Page.ColorSlider.Value);
            });
            StopTimer();
        }


        private async void ChangeBrightnessElapsed(object sender, ElapsedEventArgs e)
        {
            await Device.InvokeOnMainThreadAsync(async () =>
            {
                await Clock.SetBrightness((byte)Page.BrightnessSlider.Value);
            });
            StopTimer();
        }

        public async void OnDisappearing()
        {
            if (!ChildWindowOpen)
            {
                await Clock.Disconnect();
            }
        }

        public Command HelpCommand { get; }

        public Command AnimationsCommand { get; }

        public Command SetDigitDisplayModeCommand { get; }
        public Command Set24hTimeMode { get; }
        public Command SetBlinkerMode { get; }
        public Command SetTimeCommand { get; }

        public Command DisconnectCommand { get; }
    }
}


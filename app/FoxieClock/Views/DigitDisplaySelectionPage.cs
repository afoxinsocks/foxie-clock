using System;
using System.ComponentModel;
using Xamarin.Forms;

namespace FoxieClock
{
    public class DigitDisplaySelectionPage : ContentPage
    {
        public DigitDisplaySelectionPage(BLEClock clock)
        {
            BackgroundColor = Color.FromHex("290f4c");

            Title = "Digit display mode";

            BindingContext = new DigitDisplaySelectionPageViewModel(clock);

            var instructionsLabel = new Label
            {
                Text = "Are you using the clear edge-lit acrylic digit panels or the diffuser lid for RGB Pixel display mode?",
                FontSize = 25,
                HorizontalTextAlignment = TextAlignment.Center,
                TextColor = Color.White,
                Margin = new Thickness(10),
            };

            var edgeLitButton = new Button
            {
                Text = "Edge lit acrylics",
                FontSize = 25,
                CornerRadius = 20,
                TextColor = Color.Black,
                BackgroundColor = Color.FromHex("e4ac2a"),
                Margin = new Thickness(10)
            };
            edgeLitButton.SetBinding(Button.CommandProperty, nameof(DigitDisplaySelectionPageViewModel.EdgeLitCommand));

            var pixelDisplayButton = new Button
            {
                Text = "Pixel display",
                FontSize = 25,
                CornerRadius = 20,
                TextColor = Color.Black,
                BackgroundColor = Color.FromHex("e4ac2a"),
                Margin = new Thickness(10)
            };
            pixelDisplayButton.SetBinding(Button.CommandProperty, nameof(DigitDisplaySelectionPageViewModel.PixelCommand));


            var grid = new Grid
            {
                Margin = new Thickness(30, 30),

                ColumnDefinitions =
                {
                    new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) },
                },
                RowDefinitions =
                {
                    new RowDefinition { Height = new GridLength(0.5, GridUnitType.Star) }, // empty
                    new RowDefinition { Height = new GridLength(2, GridUnitType.Star) }, // label
                    new RowDefinition { Height = new GridLength(0.5, GridUnitType.Star) }, // empty
                    new RowDefinition { Height = new GridLength(1, GridUnitType.Star) }, // digit display mode
                    new RowDefinition { Height = new GridLength(1, GridUnitType.Star) }, // toggle 24h mode
                    new RowDefinition { Height = new GridLength(3, GridUnitType.Star) }, // set time
                }
            };

            grid.Children.Add(instructionsLabel, 0, 1);
            grid.Children.Add(edgeLitButton, 0, 3);
            grid.Children.Add(pixelDisplayButton, 0, 4);
            Content = grid;
        }
    }

    public class DigitDisplaySelectionPageViewModel
    {
        BLEClock Clock;

        public DigitDisplaySelectionPageViewModel(BLEClock clock)
        {
            Clock = clock;

            EdgeLitCommand = new Command(async () =>
            {
                await Clock.SetDisplayMode(BLEClock.DisplayMode_e.EDGE_LIT);
                await Application.Current.MainPage.Navigation.PopAsync();
            });

            PixelCommand = new Command(async () =>
            {
                await Clock.SetDisplayMode(BLEClock.DisplayMode_e.PIXEL);
                await Application.Current.MainPage.Navigation.PopAsync();
            });
        }
        public Command EdgeLitCommand { get; }
        public Command PixelCommand { get; }
    }
}


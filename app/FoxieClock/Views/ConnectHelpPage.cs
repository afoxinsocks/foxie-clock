using System;
using Xamarin.Essentials;
using Xamarin.Forms;

namespace FoxieClock
{
    public class ConnectHelpPage : ContentPage
    {
        public ConnectHelpPage()
        {
            BackgroundColor = Color.FromHex("290f4c");

            Title = "Help";

            var clockImage = new Image
            {
                Source = "FoxieClockBig"
            };
            if (Device.RuntimePlatform == Device.Android)
            {
                clockImage.Source = "foxie_clock_big.png";
            }

            var helpLabel = new Label
            {
                Text = "Trouble connecting to your Foxie Clock?\n\nCheck the following:\n\n" +
                "1. Is Bluetooth enabled?\n" +
                "2. Is the clock powered on?\n" +
                "3. Is the clock close to you?\n\n" +
                "For additional support, tap here to visit the Foxie Clock store and select \"Contact Seller\" to send a message.",
                FontSize = 25,
                HorizontalTextAlignment = TextAlignment.Start,
                VerticalTextAlignment = TextAlignment.Start,
                TextColor = Color.White,
                Margin = new Thickness(30, 5),
            };
            Uri uri = new Uri("http://www.foxieclock.com");
            helpLabel.GestureRecognizers.Add(new TapGestureRecognizer
            {
                Command = new Command(() => Browser.OpenAsync(uri))
            });

            var grid = new Grid
            {
                Margin = new Thickness(0, 0),

                ColumnDefinitions =
                {
                    new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) },
                },
                RowDefinitions =
                {
                    new RowDefinition { Height = new GridLength(1, GridUnitType.Star) },
                    new RowDefinition { Height = new GridLength(2, GridUnitType.Star) },
                }
            };

            grid.Children.Add(clockImage, 0, 0);
            grid.Children.Add(helpLabel, 0, 1);

            ScrollView scrollView = new ScrollView();
            scrollView.Content = grid;
            Content = scrollView;
        }
    }
}

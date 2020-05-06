using System;
using Xamarin.Essentials;
using Xamarin.Forms;

namespace FoxieClock
{
    public class HomeHelpPage : ContentPage
    {
        public HomeHelpPage()
        {
            BackgroundColor = Color.FromHex("290f4c");

            Title = "Help";

            var clockImage = new Image
            {
                Source = "FoxieBackSmall"
            };
            if (Device.RuntimePlatform == Device.Android)
            {
                clockImage.Source = "foxie_back_small.png";
            }

            var helpLabel = new Label
            {
                Text = "In addition to the buttons on the back of the Foxie Clock, there are several different settings that " +
                "can be configured. It is also very easy to change the clock firmware yourself to add more features, " +
                "visit github.com/afoxinsocks/foxie-clock/ for more information.\n\n" +
                "Animations - Several animations are built-in to the firmware, but you're always free to add more! " +
                "If you add more, you will want to use the \"Unlisted mode\" animation.\n\n" +
                "Color/Brightness - These sliders will change these settings for the clock, and it's important to note " +
                "that the Color slider doesn't behave the same way in all animation modes.\n\n" +
                "Digit display mode - One of the neatest features of the Foxie Clock is the ability to " +
                "use it in various display modes, including edge-lit, pixel, and more in the future (binary?!). " +
                "This option allows you to switch between them. \n\n" +
                "12/24H - Switch between 12 and 24 hour mode\n\n" +
                "Blinkers - These are the blinking LEDs between the digits and can be toggled on/off\n\n" +
                "Set Time - Connecting to your clock will always automatically synchronize the time, but you can force it " +
                "if needed.\n\n",
                FontSize = 20,
                HorizontalTextAlignment = TextAlignment.Start,
                VerticalTextAlignment = TextAlignment.Start,
                TextColor = Color.White,
                Margin = new Thickness(10),
            };

            var grid = new Grid
            {
                Margin = new Thickness(30, 30),

                ColumnDefinitions =
                {
                    new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) },
                },
                RowDefinitions =
                {
                    new RowDefinition { Height = new GridLength(1, GridUnitType.Star) },
                    new RowDefinition { Height = new GridLength(5, GridUnitType.Star) },
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

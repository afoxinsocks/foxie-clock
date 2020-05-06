using System;
using Xamarin.Forms;

namespace FoxieClock
{
    public class App : Xamarin.Forms.Application
    {
        public App()
        {
            var navigationPage = new NavigationPage(new ConnectPage())
            {
                BarBackgroundColor = Color.FromHex("290f4c"),
                BackgroundColor = Color.FromHex("290f4c"),
                BarTextColor = Color.White,
            };
            MainPage = navigationPage;
        }
    }
}

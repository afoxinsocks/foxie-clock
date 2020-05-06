using System;

using Android.App;
using Android.Content.PM;
using Android.Runtime;
using Android.Views;
using Android.Widget;
using Android.OS;
using System.Threading.Tasks;
using Android;
using Android.Support.Design.Widget;

namespace FoxieClock.Droid
{
    [Activity(Label = "FoxieClock", Icon = "@drawable/foxie_icon", Theme = "@style/MainTheme", MainLauncher = true, ScreenOrientation = ScreenOrientation.Portrait, ConfigurationChanges = ConfigChanges.ScreenSize)]
    public class MainActivity : global::Xamarin.Forms.Platform.Android.FormsAppCompatActivity
    {
        protected override void OnCreate(Bundle savedInstanceState)
        {
            TabLayoutResource = Resource.Layout.Tabbar;
            ToolbarResource = Resource.Layout.Toolbar;

            base.OnCreate(savedInstanceState);

            Xamarin.Essentials.Platform.Init(this, savedInstanceState);
            global::Xamarin.Forms.Forms.Init(this, savedInstanceState);

            EnsurePermissions();

            LoadApplication(new App());
        }
        public override void OnRequestPermissionsResult(int requestCode, string[] permissions, [GeneratedEnum] Android.Content.PM.Permission[] grantResults)
        {
            const string permission = Manifest.Permission.AccessFineLocation;
            if (CheckSelfPermission(permission) == (int)Permission.Granted)
            {

            }

            Xamarin.Essentials.Platform.OnRequestPermissionsResult(requestCode, permissions, grantResults);

            base.OnRequestPermissionsResult(requestCode, permissions, grantResults);
        }

        private void EnsurePermissions()
        {
            bool ask = false;

            ask = ask && (CheckSelfPermission(Manifest.Permission.AccessFineLocation) != (int)Permission.Granted);
            ask = ask && (CheckSelfPermission(Manifest.Permission.AccessCoarseLocation) != (int)Permission.Granted);
            ask = ask && (CheckSelfPermission(Manifest.Permission.Bluetooth) != (int)Permission.Granted);
            ask = ask && (CheckSelfPermission(Manifest.Permission.BluetoothAdmin) != (int)Permission.Granted);

            if (ask)
            {
                // ask the user
                string[] permissions = { Manifest.Permission.AccessFineLocation, Manifest.Permission.AccessCoarseLocation, Manifest.Permission.Bluetooth, Manifest.Permission.BluetoothAdmin };
                const int RequestLocationId = 0;
                Snackbar.Make(null, permissions + " access is required for connecting to your Foxie Clock.", Snackbar.LengthIndefinite)
                .SetAction("OK", v => RequestPermissions(permissions, RequestLocationId))
                .Show();
                RequestPermissions(permissions, RequestLocationId);
            }
        }
    }
}
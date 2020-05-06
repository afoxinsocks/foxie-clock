using System.Collections;
using System.Collections.Generic;
using Xamarin.Forms;

namespace FoxieClock
{
    public class AnimationPage : ContentPage
    {
        BLEClock Clock;
        AnimationPageViewModel ViewModel;
        public Entry UnlistedModeEntry;

        public AnimationPage(BLEClock clock)
        {
            Clock = clock;

            BackgroundColor = Color.FromHex("290f4c");

            Title = "Animations";

            ViewModel = new AnimationPageViewModel(clock, this);
            BindingContext = ViewModel;

            var helpLabel = new Label
            {
                Text = "Select the desired animation mode for your clock:",
                FontSize = 20,
                HorizontalTextAlignment = TextAlignment.Start,
                VerticalTextAlignment = TextAlignment.Start,
                TextColor = Color.White,
                Margin = new Thickness(5),
            };

            var animNoneBtn = new Button
            {
                Text = "None",
                FontSize = 25,
                CornerRadius = 15,
                TextColor = Color.Black,
                BackgroundColor = Color.FromHex("e4ac2a"),
                Margin = new Thickness(10)
            };
            animNoneBtn.SetBinding(Button.CommandProperty, nameof(AnimationPageViewModel.AnimNoneCommand));

            var animGlowBtn = new Button
            {
                Text = "Glow",
                FontSize = 25,
                CornerRadius = 15,
                TextColor = Color.Black,
                BackgroundColor = Color.FromHex("e4ac2a"),
                Margin = new Thickness(10)
            };
            animGlowBtn.SetBinding(Button.CommandProperty, nameof(AnimationPageViewModel.AnimGlowCommand));

            var animCycleColorsBtn = new Button
            {
                Text = "Cycle Colors",
                FontSize = 25,
                CornerRadius = 15,
                TextColor = Color.Black,
                BackgroundColor = Color.FromHex("e4ac2a"),
                Margin = new Thickness(10)
            };
            animCycleColorsBtn.SetBinding(Button.CommandProperty, nameof(AnimationPageViewModel.AnimCycleCommand));

            var animCycleFlowLeftBtn = new Button
            {
                Text = "Cycle Flow Left",
                FontSize = 25,
                CornerRadius = 15,
                TextColor = Color.Black,
                BackgroundColor = Color.FromHex("e4ac2a"),
                Margin = new Thickness(10)
            };
            animCycleFlowLeftBtn.SetBinding(Button.CommandProperty, nameof(AnimationPageViewModel.AnimCycleFlowLeftCommand));

            var animCustomBtn = new Button
            {
                Text = "Unlisted mode",
                FontSize = 25,
                CornerRadius = 15,
                TextColor = Color.Black,
                BackgroundColor = Color.FromHex("e4ac2a"),
                Margin = new Thickness(10)
            };
            animCustomBtn.SetBinding(Button.CommandProperty, nameof(AnimationPageViewModel.AnimUnlistedCommand));

            UnlistedModeEntry = new Entry
            {
                Text = "0",
                FontSize = 25,
            };

            var grid = new Grid
            {
                Margin = new Thickness(30, 30),

                ColumnDefinitions =
                {
                    new ColumnDefinition { Width = new GridLength(3, GridUnitType.Star) },
                    new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) },
                },
                RowDefinitions =
                {
                    new RowDefinition { Height = new GridLength(0.1, GridUnitType.Star) },
                    new RowDefinition { Height = new GridLength(1, GridUnitType.Star) },
                    new RowDefinition { Height = new GridLength(1, GridUnitType.Star) },
                    new RowDefinition { Height = new GridLength(1, GridUnitType.Star) },
                    new RowDefinition { Height = new GridLength(1, GridUnitType.Star) },
                    new RowDefinition { Height = new GridLength(1, GridUnitType.Star) },
                    new RowDefinition { Height = new GridLength(1, GridUnitType.Star) },
                    new RowDefinition { Height = new GridLength(2, GridUnitType.Star) }, // empty
                }
            };

            grid.Children.Add(helpLabel, 0, 1);
            Grid.SetColumnSpan(helpLabel, 2);
            grid.Children.Add(animNoneBtn, 0, 2);
            Grid.SetColumnSpan(animNoneBtn, 2);
            grid.Children.Add(animGlowBtn, 0, 3);
            Grid.SetColumnSpan(animGlowBtn, 2);
            grid.Children.Add(animCycleColorsBtn, 0, 4);
            Grid.SetColumnSpan(animCycleColorsBtn, 2);
            grid.Children.Add(animCycleFlowLeftBtn, 0, 5);
            Grid.SetColumnSpan(animCycleFlowLeftBtn, 2);
            grid.Children.Add(animCustomBtn, 0, 6);
            grid.Children.Add(UnlistedModeEntry, 1, 6);

            //Content = grid;
            ScrollView scrollView = new ScrollView();
            scrollView.Content = grid;
            Content = scrollView;
        }
    }

    public class AnimationPageViewModel
    {
        BLEClock Clock;
        AnimationPage Page;

        public AnimationPageViewModel(BLEClock clock, AnimationPage page)
        {
            Clock = clock;
            Page = page;

            AnimNoneCommand = new Command(async () =>
            {
                await Clock.SetAnimation(0);
            });

            AnimGlowCommand = new Command(async () =>
            {
                await Clock.SetAnimation(1);
            });

            AnimCycleCommand = new Command(async () =>
            {
                await Clock.SetAnimation(2);
            });

            AnimCycleFlowLeftCommand = new Command(async () =>
            {
                await Clock.SetAnimation(3);
            });

            AnimUnlistedCommand = new Command(async () =>
            {
                await Clock.SetAnimation((byte)int.Parse(page.UnlistedModeEntry.Text));
            });
        }

        public Command AnimNoneCommand { get; }
        public Command AnimGlowCommand { get; }
        public Command AnimCycleCommand { get; }
        public Command AnimCycleFlowLeftCommand { get; }
        public Command AnimUnlistedCommand { get; }
    }
}

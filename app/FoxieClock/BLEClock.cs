using Plugin.BLE;
using Plugin.BLE.Abstractions;
using Plugin.BLE.Abstractions.Contracts;
using Plugin.BLE.Abstractions.EventArgs;
using System;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;

namespace FoxieClock
{
    public class BLEClock
    {
        public bool Connected { get; private set; }
        public bool IsConnecting { get; private set; }
        IDevice Device;
        ICharacteristic AlertLevel;

        private bool Is24HTime { get; set; }
        private bool IsBlinking { get; set; }

        enum SettingNames_e
        {
            SETTING_DIGIT_TYPE = 0,
            SETTING_CUR_BRIGHTNESS,
            SETTING_MIN_BRIGHTNESS,
            SETTING_MAX_BRIGHTNESS,
            SETTING_BLINKING_SEPARATORS,
            SETTING_COLOR,
            SETTING_ANIMATION_TYPE,
            SETTING_24_HOUR_MODE,

            // Do not add settings after this line
            TOTAL_SETTINGS,
        };

        public BLEClock()
        {
            Connected = false;
            IsConnecting = false;
            Is24HTime = false;
            IsBlinking = true;
        }

        public async Task Connect()
        {
            await Disconnect();

            var ble = CrossBluetoothLE.Current;
            var adapter = ble.Adapter;

            if (!ble.IsAvailable || !ble.IsOn)
            {
                return;
            }

            adapter.ScanMode = ScanMode.Balanced;
            adapter.ScanTimeout = 10000;
            adapter.DeviceDiscovered += DiscoverDevice;

            IsConnecting = true;
            await adapter.StartScanningForDevicesAsync();

            adapter.DeviceDiscovered -= DiscoverDevice;
            IsConnecting = false;
        }

        private async void DiscoverDevice(object sender, DeviceEventArgs a)
        {
            var ble = CrossBluetoothLE.Current;
            var adapter = ble.Adapter;

            if (a.Device.Name == "Foxie Clock")
            {
                try
                {
                    CancellationTokenSource tokenSource = new CancellationTokenSource();
                    await adapter.ConnectToDeviceAsync(a.Device, new ConnectParameters(autoConnect: true, forceBleTransport: true), tokenSource.Token);
                    var services = await a.Device.GetServicesAsync();
                    for (int i = 0; i < services.Count; ++i)
                    {
                        if (services[i].Name == "Immediate Alert")
                        {
                            var immediateAlertServiceCharacteristics = await services[i].GetCharacteristicsAsync();
                            AlertLevel = immediateAlertServiceCharacteristics[0];
                            Device = a.Device;
                            IsConnecting = false;
                            Connected = true;
                            Debug.WriteLine(Device.ToString());
                            break;
                        }
                    }
                }
                catch (Exception ex)
                {
                    // don't know what to do with this
                    Plugin.BLE.Abstractions.Trace.Message(ex.Message);
                    return;
                }

                await adapter.StopScanningForDevicesAsync();
                IsConnecting = false;
            }
        }

        public async Task Disconnect()
        {
            if (IsConnecting)
            {
                await CrossBluetoothLE.Current.Adapter.StopScanningForDevicesAsync();
            }

            if (Connected)
            {
                await CrossBluetoothLE.Current.Adapter.DisconnectDeviceAsync(Device);
            }

            IsConnecting = false;
            Connected = false;
        }

        public async Task SetColorWheel(byte colorWheelValue)
        {
            byte[] settingData = { (byte)SettingNames_e.SETTING_COLOR, 0, 0, 0, colorWheelValue };
            await SendCommand(Device, Command_e.CMD_CHANGE_SETTING, settingData);
        }

        public async Task SetBrightness(byte brightness)
        {
            if (brightness < 4)
            {
                brightness = 4;
            }

            byte[] settingData = { (byte)SettingNames_e.SETTING_CUR_BRIGHTNESS, 0, 0, 0, brightness };
            await SendCommand(Device, Command_e.CMD_CHANGE_SETTING, settingData);
        }

        public enum DisplayMode_e
        {
            EDGE_LIT = 1,
            PIXEL = 2,
        }
        public async Task SetDisplayMode(DisplayMode_e mode)
        {
            byte[] settingData = { (byte)SettingNames_e.SETTING_DIGIT_TYPE, 0, 0, 0, (byte)(mode) };
            await SendCommand(Device, Command_e.CMD_CHANGE_SETTING, settingData);
        }

        public async Task SetAnimation(byte mode)
        {
            byte[] settingData = { (byte)SettingNames_e.SETTING_ANIMATION_TYPE, 0, 0, 0, mode };
            await SendCommand(Device, Command_e.CMD_CHANGE_SETTING, settingData);
        }


        public async Task Toggle24hTime()
        {
            Is24HTime = !Is24HTime;
            byte[] settingData = { (byte)SettingNames_e.SETTING_24_HOUR_MODE, 0, 0, 0, (byte)(Is24HTime ? 1 : 0) };
            await SendCommand(Device, Command_e.CMD_CHANGE_SETTING, settingData);
        }

        public async Task ToggleBlinkers()
        {
            IsBlinking = !IsBlinking;
            byte[] settingData = { (byte)SettingNames_e.SETTING_BLINKING_SEPARATORS, 0, 0, 0, (byte)(IsBlinking ? 1 : 0) };
            await SendCommand(Device, Command_e.CMD_CHANGE_SETTING, settingData);
        }

        public async Task SetTime()
        {
            byte[] timeData = { (byte)DateTime.Now.Hour, (byte)DateTime.Now.Minute, (byte)DateTime.Now.Second };
            await SendCommand(Device, Command_e.CMD_SET_TIME, timeData);
        }

        enum Command_e
        {
            CMD_SET_TIME = 0x10,
            CMD_CHANGE_SETTING = 0x11,
        };

        private async Task SendCommand(IDevice device, Command_e cmd, byte[] data)
        {
            if (!Connected)
            {
                return;
            }

            byte[] bytes = new byte[data.Length + 2];
            bytes[0] = (byte)(data.Length + 1);
            bytes[1] = (byte)cmd;
            Buffer.BlockCopy(data, 0, bytes, 2, data.Length);

            // send one byte at a time because of currently limited BT functionality with SparkFun Nano
            for (int i = 0; i < bytes.Length; ++i)
            {
                byte[] singleByte = { bytes[i] };
                await AlertLevel.WriteAsync(singleByte);
                Thread.Sleep(25);
            }
        }
    }
}

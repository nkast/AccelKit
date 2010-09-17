#region Copyright notice
//-----------------------------------------------------------------------------
// AccelerometerEmu.cs 
// Copyright (C) Kastellanos Nikos. All rights reserved.
//-----------------------------------------------------------------------------
#endregion


using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Devices.Sensors;
using System.Security;
using System.Threading;
using System.Net;
using System.Windows.Media.Animation;

namespace NKast.Sensors
{
    class AccelerometerEmu : IDisposable
    {
        private static Microsoft.Devices.Sensors.Accelerometer accelerometer;
        private Storyboard timer;
        
        public event EventHandler<AccelerometerEmuReadingEventArgs> ReadingEmuChanged;
        
        [SecuritySafeCritical]
        public AccelerometerEmu()
        {
            accelerometer = new Microsoft.Devices.Sensors.Accelerometer();
            
            if (Microsoft.Devices.Environment.DeviceType == Microsoft.Devices.DeviceType.Device)
            {
                ReadingChanged += new EventHandler<AccelerometerReadingEventArgs>(AccelerometerEmu_ReadingChanged);
            }

            return;
        }
       
        public SensorState State 
        { 
            get {return accelerometer.State;}
        }

        // Summary:
        // Occurs when new data arrives from the accelerometer.
        public event EventHandler<AccelerometerReadingEventArgs> ReadingChanged;

        [SecuritySafeCritical]
        public void Dispose()
        {
            accelerometer.Dispose();
        }
      
        [SecuritySafeCritical]
        public void Start()
        {
            accelerometer.Start();

            if (Microsoft.Devices.Environment.DeviceType == Microsoft.Devices.DeviceType.Emulator)
            {
                timer = new Storyboard();               
                timer.Duration = TimeSpan.FromMilliseconds(1000/10);
                timer.Completed += new EventHandler(TimerCallBack);
                timer.Begin();                
            }
          
        }
        //
        // Summary:
        // Stops data acquisition from the accelerometer.
        [SecuritySafeCritical]
        public void Stop()
        {
            accelerometer.Stop();
            if (Microsoft.Devices.Environment.DeviceType == Microsoft.Devices.DeviceType.Emulator)
            {
                timer.Stop();
                timer = null;
            }
        }

        void AccelerometerEmu_ReadingChanged(object sender, AccelerometerReadingEventArgs e)
        {
            ReadingChanged(this, e);
        }

        public void TimerCallBack(object sender, EventArgs e)
        {            
            WebClient wc;
            wc = new WebClient();
            wc.AllowReadStreamBuffering = false;
            wc.DownloadStringCompleted += new DownloadStringCompletedEventHandler(wc_DownloadStringCompleted);
            wc.DownloadStringAsync(new Uri("http://127.0.0.1:88/"));
            
            return;
        }

        void wc_DownloadStringCompleted(object sender, DownloadStringCompletedEventArgs e)
        {          
            if (e.Error != null) { timer.Begin(); return; }
            if (e.Result == null) { timer.Begin(); return; }
            string[] vc = e.Result.Split(new Char[] {',', ' '});
            double x = Convert.ToDouble(vc[0]);
            double y = Convert.ToDouble(vc[1]);
            double z = Convert.ToDouble(vc[2]);

            AccelerometerEmuReadingEventArgs are = new AccelerometerEmuReadingEventArgs(x, y, z);
            ReadingEmuChanged(this, are);

            timer.Begin();            
            return;
        }

              
    }
}

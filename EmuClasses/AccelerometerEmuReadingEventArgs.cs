#region Copyright notice
//-----------------------------------------------------------------------------
// AccelerometerEmuReadingEventArgs.cs 
// Copyright (C) Kastellanos Nikos. All rights reserved.
//-----------------------------------------------------------------------------
#endregion

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Devices.Sensors;

namespace NKast.Sensors
{
    class AccelerometerEmuReadingEventArgs : EventArgs
    {

        private double _X , _Y , _Z;

        public AccelerometerEmuReadingEventArgs(double x, double y, double z)
        {            
            this._X = x;
            this._Y = y;
            this._Z = z;
        }

        public DateTimeOffset Timestamp { get { return new DateTimeOffset(); } }
        public double X { get { return _X;} }
        public double Y { get { return _Y; } }
        public double Z { get { return _Z; } }
        
    }
}

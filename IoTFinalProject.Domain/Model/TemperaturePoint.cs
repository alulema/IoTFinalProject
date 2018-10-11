using System;
using System.Runtime.Serialization;

namespace IoTFinalProject.Domain.Model
{
    [DataContract]
    public class PlotDataPoint
    {
        public PlotDataPoint(float val, DateTime time)
        {
            Value = val;
            Time = time;
        }
        
        [DataMember(Name = "y")]
        public float Value { get; }
        [DataMember(Name = "x")]
        public DateTime Time { get; }
    }
}
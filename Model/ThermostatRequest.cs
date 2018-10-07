using System;

namespace IoTFinalProject.Model
{
    public class ThermostatRequest
    {
        public int Id { get; set; }
        public string DeviceId { get; set; }
        public string DeviceStatus { get; set; }
        public float Measurement { get; set; }
        public string Unit { get; set; }
        public DateTime Timestamp { get; set; }
    }
}
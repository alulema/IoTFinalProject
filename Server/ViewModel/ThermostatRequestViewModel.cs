using System.Runtime.Serialization;

namespace IoTFinalProject.ViewModel
{
    [DataContract]
    public class ThermostatRequestViewModel
    {
        [DataMember(Name = "id")]
        public string DeviceId { get; set; }
        [DataMember(Name = "status")]
        public string DeviceStatus { get; set; }
        [DataMember(Name = "measurement")]
        public float Measurement { get; set; }
        [DataMember(Name = "unit")]
        public string Unit { get; set; }
    }
}
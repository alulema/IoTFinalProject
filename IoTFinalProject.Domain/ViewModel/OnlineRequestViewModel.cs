using System;
using System.Runtime.Serialization;

namespace IoTFinalProject.Domain.ViewModel
{
    [DataContract]
    public class OnlineRequestViewModel
    {
        [DataMember(Name = "device_id")]
        public string DeviceId { get; set; }
        [DataMember(Name = "unit")]
        public string Unit { get; set; }
        [DataMember(Name = "type")]
        public string DeviceType { get; set; }

        public DateTime Timestamp { get; set; }
    }
}
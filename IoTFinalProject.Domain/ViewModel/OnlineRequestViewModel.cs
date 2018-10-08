using System;
using System.Runtime.Serialization;

namespace IoTFinalProject.Domain.ViewModel
{
    [DataContract]
    public class OnlineRequestViewModel
    {
        [DataMember(Name = "device-id")]
        public string DeviceId { get; set; }
        [DataMember(Name = "unit")]
        public string Unit { get; set; }

        public DateTime Timestamp { get; set; }
    }
}
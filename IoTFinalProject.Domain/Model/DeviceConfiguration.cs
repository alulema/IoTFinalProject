using System;
using System.Collections.Generic;
using System.Runtime.Serialization;

namespace IoTFinalProject.Domain.Model
{
    [DataContract]
    public class DeviceConfiguration
    {
        public DeviceConfiguration()
        {
            DailyPoints = new List<DailyTemperature>();
            WeekendPoints = new List<DailyTemperature>();
            SpecialDayPoints = new List<DailyTemperature>();
        }

        [DataMember(Name = "device_id")]
        public string DeviceId { get; set; }
        [DataMember(Name = "daily_points")]
        public List<DailyTemperature> DailyPoints { get; private set; }
        [DataMember(Name = "use_weekends")]
        public bool UseWeekendPoints{ get; set; }
        [DataMember(Name = "weekend_points")]
        public List<DailyTemperature> WeekendPoints { get; private set; }
        [DataMember(Name = "use_special_days")]
        public bool UseSpecialDayPoints{ get; set; }
        [DataMember(Name = "special_days")]
        public List<DailyTemperature> SpecialDayPoints { get; private set; }
    }

    [DataContract]
    public class DailyTemperature
    {
        public DailyTemperature()
        {
            IsActive = true;
        }

        [DataMember(Name = "id")]
        public string Id { get; set; }
        [DataMember(Name = "temperature")]
        public float Temperature { get; set; }
        [DataMember(Name = "start_hour")]
        public int StartHour { get; set; }
        [DataMember(Name = "start_minute")]
        public int StartMinute { get; set; }
        [DataMember(Name = "end_hour")]
        public int EndHour { get; set; }
        [DataMember(Name = "end_minute")]
        public int EndMinute { get; set; }
        [DataMember(Name = "is_active")]
        public bool IsActive { get; set; }
        [DataMember(Name = "selected_date")]
        public DateTime? SelectedDate { get; set; }
    }
}
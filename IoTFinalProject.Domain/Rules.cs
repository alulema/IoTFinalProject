using System;
using System.Globalization;
using System.IO;
using IoTFinalProject.Domain.Model;
using Newtonsoft.Json;

namespace IoTFinalProject.Domain
{
    public class Rules
    {
        public static bool DecideActivation(ThermostatRequest request, float defaultLevel)
        {
            var config = GetDeviceConfig(request.DeviceId);
            float tempTarget = defaultLevel;

            // 1. If no levels, then use default
            if (config.SpecialDayPoints.Count > 0 || config.WeekendPoints.Count > 0 || config.DailyPoints.Count > 0)
            {
                DateTime now = DateTime.Now;
                bool hasSelectedPoint = false;

                if (config.SpecialDayPoints.Count > 0)
                {
                    hasSelectedPoint = ScanSpecialDayPoints(config, ref tempTarget);

                    if (!hasSelectedPoint)
                        hasSelectedPoint = ScanWeekendPoints(config, ref tempTarget);

                    if (!hasSelectedPoint)
                        hasSelectedPoint = ScanDailyPoints(config, ref tempTarget);
                }
                else if (config.WeekendPoints.Count > 0)
                {
                    hasSelectedPoint = ScanWeekendPoints(config, ref tempTarget);

                    if (!hasSelectedPoint)
                        hasSelectedPoint = ScanDailyPoints(config, ref tempTarget);
                }
                else
                    hasSelectedPoint = ScanDailyPoints(config, ref tempTarget);

                if (!hasSelectedPoint)
                    tempTarget = defaultLevel;
            }

            return tempTarget > request.Measurement;
        }

        private static bool ScanSpecialDayPoints(DeviceConfiguration config, ref float tempTarget)
        {
            DateTime now = DateTime.Now;

            foreach (var point in config.SpecialDayPoints)
            {
                var ranges = GetTimeRange(point.SelectedDates);

                if (now > ranges[0] && now < (ranges.Length == 2 ? ranges[1] : ranges[0]).AddDays(1))
                    if ((now.Hour * 100 + now.Minute >= point.StartHour * 100 + point.StartMinute) && (now.Hour * 100 + now.Minute <= point.EndHour * 100 + point.EndMinute))
                    {
                        tempTarget = point.Temperature;
                        return true;
                    }
            }

            return false;
        }

        private static bool ScanWeekendPoints(DeviceConfiguration config, ref float tempTarget)
        {
            DateTime now = DateTime.Now;

            foreach (var point in config.WeekendPoints)
            {
                if (point.IsSaturday && now.DayOfWeek == DayOfWeek.Saturday || point.IsSunday && now.DayOfWeek == DayOfWeek.Sunday)
                    if ((now.Hour * 100 + now.Minute >= point.StartHour * 100 + point.StartMinute) && (now.Hour * 100 + now.Minute <= point.EndHour * 100 + point.EndMinute))
                    {
                        tempTarget = point.Temperature;
                        return true;
                    }
            }

            return false;
        }

        private static bool ScanDailyPoints(DeviceConfiguration config, ref float tempTarget)
        {
            DateTime now = DateTime.Now;

            foreach (var point in config.DailyPoints)
            {
                if ((now.Hour * 100 + now.Minute >= point.StartHour * 100 + point.StartMinute) && (now.Hour * 100 + now.Minute <= point.EndHour * 100 + point.EndMinute))
                {
                    tempTarget = point.Temperature;
                    return true;
                }
            }

            return false;
        }

        private static DateTime[] GetTimeRange(string selectedDates)
        {
            var parts = selectedDates.Split('-');
            var from = DateTime.ParseExact(parts[0].Trim(), "MM/dd/yyyy", CultureInfo.CurrentCulture);
            var until = DateTime.ParseExact(parts[1].Trim(), "MM/dd/yyyy", CultureInfo.CurrentCulture);

            if (until > from)
                return new[] { from, until };

            return new[] { from };
        }

        public static DeviceConfiguration GetDeviceConfig(string deviceId)
        {
            if (!Directory.Exists("devices"))
                Directory.CreateDirectory("devices");

            string filePath = $"devices/config.{deviceId}.json";

            DeviceConfiguration config;

            if (!System.IO.File.Exists(filePath))
            {
                config = new DeviceConfiguration
                {
                    DeviceId = deviceId
                };
                var json = JsonConvert.SerializeObject(config);
                StreamWriter fileStream = System.IO.File.CreateText(filePath);
                fileStream.Write(json);
                fileStream.Close();
            }
            else
            {
                var json = System.IO.File.ReadAllText(filePath);
                config = JsonConvert.DeserializeObject<DeviceConfiguration>(json);
            }

            return config;
        }
    }
}

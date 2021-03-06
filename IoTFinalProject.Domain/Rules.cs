﻿using System;
using System.Globalization;
using System.Linq;
using IoTFinalProject.Domain.Data;
using IoTFinalProject.Domain.Model;

namespace IoTFinalProject.Domain
{
    public class Rules
    {
        public static bool DecideActivation(string connString, ThermostatRequest request, float defaultLevel)
        {
            var config = DbService.GetDeviceConfig(connString, request.DeviceId);
            float tempTarget = defaultLevel;

            // 1. If no levels, then use default
            if (config.SpecialDayPoints.Count > 0 || config.WeekendPoints.Count > 0 || config.DailyPoints.Count > 0)
            {
                DateTime now = DateTime.Now;
                bool hasSelectedPoint = false;
                var hasSaturdays = config.WeekendPoints.Any(x => x.IsSaturday);
                var hasSundays = config.WeekendPoints.Any(x => x.IsSunday);

                if (config.SpecialDayPoints.Count > 0)
                {
                    hasSelectedPoint = ScanSpecialDayPoints(config, ref tempTarget);

                    if (!hasSelectedPoint)
                    {
                        if (config.WeekendPoints.Count > 0)
                        {

                            if (now.DayOfWeek == DayOfWeek.Saturday && hasSaturdays)
                                hasSelectedPoint = ScanWeekendPoints(config, ref tempTarget);
                            else if (now.DayOfWeek == DayOfWeek.Sunday && hasSundays)
                                hasSelectedPoint = ScanWeekendPoints(config, ref tempTarget);
                        }
                    }

                    if (!hasSelectedPoint)
                        hasSelectedPoint = ScanDailyPoints(config, ref tempTarget);
                }
                else if (config.WeekendPoints.Count > 0)
                {
                    if (now.DayOfWeek == DayOfWeek.Saturday && hasSaturdays)
                        hasSelectedPoint = ScanWeekendPoints(config, ref tempTarget);
                    else if (now.DayOfWeek == DayOfWeek.Sunday && hasSundays)
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
    }
}

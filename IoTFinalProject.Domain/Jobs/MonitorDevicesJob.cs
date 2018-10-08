using System;
using System.Threading.Tasks;
using IoTFinalProject.Domain.Hubs;
using Quartz;

namespace IoTFinalProject.Domain.Jobs
{
    public class MonitorDevicesJob : IJob
    {
        public Task Execute(IJobExecutionContext context)
        {
            int timeout = (int) context.JobDetail.JobDataMap["timeout"];

            foreach (var key in General.OnlineDevices.Keys)
            {
                if (General.OnlineDevices.ContainsKey(key) && General.OnlineDevices[key].Timestamp.AddSeconds(timeout) > DateTime.Now)
                {
                    General.OnlineDevices.Remove(key);
                }
            }

            if (DeviceMonitoringReferer.LocalInstance != null && DeviceMonitoringReferer.Instance != null)
                DeviceMonitoringReferer.Instance.SendTransactionUpdate("ok").GetAwaiter().GetResult();
            return Task.FromResult(1);
        }
    }
}
using System;
using System.Threading.Tasks;
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

            return Task.FromResult(1);
        }
    }
}
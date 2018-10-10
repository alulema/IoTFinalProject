using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Threading.Tasks;
using IoTFinalProject.Domain.Jobs;
using IoTFinalProject.Domain.ViewModel;
using Quartz;
using Quartz.Impl;

namespace IoTFinalProject.Domain
{
    public static class General
    {
        public static readonly SortedList<string, OnlineRequestViewModel> OnlineDevices = new SortedList<string, OnlineRequestViewModel>();

        public static async Task RunJobs(int timeout)
        {
            try
            {
                NameValueCollection props = new NameValueCollection
                {
                    { "quartz.serializer.type", "binary" }
                };
                StdSchedulerFactory factory = new StdSchedulerFactory(props);
                IScheduler scheduler = await factory.GetScheduler();

                // and start it off
                await scheduler.Start();

                // define the job and tie it to our HelloJob class
                IJobDetail job = JobBuilder.Create<MonitorDevicesJob>()
                    .WithIdentity("job1", "group1")
                    .Build();

                // Trigger the job to run now, and then repeat every 10 seconds
                ITrigger trigger = TriggerBuilder.Create()
                    .WithIdentity("trigger1", "group1")
                    .StartNow()
                    .WithSimpleSchedule(x => x
                        .WithIntervalInSeconds(5)
                        .RepeatForever())
                    .Build();
                job.JobDataMap["timeout"] = timeout;


                // Tell quartz to schedule the job using our trigger
                await scheduler.ScheduleJob(job, trigger);
            }
            catch (SchedulerException se)
            {
                Console.WriteLine(se);
            }
        }

    }
}
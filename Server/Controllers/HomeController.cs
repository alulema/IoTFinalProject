using System;
using System.Diagnostics;
using IoTFinalProject.Domain.Hubs;
using IoTFinalProject.Domain.ViewModel;
using Microsoft.AspNetCore.Mvc;
using Microsoft.AspNetCore.SignalR;

namespace IoTFinalProject.Controllers
{
    public class HomeController : Controller
    {
        private readonly IHubContext<DeviceMonitoringHub> _hubContext;

        public HomeController(IHubContext<DeviceMonitoringHub> hubContext)
        {
            _hubContext = hubContext;

            if (DeviceMonitoringReferer.LocalInstance == null)
                DeviceMonitoringReferer.LocalInstance = new Lazy<DeviceMonitoringReferer>(() => new DeviceMonitoringReferer(hubContext));
        }

        public IActionResult Index()
        {
            return View();
        }

        [ResponseCache(Duration = 0, Location = ResponseCacheLocation.None, NoStore = true)]
        public IActionResult Error()
        {
            return View(new ErrorViewModel { RequestId = Activity.Current?.Id ?? HttpContext.TraceIdentifier });
        }
    }
}
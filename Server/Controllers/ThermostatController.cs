using System;
using IoTFinalProject.Domain;
using IoTFinalProject.Domain.Data;
using IoTFinalProject.Domain.Model;
using IoTFinalProject.Domain.ViewModel;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.Logging;

namespace IoTFinalProject.Controllers
{
    [Route("api/[controller]")]
    [ApiController]
    public class ThermostatController : ControllerBase
    {
        private readonly ILogger _logger;
        private readonly string _connString = Startup.Configuration.GetConnectionString("DefaultDatabase");

        public ThermostatController(ILogger<ThermostatController> logger)
        {
            _logger = logger;
        }
        
        [HttpGet]
        public ThermostatRequest[] Get()
        {
            try
            {
                return DbService.GetLatestEntries(_connString);
            }
            catch (Exception e)
            {
                _logger.LogError("GetLatestEntries() THROWS AN ERROR", e);
                throw;
            }
        }

        [HttpPost]
        public void Post([FromBody] ThermostatRequestViewModel request)
        {
            var item = new ThermostatRequest
            {
                DeviceId = request.DeviceId,
                DeviceStatus = request.DeviceStatus,
                Measurement = request.Measurement,
                Unit = request.Unit,
                Timestamp = DateTime.Now
            };

            try
            {
                DbService.InsertLoginRequest(item, _connString);
            }
            catch (Exception e)
            {
                _logger.LogError("InsertLoginRequest(item) THROWS AN ERROR", e);
                throw;
            }

        }


        [HttpPost("online")]
        public void SetOnline([FromBody] OnlineRequestViewModel request)
        {
            if (!General.OnlineDevices.ContainsKey(request.DeviceId))
            {
                request.Timestamp = DateTime.Now;
                General.OnlineDevices.Add(request.DeviceId, request);
            }
        }
   }
}
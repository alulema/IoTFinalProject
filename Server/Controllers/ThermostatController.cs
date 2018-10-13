using System;
using System.IO;
using IoTFinalProject.Domain;
using IoTFinalProject.Domain.Data;
using IoTFinalProject.Domain.Model;
using IoTFinalProject.Domain.ViewModel;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.Logging;
using Newtonsoft.Json;

namespace IoTFinalProject.Controllers
{
    [Route("api/[controller]")]
    [ApiController]
    public class ThermostatController : ControllerBase
    {
        private readonly ILogger _logger;
        private readonly string _connString = Startup.Configuration.GetConnectionString("DefaultDatabase");
        private readonly string _defaultTempLevel = Startup.Configuration["DefaultTemperatureLevel"];

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

        [HttpGet("gethistory")]
        public object GetHistory(HistoricRate rate, string deviceId)
        {
            try
            {
                var data = DbService.GetHistoricData(rate, deviceId, _connString);
                return data;
            }
            catch (Exception e)
            {
                _logger.LogError("GetHistoricData() THROWS AN ERROR", e);
                throw;
            }
        }

        [HttpPost]
        public int Post([FromBody] ThermostatRequestViewModel request)
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
                DbService.InsertDataRequest(item, _connString);
                var activateIt = Rules.DecideActivation(item, float.Parse(_defaultTempLevel));
                return activateIt ? 1 : 0;
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

        [HttpGet("getdeviceconfig")]
        public DeviceConfiguration GetDeviceConfig(string deviceId)
        {
            return Rules.GetDeviceConfig(deviceId);
        }

        [HttpPost("savedeviceconfig")]
        public bool SaveDeviceConfig(DeviceConfiguration config)
        {
            if (!Directory.Exists("devices"))
                Directory.CreateDirectory("devices");

            string filePath = $"devices/config.{config.DeviceId}.json";

            if (System.IO.File.Exists(filePath))
                System.IO.File.Delete(filePath);

            var json = JsonConvert.SerializeObject(config);
            StreamWriter fileStream = System.IO.File.CreateText(filePath);
            fileStream.Write(json);
            fileStream.Close();

            return true;
        }
    }
}
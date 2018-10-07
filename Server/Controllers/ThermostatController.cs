using System;
using IoTFinalProject.Data;
using IoTFinalProject.Model;
using IoTFinalProject.ViewModel;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.Logging;

namespace IoTFinalProject.Controllers
{
    [Route("api/[controller]")]
    [ApiController]
    public class ThermostatController : ControllerBase
    {
        private readonly ILogger _logger;

        public ThermostatController(ILogger<ThermostatController> logger)
        {
            _logger = logger;
        }
        
        [HttpGet]
        public ThermostatRequest[] Get()
        {
            try
            {
                return DbService.GetLatestEntries();
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
                DbService.InsertLoginRequest(item);
            }
            catch (Exception e)
            {
                _logger.LogError("InsertLoginRequest(item) THROWS AN ERROR", e);
                throw;
            }

        }

        // PUT api/values/5
        [HttpPut("{id}")]
        public void Put(int id, [FromBody] string value)
        {
        }

        // DELETE api/values/5
        [HttpDelete("{id}")]
        public void Delete(int id)
        {
        }
    }
}
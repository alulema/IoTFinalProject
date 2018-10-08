using System.Threading.Tasks;
using Microsoft.AspNetCore.SignalR;

namespace IoTFinalProject.Domain.Hubs
{
    public class DeviceMonitoringHub : Hub
    {
        public async Task SendConnectedDevices(string user, string message)
        {
            await Clients.All.SendAsync("ReceiveMessage", user, message);
        }
    }
}

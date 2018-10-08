﻿using System;
using System.Collections.Concurrent;
using System.Threading.Tasks;
using Microsoft.AspNetCore.SignalR;

namespace IoTFinalProject.Domain.Hubs
{
    public class DeviceMonitoringReferer
    {
        public static Lazy<DeviceMonitoringReferer> LocalInstance;
        private readonly ConcurrentDictionary<string, string> _connections = new ConcurrentDictionary<string, string>();

        private IHubClients Clients { get; set; }
        public static DeviceMonitoringReferer Instance => LocalInstance.Value;

        public DeviceMonitoringReferer(IHubContext<DeviceMonitoringHub> context)
        {
            Clients = context.Clients;
            _connections.Clear();
        }

        public void RegisterConnection(string userId, string connectionId)
        {
            _connections.TryAdd(connectionId, userId);
        }

        public void RemoveConnection(string connectionId)
        {
            string outed;
            _connections.TryRemove(connectionId, out outed);
        }

        public async Task<bool> SendTransactionUpdate(string json)
        {
            try
            {
                //Clients.All.receiveTransactionUpdate(json);
                await Clients.All.SendAsync("ReceiveMessage", "Alexis", "ya estamos con la comunicacion");
                return true;
            }
            catch (IndexOutOfRangeException)
            {
                return false;
            }
            catch (Exception)
            {
                return false;
            }
        }
    }
}

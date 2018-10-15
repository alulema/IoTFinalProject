using System;
using System.Collections.Generic;
using System.Data;
using IoTFinalProject.Domain.Model;
using MySql.Data.MySqlClient;
using Newtonsoft.Json;

namespace IoTFinalProject.Domain.Data
{
    public class DbService
    {
        private static MySqlConnection OpenConnection(string connString)
        {
            var conn = new MySqlConnection(connString);
            conn.Open();

            return conn;
        }

        public static object GetHistoricData(HistoricRate rate, string deviceId, string connString)
        {
            var temperatures = new List<PlotDataPoint>();
            var thermocoupleStatuses = new List<PlotDataPoint>();
            MySqlConnection conn = null;
            int minsFrom = 5;

            switch (rate)
            {
                case HistoricRate.Back5Minutes:
                    minsFrom = 5;
                    break;
                case HistoricRate.Back30Minutes:
                    minsFrom = 30;
                    break;
                case HistoricRate.Back2Hours:
                    minsFrom = 120;
                    break;
                case HistoricRate.Back1Day:
                    minsFrom = 1440;
                    break;
                case HistoricRate.Back1Week:
                    minsFrom = 10080;
                    break;
            }

            conn = OpenConnection(connString);
            var query = $"CALL GetDataSamples({minsFrom}, '{deviceId}');";
            MySqlCommand cmd = new MySqlCommand(query, conn);
            var reader = cmd.ExecuteReader();

            while (reader.Read())
            {
                var item = new ThermostatRequest
                {
                    DeviceStatus = (string) reader[2],
                    Measurement = Convert.ToSingle(reader[3]),
                    Timestamp = (DateTime) reader[5]
                };
                
                temperatures.Add(new PlotDataPoint(item.Measurement, item.Timestamp.ToUniversalTime()));
                thermocoupleStatuses.Add(new PlotDataPoint(item.DeviceStatus == "ON" ? 1 : 0, item.Timestamp.ToUniversalTime()));
            }

            reader.Close();
            if (conn != null && conn.State == ConnectionState.Open)
                conn.Close();

            return new
            {
                Temperatures = temperatures,
                Statuses = thermocoupleStatuses
            };
        }

        public static void InsertDataRequest(ThermostatRequest item, string connString)
        {
            MySqlConnection conn = null;

            conn = OpenConnection(connString);
            var query =
                "INSERT INTO Temperature (device_id, device_status, measurement, unit, timestamp) VALUES " +
                "(@DeviceId, @DeviceStatus, @Measurement, @Unit, @Timestamp)";
            MySqlCommand cmd = new MySqlCommand(query, conn);

            cmd.Parameters.AddWithValue("@DeviceId", item.DeviceId);
            cmd.Parameters.AddWithValue("@DeviceStatus", item.DeviceStatus);
            cmd.Parameters.AddWithValue("@Measurement", item.Measurement);
            cmd.Parameters.AddWithValue("@Unit", item.Unit);
            cmd.Parameters.AddWithValue("@Timestamp", item.Timestamp);
            cmd.ExecuteNonQuery();

            if (conn != null && conn.State == ConnectionState.Open)
                conn.Close();
        }

        public static ThermostatRequest[] GetLatestEntries(string connString)
        {
            var items = new List<ThermostatRequest>();
            MySqlConnection conn = null;

            conn = OpenConnection(connString);
            var query = "SELECT * FROM Temperature ORDER BY id DESC LIMIT 10";
            MySqlCommand cmd = new MySqlCommand(query, conn);

            var reader = cmd.ExecuteReader();

            while (reader.Read())
            {
                var item = new ThermostatRequest
                {
                    Id = (int) reader[0],
                    DeviceId = (string) reader[1],
                    DeviceStatus = (string) reader[2],
                    Measurement = Convert.ToSingle(reader[3]),
                    Unit = (string) reader[4],
                    Timestamp = (DateTime) reader[5]
                };

                items.Add(item);
            }

            reader.Close();
            if (conn != null && conn.State == ConnectionState.Open)
                conn.Close();

            return items.ToArray();
        }

        public static DeviceConfiguration GetDeviceConfig(string connString, string deviceId)
        {
            MySqlConnection conn = null;

            conn = OpenConnection(connString);
            var query = $"SELECT configuration FROM device_config WHERE device_id=@DeviceId";
            MySqlCommand cmd = new MySqlCommand(query, conn);
            cmd.Parameters.AddWithValue("@DeviceId", deviceId);
            DeviceConfiguration config;

            var reader = cmd.ExecuteScalar();

            if (reader == null)
            {
                config = new DeviceConfiguration
                {
                    DeviceId = deviceId
                };

                query = "INSERT INTO device_config (device_id, configuration) VALUES (@DeviceId, @Configuration)";
                MySqlCommand cmd2 = new MySqlCommand(query, conn);

                cmd2.Parameters.AddWithValue("@DeviceId", deviceId);
                cmd2.Parameters.AddWithValue("@Configuration", JsonConvert.SerializeObject(config));
                cmd2.ExecuteNonQuery();
            }
            else
                config = JsonConvert.DeserializeObject<DeviceConfiguration>(reader.ToString());

            if (conn != null && conn.State == ConnectionState.Open)
                conn.Close();

            return config;
        }

        public static void SaveDeviceConfig(string connString, DeviceConfiguration config)
        {
            MySqlConnection conn = null;

            conn = OpenConnection(connString);
            var query = "UPDATE device_config SET configuration=@Configuration WHERE device_id=@DeviceId";
            MySqlCommand cmd = new MySqlCommand(query, conn);

            cmd.Parameters.AddWithValue("@DeviceId", config.DeviceId);
            cmd.Parameters.AddWithValue("@Configuration", JsonConvert.SerializeObject(config));
            cmd.ExecuteNonQuery();

            if (conn != null && conn.State == ConnectionState.Open)
                conn.Close();
        }
    }
}
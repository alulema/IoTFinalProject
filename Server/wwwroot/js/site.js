var connection = new signalR.HubConnectionBuilder().withUrl("/monitorHub").build();
var deviceArray;
var loadedConfig;

$(document)
    .ajaxStart(function () {
        $('#loadingDiv').show();
    })
    .ajaxStop(function () {
        $('#loadingDiv').hide();
    });

$(document).ready(function() {
    connection = new signalR.HubConnectionBuilder().withUrl("/monitorHub").build();

    connection.on("UpdateDeviceList", onDeviceListUpdated);
    connection.start().catch(onHubConnectionError);

    $("#dlgEditPoint").ejDialog({
        title: "Dialog",
        showOnInit: false,
        backgroundScroll: false,
        enableModal: true,
        enableResize: false,
        width: 420,
        height: 240
    });

    $("#txtTemperature").ejNumericTextbox(
        {
            name: "numeric",
            value: 65,
            minValue: 40,
            maxValue: 125,
            width: 65
        });
    $('#startTime').ejTimePicker({width: 110});
    $('#endTime').ejTimePicker({width: 110});
});

var onDeviceListUpdated = function (json) {
    if (!_.isEqual(deviceArray, json)) {
        deviceArray = json;
        var source = $("#Device-Template").html();
        var template = Handlebars.compile(source);
        $("#divDevicesList").html("");
        var html = template({Items: json});
        $("#divDevicesList").append(html);
    }
};

var onHubConnectionError = function (err) {
    return console.error(err.toString());
};

var onDeviceClick = function(device_id) {
    $.ajax({
        type: 'GET',
        url: '/api/thermostat/getdeviceconfig?deviceId=' + device_id,
        contentType: "application/json; charset=utf-8",
        dataType: 'json',
        success: onGetDeviceConfigSuccess,
        error: function () {
            console.log("Error calling Thermostat API: GetDeviceConfig");
        },
        complete: function() {
            $("#divDevice-" + device_id).addClass( "selectedwell" );
        }

    });
};

var onGetDeviceConfigSuccess = function(json) {
    loadedConfig = json;
    $("#divDeviceDetails").show();
    buildDailyPoints(loadedConfig.daily_points);
};

var addPoint = function(deviceId) {
    $("#dlgEditPoint").ejDialog("setTitle","Add Daily Point");
    $("#dlgEditPoint").ejDialog("open");
};

var editPoint = function(pointId, deviceId) {
    // $.each(deviceArray, function (i, v) {
    //     if (v.device_id === deviceId) {
    //         console.log(v);
    //         return false;
    //     }
    // });
    $("#dlgEditPoint").ejDialog("open");
};

var removePoint = function(pointId, deviceId) {
    $.each(loadedConfig.daily_points, function (i, v) {
        if (v.id === pointId) {
            loadedConfig.daily_points.splice(i, 1);
            return false;
        }
    });
    $("#divPoint-" + pointId).remove();
};

var onDailyPointAdded = function() {
    var temperature = $("#txtTemperature").ejNumericTextbox("getValue");
    var startTime = $("#startTime").ejTimePicker("getValue");
    var endTime = $("#endTime").ejTimePicker("getValue");
    
    var startParts = startTime.split(' ');
    var startTimeParts =startParts[0].split(':');
    var startHour = parseInt(startTimeParts[0]);
    startHour += startParts[1] === 'PM' ? (startHour === 12 ? 0 : 12) : (startHour === 12 ? -12 : 0);
    var startMinute = parseInt(startTimeParts[1]);

    var endParts = endTime.split(' ');
    var endTimeParts = endParts[0].split(':');
    var endHour = parseInt(endTimeParts[0]);
    endHour += endParts[1] === 'PM' ? (endHour === 12 ? 0 : 12) : 0;
    var endMinute = parseInt(endTimeParts[1]);
    
    if (((endHour * 100) + endMinute) <= ((startHour * 100) + startMinute)) {
        alert("End time can't be greater or equal than start time");
        return;
    }
    
    var point = {
        id: generateId(),
        temperature: temperature,
        start_hour: ("0" + startHour).slice(-2),
        start_minute: ("0" + startMinute).slice(-2),
        end_hour: ("0" + endHour).slice(-2),
        end_minute: ("0" + endMinute).slice(-2),
        is_active: $('#checkActive').prop('checked'),
        selected_date: null
    };
    
    loadedConfig.daily_points.push(point);
    buildDailyPoints(loadedConfig.daily_points);
    $("#dlgEditPoint").ejDialog("close");
};

var generateId = function () {
    var text = "";
    var possible = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

    for (var i = 0; i < 5; i++)
        text += possible.charAt(Math.floor(Math.random() * possible.length));

    return text;
};

var buildDailyPoints = function(json) {
    var source = $("#DailyPoint-Template").html();
    var template = Handlebars.compile(source);
    $("#divDailyPointList").html("");
    var html = template({Items: json});
    $("#divDailyPointList").append(html);
};

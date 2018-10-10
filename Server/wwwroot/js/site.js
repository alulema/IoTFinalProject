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
    $("#dlgChooseDate").ejDialog({
        title: "Special dates",
        showOnInit: false,
        backgroundScroll: false,
        enableModal: true,
        enableResize: false,
        width: 240,
        height: 140
    });

    $("#txtTemperature").ejNumericTextbox({
            name: "numeric",
            value: 65,
            minValue: 40,
            maxValue: 125,
            width: 65
        });
    $('#startTime').ejTimePicker({width: 110});
    $('#endTime').ejTimePicker({width: 110});
    $("#daterangepick").ejDateRangePicker({width:200});
});

var onDeviceListUpdated = function (json) {
    if (!_.isEqual(deviceArray, json)) {
        $("#divDeviceDetails").hide();

        if (json.length === 0)
            $("#divScanning").show();
        else
            $("#divScanning").hide();

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
    var satPoints = new Array();
    var sunPoints = new Array();
    
    $.each(loadedConfig.weekend_points, function(i, v) {
        if (v.is_saturday)
            satPoints.push(v);
        if (v.is_sunday)
            sunPoints.push(v);
    });
    buildDailyPoints("divDailyPointList", loadedConfig.daily_points);
    buildDailyPoints("divSaturdayPointList", satPoints);
    buildDailyPoints("divSundayPointList", sunPoints);
};

var addPoint = function(title, weekendOption) {
    $('#checkActive').prop('checked', true);
    $("#txtTemperature").ejNumericTextbox({
        name: "numeric",
        value: 65,
        minValue: 40,
        maxValue: 125,
        width: 65
    });
    $('#startTime').ejTimePicker({
        value : "12:00 AM",
        width: 110
    });
    $('#endTime').ejTimePicker({
        value : "12:00 AM",
        width: 110
    });
    $("#hdnDeviceId").val("");
    $("#hdnWeekendOption").val(weekendOption);

    $("#dlgEditPoint").ejDialog("setTitle",title);
    $("#dlgEditPoint").ejDialog("open");
};

var addDailyPoint = function() {
    addPoint("Add Daily Point", "");
};

var addWeekendPoint = function(option) {
    addPoint("Add Point on " + (option === 0 ? "saturday" : "sunday"), option);
};

var editPoint = function(pointId, deviceId) {
    // $.each(deviceArray, function (i, v) {
    //     if (v.device_id === deviceId) {
    //         console.log(v);
    //         return false;
    //     }
    // });
    $.each(loadedConfig.daily_points, function (i, v) {
        if (v.id === pointId) {
            $('#checkActive').prop('checked', v.is_active);
            $("#txtTemperature").ejNumericTextbox({
                    name: "numeric",
                    value: v.temperature,
                    minValue: 40,
                    maxValue: 125,
                    width: 65
                });
            $('#startTime').ejTimePicker({
                value : v.start_time,
                width: 110
            });
            $('#endTime').ejTimePicker({
                value : v.end_time,
                width: 110
            });
            $("#hdnDeviceId").val(pointId);
            return false;
        }
    });
    
    $("#dlgEditPoint").ejDialog("setTitle","Edit Daily Point");
    $("#dlgEditPoint").ejDialog("open");
};

var addDatesPoint = function() {
    $("#dlgChooseDate").ejDialog("open");
}

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
    var isNew = $("#hdnDeviceId").val() === "";
    var weekendOption = $("#hdnWeekendOption").val();
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
        id: isNew ? generateId() : $("#hdnDeviceId").val(),
        temperature: temperature,
        start_hour: ("0" + startHour).slice(-2),
        start_minute: ("0" + startMinute).slice(-2),
        start_time: startTime,
        end_hour: ("0" + endHour).slice(-2),
        end_minute: ("0" + endMinute).slice(-2),
        end_time: endTime,
        is_active: $('#checkActive').prop('checked'),
        is_saturday: weekendOption === "" ? false : (weekendOption === "0"),
        is_sunday: weekendOption === "" ? false : (weekendOption === "1"),
        selected_date: null
    };
    
    if (isNew) {
        if (weekendOption === "")
            loadedConfig.daily_points.push(point);
        else
            loadedConfig.weekend_points.push(point);
    }
    else
        $.each(loadedConfig.daily_points, function (i, v) {
            if (v.id === point.id) {
                if (weekendOption === "")
                    loadedConfig.daily_points[i] = point;
                else
                    loadedConfig.weekend_points[i] = point;
                return false;
            }
        });

    if (weekendOption === "")
        buildDailyPoints("divDailyPointList", loadedConfig.daily_points);
    else {
        var wPoints = new Array();
        $.each(loadedConfig.weekend_points, function(i, v) {
            if ((weekendOption === "0" && v.is_saturday) || (weekendOption === "1" && v.is_sunday))
                wPoints.push(v);
        });
        buildDailyPoints((weekendOption === "0" ? "divSaturdayPointList" : "divSundayPointList"), wPoints);
    }

    $("#dlgEditPoint").ejDialog("close");
};

var generateId = function () {
    var text = "";
    var possible = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

    for (var i = 0; i < 5; i++)
        text += possible.charAt(Math.floor(Math.random() * possible.length));

    return text;
};

var buildDailyPoints = function(divElem, json) {
    var source = $("#DailyPoint-Template").html();
    var template = Handlebars.compile(source);
    $("#" + divElem).html("");
    var html = template({Items: json});
    $("#" + divElem).append(html);
};

var onSelectedRange = function () {
    var range = $("#daterangepick").ejDateRangePicker("getSelectedRange");
    $("#dlgChooseDate").ejDialog("close");
    addPoint("Add Point Details", "");

};
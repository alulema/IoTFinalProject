Date.prototype.addMinutes = function (m) {
    this.setTime(this.getTime() + (m * 60 * 1000));
    return this;
};
Date.prototype.addSeconds = function (s) {
    this.setTime(this.getTime() + (s * 1000));
    return this;
};

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

$(document).ready(function () {
    connection = new signalR.HubConnectionBuilder().withUrl("/monitorHub").build();

    connection.on("UpdateDeviceList", onDeviceListUpdated);
    connection.start().catch(onHubConnectionError);

    $("#divDeviceDetails").ejTab();
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
        value: 18,
        minValue: 5,
        maxValue: 50,
        width: 65
    });
    $('#startTime').ejTimePicker({
        width: 110,
        interval : 15
    });
    $('#endTime').ejTimePicker({
        width: 110,
        interval : 15
    });
    $("#daterangepick").ejDateRangePicker({width: 200});

    $("#check11").ejToggleButton({
        size: "normal",
        showRoundedCorner: true,
        contentType: "imageonly",
        defaultPrefixIcon: "e-icon e-mediaplay e-uiLight",
        activePrefixIcon: "e-icon e-mediapause e-uiLight",
        click: "onAutoRefreshClick"
    });
});

var autoIsOn = false;
var x;
var onAutoRefreshClick = function() {
    if (autoIsOn)
        clearInterval(x);
    else {
        var countDownDate = (new Date()).addSeconds(5).getTime();
        x = setInterval(function () {

            // Get todays date and time
            var now = new Date().getTime();

            // Find the distance between now and the count down date
            var distance = countDownDate - now;

            // If the count down is over, write some text 
            if (distance < 0) {
                getStatsData(pickRate, $('#spanDevId').text(), false);
                countDownDate = (new Date()).addSeconds(5).getTime();
            }
        }, 1000);
    }

    autoIsOn = !autoIsOn;
};

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

var onDeviceClick = function (device_id) {
    $.ajax({
        type: 'GET',
        url: '/api/thermostat/getdeviceconfig?deviceId=' + device_id,
        contentType: "application/json; charset=utf-8",
        dataType: 'json',
        success: onGetDeviceConfigSuccess,
        error: function () {
            console.log("Error calling Thermostat API: GetDeviceConfig");
        },
        complete: function () {
            $("#divDevice-" + device_id).addClass("selectedwell");
        }
    });
};

var onGetDeviceConfigSuccess = function (json) {
    loadedConfig = json;
    $("#divDeviceDetails").show();
    var satPoints = new Array();
    var sunPoints = new Array();

    $.each(loadedConfig.weekend_points, function (i, v) {
        if (v.is_saturday)
            satPoints.push(v);
        if (v.is_sunday)
            sunPoints.push(v);
    });
    buildDailyPoints("divDailyPointList", loadedConfig.daily_points);
    buildDailyPoints("divSaturdayPointList", satPoints);
    buildDailyPoints("divSundayPointList", sunPoints);
    buildDailyPoints("divSpecialDaysPointList", loadedConfig.special_dates);
};

var addPoint = function (title, weekendOption) {
    $('#checkActive').prop('checked', true);
    $("#txtTemperature").ejNumericTextbox({
        name: "numeric",
        value: 18,
        minValue: 5,
        maxValue: 50,
        width: 65
    });
    $('#startTime').ejTimePicker({
        value: "12:00 AM",
        width: 110
    });
    $('#endTime').ejTimePicker({
        value: "12:00 AM",
        width: 110
    });
    $("#hdnPointId").val("");
    $("#hdnWeekendOption").val(weekendOption);

    $("#dlgEditPoint").ejDialog("setTitle", title);
    $("#dlgEditPoint").ejDialog("open");
};

var addDailyPoint = function () {
    addPoint("Add Daily Point", "");
};

var addWeekendPoint = function (option) {
    addPoint("Add Point on " + (option === 0 ? "saturday" : "sunday"), option);
};

var tmpRangeHolder;

var editPoint = function (pointId, deviceId) {
    var tmpIndex;
    var tmpArray = [loadedConfig.daily_points, loadedConfig.weekend_points, loadedConfig.special_dates];
    $.each(tmpArray, function (i, v) {
        $.each(v, function (_i, _v) {
            if (_v.id === pointId) {
                tmpIndex = i;
                if (i === 1) // is weekend point
                    $("#hdnWeekendOption").val(_v.is_saturday ? "0" : "1");
                if (i === 2)
                    tmpRangeHolder = _v.selected_dates;

                $("#hdnPointId").val(_v.id);
                $('#checkActive').prop('checked', _v.is_active);
                $("#txtTemperature").ejNumericTextbox({
                    name: "numeric",
                    value: _v.temperature,
                    minValue: 5,
                    maxValue: 50,
                    width: 65
                });
                $('#startTime').ejTimePicker({
                    value: _v.start_time,
                    width: 110
                });
                $('#endTime').ejTimePicker({
                    value: _v.end_time,
                    width: 110
                });
                return false;
            }
        });
    });

    var dlgTitle;
    switch (tmpIndex) {
        case 0: // daily point
            dlgTitle = "Edit Daily Point";
            $("#hdnWeekendOption").val("");
            break;
        case 1: // weekend point
            dlgTitle = "Edit Weekend Point";
            break;
        case 2: // special days point
            dlgTitle = "Edit Special Days Point: " + tmpRangeHolder;
            $("#hdnWeekendOption").val("2");
            break;
    }

    $("#dlgEditPoint").ejDialog("setTitle", dlgTitle);
    $("#dlgEditPoint").ejDialog("open");
};

var addDatesPoint = function () {
    $("#dlgChooseDate").ejDialog("open");
}

var removePoint = function (pointId, deviceId) {
    $.each(loadedConfig.daily_points, function (i, v) {
        if (v.id === pointId) {
            loadedConfig.daily_points.splice(i, 1);
            return false;
        }
    });
    $("#divPoint-" + pointId).remove();
};

var onDailyPointAdded = function () {
    var isNew = $("#hdnPointId").val() === "";
    var weekendOption = $("#hdnWeekendOption").val();
    var temperature = $("#txtTemperature").ejNumericTextbox("getValue");
    var startTime = $("#startTime").ejTimePicker("getValue");
    var endTime = $("#endTime").ejTimePicker("getValue");

    var startParts = startTime.split(' ');
    var startTimeParts = startParts[0].split(':');
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

    var strRange = null;
    var range = weekendOption === "2" ? $("#daterangepick").ejDateRangePicker("getSelectedRange") : null;

    if (weekendOption === "2") {
        if (range.startDate === null && range.endDate === null)
            strRange = tmpRangeHolder;
        else
            strRange = formatDate(range.startDate) + " - " + formatDate(range.endDate);
    }

    var point = {
        id: isNew ? generateId() : $("#hdnPointId").val(),
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
        selected_dates: strRange
    };

    if (isNew) {
        if (weekendOption === "")
            loadedConfig.daily_points.push(point);
        else if (weekendOption === "0" || weekendOption === "1")
            loadedConfig.weekend_points.push(point);
        else
            loadedConfig.special_dates.push(point);
    }
    else {
        $.each(loadedConfig.daily_points, function (i, v) {
            if (v.id === point.id) {
                loadedConfig.daily_points[i] = point;
                return false;
            }
        });
        $.each(loadedConfig.weekend_points, function (i, v) {
            if (v.id === point.id) {
                loadedConfig.weekend_points[i] = point;
                return false;
            }
        });
        $.each(loadedConfig.special_dates, function (i, v) {
            if (v.id === point.id) {
                loadedConfig.special_dates[i] = point;
                return false;
            }
        });
    }

    var satPoints = new Array();
    var sunPoints = new Array();

    $.each(loadedConfig.weekend_points, function (i, v) {
        if (v.is_saturday)
            satPoints.push(v);
        if (v.is_sunday)
            sunPoints.push(v);
    });

    buildDailyPoints("divDailyPointList", loadedConfig.daily_points);
    buildDailyPoints("divSaturdayPointList", satPoints);
    buildDailyPoints("divSundayPointList", sunPoints);
    buildDailyPoints("divSpecialDaysPointList", loadedConfig.special_dates);

    $("#dlgEditPoint").ejDialog("close");
};

var generateId = function () {
    var text = "";
    var possible = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

    for (var i = 0; i < 5; i++)
        text += possible.charAt(Math.floor(Math.random() * possible.length));

    return text;
};

var buildDailyPoints = function (divElem, json) {
    var source = $("#DailyPoint-Template").html();
    var template = Handlebars.compile(source);
    $("#" + divElem).html("");
    var html = template({Items: json});
    $("#" + divElem).append(html);
};

var onSelectedRange = function () {
    var range = $("#daterangepick").ejDateRangePicker("getSelectedRange");
    $("#dlgChooseDate").ejDialog("close");
    addPoint("Add Point Details: " + formatDate(range.startDate) + " - " + formatDate(range.endDate), 2);
};

var formatDate = function (date) {
    return (date.getMonth() + 1) + '/' + date.getDate() + '/' + date.getFullYear();
};

var saveConfig = function (deviceId) {
    var timeoffset = new Date().getTimezoneOffset();
    loadedConfig.timeoffset = timeoffset;

    $.ajax({
        type: 'POST',
        url: '/api/thermostat/savedeviceconfig',
        contentType: "application/json; charset=utf-8",
        data: JSON.stringify(loadedConfig),
        success: function() {
            toastr.options.positionClass = "toast-bottom-right";
            toastr.success("Configuration has been saved!");
        },
        error: function () {
            console.log("Error calling Thermostat API: SaveDeviceConfig");
        },
        complete: function () {
            $("#divDevice-" + deviceId).addClass("selectedwell");
        }
    });
};

var pickRate = 0;
var getStatsData = function (rate, deviceId, isGlobal) {
    pickRate = rate;
    $.ajax({
        type: 'GET',
        url: '/api/thermostat/gethistory?rate=' + rate + '&deviceId=' + deviceId,
        contentType: "application/json; charset=utf-8",
        global: isGlobal,
        dataType: 'json',
        success: function (json) {
            $.each(json.statuses, function (i, v) {
                json.statuses[i].x = new Date(v.x);
            });
            $.each(json.temperatures, function (i, v) {
                json.temperatures[i].x = new Date(v.x);
            });

            var minSpan = 0;
            var minInterval = 0;
            var minType = '';
            var extraTitle = '';

            switch (rate) {
                case 0:
                    minSpan = -5;
                    minInterval = 1;
                    minType = 'Minutes';
                    extraTitle = " (5 min)";
                    break;
                case 1:
                    minSpan = -30;
                    minInterval = 3;
                    minType = 'Minutes';
                    extraTitle = " (30 min)";
                    break;
                case 2:
                    minSpan = -120;
                    minInterval = 12;
                    minType = 'Minutes';
                    extraTitle = " (2 hrs)";
                    break;
                case 3:
                    minSpan = -1440;
                    minInterval = 144;
                    minType = 'Minutes';
                    extraTitle = " (1 day)";
                    break;
                case 4:
                    minSpan = -10080;
                    minInterval = 1;
                    minType = 'Days';
                    extraTitle = " (1 week)";
                    break;
            }

            var split = new Date().toString().split(" ");

            $("#plot1").ejChart({
                primaryXAxis: {
                    title: {text: "Time (" + split[5] + ")"},
                    range: {min: new Date().addMinutes(minSpan), max: new Date(), interval: minInterval},
                    intervalType: minType,
                    valueType: 'datetime'
                },
                primaryYAxis: {
                    title: {text: "Temperature (ºC)"},
                    rangePadding: 'additional'
                },
                commonSeriesOptions: {
                    tooltip: {
                        visible: false
                    }
                },
                series: [
                    {
                        points: [],
                        name: 'Sales',
                        type: 'line',
                        enableAnimation: true,
                        border: {width: 3},
                        tooltip: {format: " Date : #point.x#  <br/> Profit : #point.y# "}
                    }
                ],
                isResponsive: true,
                load: "loadTheme",
                title: {text: 'Thermostat Temperature' + extraTitle},
                palette: ["blue"],
                size: {height: "300"},
                legend: {visible: false}
            });
            $("#plot2").ejChart({
                primaryXAxis: {
                    title: {text: "Time (" + split[5] + ")"},
                    range: {min: new Date().addMinutes(minSpan), max: new Date(), interval: minInterval},
                    intervalType: minType,
                    valueType: 'datetime'
                },
                primaryYAxis: {
                    title: {text: "ON / OFF"},
                    rangePadding: 'additional'
                },
                commonSeriesOptions: {
                    tooltip: {
                        visible: false
                    }
                },
                series: [
                    {
                        points: [],
                        name: 'Sales',
                        type: 'line',
                        enableAnimation: true,
                        border: {width: 3, color : "green"},
                        tooltip: {format: " Date : #point.x#  <br/> Profit : #point.y# "}
                    }
                ],
                isResponsive: true,
                load: "loadTheme",
                title: {text: 'Thermocouple Status'},
                size: {height: "300"},
                legend: {visible: false}
            });

            var chartObj1 = $("#plot1").ejChart("instance");
            chartObj1.model.series[0].points = json.temperatures;
            $("#plot1").ejChart("redraw");

            var chartObj2 = $("#plot2").ejChart("instance");
            chartObj2.model.series[0].points = json.statuses;
            $("#plot2").ejChart("redraw");
        },
        error: function () {
            console.log("Error calling Thermostat API: GetDeviceConfig");
        },
        complete: function () {
            //$("#divDevice-" + device_id).addClass("selectedwell");
        }
    });
};
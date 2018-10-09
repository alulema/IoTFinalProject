var connection = new signalR.HubConnectionBuilder().withUrl("/monitorHub").build();

$(document).ready(function() {
    connection = new signalR.HubConnectionBuilder().withUrl("/monitorHub").build();

    connection.on("UpdateDeviceList", onDeviceListUpdated);
    connection.start().catch(onHubConnectionError);
});

var onDeviceListUpdated = function (json) {
    console.log(json);
};

var onHubConnectionError = function (err) {
    return console.error(err.toString());
};

//document.getElementById("sendButton").addEventListener("click", function (event) {
//    var user = document.getElementById("userInput").value;
//    var message = document.getElementById("messageInput").value;
//    connection.invoke("SendMessage", user, message).catch(function (err) {
//        return console.error(err.toString());
//    });
//    event.preventDefault();
//});
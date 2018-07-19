function log(msg) {
    console.log(msg);
    $("#output").append(msg+"\n");
}


// Create WebSocket connection.
// const socket = new WebSocket('ws://localhost:3000/socket');
const socket = new WebSocket(config.socketPath);


// Connection opened
socket.addEventListener('open', function (event) {
    log("Socket Open");
    socket.send('Hello Server!');
});

// Listen for messages
socket.addEventListener('message', function (event) {
    log("From Server: " + event.data);
});

socket.addEventListener('error', function(event) {
    log("Socket error: ", event.data);
});

socket.addEventListener('close', function(event) {
    log("Socket closed");
});



function getAnims() {
    log("Can haz animations?");
    socket.send("GA");
}

function getPalettes() {
    socket.send("GP");
}

function getState() {
    socket.send("GX");
}

function setUnit() {
    var t = $("#unitToSet").val().trim();
    if (!t) {
        alert("Nothing to set");
        return;
    }

    socket.send("SU"+t);
}

function setPalette() {
    var t = $("#paletteToSet").val().trim();
    if (!t) {
        alert("Nothing to set");
        return;
    }

    socket.send("SP"+t);
}

function setAnim() {
    var t = $("#animToSet").val().trim();
    if (!t) {
        alert("Nothing to set");
        return;
    }

    socket.send("SA"+t);
}

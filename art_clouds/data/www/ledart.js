function log(msg) {
    console.log(msg);
    $("#log").append(msg+"\n");
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

/////////

function setUnitType(num) {
    socket.send("SU"+num);    
}

function setPaletteNum(num) {
    socket.send("SP"+num);    
}

function setBrightness() {
    var t = $("#brightnessToSet").val().trim();
    if (!t) {
        alert("Nothing to set");
        return;
    }

    socket.send("SB"+t);
}


function setDuration() {
    var t = $("#durationToSet").val().trim();
    if (!t) {
        alert("Nothing to set");
        return;
    }

    socket.send("SD"+t);
}

function setSpeedFactor() {
    var t = $("#speedFactorToSet").val().trim();
    if (!t) {
        alert("Nothing to set");
        return;
    }

    var cents = parseInt(t * 100.0);

    socket.send("SF"+cents);
}


function setReverse(shouldReverse) {
    socket.send("SR"+(shouldReverse?"+":"-"));
}

function showColorChooser(shouldShow) {
    if (shouldShow) {
        socket.send("C+");
    } else {
        socket.send("C-");
    }
}

function setColorChooserColor(val) {
    // log("Setting color "+val);
    socket.send("C"+val);
}

function setNexusColor(isForeground) {
    var val = $("#colorChooser").spectrum("get");
    var color = tinycolor(val);
    var hex = color.toHexString();

    socket.send("C"+(isForeground?"F":"B") + val);
}

function updateColor() {
    var val = $("#colorChooser").spectrum("get");

    var color = tinycolor(val);
    var rgb = color.toRgb();
    $("#colorString").text("RgbColor("+rgb.r+", "+rgb.g+", "+rgb.b+")");

    var hex = color.toHexString();
    setColorChooserColor(hex);
}

function colorMove(event) {
    // log("move");

    updateColor()
}

function colorChange(event) {
    var val = $("#colorChooser").spectrum("get");
    log("change: "+val);
}
/////

$("#colorChooser").spectrum({
    flat: true
    , showInput: true
    , showButtons: false
    , preferredFormat: "hex" 
    , move: colorMove
    , change: colorChange
});

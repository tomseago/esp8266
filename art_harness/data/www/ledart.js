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
    send('Hello Server!');
    getAnims();
});

// Listen for messages
socket.addEventListener('message', function (event) {
    var msg = event.data;

    log("<---RECV: '" + msg + "'");

    var parts = msg.split(":");
    if (parts && parts.length > 0) {
        handleMessageParts(parts);
    }
});

socket.addEventListener('error', function(event) {
    log("Socket error: ", event.data);
});

socket.addEventListener('close', function(event) {
    log("Socket closed");
});

function send(msg) {
    if (socket.readyState != WebSocket.OPEN) {
        alert("Socket not open");
        return;
    }

    // Seems ok I guess??
    log("--->SEND: '"+msg+"'");
    socket.send(msg);
}


function handleMessageParts(parts) {
    if (parts[0]=="ANIMS") {
        handleAnimsList(parts[1].split(";"));
        return;
    }
}

function handleAnimsList(list) {
    log("Anim list = "+list);
    if (!list || list.length == 0) {
        return;
    }

    var el = $("#animButtons");
    el.empty();

    list.forEach(function(name) {
        log("Adding animation: '"+name+"'");
        el.append("<button onClick='requestAnim(\""+name+"\");'>"+name+"</button>");
    });
}

function requestNextAnim(randomize) {
    send("SA"+ (randomize ? "+" : "-"));
}

function requestAnim(name) {
    send("SA-"+name);
}

function getAnims() {
    send("GA");
}

function getPalettes() {
    send("GP");
}

function getState() {
    send("GX");
}

function setUnit() {
    var t = $("#unitToSet").val().trim();
    if (!t) {
        alert("Nothing to set");
        return;
    }

    send("SU"+t);
}

function setPalette() {
    var t = $("#paletteToSet").val().trim();
    if (!t) {
        alert("Nothing to set");
        return;
    }

    send("SP"+t);
}

function setAnim() {
    var t = $("#animToSet").val().trim();
    if (!t) {
        alert("Nothing to set");
        return;
    }

    send("SA"+t);
}

/////////

function setUnitType(num) {
    send("SU"+num);    
}

function setPaletteNum(num) {
    send("SP"+num);    
}

function setBrightness(val) {
    if (typeof val == "undefined")
    {
        var t = $("#brightnessToSet").val().trim();
        if (!t) {
            alert("Nothing to set");
            return;
        }
    }
    else
    {
        var t = ""+val;
        $("#brightnessToSet").val(t);
    }

    send("SB"+t);
}


function setDuration() {
    var t = $("#durationToSet").val().trim();
    if (!t) {
        alert("Nothing to set");
        return;
    }

    send("SD"+t);
}

function setSpeedFactor() {
    var t = $("#speedFactorToSet").val().trim();
    if (!t) {
        alert("Nothing to set");
        return;
    }

    var cents = parseInt(t * 100.0);

    send("SS"+cents);
}


function setReverse(shouldReverse) {
    send("SR"+(shouldReverse?"+":"-"));
}

function showColorChooser(shouldShow) {
    if (shouldShow) {
        send("C+");
    } else {
        send("C-");
    }
}

function setColorChooserColor(val) {
    // log("Setting color "+val);
    send("C"+val);
}

function setNexusColor(isForeground) {
    var val = $("#colorChooser").spectrum("get");
    var color = tinycolor(val);
    var hex = color.toHexString();

    send("C"+(isForeground?"F":"B") + val);
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

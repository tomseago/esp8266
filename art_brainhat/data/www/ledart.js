function log(msg) {
    console.log(msg);
    $("#log").append(msg+"\n");
}

function clearLog() {
    $("#log").empty();    
}


// Create WebSocket connection.
// const socket = new WebSocket('ws://localhost:3000/socket');

var showNetwork = true;
var socket;

function openSocket() {
    if (socket) {
        // close the lod one first
        log("closing old socket");
        socket.close();
        socket = false;
    }

    socket = new WebSocket(config.socketPath);

    // Connection opened
    socket.addEventListener('open', function (event) {
        log("Socket Open");
        send('Hello Server!');
        getAnims();
        getGeoms();

        setSocketOpen();
    });

    // Listen for messages
    socket.addEventListener('message', function (event) {
        var msg = event.data;

        if (showNetwork) log("<---RECV: '" + msg + "'");

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
        socket = false;
        setSocketClosed();
    });
}

function setSocketOpen() {
    $("#socketClosed").hide();
    $("#socketOpen").show();    
}

function setSocketClosed() {
    $("#socketClosed").show();
    $("#socketOpen").hide();    
}


function send(msg) {
    if (socket.readyState != WebSocket.OPEN) {
        alert("Socket not open");
        return;
    }

    // Seems ok I guess??
    if (showNetwork) log("--->SEND: '"+msg+"'");
    socket.send(msg);
}


function handleMessageParts(parts) {
    if (parts[0]=="GEOMS") {
        handleGeomsList(parts[1].split(";"));
        return;
    }

    if (parts[0]=="ANIMS") {
        handleAnimsList(parts[1].split(";"));
        return;
    }

    if (parts[0]=="L") {
        var out = parts.slice(1,parts.length);
        out.unshift("DEV) ");
        log(out.join(":").trim());
    }
}

function handleGeomsList(list) {
    log("Geoms list = "+list);

    if (!list || list.length == 0) {
        return;
    }

    // Update the buttons
    var el = $("#geomButtons");
    el.empty();

    list.forEach(function(name) {
        if (!name) return;
        var canRotate = name[0] == '+';

        name = name.slice(1, name.length);
        if (!name) return;

        log("Adding geom: '"+name+"'");
        el.append("<button onClick='requestGeom(\""+name+"\", false);'>"+name+"</button>");

        if (canRotate) {
            // A second rotated version
            el.append("<button onClick='requestGeom(\""+name+"\", true);'>"+name+" Rotated</button>");
        }            
    });
}

function handleAnimsList(list) {
    log("Anim list = "+list);
    if (!list || list.length == 0) {
        return;
    }

    var el = $("#animButtons");
    el.empty();

    list.forEach(function(name) {
        if (!name) return;
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

function requestGeom(name, rotated) {
    send("SG"+(rotated?"+":"-")+name);
}

function getAnims() {
    send("GA");
}

function getGeoms() {
    send("GG");
}

function getPalettes() {
    send("GP");
}

function getState() {
    send("GX");
}

// function setUnit() {
//     var t = $("#unitToSet").val().trim();
//     if (!t) {
//         alert("Nothing to set");
//         return;
//     }

//     send("SU"+t);
// }

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

// function setUnitType(num) {
//     send("SU"+num);    
// }

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

function setWantDeviceLogs(wantLogs) {
    send("SL"+(wantLogs?"+":"-"));    
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
function cbDeviceLogChange(event) {
    var val = $("#cbDeviceLog").prop("checked");
    log("want device logs "+val);

    setWantDeviceLogs(val);
}

function cbShowNetworkChange(event) {
    var val = $("#cbShowNetwork").prop("checked");

    showNetwork = val;
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


/////////////////
openSocket();
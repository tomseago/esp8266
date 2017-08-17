function log(msg) {
    console.log(msg);
    $("#log").append(msg+"\n");
}


//////////////
// UI

function c_welcome() {
    sendToggle("a");
}

function c_hom() {
    sendToggle("b");
}

function c_o() {
    sendToggle("c");
}

function c_e() {
    sendToggle("d");    
}

function c_pink_dancer() {
    sendToggle("e");    
}

function c_pink_fucker() {
    sendToggle("f");
}

function c_blue_dancer() {
    sendToggle("g");    
}

function c_blue_fucker() {
    sendToggle("h");
}

function c_pink_action_1() {
    sendToggle("i");
}

function c_pink_action_2() {
    sendToggle("j");
}

function c_blue_action_1() {
    sendToggle("k");
}

function c_blue_action_2() {
    sendToggle("l");
}

function c_sex_action_1() {
    sendToggle("m");
}

function c_sex_action_2() {
    sendToggle("n");
}

function c_sex_action_3() {
    sendToggle("o");
}

function c_sex_action_4() {
    sendToggle("p");
}


$("#btn_welcome").on("touch", c_welcome)
$("#btn_welcome").on("click", c_welcome)

$("#btn_hom").on("touch", c_hom)
$("#btn_hom").on("click", c_hom)

$("#btn_o").on("touch", c_o)
$("#btn_o").on("click", c_o)

$("#btn_e").on("touch", c_e)
$("#btn_e").on("click", c_e)

$("#btn_pink_dancer").on("touch", c_pink_dancer)
$("#btn_pink_dancer").on("click", c_pink_dancer)

$("#btn_pink_fucker").on("touch", c_pink_fucker)
$("#btn_pink_fucker").on("click", c_pink_fucker)

$("#btn_blue_dancer").on("touch", c_blue_dancer)
$("#btn_blue_dancer").on("click", c_blue_dancer)

$("#btn_blue_fucker").on("touch", c_blue_fucker)
$("#btn_blue_fucker").on("click", c_blue_fucker)

$("#btn_pink_action_1").on("touch", c_pink_action_1)
$("#btn_pink_action_1").on("click", c_pink_action_1)

$("#btn_pink_action_2").on("touch", c_pink_action_2)
$("#btn_pink_action_2").on("click", c_pink_action_2)

$("#btn_blue_action_1").on("touch", c_blue_action_1)
$("#btn_blue_action_1").on("click", c_blue_action_1)

$("#btn_blue_action_2").on("touch", c_blue_action_2)
$("#btn_blue_action_2").on("click", c_blue_action_2)

$("#btn_sex_action_1").on("touch", c_sex_action_1)
$("#btn_sex_action_1").on("click", c_sex_action_1)

$("#btn_sex_action_2").on("touch", c_sex_action_2)
$("#btn_sex_action_2").on("click", c_sex_action_2)

$("#btn_sex_action_3").on("touch", c_sex_action_3)
$("#btn_sex_action_3").on("click", c_sex_action_3)

$("#btn_sex_action_4").on("touch", c_sex_action_4)
$("#btn_sex_action_4").on("click", c_sex_action_4)


function sendToggle(channel) {
    //log("Toggle "+channel);
    socket.send("T"+channel);
}

var GREY = "#444"
function setState(val) {    

    $("#stateNumber").text(("000" + val.toString(16)).substr(-4));

    var bit = val & 0x01;

    $("#text_welcome").css("stroke", bit ? "#f00" : GREY);

    bit = (val >> 1) & 0x01;
    $("#text_hom").css("stroke", bit ? "#0f0" : GREY);

    bit = (val >> 2) & 0x01;
    $("#text_o").css("stroke", bit ? "#0f0" : GREY);

    bit = (val >> 3) & 0x01;
    $("#text_e").css("stroke", bit ? "#0f0" : GREY);


    bit = (val >> 4) & 0x01;
    $("#pink_dancer").css("stroke", bit ? "#f0f" : GREY);
    bit = (val >> 5) & 0x01;
    $("#pink_fucker_1").css("stroke", bit ? "#f0f" : GREY);
    $("#pink_fucker_2").css("stroke", bit ? "#f0f" : GREY);

    bit = (val >> 6) & 0x01;
    $("#blue_dancer").css("stroke", bit ? "#00f" : GREY);
    bit = (val >> 7) & 0x01;
    $("#blue_fucker_1").css("stroke", bit ? "#00f" : GREY);
    $("#blue_fucker_2").css("stroke", bit ? "#00f" : GREY);
    $("#blue_fucker_3").css("stroke", bit ? "#00f" : GREY);

    bit = (val >> 8) & 0x01;
    $("#pink_dancer_action_1").css("stroke", bit ? "#fff" : GREY);
    bit = (val >> 9) & 0x01;
    $("#pink_dancer_action_2").css("stroke", bit ? "#fff" : GREY);

    bit = (val >> 10) & 0x01;
    $("#blue_dancer_action_1").css("stroke", bit ? "#fff" : GREY);
    bit = (val >> 11) & 0x01;
    $("#blue_dancer_action_2").css("stroke", bit ? "#fff" : GREY);

    bit = (val >> 12) & 0x01;
    $("#sex_action_1").css("stroke", bit ? "#fff" : GREY);
    bit = (val >> 13) & 0x01;
    $("#sex_action_2").css("stroke", bit ? "#fff" : GREY);
    bit = (val >> 14) & 0x01;
    $("#sex_action_3").css("stroke", bit ? "#fff" : GREY);
    bit = (val >> 15) & 0x01;
    $("#sex_action_4").css("stroke", bit ? "#fff" : GREY);
}

// Create WebSocket connection.
// const socket = new WebSocket('ws://localhost:3000/socket');
const socket = new WebSocket(config.socketPath);

socket.binaryType = "arraybuffer";

// Connection opened
socket.addEventListener('open', function (event) {
    log("Socket Open");
    socket.send('Hello Server!');
});

// Listen for messages
socket.addEventListener('message', function (event) {
    if (event.data instanceof ArrayBuffer) {
        // Yay! Binary data
        var b = new Uint16Array(event.data);

        setState(b[0]);
    } else {
        log("TXT from Server: " + event.data);
    }
});

socket.addEventListener('error', function(event) {
    log("Socket error: ", event.data);
});

socket.addEventListener('close', function(event) {
    log("Socket closed");
});



// function getAnims() {
//     log("Can haz animations?");
//     socket.send("GA");
// }

// function getPalettes() {
//     socket.send("GP");
// }

// function getState() {
//     socket.send("GX");
// }

// function setUnit() {
//     var t = $("#unitToSet").val().trim();
//     if (!t) {
//         alert("Nothing to set");
//         return;
//     }

//     socket.send("SU"+t);
// }

// function setPalette() {
//     var t = $("#paletteToSet").val().trim();
//     if (!t) {
//         alert("Nothing to set");
//         return;
//     }

//     socket.send("SP"+t);
// }

// function setAnim() {
//     var t = $("#animToSet").val().trim();
//     if (!t) {
//         alert("Nothing to set");
//         return;
//     }

//     socket.send("SA"+t);
// }

// /////////

// function setUnitType(num) {
//     socket.send("SU"+num);    
// }

// function setPaletteNum(num) {
//     socket.send("SP"+num);    
// }

// function setBrightness() {
//     var t = $("#brightnessToSet").val().trim();
//     if (!t) {
//         alert("Nothing to set");
//         return;
//     }

//     socket.send("SB"+t);
// }


// function setDuration() {
//     var t = $("#durationToSet").val().trim();
//     if (!t) {
//         alert("Nothing to set");
//         return;
//     }

//     socket.send("SD"+t);
// }

// function setSpeedFactor() {
//     var t = $("#speedFactorToSet").val().trim();
//     if (!t) {
//         alert("Nothing to set");
//         return;
//     }

//     var cents = parseInt(t * 100.0);

//     socket.send("SF"+cents);
// }


// function setReverse(shouldReverse) {
//     socket.send("SR"+(shouldReverse?"+":"-"));
// }

// function showColorChooser(shouldShow) {
//     if (shouldShow) {
//         socket.send("C+");
//     } else {
//         socket.send("C-");
//     }
// }

// function setColorChooserColor(val) {
//     // log("Setting color "+val);
//     socket.send("C"+val);
// }

// function setNexusColor(isForeground) {
//     var val = $("#colorChooser").spectrum("get");
//     var color = tinycolor(val);
//     var hex = color.toHexString();

//     socket.send("C"+(isForeground?"F":"B") + val);
// }

// function updateColor() {
//     var val = $("#colorChooser").spectrum("get");

//     var color = tinycolor(val);
//     var rgb = color.toRgb();
//     $("#colorString").text("RgbColor("+rgb.r+", "+rgb.g+", "+rgb.b+")");

//     var hex = color.toHexString();
//     setColorChooserColor(hex);
// }

// function colorMove(event) {
//     // log("move");

//     updateColor()
// }

// function colorChange(event) {
//     var val = $("#colorChooser").spectrum("get");
//     log("change: "+val);
// }
// /////

// $("#colorChooser").spectrum({
//     flat: true
//     , showInput: true
//     , showButtons: false
//     , preferredFormat: "hex" 
//     , move: colorMove
//     , change: colorChange
// });

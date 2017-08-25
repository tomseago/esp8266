function log(msg) {
    console.log(msg);
    $("#log").append(msg+"\n");
}


//////////////
// UI

function c_welcome() { 
    // a 0 ***
    sendToggle("a"); // 0
}

function c_hom() { 
    // k 10
    sendToggle("b"); // 1
}

function c_o() { 
    // l 11
    sendToggle("c"); // 2
}

function c_e() { 
    // i 8
    sendToggle("d"); // 3
}

function c_pink_dancer() { 
    // c 2
    sendToggle("e"); // 4
}

function c_pink_fucker() { 
    // f 5 ***
    sendToggle("f"); // 5
}

function c_blue_dancer() { 
    // m 12
    sendToggle("g"); // 6
}

function c_blue_fucker() { 
    // e 4
    sendToggle("h"); // 7
}

function c_pink_action_1() { 
    // d 3
    sendToggle("i"); // 8
}

function c_pink_action_2() { 
    // b 1
    sendToggle("j"); // 9
}

function c_blue_action_1() {
    // p 15
    sendToggle("k"); // 10
}

function c_blue_action_2() {
    // n 13
    sendToggle("l"); // 11
}

function c_sex_action_1() {
    // j 9
    sendToggle("m"); // 12
}

function c_sex_action_2() {
    // g 6
    sendToggle("n"); // 13
}

function c_sex_action_3() {
    // h 7
    sendToggle("o"); // 14
}

function c_sex_action_4() {
    // o 14
    sendToggle("p"); // 15
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

function sendState(state) {
    var b = new Uint16Array(1);

    b[0] = state;

    socket.send(b);
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


pauseAnim = false;
timer = false;

function homoLoop1() {
    if (pauseAnim) return;

    sendState(6);
    timer = setTimeout(homoLoop2, 1400);
}

function homoLoop2() {
    if (pauseAnim) return;

    sendState(1);
    timer = setTimeout(homoLoop1, 800);
}

function runHomoLoop() {
    pauseAnim = false;
    homoLoop1();
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

///////////////////////////

var homeHomoFrames = [
    [0x0f5b, 1000],
    [0x055b,  300],
    [0x065b,  300],
    [0x0a5b,  300],
    [0x0f5b, 1000],

    [0x50a7,  200],
    [0xa0a7,  200],
    [0x50a7,  200],
    [0xa0a7,  200],
    [0x50a7,  200],
    [0xa0a7,  200],
    [0x50a7,  200],
    [0xa0a7,  200],
];

var nextFrameIx = 0;
function nextHomeHomo() {
    if (nextFrameIx >= homeHomoFrames.length) {
        nextFrameIx = 0;
    }

    var frame = homeHomoFrames[nextFrameIx];
    nextFrameIx++;
    sendState(frame[0]);

    timer = null;
    if (!pauseAnim) {
        timer = setTimeout(nextHomeHomo, frame[1]);
    }
}

function runHomeHomo() {
    if (timer) {
        clearTimeout(timer);
        timer = null;
    }

    pauseAnim = false;
    nextFrameIx = 0;
    nextHomeHomo();
}

function stopAnim() {
    if (timer) {
        clearTimeout(timer);
        timer = null;
    }

    pauseAnim = true;
}

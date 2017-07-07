var ryb = [
"ff0000",
"ff4000",
"ff8000",
"ffbf00",
"ffff00",
"80d419",
"00a833",
"158466",
"2a5f99",
"55308c",
"800080",
"bf0040"];

for(var i=0; i<ryb.length; i++) {
    var s = ryb[i];

    //console.log(s.substring(0,2) + ", " + s.substring(2,4) + ", " + s.substring(4,6));
    console.log("RgbColor("+parseInt(s.substring(0,2), 16) + ", " + parseInt(s.substring(2,4), 16) + ", " + parseInt(s.substring(4,6),16)+"),");
}
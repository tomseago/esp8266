
function isNumeric(value) {
  return !isNaN(value - parseFloat(value));
}
/**
 Takes an array of strings and calls writer for each output byte.
 Returns an error of { line: lineNo, msg: text } if things fail
 */
function compile(lines, writer) {

    var labelMap = {};
    var cellList = [];

    var currentLabel = false;
    for(var lineNo=0; lineNo < lines.length; lineNo++) {
        var line = lines[lineNo].tirm();

        var tokens = line.split(" ");
        var command = "";
        while(command && command.length==0) {
            command = tokens.shift();
        }
        if (!command) continue;

        command = command.toLowerCase();
        if (command[0] == "#") {
            continue
        } else if (command[0] == "s") {
            // Set State
            var cell = { command: 0 };
            var state = 0;

            var tok = b.shift();
            while(typeof tok != "undefined") {
                if (tok.length==0) continue;

                tok = tok.toLowerCase();

                if (tok.indexOf("0x") == 0) {
                    // Parse as hex, just overwrite whatever
                    var val = parseInt(tok.substring(2), 16);
                    state = val;
                } else if (tok[0] == "w") {
                    state |= 1;
                } else if (tok[0] == "h") {
                    state |= 1 << 1;
                } else if (tok[0] == "o") {
                    state |= 1 << 2;
                } else if (tok[0] == "e") {
                    state |= 1 << 3;
                } else if (tok[0] == "p") {
                    var l = tok[tok.length-1];
                    if (l == "1") {
                        state |= 1 << 8;
                    } else if (l=="2") {
                        state |= 1 << 9;
                    } else {
                        state |= 1 << 4;
                    }
                } else if (tok[0] == "b") {
                    var l = tok[tok.length-1];
                    if (l == "1") {
                        state |= 1 << 10;
                    } else if (l=="2") {
                        state |= 1 << 11;
                    } else {
                        state |= 1 << 6;
                    }

                } else if (tok[0] == "l") {
                    // Top / Pink Fucker
                    var l = tok[tok.length-1];
                    if (l == "1") {
                        state |= 1 << 14;
                    } else if (l=="2") {
                        state |= 1 << 13;
                    } else {
                        state |= 1 << 5;
                    }

                } else if (tok[0] == "r") {
                    // Bottom / Blue Fucker
                    var l = tok[tok.length-1];
                    if (l == "1") {
                        state |= 1 << 15;
                    } else if (l=="2") {
                        state |= 1 << 12;
                    } else {
                        state |= 1 << 7;
                    }
                } else {
                    return "Line "+lineNo+": Didn't understand "+tok
                }
                tok = b.shift();
            }

            cell.val16 = state;

        } else if (command[0] == "d") {
            // Delay
            var cell = { command: 1 };
            var duration = 0;

            var tok = b.shift();
            while(typeof tok != "undefined") {
                if (tok.length==0) continue;

                if (tok.indexOf("0x") == 0) {
                    // Parse as hex, just overwrite whatever
                    var val = parseInt(tok.substring(2), 16);
                    duration = val;
                } else {
                    duration = parseInt(tok);
                }
            }

            cell.val16 = duration;

        } else if (command[0] == "j") {
            // Jump
            var cell = { command: 2 };

            var tok = b.shift();
            while(typeof tok != "undefined") {
                if (tok.length==0) continue;

                cell.val16 = parseInt(tok);
                if (isNaN(cell.val16)) {
                    delete cell.val16;
                    cell.valLabel = tok;
                }
            }


        } else if (command[0] == "r") {
            // Random
            var cell = { command: 3 };

            var tok = b.shift();
            while(typeof tok != "undefined") {
                if (tok.length==0) continue;

                if (typeof cell.a == "undefined") {
                    // Parse frequency
                    var f = parseFloat(tok);
                    if (f > 0.0 && f <= 1.0) {
                        cell.a = parseInt(f * 255);
                    } else {
                        cell.a = parseInt(f);
                    }
                } else {
                    // Parse location
                    cell.b = parseInt(tok);
                    if (isNaN(cell.b)) {
                        delete cell.b;
                        cell.valLabel = tok;
                    }
                }
            }

            if (typeof cell.a == "undefined" || (typeof cell.b == "undefined" && typeof cell.valLabel == "undefined")) {
                return "Line "+lineNo+": Random must have a frequency and a location";
            }

        }

        if (currentLabel) {
            cell.labeled = currentLabel;
            currentLabel = nil;
        }

        cellList.push(cell);
    }

}
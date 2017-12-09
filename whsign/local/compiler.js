
function isNumeric(value) {
  return !isNaN(value - parseFloat(value));
}
/**
 Takes an array of strings and calls writer for each output cell.
 Returns an error of { line: lineNo, msg: text } if things fail
 */
function compile(lines, writer) {

    var labelMap = {};
    var cellList = [];

    // First pass build the intermediate representation into cellList
    // and also collect all the label positions for use during jump output
    var currentLabel = false;
    var commandCount = 0;
    for(var lineNo=0; lineNo < lines.length; lineNo++) {
        var line = lines[lineNo].trim();

        var tokens = line.split(" ");
        //console.log("Tokens: "+tokens);
        var command = "";
        while((typeof command != "undefined") && command.length==0) {
            command = tokens.shift();
        }
        if (!command){
            // Line ws only whitespace
            cellList.push({isWhitespace: true});
            continue;
        }

        command = command.toLowerCase();
        //console.log("command: '"+command+"'");
        if (command[0] == "#") {
            // comment
            var cell = { comment: tokens.join(" ")};
            cellList.push(cell)
            continue
        } else if (command[0] == "!") {
            // label
            currentLabel = tokens.shift();
            continue
        } else if (command[0] == "s") {
            // Set State
            var cell = { command: 0 };
            var state = 0;

            var tok = tokens.shift();
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
                tok = tokens.shift();
            }

            cell.val16 = state;

        } else if (command[0] == "d") {
            // Delay
            var cell = { command: 1 };
            var duration = 0;

            var tok = tokens.shift();
            while(typeof tok != "undefined") {
                if (tok.length==0) continue;

                if (tok.indexOf("0x") == 0) {
                    // Parse as hex, just overwrite whatever
                    var val = parseInt(tok.substring(2), 16);
                    duration = val;
                } else {
                    duration = parseInt(tok);
                }
                tok = tokens.shift();
            }

            cell.val16 = duration;

        } else if (command[0] == "j") {
            // Jump
            var cell = { command: 2 };

            var tok = tokens.shift();
            while(typeof tok != "undefined") {
                if (tok.length==0) continue;

                cell.val16 = parseInt(tok);
                if (isNaN(cell.val16)) {
                    delete cell.val16;
                    cell.valLabel = tok;
                }
                tok = tokens.shift();
            }


        } else if (command[0] == "r") {
            // Random
            var cell = { command: 3 };

            var tok = tokens.shift();
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
                tok = tokens.shift();
            }

            if (typeof cell.a == "undefined" || (typeof cell.b == "undefined" && typeof cell.valLabel == "undefined")) {
                return "Line "+lineNo+": Random must have a frequency and a location";
            }

        }

        if (currentLabel) {
            cell.labeled = currentLabel;
            labelMap[currentLabel] = commandCount;
            currentLabel = null;
        }

        cellList.push(cell);
        commandCount++;
    }

    console.log("cellList = ",cellList);
    console.log("labelMap = ",labelMap);

    for(var pos = 0; pos < cellList.length; pos++) {
        var cell = cellList[pos];

        // Resolve anything with a "valLabel"
        if (cell.valLabel) {
            var resolved = labelMap[cell.valLabel];
            if (typeof resolved == "undefined") {
                return "Line "+pos+": Unresolvable label '"+cell.valLabel+"'";
            }

            // Put it in the right spot
            if (cell.command == 3) {
                cell.b = resolved;
            } else {
                cell.val16 = resolved;
            }
        }

        writer(pos, cell);
    }
}


function stateToSegments(state) {
    var line = "";

    if (( state       ) & 0x01) {
        line += "w ";
    }
    if (( state >>  1 ) & 0x01) {
        line += "h ";
    }
    if (( state >>  2 ) & 0x01) {
        line += "o ";
    }
    if (( state >>  3 ) & 0x01) {
        line += "e ";
    }

    if (( state >>  4 ) & 0x01) {
        line += "p ";
    }
    if (( state >>  8 ) & 0x01) {
        line += "p1 ";
    }
    if (( state >>  9 ) & 0x01) {
        line += "p2 ";
    }

    if (( state >>  6 ) & 0x01) {
        line += "b ";
    }
    if (( state >> 10 ) & 0x01) {
        line += "b1 ";
    }
    if (( state >> 11 ) & 0x01) {
        line += "b2 ";
    }

    if (( state >>  5 ) & 0x01) {
        line += "l ";
    }
    if (( state >> 14 ) & 0x01) {
        line += "l1 ";
    }
    if (( state >> 13 ) & 0x01) {
        line += "l2 ";
    }

    if (( state >>  7 ) & 0x01) {
        line += "r ";
    }
    if (( state >> 15 ) & 0x01) {
        line += "r1 ";
    }    
    if (( state >> 12 ) & 0x01) {
        line += "r2 ";
    }

    return line;
}


function createEchoWriter(stringWriter) {
    return function(pos, cell) {
        //console.log("Echo for: pos="+pos+" -> ", cell);

        if (cell.isWhitespace) {
            stringWriter.write("");
        }

        if (cell.comment) {
            // output a comment
            stringWriter.write("# "+cell.comment);
            return;
        }

        // If the cell has a label output that before the cell line
        if (cell.labeled) {
            stringWriter.write("! "+cell.labeled);
        }

        // Go from byte value back to string value
        var line = ""
        switch(cell.command) {
            case 0:
                var state = cell.val16 || 0;

                // Hex value as a comment
                line += "# 0x" + state.toString(16);
                stringWriter.write(line);

                // Now the real command uses the symbolic representation
                line = "S " + stateToSegments(state);

                stringWriter.write(line);
                break;

            case 1:
                line = "D " + cell.val16;
                stringWriter.write(line);
                break;

            case 2:
                line = "J ";
                if (cell.valLabel) {
                    line += cell.valLabel;
                } else {
                    line += cell.val16;
                }
                stringWriter.write(line);
                break;

            case 3:
                line = "R " + cell.a + " ";
                if (cell.valLabel) {
                    line += cell.valLabel;
                } else {
                    line += cell.b;
                }
                stringWriter.write(line);
                break;
        }
    }
}

///////////////
// Outputs binary encoded show to the array as a series
// of integers. The array can then be saved or otherwise
// dealt with externally.
function createArrayWriter(array) {
    return function(pos, cell) {
        //console.log("Echo for: pos="+pos+" -> ", cell);

        if (cell.isWhitespace) {
            return;
        }

        if (cell.comment) {
            return;
        }

        switch(cell.command) {
            case 0:
                array.push(0);
                array.push((cell.val16 >>> 8) & 0x00ff);
                array.push(cell.val16 & 0x00ff);
                break;

            case 1:
                array.push(1);
                array.push((cell.val16 >>> 8) & 0x00ff);
                array.push(cell.val16 & 0x00ff);
                break;

            case 2:
                array.push(2);
                array.push((cell.val16 >>> 8) & 0x00ff);
                array.push(cell.val16 & 0x00ff);
                break;

            case 3:
                array.push(3);
                array.push(cell.a & 0x00ff);
                array.push(cell.b & 0x00ff);
                break;
        }
    }
}

function hexDump(array) {
    var out = [];

    var line = "";
    for(var i=0; i<array.length; i++) {
        line += " ";

        var s = array[i].toString(16);
        if (s.length < 2) {
            line += "0";
        }
        line += s;

        // if ((i+1) % 4 == 0) {
        //     // Space
        //     line += " ";
        // }

        if ((i+1) % 3 == 0) {
            // Newline!
            s = parseInt(i/3).toString();
            while (s.length < 3) {
                s = "0" + s;
            }
            out.push(s+": "+line);
            line = "";
        }
    }
    if (line) {
        console.log("Extra bytes???? " + line);
    }

    return out.join("\n");
}

function decompile(a, b, c) {
    var out = "";

    var v16 = (b << 8) + c;
    switch(a) {
        case 0:
            out += "S "+stateToSegments(v16);
            break;

        case 1:
            out += "D "+v16;
            break;

        case 2:
            out += "J "+v16;
            break;

        case 3:
            out += "R "+b+" "+c;
            break;
    }

    return out;
}

function arrayToC(array) {
    var out = [];

    for(var cursor = 0; cursor < array.length; cursor += 3) {
        var commandNo = cursor / 3;

        var line = "    ";
        line += array[cursor];
        line += ", ";

        line += array[cursor+1];
        line += ", ";

        line += array[cursor+2];
        line += ", ";

        line += "// ";
        var s = ""+commandNo;
        if (s.length < 2) {
            line += "0";
        }
        line += s;
        line += ": ";
        line += decompile(array[cursor], array[cursor+1], array[cursor+2]);

        out.push(line);
    }

    return out.join("\n");
}

function arrayToJS(array) {
    var out = "[";

    for (var i = 0; i<array.length; i += 1) {
        out += array[i];
        if (i+1 < array.length) {
            out += ",";
        }
    }
    out += "]";
    return out;
}

//-----------------------------------------------

if (module && !module.parent) {
    var fs = require("fs");

    var process = require("process");

    var srcFile = process.argv[2] || "test-all.wh";

    console.log("Using file ", srcFile);
    var src = fs.readFileSync(srcFile).toString();
    //console.log("src = "+src);

    var consoleStringWriter = {
        write: function(line) {
            console.log(line);
        }
    }

    var writer = createEchoWriter(consoleStringWriter);
    var err = compile(src.split("\n"), writer);
    if (err) {
        console.log("ERROR: ", err);
        return;
    }

    console.log("========");
    var array = [];
    writer = createArrayWriter(array);
    var err = compile(src.split("\n"), writer);
    if (err) {
        console.log("ERROR: ", err);
        return;
    }
    console.log(hexDump(array));

    console.log("------");

    console.log(arrayToC(array));

    console.log("------");

    console.log(arrayToJS(array));

    // Write it to a buffer
    var b = new Buffer(array.length);
    for(var i=0; i<array.length; i++) {
        b[i] = array[i];
    }

    var outFile = srcFile + "b";
    fs.writeFileSync(outFile, b);
    console.log("Wrote to ", outFile);
}

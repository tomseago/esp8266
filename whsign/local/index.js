const express = require('express')
const path = require('path')
const WebSocket = require('ws');

const app = express()

// app.get('/', function (req, res) {
//   res.send('Hello World!')
// })

var server = require('http').Server(app);

const wss = new WebSocket.Server({ 
    server: server
    , path: "/socket"
});

app.get("/config.js", function(req, resp) {
    var r = [];

    var config = {
        socketPath: "ws://localhost:3000/socket"
    };

    r.push("config = ");
    r.push(JSON.stringify(config));
    r.push(";");

    resp.send(r.join(""));
});

app.use(express.static(path.join(__dirname, '../data/www')));


wss.on('connection', function connection(ws, req) {
  // const location = url.parse(req.url, true);
  // You might use location.query.access_token to authenticate or share sessions
  // or req.headers.cookie (see http://stackoverflow.com/a/16395220/151312)

  ws.on('message', function incoming(message) {
    if (typeof message == "string") {
      // It's a toggle probably?
      ws.send("Echo: '"+message+"'");
    } else {
      // It should be a buffer
      ws.send(message);
    }
  });

  ws.send('a text');

  b = Buffer.alloc(2);
  b[0] = 255;
  b[1] = 255;
  ws.send(b)
});


server.listen(3000, function () {
  console.log('Example app listening on port 3000!')
})



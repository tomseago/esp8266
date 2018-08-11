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

    // config.socketPath = "ws://192.168.0.13:3000/socket"

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
    console.log('received: %s', message);

    if (message=="GA") {
      ws.send("ANIMS:WebUIStatus;Sparkle;Rainbow;Rando Fill");
      return;
    }

    if (message=="GG") {
      ws.send("GEOMS:+Basic;+All;-Special;");
      return;
    }

    if (message=="GJ") {
      ws.send('STATEJSON:{"palette":5,"speedFactor":0.500,"maxDuration":64000.000,"maxBrightness":170.000,"foreground":[173,50,204],"background":[66,35,59],"reverse":true,"geomRotated":true,"geomName":"All","animName":"WebUIStatus","chooserShown":true}');
      return;
    }

  });

  ws.send('something');
});


server.listen(3000, function () {
  console.log('Local app listening on port 3000!')
})



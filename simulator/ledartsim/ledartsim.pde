import peasy.*;
import processing.net.*;

PeasyCam cam;

LEDArtPiece pieces[];
Server laseServer;

HashMap laseByClients = new HashMap();

float phWidth = 100;
float phHeight = 50;
float phSpacing = 10;

void setup() {
  size(1024, 1024, P3D);
  
  cam = new PeasyCam(this, 5000);
  cam.setMinimumDistance(2000);
  cam.setMaximumDistance(10000);
  
  //// The art pieces
  setupSimpleStrips(20, 30);
  
  //// A server to receive data from the clients
  laseServer = new Server(this, 7453);
}

void serverEvent(Server s, Client c) {
  // Somebody new connected to us!
  LaseClient lase = new LaseClient();
  laseByClients.put(c, lase);
}

void draw() {  
  // Handle any data from our clients
  Client c = laseServer.available();
  while(c != null) {
    LaseClient lase = (LaseClient)laseByClients.get(c);
    if (lase != null) {
      lase.receiveData(c);
    }
    c = laseServer.available();
  }
  
  // An ambient environment
  background(0);
  ambientLight(255, 255, 255);

  // A ground reference plane
  pushMatrix();
  rotateX(-HALF_PI);
  translate(0,0,1000);
  fill(#2F7C40);
  
  rect(-10000, -10000, 20000, 20000);
  popMatrix();
  
  // Draw the art pieces
  for(int i=0; i<pieces.length; i++) {
    pieces[i].draw();
  }
  
  // Draw the hud for state variables
  cam.beginHUD();
  int hx=0, hy=0;
  for(int i=0; i<pieces.length; i++) {
    pieces[i].drawHUD(i);
    
    if (hx + (2 * (phSpacing + phWidth)) > width) {
      // Overflow, time to wrap!
      translate(-hx, phSpacing + phHeight);
      hx = 0;
      hy += phSpacing + phHeight;
    } else {
      // Just advance to the right
      translate(phSpacing + phWidth, 0);
      hx += phSpacing + phWidth;
    }
  }
  cam.endHUD();
}


void setupSimpleStrips(int numStrips, int numLeds) {
  pieces = new LEDArtPiece[numStrips];
  
  PVector origin = new PVector(0,0,0);
  PVector direction = new PVector(1,0,0);
  for(int i=0; i<numStrips; i++) {
    pieces[i] = new SimpleStrip(numLeds, origin, direction);
    
    origin.y -= 100;
  }
}  

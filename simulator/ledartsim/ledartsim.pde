import peasy.*;
import processing.net.*;

PeasyCam cam;

LEDArtPiece pieces[];
Server laseServer;

HashMap laseByClients = new HashMap();

float phWidth = 150;
float phHeight = 160;
float phSpacing = 10;

boolean drawHUD = true;
int maxHUDIx = 12;

void setup() {
  size(2048, 1024, P3D);
  
  cam = new PeasyCam(this, 5000);
  cam.setMinimumDistance(1000);
  cam.setMaximumDistance(10000);
  
  //// The art pieces
  //setupSimpleStrips(4, 104);
  
  setupInfinityBed();
  
  //// A server to receive data from the clients
  laseServer = new Server(this, 7453);
}

void serverEvent(Server s, Client c) {
  // Somebody new connected to us!
  println("New connectino from "+c.ip());
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
  //fill(#2F7C40);
  fill(30);
  
  rect(-10000, -10000, 20000, 20000);
  popMatrix();
  
  // XYZ reference axis 500mm in length
  float axisLen = 500;
  stroke(#ff0000);
  line(0,0,0, axisLen, 0, 0);
  stroke(#00ff00);
  line(0,0,0, 0, axisLen, 0);
  stroke(#0000ff);
  line(0,0,0, 0, 0, axisLen);
  
  
  // Draw the art pieces
  for(int i=0; i<pieces.length; i++) {
    pieces[i].draw();
  }
  
  // Draw the hud for state variables
  if (drawHUD) {
    cam.beginHUD();
    int hx=0, hy=0;
    for(int i=0; i<pieces.length && i<maxHUDIx; i++) {
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
  
  // Now some horrible things
  /*
  cam.beginHUD();
  translate(10, 800);
  fill(0);
  rect(0,0, 400, 200);
  
  translate(200, 0);
  fill(255);
  textSize(12);
  float lineh = textAscent() + textDescent() + 2;
  for(int i=0; i<pieces.length; i++) {
    textAlign(RIGHT, TOP);
    Object obj = pieces[i].values.get(3);
    if (obj == null) {
      obj = "NULL";
    }
    text(obj.toString(), 0, 0);
    textAlign(LEFT, TOP);
    obj = pieces[i].values.get(5);
    if (obj == null) {
      obj = "NULL";
    }
    text(obj.toString(), 2, 0);
    translate(0, lineh);
  }
  cam.endHUD();
  */
  
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


void setupInfinityBed() {
  pieces = new LEDArtPiece[12];
  
  float widthSpace = 100;
  // front left end
  PVector origin = new PVector(-(IPWidth + widthSpace * 1.5), -IPHeight, (IPHeight / 2.0) + 20);
  PVector first = new PVector(-1, 0, 0); // left
  PVector second = new PVector(0, 1, 0); // down
  
  pieces[0] = new InfinityPanel(origin, first, second);

  // front left middle
  origin.x += widthSpace + IPWidth;
  pieces[1] = new InfinityPanel(origin, first, second);
  
  // front right middle
  origin.x += widthSpace + IPWidth;
  pieces[2] = new InfinityPanel(origin, first, second);
  
  // front right end
  origin.x += IPWidth + widthSpace;
  pieces[3] = new InfinityPanel(origin, first, second);
  
  
  //////
  
  // top left end
  origin = new PVector(-(IPWidth + widthSpace * 1.5), -IPHeight - 20, -(IPHeight / 2.0));
  first = new PVector(-1, 0, 0); // left
  second = new PVector(0, 0, 1); // towards
  pieces[4] = new InfinityPanel(origin, first, second);

  // top left middle
  origin.x += widthSpace + IPWidth;
  pieces[5] = new InfinityPanel(origin, first, second);
  
  // top right middle
  origin.x += widthSpace + IPWidth;
  pieces[6] = new InfinityPanel(origin, first, second);
  
  // top right end
  origin.x += IPWidth + widthSpace;
  pieces[7] = new InfinityPanel(origin, first, second);
  

  origin = new PVector(-(IPWidth + widthSpace * 1.5), -IPHeight, -(IPHeight / 2.0) - 20);
  first = new PVector(-1, 0, 0); // left
  second = new PVector(0, 1, 0); // down
  
  pieces[8] = new InfinityPanel(origin, first, second);

  // front left middle
  origin.x += widthSpace + IPWidth;
  pieces[9] = new InfinityPanel(origin, first, second);
  
  // front right middle
  origin.x += widthSpace + IPWidth;
  pieces[10] = new InfinityPanel(origin, first, second);
  
  // front right end
  origin.x += widthSpace + IPWidth;
  pieces[11] = new InfinityPanel(origin, first, second);
 }

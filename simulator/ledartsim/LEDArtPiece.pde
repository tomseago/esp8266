public enum PixelType {
  PT_RGB,
  PT_RBG,
  PT_RGBW
};

int DATA_TIMEOUT = 1000;

public float SPACING_30_LED = 1000.0 / 30;
public float SPACING_60_LED = 1000.0 / 60;
public float SPACING_144_LED = 1000.0 / 144;


class LEDArtPiece {

  LEDArtPixel pixels[];
  PVector position;
  PVector rotation;
  
  PixelType type = PixelType.PT_RGB;
  
  private HashMap labels = new HashMap();
  private HashMap values = new HashMap();
  
  private int lastData;
  FPSCounter pixelTimes = new FPSCounter(20);
  
  public LEDArtPiece() {
  }
  
  public void setPixelType(PixelType t) {
    this.type = t;
  }

  protected void setNumberPixels(int num) {
    pixels = new LEDArtPixel[num];
  }
  
  /**
   * Adds a strip of pixels from a point in a given direction.
   * The spacing is the unit distance between pixels (the first pixel being
   * set half a unit distance from the anchor point). Common values are
   * defined as constants above.
   *
   * The pixels array is assumed to already exist with sufficient capacity
   * to include the new pixels that will be generated by this function.
   *
   * @returns A PVector for the end of the strip, which is half a unit distant
   *          from the last pixel position. This is appropriate for starting a
   *          new strip that may go off in a different direction.
   */
  protected PVector createPixelStrip(float spacing, PVector origin, PVector direction, int offset, int length) {
    int pixelIx = offset;
    PVector pos = new PVector();
    pos.set(origin);
    direction.setMag(spacing/2.0);
    pos.add(direction);
    direction.setMag(spacing);
    
    // Now ready to create our pixels
    for(int i=0; i<length; i++, pixelIx++) {
      LEDArtPixel p = new LEDArtPixel(pos.x, pos.y, pos.z);
      pixels[pixelIx] = p;
      
      pos.add(direction);
    }
    
    // Back up half a space
    direction.setMag(spacing/2.0);
    pos.add(PVector.mult(direction, -1.0));
    return pos;
  }
  
  public void draw() {
    if (rotation != null) {
      rotateX(rotation.x);
      rotateY(rotation.y);
      rotateZ(rotation.z);
    }
    if (position != null) {
      translate(position.x, position.y, position.z);
    }
    
    noStroke();
    for (int i=0; i<pixels.length; i++) {
      pixels[i].draw();
    }
  }
  
  public void drawHUD(int number) {
    boolean dataIsCurrent = millis() - lastData < DATA_TIMEOUT;
    
    // Assume the origin has been placed at the top left corner of our space
    // Just start drawing some text into a little box thingie
    if (dataIsCurrent) {
      fill(#00831D);
    } else {
      // OMG! Give me some data!
      fill(#D30909);
    }
    rect(0,0,phWidth,phHeight); // We are in pixel space now
    
    float indexWidth = 28; // based on the textSize for the index number
    textAlign(RIGHT);
    fill(#ffffff);
    textSize(20);
    text(number, indexWidth, 20); 
    
    float tableWidth = phWidth - indexWidth - 2;
    float tableSep = phWidth - (0.55 * tableWidth);
        
    textSize(12);
    float vH = textAscent() + textDescent();
    float vSpacing = 2;
    float vHSpace = 2;
    
    float lineY = vSpacing;
    for(int i=1; i<=8; i++) {
      Integer key = new Integer(i);
      String label = (String)labels.get(key);

      //label = "label";
      if (label != null) {
        textAlign(RIGHT, TOP);
        text(label, tableSep - vHSpace, lineY);
      }
      
      Object value = values.get(key);
      //value = "VAL!";
      if (value != null) {
        textAlign(LEFT, TOP);
        text(value.toString(), tableSep, lineY);
      }
      
      lineY += vH + vSpacing;      
    }
    
    text(Integer.toString(pixelTimes.avgIFTms()), tableSep, lineY); 
  }
  
  public void setPixelData(byte[] data) {
    pixelTimes.mark();
    
    lastData = millis();
    int cursor = 0;
    int pixelIx = 0;
    int length = data.length;
    
    while(cursor < length && pixelIx < pixels.length) {
      int r = data[cursor++] & 0x00ff;

      if (cursor==length) {
        break;
      }      
      int g = data[cursor++] & 0x00ff;
      
      if (cursor==length) {
        break;
      }      
      int b = data[cursor++] & 0x00ff;
      
      if (type == PixelType.PT_RGBW) {
        if (cursor==length) {
          break;
        }
        int w = data[cursor++] & 0x00ff;
        pixels[pixelIx].setColor(r,g,b,w);
      } else {
        if (type == PixelType.PT_RBG) {
          // SetColor wants RGB, but we have b & g swapped because of the type
          pixels[pixelIx].setColor(r,b,g);
        } else {
          //println("Set pixel "+pixelIx+" to "+r+", "+g+", "+b);
          pixels[pixelIx].setColor(r,g,b);
        }
      }

      pixelIx++;
    }
  }
  
  public void setLabels(HashMap labels) {
    this.labels = labels;
  }
  
  public void setValues(HashMap values) {
    this.values = values;
  }
}

class SimpleStrip extends LEDArtPiece {

  PVector origin = new PVector(0,0,0);
  PVector direction = new PVector(1,0,0);

  int numLeds;
  
  SimpleStrip(int numLeds) {
    this.numLeds = numLeds;
    createPixels();
  }
  
  SimpleStrip(int numLeds, PVector origin, PVector direction) {
    this.numLeds = numLeds;
    this.origin = origin;
    this.direction = direction;
    createPixels();
  }
  
  protected void createPixels() {    
    setNumberPixels(numLeds);
    createPixelStrip(SPACING_30_LED, origin, direction, 0, numLeds);
  }
}

final int NUM_SHORT = 16;
final int NUM_LONG = 34;

float IPWidth = NUM_SHORT * SPACING_30_LED;
float IPHeight = NUM_LONG * SPACING_30_LED;

class InfinityPanel extends LEDArtPiece {
  PVector origin;
  PVector firstDir;
  PVector secondDir;  
  
  InfinityPanel(PVector origin, PVector first, PVector second) {
    this.origin = origin;
    this.firstDir = first;
    this.secondDir = second;
    
    createPixels();
  }
  
  protected void createPixelsOld() {
    type = PixelType.PT_RGBW;
    setNumberPixels(2 * (NUM_SHORT + NUM_LONG));
    
    // First strip from origin 
    PVector pos = createPixelStrip(SPACING_30_LED, origin, firstDir, 0, NUM_SHORT);
    
    // Now a long one
    pos = createPixelStrip(SPACING_30_LED, pos, secondDir, NUM_SHORT, NUM_LONG);
    
    // Reverse the short
    pos = createPixelStrip(SPACING_30_LED, pos, PVector.mult(firstDir,-1.0), NUM_SHORT+NUM_LONG, NUM_SHORT);
    
    // Reverse the long
    pos = createPixelStrip(SPACING_30_LED, pos, PVector.mult(secondDir,-1.0), NUM_SHORT+NUM_LONG+NUM_SHORT, NUM_LONG);
  }
  
  protected void createPixels() {
    type = PixelType.PT_RGBW;
    setNumberPixels(2 * (NUM_SHORT + NUM_LONG));
    
    // First strip from origin 
    int ssShort = 7;
    int ssLong = NUM_SHORT-ssShort;
    
    PVector pos = origin.copy();
    PVector offset = firstDir.copy().normalize().mult(SPACING_30_LED * ssLong);
    pos.add(offset);
    pos = createPixelStrip(SPACING_30_LED, pos, firstDir, 0, ssShort);
    int index = ssShort;
    
    // Now a long one
    pos = createPixelStrip(SPACING_30_LED, pos, secondDir, index, NUM_LONG);
    index += NUM_LONG;
    
    // Reverse the short
    pos = createPixelStrip(SPACING_30_LED, pos, PVector.mult(firstDir,-1.0), index, NUM_SHORT);
    index += NUM_SHORT;
    
    // Reverse the long
    pos = createPixelStrip(SPACING_30_LED, pos, PVector.mult(secondDir,-1.0), index, NUM_LONG);
    index += NUM_LONG;
    
    // Finish with the ss long
    pos = createPixelStrip(SPACING_30_LED, pos, firstDir, index, ssLong);
  }
}

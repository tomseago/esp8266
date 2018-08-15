class LEDArtPixel {
  private float x,y,z;
  private int r,g,b;

  public LEDArtPixel(float x, float y, float z) {
    this.x = x;
    this.y = y;
    this.z = z;
    
    this.r = 255;
    this.g = 0;
    this.b = 0;
  }
  
  public void draw() {
    pushMatrix();    
    translate(x, y, z);
    fill(r, g, b);
    box(5);
    popMatrix();
  }
  
  public void setColor(int r, int g, int b) {
    this.r = r;
    this.g = g;
    this.b = b;
  }
  
  public void setColor(int r, int g, int b, int w) {
    this.r = r;
    this.g = g;
    this.b = b;
    // TODO: w!
  }
}

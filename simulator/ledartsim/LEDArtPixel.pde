class LEDArtPixel {
  private float x,y,z;
  private int r,g,b,w;

  public LEDArtPixel(float x, float y, float z) {
    this.x = x;
    this.y = y;
    this.z = z;
    
    this.r = 255;
    this.g = 0;
    this.b = 0;
    this.w = 0;
  }
  
  public void draw() {
    pushMatrix();    
    translate(x, y, z);
    fill(r, g, b);
    box(5);
    translate(5, 0, 0);
    fill(this.w);
    box(7);
    popMatrix();
  }
  
  public void setColor(int r, int g, int b) {
    this.r = r;
    this.g = g;
    this.b = b;
    this.w = 0;
  }
  
  public void setColor(int r, int g, int b, int w) {
    this.r = r;
    this.g = g;
    this.b = b;
    
    // Add in w caping things at 255
    this.w = w;
    //this.r += w;
    //if (this.r > 255) this.r = 255; 
    //this.g += w;
    //if (this.g > 255) this.g = 255; 
    //this.b += w;
    //if (this.b > 255) this.b = 255; 
  }
}

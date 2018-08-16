public class FPSCounter {
  int interFrameTimes[];
  int capacity;
  int nextSlot = 0;
  int lastFrameAt = 0;
  
  FPSCounter(int window) {
    interFrameTimes = new int[window];
    capacity = window;
  }
  
  public void mark() {
    int now = millis();
    int elapsed = now - lastFrameAt;
    
    interFrameTimes[nextSlot] = elapsed;
    nextSlot++;
    if (nextSlot == capacity) {
      nextSlot = 0;
    }
    
    lastFrameAt = now;
  }
  
  public int avgIFTms() {
    int accum = 0;
    for(int i=0; i<capacity; i++) {
      accum += interFrameTimes[i];
    }
    
    return accum / capacity;
  }
}

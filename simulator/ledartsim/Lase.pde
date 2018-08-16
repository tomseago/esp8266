
enum PS {
  ReadInt3,
  ReadInt2,
  ReadInt1,
  ReadInt0,

  ReadString,
  
  WantMsgType,
  
  // message type 1
  // an array of bytes based on the setup message
  WantPixels, 
  
  // message type 2 meta data (setup)
  // uint16 pieceNumber
  // uint16 numPixels
  // uint8  bytesPerPixel
  GotPieceNumber,
  GotNumPixels,
  
  // message type 3 state labels
  // uint8   state variable tag, 0 indicates end of message
  // string  label
  WantStateLabelTag,
  GotStateLabelString,
  
  // message type 4 state values
  // uint8  state variable tag, 0 indicated end of message
  // uint8  variable type, 0 string, 1 int, 2 float
  // string or 4 bytes of data per type
  WantVarTag,
  WantVarType,
  GotVarValue,
  
  MustStop,
}

public class LaseClient {
  
  int pieceNumber; // Which art piece am I in a constellation of several pieces
  int numPixels;   // The client tells us how many pixels it will be sending data for
  byte bytesPerPixel; 
  
  byte[] pixelData;
  
  HashMap stateLabels = new HashMap();
  HashMap stateVars = new HashMap();
  
  // Parsing things
  byte messageType;
  
  byte[] buffer = new byte[10240];
  PS parseState = PS.WantMsgType;
  PS afterRead;
  
  int pixelDataCursor;
  int intBuffer;
  StringBuilder stringBuffer = new StringBuilder();
  
  byte varTag;
  byte varType;
  
  /**
   * Get data from the attached client, which may result in an update
   * to one of the known art pieces.
   */
  void receiveData(Client client) {
    int read = client.readBytes(buffer);
    //printBytes(buffer, read);
    
    int cursor = 0;
    while(cursor < read) {
      //println(cursor, buffer[cursor] & 0x00ff, parseState);
      switch(parseState) {
        /////
        case WantMsgType:
          messageType = buffer[cursor++];
          handleMessageType();
          break;
          
          
        /////
        case WantPixels:
          // We want pixels and there is data, so consume it
          pixelData[pixelDataCursor++] = buffer[cursor++];
          
          // Have we read enough data?
          if (pixelDataCursor == (numPixels * bytesPerPixel)) {
            // Have read a full set of pixel data so dispatch it
            sendPixelData();
            parseState = PS.WantMsgType;
          }
          break;
          
        /////
        case GotPieceNumber:
          pieceNumber = intBuffer;
          afterRead = PS.GotNumPixels;
          intBuffer = 0;            
          parseState = PS.ReadInt1;
          break;
          
        case GotNumPixels:
          numPixels = intBuffer;
          
          bytesPerPixel = buffer[cursor++];
          
          // This should always be the first message, so we create a new buffer here
          pixelData = new byte[numPixels * bytesPerPixel];
          
          parseState = PS.WantMsgType;
          break;
        
        
        /////
        case WantStateLabelTag:
          intBuffer = buffer[cursor++];
          if (intBuffer == 0) {
            // No more labels
            sendStateLabels();
            parseState = PS.WantMsgType;
          } else {
            // Need a string for it
            afterRead = PS.GotStateLabelString;
            stringBuffer.setLength(0);
            parseState = PS.ReadString;
          }
          break;
        
        case GotStateLabelString:
          stateLabels.put(new Integer(intBuffer), stringBuffer.toString());
          parseState = PS.WantStateLabelTag;
          break;
          
        /////
        case WantVarTag:
          varTag = buffer[cursor++];
          if (varTag == 0) {
            // No more vars
            sendStateVars();
            parseState = PS.WantMsgType;
          } else {
            // Need a type and a value
            parseState = PS.WantVarType;
          }
          break;
          
        case WantVarType:
          varType = buffer[cursor++];
          handleVarType();
          break;
          
        case GotVarValue:
          handleVarValue();
          break;
          
        /////
        case ReadInt3:
          intBuffer = intBuffer << 8;
          intBuffer += buffer[cursor++] & 0x00ff;
          parseState = PS.ReadInt2;
          break;
          
        case ReadInt2:
          intBuffer = intBuffer << 8;
          intBuffer += buffer[cursor++] & 0x00ff;
          parseState = PS.ReadInt1;
          break;
          
        case ReadInt1:
          intBuffer = intBuffer << 8;
          intBuffer += buffer[cursor++] & 0x00ff;
          parseState = PS.ReadInt0;
          break;
          
        case ReadInt0:
          intBuffer = intBuffer << 8;
          intBuffer += buffer[cursor++] & 0x00ff;
          parseState = afterRead;
          //println("Read int "+intBuffer);
          break;
          
        
        case ReadString:
          if (buffer[cursor]==0) {
            // String is done
            cursor++; // eat that null though of course...
            parseState = afterRead;
          } else {
            // Add this character
            try {
              stringBuffer.appendCodePoint(buffer[cursor++]);
            } catch (Exception ex) {
              println("ERROR with string data, closing the client");
              client.stop();
              return;
            }
          }
          break;
          
        case MustStop:
          client.stop();
          return;
      }
    }
  }
  
  void handleMessageType() {
    switch(messageType) {
      case 1: // Pixel Data
        pixelDataCursor = 0;
        parseState = PS.WantPixels;
        break;
        
      case 2: // Basic meta data
        // Number of pixels
        afterRead = PS.GotPieceNumber;
        intBuffer = 0;            
        parseState = PS.ReadInt1;
        break;
        
      case 3: // Label tags
        parseState = PS.WantStateLabelTag;
        break;
        
      case 4: // State values
        parseState = PS.WantVarTag;
        break;
        
      default:
        println("ERROR: Unknown message type");
        parseState = PS.MustStop;
        break;
    }
  }
  
  void handleVarType() {
    afterRead = PS.GotVarValue;
    switch(varType) {
      case 0: // String
        stringBuffer.setLength(0);
        parseState = PS.ReadString;
        break;
        
      case 1: // Int
        intBuffer = 0;
        parseState = PS.ReadInt3;
        break;
        
      case 2: // Float
        intBuffer = 0;
        parseState = PS.ReadInt3;
        break;
    }
  }
  
  void handleVarValue() {
    Object value = null;
    switch(varType) {
      case 0:
        value = stringBuffer.toString();
        break;
        
      case 1:
        value = new Integer(intBuffer);
        break;
        
      case 2:
        value = new Float(Float.intBitsToFloat(intBuffer));
        break;
    }
    if (value != null) {
      //println("Got state value tag="+varTag+"  value="+value);
      stateVars.put(new Integer(varTag), value);
    }
    
    parseState = PS.WantVarTag;
  }
  
  /////
  LEDArtPiece findPiece() {    
    if (pieceNumber < 0 || pieceNumber >= pieces.length) {
      // We don't know her
      println("Don't know pieceNumber "+pieceNumber);
      return null;
    }
    
    return pieces[pieceNumber];
  }
    
  void sendPixelData() {
    //print("SetPixelData ");
    //printBytes(pixelData, pixelData.length);
    
    LEDArtPiece piece = findPiece();
    if (piece == null) return;
    
    piece.setPixelData(pixelData);
  }
 
  void sendStateLabels() {
    LEDArtPiece piece = findPiece();
    if (piece == null) return;
   
    piece.setLabels(stateLabels);
  }
  
  void sendStateVars() {
    LEDArtPiece piece = findPiece();
    if (piece == null) return;
   
    piece.setValues(stateVars);
  }
  
  void printBytes(byte[] buffer, int length) {
    StringBuilder out = new StringBuilder();
    
    for(int i=0; i<length; i++) {
      out.append(buffer[i] & 0x00ff);
      out.append(" ");
    }
    
    println(out);
  }
}

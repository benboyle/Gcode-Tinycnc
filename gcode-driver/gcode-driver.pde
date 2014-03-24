
String[] lines;
int index = 0;
import processing.serial.*; //import the Serial library
Serial myPort;  //the Serial port object
String val;

void setup() {
  size(200, 200);
  background(0);
  stroke(255);
 //lines = loadStrings("star.gcode");
 // lines = loadStrings("spirograph2.gcode");
  // lines = loadStrings("boxex.gcode");
  lines = loadStrings("grids.gcode");
  myPort = new Serial(this, Serial.list()[0], 9600);
}

void draw() {
}

void serialEvent( Serial myPort) {
  val = myPort.readStringUntil('\n');
  if (val != null) {
    val = trim(val);
    println("<<"+val+">>");
    if (val.equals("OK")) {
      myPort.clear();
      myPort.write(lines[index]+"\r"); 
      print(">>"+lines[index]+"<<\n");
      index++;
      val="not ok";
    }
  }
}


/*
	this progrom interprets gcode for tinycnc
 */

#include <Servo.h>

#define SERVOPINX 9
#define SERVOPINY 10
#define SERVOPINZ 11
#define STARTX 90										
#define STARTY 90
#define PENUPZ 30
#define PENDOWNZ 112
#define MAXX 180
#define MAXY 180
#define MAXZ 180
#define MINX 0
#define MINY 0
#define MINZ 0
// DiameterXY 49.74mm                ; this is the diameter of the gear point to point
// PI 3.142
// circumferenceXY = diameterXY * PI ; 156.26mm     if we could rotate 360 degrees  
// maxXYmm = circumferenceXY / 2     ; 78.13mm      but servos only rotate 180 degrees      
// mm per Degree = 180 / maxXYMM     ; 2.432deg     so each mm can have 2.432 degrees
// Degree per mm =  maxXYMM / 180    ; .396mm       and each degree represents .396mm
#define MMDEGREEXY 2.432

#define MAXINPUT 256
char inputLine[MAXINPUT] = "";

//int oldZ = 0;
int newX = 0;
int newY = 0;
int newZ = 0;
float newXmm = 0.0;
float newYmm = 0.0;


/* Naming servos    */
Servo servoX;
Servo servoY;
Servo servoZ;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  servoX.attach(SERVOPINX);
  servoY.attach(SERVOPINY);
  servoZ.attach(SERVOPINZ);

  // center pen
  servoX.write(180/2);
  servoY.write(180/2);
  // put pen up
  servoZ.write(PENUPZ);

}

void loop() {

	// tell pc we are ready for next line of gcode
  Serial.println("OK");

	// get a line of gcode
  getLine();  

  // G0 X12.3 Y14.5 Z12.3 F1200   fast move
  // G1 X12.3 Y12.3 Z12.3 F1200   regular move
  if (strncmp(inputLine,"G0",2)==0 || strncmp(inputLine,"G1",2)==0) {

    char* indexX = strchr( inputLine+2, 'X' ); // search for X offset
    if (indexX > 0) {             // if we find it  
      newXmm = atof(indexX+1) + 180/MMDEGREEXY/2; // assume relative to center of x
    }
    char* indexY = strchr( inputLine+2, 'Y'); // search for Y offset
    if (indexY > 0) {             // if we find it  
      newYmm = atof(indexY+1) + 180/2/MMDEGREEXY; // assume relative to center of y
    }

    // convert mm in gcode to degrees for servos, rounding up .5 degrees
    newX = (int)((newXmm*MMDEGREEXY)+.5);
    newY = (int)((newYmm*MMDEGREEXY)+.5);

    drawLine(newX, newY);  // draw a line to the new point
  }

  // G04 Pxxx delay xxxms
  if (strncmp(inputLine,"G4",2)==0) { 
    char* indexP = strchr( inputLine, 'P' ); // find the P
    if (indexP > 0) {             // if we find it  
      int delayMS = atoi(indexP+1); // convert
      delay(delayMS);             // wait
    } 
  }
  // M300 S30 -> pendown S50 -> pen up
  if (strncmp(inputLine,"M300",4)==0) { 
    char* indexS = strchr( inputLine, 'S' ); // find the S
    if (indexS > 0) { // if we find it  
			delay(25);  
      int newS = atoi(indexS+1); // convert
      if (newS == 30) {
        servoZ.write(PENDOWNZ);
				delay(25);
      } 
      else if (newS == 50) {
        servoZ.write(PENUPZ);
				delay(25);
      } 
    } 
  }
}                             
/*
	getLine
 get a complete line, ignore comments
 */
void getLine() { 

  int newLine = false;   
  int linePtr = 0;
  int inComment = false;
	
  while (!newLine) {  // loop until we ge a complete line

    if (Serial.available()>0){
      
			char thisChar = Serial.read();   // get the next char of input
			
   //   // strip comments
			if (inComment) {
				if (thisChar == ')') {   // end of comment
					inComment = false;
				}
				continue; // continue while loop skip comment
			}
		  if (thisChar == '(') {     // start of comment
				inComment = true;
				continue; // continue while loop skip comment
			}
	
			if (thisChar == 13) {
				newLine = true;
				inputLine[linePtr++] = '\0';
				continue;
			} 
			else {
				// otherwise append it to line
				linePtr = min(MAXINPUT-1, linePtr);
				inputLine[linePtr++] = thisChar;
			}
		}
  }
}

/*
     drawline from where we are now
 to x1 and y1 
 */

void drawLine(int x1, int y1) {

  int x0 = servoX.read();  // this is where we are
  int y0 = servoY.read();   

  // sanity check
  x1 = max(MINX,x1);
  x1 = min(MAXX,x1);
  y1 = max(MINY,y1);
  y1 = min(MAXY,y1);

  int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;  // find distance and direction on x axis
  int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1;  // find distance and direction on y axis
  int err = (dx>dy ? dx : -dy)/2;            // 

  for(;;){

    servoX.write(x0);
    servoY.write(y0);
    
		if (servoZ.read() == PENDOWNZ) {
			delay(25);   // give servos some time
		}
	
    if (x0==x1 && y0==y1) break; // we are where we were asked to go
    
		int e2 = err;   // save current err
    if (e2 >-dx) { 
      err -= dy; 
      x0 += sx; 
    }
    if (e2 < dy) { 
      err += dx; 
      y0 += sy; 
    }
  }
}







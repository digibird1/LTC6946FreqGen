#include <SPI.h>
#include <LiquidCrystal.h>

const int chipSelectPin = 10;

const int LCD_RW=3;//PD3;
const int LCD_RS=4;//PD4;
const int LCD_E=5;//PD5;
const int LCD_D4=6;//PD6;
const int LCD_D5=7;//PD7;
const int LCD_D6=8;//PB0;
const int LCD_D7=9;//PB1;


const int PushButtonMenu=2;//B1
const int PushButtonSelect=A5;
const int PushButtonLeft=A4;
const int PushButtonUp=A3;
const int PushButtonRight=A2;
const int PushButtonDown=A1;


int LastButtonState=LOW;
long ButtonPushCount=0;


LiquidCrystal lcd(LCD_RS, LCD_RW, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

unsigned long int GlobalFrequency=400000000;
const unsigned int R=41;//the low 8 bits of the R div, try R=41
const unsigned long f_PFD=19680000/R;//Hz

unsigned int FreqIncr=0;//factor by what to incrase freq 0 = pfd, 1=1MHz, 2=10 MHz, 3=100Mhz


void setup() {
  //init LCD
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Initializing ...");
   
  // put your setup code here, to run once:
  Serial.begin(9600);

  // start the SPI library:
  SPI.begin();
  pinMode(chipSelectPin, OUTPUT);


}

boolean ButtonIsPushed(int ButtonPin){
  if(digitalRead(ButtonPin)==HIGH){
    if(LastButtonState==LOW){//Check if we released the button in the meantime or push for longer time
      LastButtonState=HIGH;
      delay(100);
      return true;
    }
    else{
      ButtonPushCount++;//count how long the button is pressed
      delay(100);
      if(ButtonPushCount>20000){
        return true;
      }  
      return false; 
    }
  }
  else{//Button is not pressed
    LastButtonState=LOW;
    ButtonPushCount=0;
    return false;
  }
}

bool checkButtonAction(){
  //we have a pfd freqyency f_PFD
  //depending on the OD (output divioder we have different step values
  //depending on teh frequeny range

  unsigned long NewValue=0;
  
   if(ButtonIsPushed(PushButtonUp)){//up count
    if(GlobalFrequency<=623000000)NewValue=GlobalFrequency+80000;
    else if(GlobalFrequency<=748000000)NewValue=GlobalFrequency+96000;
    else if(GlobalFrequency<=935000000)NewValue=GlobalFrequency+120000;
    else if(GlobalFrequency<=1247000000)NewValue=GlobalFrequency+160000;
    else if(GlobalFrequency<=1870000000)NewValue=GlobalFrequency+240000;
    else NewValue=GlobalFrequency+480000;

    if(FreqIncr==1) NewValue=GlobalFrequency+1000000;
    if(FreqIncr==2) NewValue=GlobalFrequency+10000000;
    if(FreqIncr==3) NewValue=GlobalFrequency+100000000;
  }

  if(ButtonIsPushed(PushButtonDown)){//down count
    if(GlobalFrequency<=623000000)NewValue=GlobalFrequency-80000;
    else if(GlobalFrequency<=748000000)NewValue=GlobalFrequency-96000;
    else if(GlobalFrequency<=935000000)NewValue=GlobalFrequency-120000;
    else if(GlobalFrequency<=1247000000)NewValue=GlobalFrequency-160000;
    else if(GlobalFrequency<=1870000000)NewValue=GlobalFrequency-240000;
    else NewValue=GlobalFrequency-480000;

    if(FreqIncr==1) NewValue=GlobalFrequency-1000000;
    if(FreqIncr==2) NewValue=GlobalFrequency-10000000;
    if(FreqIncr==3) NewValue=GlobalFrequency-100000000;
  }

  if(ButtonIsPushed(PushButtonLeft)){//up count
     if(FreqIncr<3)FreqIncr++;
     return true;
  }

  if(ButtonIsPushed(PushButtonRight)){//down count
    if(FreqIncr>0)FreqIncr--;
    return true;
  }


    if(NewValue>=373000000 && NewValue<=3740000000){
      GlobalFrequency=NewValue;
      return true;
    }  
    return false;
}

void writeToReg(int Addr, int value){
  // take the chip select low to select the device:
  digitalWrite(chipSelectPin, LOW);

  SPI.transfer(Addr<<1|0x0); //Send Addr location with 0 for writing
  SPI.transfer(value);//send value
  // take the chip select high to de-select:
  digitalWrite(chipSelectPin, HIGH);
  
}

int readReg(int Addr){
  // take the chip select low to select the device:
  digitalWrite(chipSelectPin, LOW);

  SPI.transfer(Addr<<1|0x1); //Send Addr location with 1 for reading
  int inByte = SPI.transfer(0x0);//send value
  // take the chip select high to de-select:
  digitalWrite(chipSelectPin, HIGH);
  
  return inByte;
}

void printAllReg(){
  Serial.print("Reg0 0b");
  Serial.println(readReg(0x0), BIN); 
  Serial.print("Reg1 0b");
  Serial.println(readReg(0x1), BIN); 
  Serial.print("Reg2 0b");
  Serial.println(readReg(0x2), BIN); 
  Serial.print("Reg3 0b");
  Serial.println(readReg(0x3), BIN); 
  Serial.print("Reg4 0b");
  Serial.println(readReg(0x4), BIN); 
  Serial.print("Reg5 0b");
  Serial.println(readReg(0x5), BIN); 
  Serial.print("Reg6 0b");
  Serial.println(readReg(0x6), BIN); 
  Serial.print("Reg7 0b");
  Serial.println(readReg(0x7), BIN);
  Serial.print("Reg8 0b");
  Serial.println(readReg(0x8), BIN); 
  Serial.print("Reg9 0b");
  Serial.println(readReg(0x9), BIN); 
  Serial.print("RegA 0b");
  Serial.println(readReg(0xA), BIN);  
  Serial.print("RegB 0b");
  Serial.println(readReg(0xB), BIN); 
}

void printLCDFreq(unsigned long int f){
  lcd.clear();
  lcd.setCursor(0, 0);
  String myf_String=String(f);
  String myf_StringMHz=myf_String.substring(0,myf_String.length()-6)+String(".")+myf_String.substring(myf_String.length()-6,myf_String.length());
  Serial.println(myf_StringMHz);
  lcd.print(myf_StringMHz);
  lcd.setCursor(12, 0);
  lcd.print("MHz");
}

long setFrequencyHz(unsigned long f){
  
  unsigned long N=0;
  unsigned int OD=6;

  if(f<373000000){
    Serial.println("Frequency too low");
    return 0;
  }

  if(f>3740000000){
    Serial.println("Frequency too high");
    return 0;  
  }

  if(f<623000000){ //OD Div=6
    OD=6;
    //caluclate the RDiv
    N=(f*OD)/f_PFD;
  }
  else if(f<748000000){ //OD Div=5
    OD=5;
    //caluclate the RDiv
    N=(f*OD)/f_PFD;
  }
  else if(f<935000000){ //OD Div=4
    OD=4;
    //caluclate the RDiv
    N=(f*OD)/f_PFD;
  }
  else if(f<1247000000){ //OD Div=3
    OD=3;
    //caluclate the RDiv
    N=(f*OD)/f_PFD;
  }  
  else if(f<1870000000){ //OD Div=2
    OD=2;
    //caluclate the RDiv
    N=(f*OD)/f_PFD;
  }
  else if(f<=3740000000){ //OD Div=1
    OD=1;
    //caluclate the RDiv
    N=(f*OD)/f_PFD;
  }
  else{
    Serial.println("Frequency Error");
    return 0;
  }
  Serial.print("Value N Div:");
  Serial.println(N);

  Serial.print("Setting: ");
  Serial.print((f_PFD*N)/OD);
  Serial.println("Hz");


  unsigned int N_High=(N>>8)&0xFF;
  unsigned int N_Low=(N)&0xFF;

  writeToReg(0x2,0x0A);//turn off the output
  writeToReg(0x3,0x0); //Bdiv and RDivHigh
  writeToReg(0x4,R);//Rdiv low
 // writeToReg(0x5,0b00001101);//N counter high
 // writeToReg(0x6,0b10101100); //Ncounter low

  writeToReg(0x5,N_High);//N counter high
  writeToReg(0x6,N_Low); //Ncounter low
  
  writeToReg(0x7,0x63);
  delay(5);
  writeToReg(0x8,0b10111000|OD);//O divider last three bits
  writeToReg(0x9,0b10011011);
  writeToReg(0xA,0xC0);
  delay(5);
  writeToReg(0x2,0x08); //trun the output on

  //lcd.clear();
  //lcd.setCursor(0, 0);

  unsigned long int myf=((f_PFD*N)/OD); //we get wrong number when using double
  printLCDFreq(myf);

  /*String myf_String=String(myf);
  String myf_StringMHz=myf_String.substring(0,myf_String.length()-6)+String(".")+myf_String.substring(myf_String.length()-6,myf_String.length());
  Serial.println(myf_StringMHz);
  lcd.print(myf_StringMHz);
  lcd.setCursor(12, 0);
  lcd.print("MHz");
*/
  return (f_PFD*N)/OD;
}

int checkError(){
  int err=readReg(0x0);
  String ErrStr="";
  if(err&0b1) ErrStr=String(ErrStr+"TLO ");
  if(err&0b10) ErrStr=String(ErrStr+"THI ");
  if(err&0b100) ErrStr=String(ErrStr+"LOCK ");
  if(err&0b1000) ErrStr=String(ErrStr+"ALCLO ");
  if(err&0b10000) ErrStr=String(ErrStr+"ALCHI ");
  if(err&0b100000) ErrStr=String(ErrStr+"UNLOCK ");
  lcd.setCursor(0, 1);
  lcd.print(ErrStr);
}


void loop() {

/*  while (true){

  lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  lcd.print(millis() / 1000);
  Serial.println("I received: ");
  }*/
  // put your main code here, to run repeatedly:

  
/*  writeToReg(0x2,0x0A);//turn off the output
  writeToReg(0x3,0x0); //Bdiv and RDivHigh
  writeToReg(0x4,0b00010111);//Rdiv low
  writeToReg(0x5,0b00001101);//N counter high
  writeToReg(0x6,0b10101100); //Ncounter low
  
  writeToReg(0x7,0x63);
  delay(5000);
  writeToReg(0x8,0b10111110);
  writeToReg(0x9,0b10011011);
  writeToReg(0xA,0xC0);
  delay(5000);
  writeToReg(0x2,0x08); //trun the output on*/

  printAllReg();
  

  setFrequencyHz(GlobalFrequency);

  bool NewFreq=false;
  unsigned int WaitCounter=0;
  unsigned int CheckErrCounter=0;


  while(true){
    //Serial.print("Reg0 0b");
    //Serial.println(readReg(0x0), BIN);

    
    delay(100);

    if(checkButtonAction()){//check if a button was pressed
      printLCDFreq(GlobalFrequency);
      lcd.setCursor(0, 1);
      lcd.print("Freq. Setup...");
      NewFreq=true;
      WaitCounter=0;
      
      //calculate the position of the dot, where the blinkin curser should appear
      if(GlobalFrequency/1000000000>0){
        lcd.setCursor(4-FreqIncr, 0);
      }
      else{
        lcd.setCursor(3-FreqIncr, 0);
      }

      
      lcd.blink(); 
    }

    //wait a litte untill we register the new frequency in the PLL
    if(NewFreq && WaitCounter>=20){
      setFrequencyHz(GlobalFrequency);
      NewFreq=false;
      lcd.noBlink(); 
    }

    if(CheckErrCounter>20 && !NewFreq){
      checkError();
      CheckErrCounter=0;
    }
    CheckErrCounter++;
    WaitCounter++;
    
    // send data only when you receive data:
    if (Serial.available() > 0) {
            // read the incoming byte:
            long incoming = Serial.parseInt();

            // say what you got:
            Serial.print("I received: ");
            Serial.println(incoming);

            setFrequencyHz(incoming*1000000);

           // printAllReg();
    }
     
  }

}

int  relaypin= 2;
int analogInput = 0;
float vout = 0.0;
float vin = 0.0;
float R1 = 470000.0;
float R2 = 200000.0;
float factor ,factor1;
float vsmooth,alpha,alpha1; 
float inputbuff[20];
unsigned long chargetime;
int value = 0;
static char disp[7];
float vcrit=12.15;

float getvolts(){
   value = analogRead(analogInput);
   vin = float(value)*factor;
   return(vin);
}

float getvoltstatus(){
  int i;
  float maxvolts=0;
  float minvolts=100;
  float midvolts;
  for(i = 1;i<=10;++i){
    delay(1000);
      inputbuff[i]=getvolts();
      //Serial.print(inputbuff[i],2);
     // Serial.print(" ");
      if(inputbuff[i]<minvolts){
        minvolts=inputbuff[i];
      } else if (inputbuff[i]>maxvolts ){
        maxvolts=inputbuff[i];
      }
    }
  midvolts=(maxvolts+minvolts)/2;
  for(i=1;i<=10;++i){
    if(inputbuff[i]>=midvolts){
       vsmooth=alpha*inputbuff[i]+alpha1*vsmooth;
    }
  }
  //Serial.println();
  return(vsmooth);
}
void setup(){
  
   Serial.begin(57600);
   pinMode(analogInput, INPUT);
   pinMode(relaypin, OUTPUT);
   factor=5.0/(1024.0);
   alpha=0.1;
   alpha1=1-alpha;
   factor1=(R2/(R2+R1));
   factor=factor/factor1;
   vsmooth=getvolts();
   Serial.print("Vcrit ");
   Serial.println(vcrit,2);
   Serial.print("Voltage ");
   Serial.println(vsmooth,2);
   chargetime = 600000;
}
void loop(){
   // read the value at analog input
   vin=getvoltstatus();
   
   value = analogRead(analogInput);

   Serial.print("rawval ");
   Serial.println(float(value),2);   
   Serial.print("Vcrit ");
   Serial.println(vcrit,2);
   Serial.print("Voltage ");
   Serial.println(vin,2);
   if (vin<= vcrit){
    // switch on charger of set time
    digitalWrite(relaypin, HIGH);
    Serial.println("Turn relay on");
    Serial.println(String(chargetime));
    delay(chargetime);
    digitalWrite(relaypin, LOW);
    Serial.println("Turn relay off");
    delay(1000);
    vsmooth=getvolts();
   }
   
}

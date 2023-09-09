
#define BUZERPIN 3
#define SWITCHPIN 4
#define DECPIN  5  // low bite
#define divisor 2  //factor for dividing dev times

 //////////////////////////////////////////////////
void pulse(unsigned long dur){
  
  digitalWrite(BUZERPIN, HIGH);
  delay(dur);
  digitalWrite(BUZERPIN, LOW); 
}
///////////////////////////////////////////////////
// Repeated pulse pulse times for a duration of dur 
// interspursed with a 100ms pause
//////////////////////////////////////////////////
void triple(int times,unsigned long dur){
  for(int i=1;i<=times;++i){
  pulse(100);
  delay(dur-100);
  }
}

unsigned int read_dec(){
  int i,v;
  v=0;
  for (i=3;i>= 0; --i) {
    v=v*2+not(digitalRead(DECPIN+i));
    Serial.print("pin  :");
    Serial.print(DECPIN);
    Serial.print(not(digitalRead(DECPIN+i)));
    Serial.print(":");
    Serial.println(v);
  }
  return v;
}
void wait(unsigned long thisend,int pulses,int dur){
  thisend=thisend-pulses*dur;
  while(millis()<thisend){}
    triple(pulses,dur);
}

void timer(){
  
  unsigned long duration[][6]= {{60,210,390,180,60,0} // unicolor
  ,{810,60,300,0,0,0} //01 13.5 min
  ,{780,60,300,0,0,0} //02 13.0 min
  ,{750,60,300,0,0,0} //03 12.5 min
  ,{720,60,300,0,0,0} //04 12.0 min
  ,{690,60,300,0,0,0} //05 11.5 min
  ,{660,60,300,0,0,0} //06 11.0 min
  ,{630,60,300,0,0,0} //07 10.5 min
  ,{600,60,300,0,0,0} //08 10.0 min
  ,{570,60,300,0,0,0} //09 9.5 min
  ,{540,60,300,0,0,0} //10 9.0 min
  ,{510,60,300,0,0,0} //11 8.5 min
  ,{480,60,300,0,0,0} //12 8 min
  ,{450,60,300,0,0,0} //13 7.5 min
  ,{420,60,300,0,0,0} //14 7 min 
  ,{390,60,300,0,0,0} //15 6.5 min
  };
  unsigned long event_start,event_end,agitate_period,run_start,next_Agitate;
  unsigned this_event;
  int program;
  boolean was; 
  agitate_period=30000;

  //Warn that ready to run
  pulse(100);
 

  //////////////////////////////////////////////////
  // Hang until the button pressed
  /////////////////////////////////////////////////
  
  was=digitalRead(SWITCHPIN);
  do{ 
  } while(digitalRead(SWITCHPIN)==was);
  //Alert that buzzer has been pressed before jumping to timer.
  
  program=read_dec(); // read programe
  triple(program+1,1000); // echo back program
  duration[program][0]=duration[program][0]/divisor
  
  //wait 30 seconds with 10 second countdown
  wait(millis()+30000,10,1000);
  this_event=0;
  run_start=millis(); 
  while(duration[program][this_event]>0){
    event_end=run_start+1000*duration[program][this_event];
    next_Agitate=millis()+agitate_period;
    while(event_end>next_Agitate+15000){
      wait(next_Agitate,3,1000); // three pulses just prior to next agitate
      next_Agitate=millis()+agitate_period;
    };
    wait(event_end,10,1000);
    run_start=event_end;
    ++this_event;
    }
 pulse(100);  
}
  
void setup() {
  // start serial port
  
  Serial.begin(9600);
  pinMode(BUZERPIN,OUTPUT);
  pinMode(SWITCHPIN,INPUT_PULLUP);
  pinMode(DECPIN,INPUT_PULLUP);
  pinMode(DECPIN+1,INPUT_PULLUP);
  pinMode(DECPIN+2,INPUT_PULLUP);
  pinMode(DECPIN+3,INPUT_PULLUP);
  timer();
  
}

void loop() {
  // put your main code here, to run repeatedly:
wait(millis()+10000,3,2000);
}

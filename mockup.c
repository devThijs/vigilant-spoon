
#define PORTB <secondsinterrupt>    //portb has just one int

main(){
//initialisation
//set interrupts
//set <accelerometer> interrupt 
//set <ischarging> interrupt (pgood)
//set <pushbutton> interrupt (rotary encoder)
//check if RTC calender interrupt is on

  sei();  //enables global interrupts

  set array for Time information.
  bool active = false;
  bool secondsEnabled = false;
  int count=0;                    //count used to store time since device active for auto turnoff


  ISR<PORTA>(){    //when device is held 45 deg or pushbutton is pressed device turns on
    if(active==false){
    active=true;
    //turn on seconds interrupt in RTC
    //disable turnon interrupts (accelerometerINT, pushbutton)
    //clear turnon flagsflag
    goto: normal routine    } 

    else if(active==true){


    }

  }

  ISR<RTC>{   //Real time counter counting to 5 seconds, or 30 seconds->generates interrupt at overflow which turns the device into sleep mode
    //clear flag
    //goto sleep routine
  }
  
  normal routine
  {

    displayTime();      //display time in format:   HH::MIN::SEC  (seconds only if it's enabled);

    while(count <= 5){
    //do nothing, just idle lol, let the interrupts handle most of the work
    }
    CLI();    //disable global int
    active=false;
    sei();    enable global int
    //goto sleep
  }

sleep routine
{    
    //gotosleep
    //zzzzz
}

configure routine
{

  
}

}
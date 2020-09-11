/*******************MQ-6 LPG Gas Sensor Using Arduino UNO*****************************
This programm will detect LPG gas leakage and turn on a alarm (say RED LED BINLINKING) and turn of MCB (say GREEN LED changes its state)
 written by Kingsuk Majumdar 
 Date: 6th May, 2016
 Programming for : ARDIUNO UNO BOARD
 copyleft (c)^1 2016 Kingsuk Majumdar
 

************************************************************************************/

/************************Hardware Related Macros************************************/
#define           MQ_PIN                        A0    //define which analog input channel you are going to use
#define          ALARM_PIN                       (8)   // digital pin 8 for alarm 
#define          MCB_PIN                        (7)    // digital pin 7 for alarm 
#define         RL_VALUE                     (20)     //define the load resistance on the board, in kilo ohms from data sheet MQ_6
#define         RO_CLEAN_AIR_FACTOR          (9.84)  //RO_CLEAR_AIR_FACTOR=(Sensor resistance in clean air)/RO,
                                                     //which is derived from the chart in datasheet of MQ-6

/***********************Software Related Macros************************************/
#define         CALIBARAION_SAMPLE_TIMES     (50)    //define how many samples you are going to take in the calibration phase
#define         CALIBRATION_SAMPLE_INTERVAL  (500)   //define the time interal(in milisecond) between each samples in the
                                                     //cablibration phase
#define         READ_SAMPLE_INTERVAL         (50)    //define how many samples you are going to take in normal operation
#define         READ_SAMPLE_TIMES            (5)     //define the time interal(in milisecond) between each samples in 
                                                     //normal operation
#define         LPG_ALART                    (100) // if LPG more than 200ppm thn alam and trip circuit will be activated                                                     
                                                     



/*****************************Globals***********************************************/
                                                    
float           Ro           =  10;                 //Ro is initialized to 10 kilo ohms
int         LPGppm           = 0;                   // LPG in ppm

void setup()
{
  pinMode(ALARM_PIN, OUTPUT);
  pinMode(MCB_PIN, OUTPUT);
  Serial.begin(9600);                               //UART setup, baudrate = 9600bps
  Serial.print("Calibrating...\n");                
  Ro = MQCalibration(MQ_PIN);                       //Calibrating the sensor. Please make sure the sensor is in clean air 
                                                    //when you perform the calibration                    
  Serial.print("\nCalibration is done...\n"); 
  Serial.print("Ro=");
  Serial.print(Ro);
  Serial.print("kohm \n");
}

void loop()
{
   Serial.print("LPG:"); 
   LPGppm=MQGetGasPercentage(MQRead(MQ_PIN)/Ro);
   Serial.println(LPGppm );
   Serial.print( "ppm" );
   Serial.print("    \n");   
   delay(200);
    if (LPGppm > LPG_ALART) {
      Serial.print("\n caution ... LPG gas leakage...\n\n");
   Serial.println(LPGppm );
   Serial.print( "ppm" );
   Serial.print("    \n");  
     // Activate digital output pin 7 - MCB the LED will light up
      digitalWrite(MCB_PIN, HIGH);
    // Activate digital output pin 8 - the LED will light up
    digitalWrite(ALARM_PIN, HIGH);
    delay(2000);              // wait for 2 second
    digitalWrite(ALARM_PIN, LOW);
    delay(500);
    digitalWrite(ALARM_PIN, HIGH);
  }
  else {
    // Deactivate digital output pin 8 - the LED will not light up
    digitalWrite(ALARM_PIN, LOW);
  }

}

/****************** MQResistanceCalculation ****************************************
Input:   raw_adc - raw value read from adc, which represents the voltage
Output:  the calculated sensor resistance
Remarks: The sensor and the load resistor forms a voltage divider. Given the voltage
         across the load resistor and its resistance, the resistance of the sensor
         could be derived.
************************************************************************************/ 
float MQResistanceCalculation(int raw_adc)
{
  return ( ((float)RL_VALUE*(1023-raw_adc)/raw_adc)); // Arduino uno analog pin 5V == 1023 by default. ie 10 bit i.e. 2^10-1=1024-1 
}

/***************************** MQCalibration ****************************************
Input:   mq_pin - analog channel
Output:  Ro of the sensor
Remarks: This function assumes that the sensor is in clean air. It use  
         MQResistanceCalculation to calculates the sensor resistance in clean air 
         and then divides it with RO_CLEAN_AIR_FACTOR. RO_CLEAN_AIR_FACTOR is about 
         10, which differs slightly between different sensors.
************************************************************************************/ 
float MQCalibration(int mq_pin)
{
  int i;
  float val=0;

  for (i=0;i<CALIBARAION_SAMPLE_TIMES;i++) {            //take multiple samples
    val += MQResistanceCalculation(analogRead(mq_pin));
    delay(CALIBRATION_SAMPLE_INTERVAL);
  }
  val = val/CALIBARAION_SAMPLE_TIMES;                   //calculate the average value

  val = val/RO_CLEAN_AIR_FACTOR;                        //divided by RO_CLEAN_AIR_FACTOR yields the Ro 
                                                        //according to the chart in the datasheet 

  return val; 
}
/*****************************  MQRead *********************************************
Input:   mq_pin - analog channel
Output:  Rs of the sensor
Remarks: This function use MQResistanceCalculation to caculate the sensor resistenc (Rs).
         The Rs changes as the sensor is in the different consentration of the target
         gas. The sample times and the time interval between samples could be configured
         by changing the definition of the macros.
************************************************************************************/ 
float MQRead(int mq_pin)
{
  int i;
  float rs=0;

  for (i=0;i<READ_SAMPLE_TIMES;i++) {
    rs += MQResistanceCalculation(analogRead(mq_pin));
    delay(READ_SAMPLE_INTERVAL);
  }

  rs = rs/READ_SAMPLE_TIMES;

  return rs;  
}

/*****************************  MQGetGasPercentage **********************************
Input:   rs_ro_ratio - Rs divided by Ro
Output:  ppm of the LPG gas
Remarks: This function passes LPG of MQ-6 as
y=mx+c
where: y=log(Rs/Ro) and x=log(ppm)
ppm=(-(Rs/R0)+1.2953)/0.42817 ; from MQ-6 chat. anlysis here, m=-0.42817 and c=1.2953
************************************************************************************/ 
int MQGetGasPercentage(float rs_ro_ratio)
{
  float LPGppmx;
LPGppmx=(-log(rs_ro_ratio)+1.2953)/0.42817;
LPGppm=pow(10,LPGppmx);
return LPGppm;
}

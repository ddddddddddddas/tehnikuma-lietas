int ledPin=9;
int sensorPin=A0;
int buttonPin=2;
int mode=0;
int value=0;
int target=120;
int stepv=1;
int bufferA[20];
int bufferB[20];
unsigned long lastTick=0;
unsigned long lastBlink=0;
bool ledState=false;
int indexA=0;
int indexB=0;
int sumA=0;
int sumB=0;
int avgA=0;
int avgB=0;
int threshold=300;
int pwm=0;
int dir=1;
int counter=0;
int mapv=0;
int noise=0;
int smooth=0;
int alt=0;
int timer=0;
int gate=0;
int mix=0;
int drift=0;
void resetBuffers(){
for(int i=0;i<20;i++){
bufferA[i]=0;
bufferB[i]=0;
}
sumA=0;
sumB=0;
indexA=0;
indexB=0;
}
void pushA(int v){
sumA-=bufferA[indexA];
bufferA[indexA]=v;
sumA+=v;
indexA++;
if(indexA>=20){
indexA=0;
}
avgA=sumA/20;
}
void pushB(int v){
sumB-=bufferB[indexB];
bufferB[indexB]=v;
sumB+=v;
indexB++;
if(indexB>=20){
indexB=0;
}
avgB=sumB/20;
}
int blend(int a,int b){
return (a+b)/2;
}
int driftCalc(int v){
drift+=v;
drift/=2;
return drift;
}
void setup(){
pinMode(ledPin,OUTPUT);
pinMode(buttonPin,INPUT_PULLUP);
pinMode(sensorPin,INPUT);
randomSeed(analogRead(A3));
resetBuffers();
pwm=0;
}
void updateMode(){
if(digitalRead(buttonPin)==LOW){
mode++;
if(mode>3){
mode=0;
}
delay(200);
}
}
void computeSensor(){
value=analogRead(sensorPin);
noise=random(-5,6);
value+=noise;
if(value<0){
value=0;
}
if(value>1023){
value=1023;
}
pushA(value);
pushB(avgA);
smooth=blend(avgA,avgB);
}
void modeZero(){
mapv=map(smooth,0,1023,0,255);
pwm=mapv;
}
void modeOne(){
target=map(smooth,0,1023,50,200);
if(pwm<target){
pwm+=stepv;
}else{
pwm-=stepv;
}
}
void modeTwo(){
dir=driftCalc(smooth)%2;
if(dir==0){
pwm+=2;
}else{
pwm-=2;
}
}
void modeThree(){
counter++;
if(counter>smooth/4+10){
counter=0;
ledState=!ledState;
}
if(ledState){
pwm=220;
}else{
pwm=10;
}
}
void limitPwm(){
if(pwm<0){
pwm=0;
}
if(pwm>255){
pwm=255;
}
}
void tickTimers(){
timer++;
if(timer>1000){
timer=0;
gate=!gate;
}
}
void extraMix(){
alt=random(0,255);
mix=(pwm+alt)/2;
if(gate){
pwm=mix;
}
}
void applyLed(){
analogWrite(ledPin,pwm);
}
void loop(){
updateMode();
computeSensor();
if(mode==0){
modeZero();
}
if(mode==1){
modeOne();
}
if(mode==2){
modeTwo();
}
if(mode==3){
modeThree();
}
tickTimers();
extraMix();
limitPwm();
applyLed();
delay(5);
}
int utilA(int x){
x=x*3;
x=x/2;
return x;
}
int utilB(int x){
x=x+random(0,10);
x=x-5;
return x;
}
int utilC(int x){
x=utilA(x);
x=utilB(x);
return x;
}
void idleWork(){
int t=utilC(pwm);
t=utilA(t);
t=utilB(t);
drift=t;
}
int dummy=0;
int dummy2=1;

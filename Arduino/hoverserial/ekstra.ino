
dataSpeedStear calcSpeedStear(int speed, int stear, float tol){
  float fart = sqrt(pow(speed,2)+pow(stear,2));
  if (fart < tol){ //For å styre fremover og bakover med en hastighet
      fart = 0;
      stear = 0;
  }
  if (speed < 0){
    fart = -1*fart;
  }
  return {fart, stear};
}

/*
dataSpeedStear getRCSignal(){
  int gassValue = 0;
  int svingValue = 0;
  gassValue = pulseIn(inpGassPin, HIGH) + trimSpeed;
  svingValue = pulseIn(inpSvingPin, HIGH) + trimStear;
  if (gassValue < 1000 || gassValue > 2000 || svingValue < 1000 || svingValue> 2000) return {0,0}; 
  gassValue = constrain(map(gassValue, 1000, 2000, -200, 200), -200, 200);
  svingValue = -constrain(map(svingValue, 1300, 1700, -100, 100), -100, 100);
  return calcSpeedStear(gassValue, svingValue, toleranse);
}
*/

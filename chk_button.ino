void chk_button() {
  
  buttonState=digitalRead(buttonPin);
  
  if (buttonState==HIGH) {
    digitalWrite(RED_LED, HIGH);
  }
  
  else
  {
    digitalWrite(RED_LED, LOW);
  }
  
}

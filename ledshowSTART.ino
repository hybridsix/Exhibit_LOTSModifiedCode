/// This block is a place holder for the startup LED SHOW that will be added later.  The below code is testing jargon, disregard.

void ledshowSTART()
{
 
  int i;
  
  for (i=0; i<5; i++)
  {
  digitalWrite(23, HIGH);   // blink the startup led until startup is complete
  delay(50);               //wait
  digitalWrite(23, LOW);   // blink the startup led until startup is complete
  delay(20);               //wait
  
  digitalWrite(24, HIGH);   // blink the startup led until startup is complete
  delay(50);               //wait
  digitalWrite(24, LOW);   // blink the startup led until startup is complete
  delay(20);               //wait
  
  digitalWrite(25, HIGH);   // blink the startup led until startup is complete
  delay(50);               //wait
  digitalWrite(25, LOW);   // blink the startup led until startup is complete
  delay(20);               //wait
  
  }
}

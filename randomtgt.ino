int randomtgt()
{
  int foundTgt = -1;
  
  if( p1_sequence_ndx >= 2 )
  {
    return -1;
  }

  lastHitTime = millis(); // count any new target select as an implied hit, update timeout reference
  p1_sequence_ndx++; // random numbers are already stored in the target sequence used by BOTH players  
  
  foundTgt = 1;
  p1_tgt_ndx = target_sequence[p1_sequence_ndx];
  photocellPin = p1_tgtSensorPinMap[ p1_tgt_ndx ];
  ledPin = p1_tgtLEDPinMap[ p1_tgt_ndx ];
  digitalWrite(p1_tgtLEDPinMap[ p1_tgt_ndx ], HIGH);
}

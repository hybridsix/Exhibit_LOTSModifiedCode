int p2_randomtgt()
{
  int foundTgt = -1;
  
  if( p2_sequence_ndx >= 2 )
  {
    return -1;
  }
 
  lastHitTime = millis(); // count any new target select as an implied hit, update timeout reference  
  p2_sequence_ndx++; // random numbers are already stored in the target sequence used by BOTH players  
 
  foundTgt = 1;
  p2_tgt_ndx = target_sequence[p2_sequence_ndx];
  p2_photocellPin = p2_tgtSensorPinMap[ p2_tgt_ndx ];
  p2_ledPin = p2_tgtLEDPinMap[ p2_tgt_ndx ];
  digitalWrite(p2_tgtLEDPinMap[ p2_tgt_ndx ], HIGH);
}

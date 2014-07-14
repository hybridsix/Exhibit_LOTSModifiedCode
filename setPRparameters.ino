void setPRparameters(int player, int ndx)
{
   if( PLAYER_ONE == player )
   {
       p1_tgtPRThresholds[ ndx ] = analogRead(p1_tgtSensorPinMap[ ndx ]) + PR_DELTA_THRESHOLD;    
   }
   else if ( PLAYER_TWO == player )
   {
       p2_tgtPRThresholds[ ndx ] = analogRead(p2_tgtSensorPinMap[ ndx ]) + PR_DELTA_THRESHOLD;     
   } 
}


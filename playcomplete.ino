
// Plays a full file from beginning to end with no pause.
// Will allow other activities to interrupt the playback,
// does not wait.

void playcomplete(char *name) 
{
  
  if( BIT_pass == false ) return;
  
  // call our helper to find and play this name
  playfile(name);
  //while (wave.isplaying) 
  //{
  //  // do nothing while its playing
  //}
  //  // now its done playing
  //root.rewind();
}

void playfile(char *name) 
{
  root.rewind();
  
  // see if the wave object is currently doing something
  if (wave.isplaying) 
  {
    // already playing something, so stop it!
    wave.stop(); // stop it
  }
  // look in the root directory and open the file
  if (!f.open(root, name)) 
  {
    putstring("Couldn't open file "); Serial.print(name); return;
  }
  // OK read the file and turn it into a wave object
  if (!wave.create(f)) 
  {
    putstring_nl("Not a valid WAV"); return;
  }
  
  // ok time to play! start playback
  wave.play();
  runxtimes = 0;
}

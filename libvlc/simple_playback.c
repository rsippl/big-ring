 #include <stdio.h>
 #include <stdlib.h>
 #include <vlc/vlc.h>
 
 int main(int argc, char* argv[])
 {
     if (argc < 2) {
	return 1;
     }
     char* filename = argv[1];
     libvlc_instance_t * inst;
     libvlc_media_player_t *mp;
     libvlc_media_t *m;
     
     /* Load the VLC engine */
     inst = libvlc_new (0, NULL);
  
     /* Create a new item */
     m = libvlc_media_new_path (inst, filename);
        
     /* Create a media player playing environement */
     mp = libvlc_media_player_new_from_media (m);
     
     /* No need to keep the media now */
     libvlc_media_release (m);
 
 #if 0
     /* This is a non working code that show how to hooks into a window,
      * if we have a window around */
      libvlc_media_player_set_xdrawable (mp, xdrawable);
     /* or on windows */
      libvlc_media_player_set_hwnd (mp, hwnd);
     /* or on mac os */
      libvlc_media_player_set_nsobject (mp, view);
  #endif
 
	//libvlc_set_fullscreen(mp, 1);
     /* play the media_player */
     libvlc_media_player_play (mp);
    
	libvlc_media_player_set_rate(mp, 0.25);
     sleep (2); /* Let it play a bit */

	float pos = libvlc_media_player_get_position(mp);
	printf("position is :%f\n", pos);
	libvlc_media_player_set_position(mp, .3);

     sleep (10); /* Let it play a bit */
     /* Stop playing */
     libvlc_media_player_stop(mp);

     /* Free the media_player */
     libvlc_media_player_release (mp);
 
     libvlc_release (inst);
 
     return 0;
 }

# RocClientThrottle
A WiFi Rocrail Client throttle 


Improved version of octothrottle. 
Uses the new Rocrail 'lcprops' command to get the Loco list. This method sends each loc as a separate Mqtt message, so does not suffer the Message Limit that Octo Throttle had. 

This code allows for up to 126 locos. Above this the code gets unstable.



# solarize
faux solarization functionality for images

Works by generating a histogram, smoothing it, and applying the result as a
function on each pixel.

To compile: ```make```

To run: ```./solarize [-v] [-g] [-i] [-t THRESHOLD] [-w WINDOW] FILE...```

* ```-v```: Currently does nothing, should probably be verbosity control.
* ```-g```: Converts color image to grayscale.
* ```-i```: Invert the final image. Sometimes has better results? Play with it.
* ```-t THRESHOLD```: Sets the threshold pixel at which the image's original
                      value will be used (i.e. the curve will be bypassed for
                      very black pixels). This can prevent large swathes of
                      black in some images. Defaults to 0.
* ```-w WINDOW```: Sets the size of the moving average window. A larger
                   window results in less "rough" images (less starkly
                   saturated). Somewhere around 20-40 seems to be the sweet
                   spot.
* You can use multiple files. It will spawn a thread per image, hopefully
  taking advantage of parallelism.

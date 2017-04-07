# Cilk Plus tutorial

This folder is dedicated to set our Cilk Plus environment and run our first
parallel programs. In the [Tutorial](Tutorial.pdf) files you can find the
details of Cilk Plus installation and the basic configuration of Cilk Plus to
run our paralle code. The examples used in the tutorial are those:

* *fibonacci.c*: This example shows the usage of **cilk_spawn** and **cilk_sync** keywords.
* *example_for.c*: This example shows a simple usage of the **cilk_for** keyword. Besides, this example allow us to see the limits of the *grainsize* used by **cilk_for**
* *image-processing.c*: This example shows a more complex example of **cilk_for**, by processing an image in parallel. The image is represented as a 2D-matrix. Each time that the function *process_image* is called, the pixels of the image change. The new value of each pixel is defined as the average of its neighbors. To visualize the image, use the Python script [print_image.py](print_image.py).

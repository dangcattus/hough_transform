# Generalized Hough Transform

This is an implementation of the Generalized Hough Transform used for a Model-Based
Computer Vision course at Simon Fraser University.

## To Build

This project was built in X Code for Mac OSX using C++ and Open CV 3.0.

To run the project, run the project in XCode.

## Running Hough Transform

### Edge Detection

The program will create a Generalized Hough Transform on the template image. First, the template image is converted to grayscale. The template image is blurred. The blurImage function in hough.cpp allows the user to choose which function to use to blue the image. Canny edge detection is applied on the blurred template image.

### Finding Image Gradient

From the Canny template image, we create 2 matrices for the x and y gradient information using the Sobel edge detector. We then convert the gradient to be between -90 and 90 degrees, and return the gradient matrix.

To find the magnitude of the image, I was not able to extract the data from the canny edge operator in opencv, and therefore after the edge detection, I used the Sobel operator and calculated the gradient using atan2. After this, I discretized it into the bins for values -pi/2 to pi/2

### Finding Image Centroid

We then find the template image centroid coordinates by summing all the (i,j) coordinates of the edges, as well as summing all edge pixels. The centroid is then given by

```
( ( iCount / pointCount), (jCount / pointCount))
```

where iCount and jCount are the sum of all points in row i and column j respectfully, and pointCount is the sum of all points.

### Constructing the R-Table

In the R-Table implementation, I used 18 bins for the angles as I found it was able to discretize the angles without being too precise or too vague. I experimented with 36 bins, but found that some of the results were less precise. Although the exact centroid of the object was rarely picked, the objects detected were within the contours.

The algorithm for constructing the R-Table is listed on the wikipedia page below.

### Locating the object

To locate the object in the main image, we convert the image to grayscale, blur the image and apply the Canny edge detection algorithm.
We then accumulator array which uses the R-Table to compute candidate reference point. The object is likely to be located where the most points in the accumulator table lie.

### Test images

I have included some test images.
The first, PA2-testimages/template_dog.png, is the template image we will be locating.
PA2-testimages/animals2.jpg is the image in which we will locate the test images.
These can be changed in main.cpp, where imObject is the template and image is the main image.

## Known Errors

In my project, I was unable to locate images that were scaled or rotated. This version
of the Hough transform works for detecting images.

### References

Generalized Hough transform
https://en.wikipedia.org/wiki/Generalised_Hough_transform#Object_localization

Centroids
http://aishack.in/tutorials/image-moments/

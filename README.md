# Seamless-Image-Cloning
Seamless Cloning implemented in OpenFrameworks with C++, details covered in the paper "Poisson Image Editing" by Patrick Pérez, Michel Gangnet, and Andrew Blake. 

This program takes in three images and produces a new image, these images are: Source image, Mask image, and Destination image. Using Mask image, we take a portion out of the source image then move it on the Destination image while maintaining the gradient of either or a mix of both depending on our desired result and parameters.

Images and their results can be seen in the data folder which also holds our input images.

Paper: Patrick Pérez, Michel Gangnet, and Andrew Blake. 2023. Poisson Image Editing. Seminal Graphics Papers: Pushing the Boundaries, Volume 2 (1st ed.). Association for Computing Machinery, New York, NY, USA, Article 60, 577–582. https://doi.org/10.1145/3596711.3596772 

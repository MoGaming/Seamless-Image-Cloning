# Seamless-Image-Cloning
This project implements Seamless Cloning in C++ using OpenFrameworks, based on the seminal paper "Poisson Image Editing" by Patrick Pérez, Michel Gangnet, and Andrew Blake (2023).

## Program Use
Given three input images:
* **Source image** – the image to copy from,
* **Mask image** – a binary image defining the region to clone from the source image,
* **Destination image** – the target image to paste the source image into.

This program blends the source region into the destination image seamlessly, preserving gradient information from the source image (or both, using mixed gradients). This results in a more natural-looking composite and source image boundary, as shown in the original paper.

Example results and input images are located in the `data/` folder. 

### Alpha Images
Source images with a wide range of alpha (transparency) values can produce confusing or poorly blend results. This happens because the alpha channel gradients are weaker or more subtle compared to color channels, causing the solver to converge faster which sometimes results in less accurate or visually smooth transitions. As a result, artifacts or jagged edges may appear in areas with complex alpha variation, and we can even get results that appear to be direct pasting when our source image's alpha channel is constant and transparent.

## Program Origin

This project was created as a bonus assignment for a graduate-level computer graphics course. Since not everyone in the course had a strong background in math or computer science, I:

* Wrote my own linear solver from scratch,
* Avoided using off-the-shelf libraries,
* Heavily commented the code for clarity and learning purposes.

My goal was to understand the inner workings of Poisson Image Editing, Seamless Image Cloning, and to be able to teach others it by looking just at my codebase and report, especially the construction and solution of the underlying sparse linear system.

## Performance Notes

* The solver uses a basic and understandable Gauss-Seidel iterative method, written from scratch for educational purposes.
* Many simple optimizations (e.g., region-only iteration, pixel cache reuse, multigrid, etc.) are not yet implemented, feel free to explore and improve performance.
* You are welcome to replace the solver with a faster or more advanced one if desired, either your own or an off-the-shelf library.

## Reference Paper 

Paper: Patrick Pérez, Michel Gangnet, and Andrew Blake. 2023. Poisson Image Editing. Seminal Graphics Papers: Pushing the Boundaries, Volume 2 (1st ed.). Association for Computing Machinery, New York, NY, USA, Article 60, 577–582. https://doi.org/10.1145/3596711.3596772 

Additionally, contact me for the written report as it covers all the necessary equations, code implementation details, failure cases, and success cases. I've chosen not to include it in this public repository out of respect for the course instructor, in case the paper or assignment is reused in future coursework.



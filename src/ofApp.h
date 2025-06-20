#pragma once
#include "ofMain.h"

class ofApp : public ofBaseApp
{
	public:
		void setup(); // runs when program starts
		void draw(); // runs every draw frame

		// Function to perform seamless cloning.
		// Input:
		//		src  - the source image from we get the details for the cloning.
		//		mask - a binary mask (white = inside cloning region) telling us the region in src to clone.
		//		dst  - the destination image where the cloning is applied to.
		// Output:
		//		an ofImage containing the destination with seamlessly cloned content.
		ofImage seamlessClone(const ofImage& src, const ofImage& mask, const ofImage& dst);

		// Solver parameters (500 provides a fast and easy estimate, if it looks bad from there then we can increase it by a factor and test it again then repeat until a good output).
		const int maxIterations = 500;    // Maximum iterations for the iterative solver.
		const double tolerance = 1e-3;     // Convergence tolerance to stop iterative solver early. I recommended a maximum of ~1e-3.
		const double relaxationFactor = 1; // Optimal for most cases (tune between 1.25-1.95), over 2 could cause divergence, set to 1 to disable this
		const bool guessDestination = true; // true = initial guess of destination image, false = initial guess of source image
		const bool mixedGradients = false; // true = uses mixed gradients, false = does not use mixed gradients, I recommend true in cases where holes are in mask image like paper's fig 6, or lots of alpha like paper's fig 7.
		
		// Input images and result image.
		ofImage sourceImage;
		ofImage maskImage;
		ofImage destinationImage; 
		ofImage resultImage;
};
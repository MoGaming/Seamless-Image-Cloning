#include "ofApp.h"

////////////////        SAVE FILE        ////////////////

// Function to generate a timestamp string (YYYY-MM-DD-HH-MM-SS). Used to create uniquely named saved files.
string getCurrentTimestamp()
{
	auto now = chrono::system_clock::now();
	time_t now_c = chrono::system_clock::to_time_t(now);
	tm local_tm = *localtime(&now_c);

	ostringstream oss;
	oss << put_time(&local_tm, "%Y-%m-%d-%H-%M-%S");

	return oss.str();
}

// Function to save the final output image with a timestamped filename.
void saveImage(ofImage& myImg)
{
	cout << (myImg.save("Image-" + getCurrentTimestamp() + ".png", ofImageQualityType::OF_IMAGE_QUALITY_BEST) ? "Image-" + getCurrentTimestamp() + " saved" : "image not saved") << endl;
}

////////////////        UTILITIES        ////////////////

// Helper inline function: Convert 2D coordinates (x, y) to a 1D index. (This is inline because it reduces function call overhead, which could slow down the code by a large margin if we have a large maxIterations value.)
inline int idx(int x, int y, int width)
{
	return y * width + x;
}

////////////////      POISSON EDITING     ////////////////

//  seamlessClone function:
//      This implementation sets up the discrete system from equation (7) modified with equation (11):
//          |N_p| f_p - \sum_{q \in N_p \cap \Omega}{f_q} = \sum_{q \in N_p \cap \partial\Omega} f*_q + sum_{q \in N_p}{(g_p - g_q)}
//      for every pixel p in \Omega.
//  We use a Gauss-Seidel iterative solver to approximate the solution.
//      Example of this solver can be seen at this page: https://en.wikipedia.org/wiki/Gauss%E2%80%93Seidel_method#An_example_using_Python_and_NumPy and https://en.wikipedia.org/wiki/Successive_over-relaxation#Algorithm
ofImage ofApp::seamlessClone(const ofImage& src, const ofImage& mask, const ofImage& dst)
{
	// Get Dimensions of image, we assume all three input images are the same size.
	int width = dst.getWidth();
	int height = dst.getHeight();

	// Get pixel data from inputted images.
	ofPixels srcPixels = src.getPixels();
	ofPixels maskPixels = mask.getPixels();
	ofPixels dstPixels = dst.getPixels();

	ofPixels resultPixels = dstPixels; // Create a copy of destination pixels to hold our final result.

	vector<bool> inside(width * height, false); // Boolean array to mark pixels inside the cloning region \Omega.
	int countInside = 0; // Log how many pixels are inside the cloning region.

	// Set the inside of \Omega using a threshold 127 for each pixel in the mask.
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
		{
			int i = idx(x, y, width);
			if ((int)maskPixels[i] > 127) // Since it is a grayscale image, we only check the first channel of each pixel.
			{
				inside[i] = true;
				countInside++;
			}
		}

	ofLogNotice() << "Cloning region pixel count: " << countInside; // Allows us to know ahead of time what the runtime might be, larger pixel count = longer runtime.

	// Allocate arrays to store the current estimate f (the unknown function) for each channel, double is needed for percision over multiple iterations.
	vector<double> fR(width * height, 0.0);
	vector<double> fG(width * height, 0.0);
	vector<double> fB(width * height, 0.0);
	vector<double> fA(width * height, 0.0); // Alpha channel for transparency.

	// Initialize f values for pixels in \Omega. We use either destination or source pixel value as the initial guess.
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
		{
			int i = idx(x, y, width);
			if (inside[i])
			{
				// Getting color from dstPixels blends very well, but srcPixels makes the image look very accurate to the input in terms of color, both produce good results
				ofColor dCol;
				if (guessDestination)
					dCol = dstPixels.getColor(x, y);
				else
					dCol = srcPixels.getColor(x, y);
				fR[i] = dCol.r;
				fG[i] = dCol.g;
				fB[i] = dCol.b;
				fA[i] = dCol.a;
			}
		}

	// Offsets for the 4-connected neighborhood (left, right, up, down) used as described in section 2.
	const int dx[4] = { -1, 1,  0, 0 };
	const int dy[4] = { 0, 0, -1, 1 };

	// Iteratively solve the linear system using Gauss-Seidel.
	for (int iteration = 0; iteration < maxIterations; iteration++)
	{
		double maxDiff = 0.0; // maxDiff represents the largest pixel change in this iteration.
		// For each pixel in the image.
		for (int y = 0; y < height; y++)
			for (int x = 0; x < width; x++)
			{
				int i = idx(x, y, width);
				if (!inside[i]) continue; // Process only if this pixel is inside \Omega.

				ofColor srcColP = srcPixels.getColor(x, y); // Get source color for pixel p.
				ofColor dstColP = dstPixels.getColor(x, y); // Get dst color for pixel p.
				double sumR = 0.0;
				double sumG = 0.0;
				double sumB = 0.0;
				double sumA = 0.0;
				int nCount = 0; // Count of valid neighbors used for averaging.

				// Loop over 4-connected neighbors.
				for (int n = 0; n < 4; n++)
				{
					int nx = x + dx[n];
					int ny = y + dy[n];
					// Skip out-of-bound neighbor positions.
					if (nx < 0 || nx >= width || ny < 0 || ny >= height) continue;
					int j = idx(nx, ny, width);
					nCount++;

					// Difference of source and destination gradients (g_p - g_q) and (f*_p - f*_q), this is the guidance field (equation 11).
					ofColor srcColNeighbor = srcPixels.getColor(nx, ny);
					ofColor dstColNeighbor = dstPixels.getColor(nx, ny);

					// Compute both source and destination gradients
					double srcGuidanceR = (double)srcColP.r - (double)srcColNeighbor.r;
					double srcGuidanceG = (double)srcColP.g - (double)srcColNeighbor.g;
					double srcGuidanceB = (double)srcColP.b - (double)srcColNeighbor.b;

					double dstGuidanceR = (double)dstColP.r - (double)dstColNeighbor.r;
					double dstGuidanceG = (double)dstColP.g - (double)dstColNeighbor.g;
					double dstGuidanceB = (double)dstColP.b - (double)dstColNeighbor.b;

					// Apply Mixed Gradients, if set to true, select the stronger gradient between source and destination (equation 12).
					double guidanceR = (mixedGradients && abs(dstGuidanceR) > abs(srcGuidanceR)) ? dstGuidanceR : srcGuidanceR;
					double guidanceG = (mixedGradients && abs(dstGuidanceG) > abs(srcGuidanceG)) ? dstGuidanceG : srcGuidanceG;
					double guidanceB = (mixedGradients && abs(dstGuidanceB) > abs(srcGuidanceB)) ? dstGuidanceB : srcGuidanceB;

					// Always use regular poisson blending for alpha channel and the regular guidance value, no Mixed Gradients applied to alpha channel since that can cause artifacting and jagged edges.
					double guidanceA = (double)srcColP.a - (double)srcColNeighbor.a;

					// If neighbor is inside \Omega, use the current estimate f_j, otherwise, use the destination pixel value (this is the boundary condition).
					if (inside[j])
					{
						sumR += fR[j];
						sumG += fG[j];
						sumB += fB[j];
						sumA += fA[j];
					}
					else
					{
						sumR += dstColNeighbor.r;
						sumG += dstColNeighbor.g;
						sumB += dstColNeighbor.b;
						sumA += dstColNeighbor.a;
					}
					// Add the guidance value for this neighbor.
					sumR += guidanceR;
					sumG += guidanceG;
					sumB += guidanceB;
					sumA += guidanceA;
				}
				// Calculate the new value for pixel p (for each channel).
				double newR = sumR / nCount;
				double newG = sumG / nCount;
				double newB = sumB / nCount;
				double newA = sumA / nCount;

				// Update max difference for convergence check.
				maxDiff = max(maxDiff, abs(newR - fR[i]));
				maxDiff = max(maxDiff, abs(newG - fG[i]));
				maxDiff = max(maxDiff, abs(newB - fB[i]));
				maxDiff = max(maxDiff, abs(newA - fA[i]));

				// Update the solution for this pixel using relaxationFactor.
				fR[i] = (1.0 - relaxationFactor) * fR[i] + relaxationFactor * newR;
				fG[i] = (1.0 - relaxationFactor) * fG[i] + relaxationFactor * newG;
				fB[i] = (1.0 - relaxationFactor) * fB[i] + relaxationFactor * newB;
				fA[i] = (1.0 - relaxationFactor) * fA[i] + relaxationFactor * newA;
			}

		// Log progress every 10 iterations so we know where we are at.
		if (iteration % 10 == 0)
			ofLogNotice() << "Iteration " << iteration << "/" << maxIterations << ", maxDiff: " << maxDiff;

		// Check if the solution has converged:
		//      If maxDiff is smaller than tolerance, it means the updates are negligible so further iterations would not significantly improve the result.
		//      Which lets us stop early without sacrificing quality which makes it faster by not wasting more iterations.
		if (maxDiff < tolerance)
		{
			ofLogNotice() << "Converged after " << iteration << " iterations.";
			break;
		}
	}

	// Write the computed values back to the result pixels from the cloning region.
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
		{
			int i = idx(x, y, width);
			if (inside[i])
				resultPixels.setColor(x, y, ofColor(ofClamp(fR[i], 0, 255), ofClamp(fG[i], 0, 255), ofClamp(fB[i], 0, 255), ofClamp(fA[i], 0, 255)));
		}

	// Create an image from the result pixels.
	ofImage result;
	result.setFromPixels(resultPixels);
	return result;
}

////////////////      OPENFRAMEWORKS      ////////////////

void ofApp::setup()
{
	ofBackground(255, 0, 255); // for image constrast
	ofSetWindowTitle("Poisson Image Editing: Seamless Cloning"); // set title of window

	// Log startup and date for time comparison
	ofLogNotice() << "Setup called. @" << getCurrentTimestamp();

	// Load the images
	bool srcLoaded = sourceImage.load("source.png"); // the image we will be using in combination with the mask to clone onto the destination image
	bool maskLoaded = maskImage.load("mask.png");  // A binary mask image of white and black pixels, white tells us what to keep and black is disregarded.
	bool dstLoaded = destinationImage.load("destination.png"); // the image we are cloning the source onto.

	if (!srcLoaded || !dstLoaded || !maskLoaded) // encase I didn't put in the correct images
	{
		ofLogError() << "One or more images failed to load. Check file paths.";
		return;
	}

	// Ensure that we are not keeping any colour in the mask image.
	maskImage.setImageType(OF_IMAGE_GRAYSCALE);

	// Log image dimensions.
	ofLogNotice() << "Source: " << sourceImage.getWidth() << " x " << sourceImage.getHeight();
	ofLogNotice() << "Destination: " << destinationImage.getWidth() << " x " << destinationImage.getHeight();
	ofLogNotice() << "Mask: " << maskImage.getWidth() << " x " << maskImage.getHeight();

	// Perform seamless cloning.
	resultImage = seamlessClone(sourceImage, maskImage, destinationImage);

	// Log end and date for time comparison
	ofLogNotice() << "Ended, seamless cloning completed. @" << getCurrentTimestamp();
	saveImage(resultImage);
}

void ofApp::draw()
{
	// Draw the resulting image.
	resultImage.draw(0, 0);
}
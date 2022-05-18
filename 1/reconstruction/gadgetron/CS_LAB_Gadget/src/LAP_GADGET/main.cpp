#include <cstring>
#include <iostream>
#include <fstream>
#include <armadillo>

#include "lap3d.h"
#include "shiftengine3d.h"

using namespace arma;

float CG_PSNR3D(CubeType Base_image, CubeType Recon_image);

int main(int argc, char *argv[])
{
	std::string firstImagePath;
	std::string secondImagePath;
	std::string resultImagePath;

	SizeType rows;
	SizeType cols;
	SizeType slices;

	ColType i1_v;
	ColType i2_v;

	if (argc < 2) {
		std::cerr << "Usage: " << std::endl;
		std::cerr << std::endl;
		std::cerr << "  " << argv[0] << " <path-to-image1> <path-to-image2> <path-to-result-image> <image-rows> <image-columns> <image-slices>" << std::endl;
		std::cerr << "  " << argv[0] << " <pointer-to-image1> <pointer-to-image2> <pointer-to-result-image>, <image-rows> <image-columns> <image-slices>" << std::endl;
		std::cerr << std::endl;
		std::cerr << "Run '" << argv[0] << " --help' for more information" << std::endl;
		std::cerr << std::endl;

		return EXIT_FAILURE;
	}

	// Output help string and exit (otherwise segmentation fault!)
	if (strcmp(argv[1], "--help") == 0) {
		std::cout << "There are no more information implemented yet." << std::endl;

		return EXIT_FAILURE;
	}

	firstImagePath = argv[1];
	secondImagePath = argv[2];
	resultImagePath = argv[3];

	rows = strtol(argv[4], NULL, 10);
	cols = strtol(argv[5], NULL, 10);
	slices = strtol(argv[6], NULL, 10);

	//Load iamges (saved as raw ascii)
	i1_v.load(firstImagePath, raw_binary);
	i2_v.load(secondImagePath, raw_binary);

	//Shape them to 3D pictures with correct dimensions
	CubeType i1(i1_v.memptr(), rows, cols, slices);
	CubeType i2(i2_v.memptr(), rows, cols, slices);

	i1.subcube(span(0,3),span(0,3),span(0,3)).print("i1");

	//Construct two 3D Images
	//Construct the LocalAllpass Algorithm Object with Level min and max
	Gadgetron::LAP3D mLAP3D(i1, i2, 0, 4);

	field<CubeType> flow_estimation = mLAP3D.exec();

	//Shift first image according to estimated optical flow
	Gadgetron::ShiftEngine3D shifter(i1, flow_estimation(0), flow_estimation(1), flow_estimation(2));
	CubeType i_hat_fast = shifter.execCubicShift();
	//Save result image
	Col<PixelType> i_hat_fast_v = vectorise(i_hat_fast);
	i_hat_fast_v.save(resultImagePath, raw_ascii);

	std::cout << "PSNR_uB_Fast: " << CG_PSNR3D(i2, i_hat_fast) << std::endl;

	return EXIT_SUCCESS;
}

float CG_PSNR3D(CubeType Base_image, CubeType Recon_image)
{
	int m = Recon_image.n_rows;
	int n = Recon_image.n_cols;
	int p = Recon_image.n_slices;

	float Max_I = arma::max(arma::abs(vectorise(Base_image)));

	float MSE = accu(arma::pow(arma::abs(Base_image-Recon_image),2)) / (n*m*p);

	float PSNR = 10 * std::log10(std::pow(Max_I, 2) / MSE);

	return PSNR;
}

/*
file name	: 	CS_FOCUSS_2D.cpp
author		: 	Martin Schwartz	(martin.schwartz@med.uni-tuebingen.de)
version		: 	1.1
date		: 	13.02.2015
description	: 	implementation of the class "CS_FOCUSS_2D" (file CS_FOCUSS.h)
notes		:	no Total Variation and ESPReSSo constraint
*/

#include "CS_FOCUSS.h"

using namespace Gadgetron;

//--------------------------------------------------------------------------
//------------- process - CG-FOCUSS with additional constraints ------------
//--------------------------------------------------------------------------
int CS_FOCUSS_2D::process(GadgetContainerMessage<ISMRMRD::ImageHeader> *m1, GadgetContainerMessage<hoNDArray<std::complex<float> > > *m2)
{
	//------------------------------------------------------------------------
	//------------------------------ initial ---------------------------------
	//------------------------------------------------------------------------
	//--- set variables - store incoming data - permute incoming data --------
	//------------------------------------------------------------------------
	GINFO("Starting FOCUSS reconstruction\n");

	// init member values based on header information
	fInitVal(m1);

	// declare recon output
	hoNDArray<std::complex<float> > hacfOutput;

	// FOCUSS reconstruction - this function is also called by the Matlab implementation
	fRecon(*m2->getObjectPtr(), hacfOutput);

	// new GadgetContainer
	GadgetContainerMessage<hoNDArray<std::complex<float> > > *cm2 = new GadgetContainerMessage<hoNDArray<std::complex<float> > >();

	// concatenate data with header
	m1->cont(cm2);

	// create output
	try {
		cm2->getObjectPtr()->create(&vtDim_);
	} catch (std::runtime_error &err) {
		GEXCEPTION(err,"Unable to allocate new image array\n");
		m1->release();

		return GADGET_FAIL;
	}

	// copy data
	memcpy(cm2->getObjectPtr()->get_data_ptr(), hacfOutput.begin(), sizeof(std::complex<float>)*hacfOutput.get_number_of_elements());

	// pass data on stream or to control object
	if (!bControl_) {
		//Now pass on image
		if (this->next()->putq(m1) < 0) {
			return GADGET_FAIL;
		}
	}

	return GADGET_OK;
}


int CS_FOCUSS_2D::fRecon(hoNDArray<std::complex<float> > &hacfInput, hoNDArray<std::complex<float> > &hacfRecon)
{
	// input dimensions
	vtDim_ = *hacfInput.get_dimensions();

	// number of channels
	iNChannels_ = (int)vtDim_[2];

	// if Matlab is used, initialize singleton variables
	/*if (bMatlab_) {
		for (int iI = 0; iI < 20; iI++) {
			GlobalVar::instance()->vbStatPrinc_.push_back(false);
			GlobalVar::instance()->vfPrincipleComponents_.push_back(new hoNDArray<std::complex<float> > ());
		}
	}*/

	// const complex values
	const std::complex<float> cfZero(0.0);
	const std::complex<float> cfOne(1.0);

	// store incoming data in array
	hoNDArray<std::complex<float> > hacfKSpace = hacfInput;

	// permute kSpace: x-y-c -> y-x-c
	std::vector<size_t> vtDimOrder; vtDimOrder.push_back(1); vtDimOrder.push_back(0); vtDimOrder.push_back(2);
	hacfKSpace = permute(hacfKSpace, vtDimOrder);

	// update dim_ vector
	vtDim_.clear();
	vtDim_ = *hacfKSpace.get_dimensions();

	//------------------------------------------------------------------------
	//-------------------------- sampling mask -------------------------------
	//------------------------------------------------------------------------
	hoNDArray<std::complex<float> > hacfFullMask(hacfKSpace.get_dimensions());
	hoNDArray<bool> habFullMask(hacfKSpace.get_dimensions());

	for (size_t i = 0; i < hacfKSpace.get_number_of_elements(); i++) {
		if (hacfKSpace.at(i) != cfZero) {
			hacfFullMask.at(i) = cfOne;
			habFullMask.at(i) = true;
		} else {
			hacfFullMask.at(i) = cfZero;
			habFullMask.at(i) = false;
		}
	}

	//-------------------------------------------------------------------------
	//---------------- iFFT x direction - x ky kz ^= v (n�) -------------------
	//-------------------------------------------------------------------------
	if (Transform_fftBA_->get_active()) {
		GINFO("FFT in read direction..\n");
		Transform_fftBA_->FTransform(hacfKSpace);
	}

	hoNDArray<std::complex<float> > hacfWWindowed = hacfKSpace;

	//------------------------------------------------------------------------
	//---------------------------- windowing ---------------------------------
	//------------------------------------------------------------------------
	GINFO("get calib size..\n");
	fGetCalibrationSize(habFullMask);
	fWindowing(hacfWWindowed);

	/*-------------------------------------------------------------------------
	------------------------- initial estimate --------------------------------
	--------------------------------------------------------------------------*/
	GINFO("Prepare initial estimate..\n");

	// W in x-y-z-cha --> new base
	Transform_KernelTransform_->FTransform(hacfWWindowed);

	// W = abs(FTrafo(W)).^p - p = .5
	fAbsPow(hacfWWindowed, fP_);

	// calculate energy and divide windowed image for initial estimate
	hoNDArray<std::complex<float> > hacfTotEnergy(hacfWWindowed.get_dimensions());
	for (int iCha = 0; iCha < iNChannels_; iCha++) {
		size_t tOffset = hacfWWindowed.get_size(0)*hacfWWindowed.get_size(1)*iCha;
		hoNDArray<std::complex<float> > hacfEnergyPerChannel(hacfWWindowed.get_size(0), hacfWWindowed.get_size(1), hacfWWindowed.get_data_ptr() + tOffset, false);
		float channel_max_energy = fCalcEnergy(hacfEnergyPerChannel);

		GDEBUG("energy in channel[%i]: %e..\n", iCha, channel_max_energy);

		// fill channel
		#pragma omp parallel for
		for (size_t i = 0; i < hacfEnergyPerChannel.get_number_of_elements(); i++) {
			hacfTotEnergy.at(i+tOffset) = std::complex<float>(channel_max_energy);
			hacfWWindowed.at(i+tOffset) /= std::complex<float>(channel_max_energy);
		}
	}

	/*-------------------------------------------------------------------------
	--------------------- iterative calculation -------------------------------
	--------------------------------------------------------------------------*/
	// initial estimates for CG - all zero (except g_old)
	hoNDArray<std::complex<float> > hacfQ(hacfWWindowed.get_dimensions());
	hacfQ.fill(cfZero);

	hoNDArray<std::complex<float> > hacfRho = hacfQ, hacfG_old = hacfQ, hacfD = hacfQ, hacfRho_fft = hacfQ, hacfE = hacfQ, hacfG = hacfQ, hacfE_ifft = hacfQ, hacfBeta = hacfQ, hacfZ = hacfQ, hacfAlpha = hacfQ, hacfGradient_ESPReSSo = hacfQ;

	// outer loop for FOCUSS
	for (int iOuter = 0; iOuter < GlobalVar::instance()->iNOuter_; iOuter++) {
		GDEBUG("FOCUSS loop: %i\n", iOuter);

		// reset initial values
		hacfRho.fill(cfZero);
		hacfD.fill(cfZero);
		hacfQ.fill(cfZero);
		hacfG_old.fill(std::complex<float>(1.0, 1.0));

		// inner loop for CG
		for (int iInner = 0; iInner < GlobalVar::instance()->iNInner_; iInner++) {
			try {
				GDEBUG("CG Loop: %i\n", iInner);

				// rho: x-y ---> x-ky
				hacfRho_fft = hacfRho;
				Transform_KernelTransform_->BTransform(hacfRho_fft);

				// e = v - Phi*F*rho - e: x-ky
				multiply(hacfFullMask, hacfRho_fft, hacfE);
				subtract(hacfKSpace, hacfE, hacfE);

				//l2 norm calculation - check epsilon
				std::vector<float> vfVec;
				for (int iCha = 0; iCha < iNChannels_; iCha++) {
					size_t tOffset = hacfE.get_size(0)*hacfE.get_size(1)*iCha;
					hoNDArray<std::complex<float> > eCha(hacfE.get_size(0), hacfE.get_size(1), hacfE.get_data_ptr() + tOffset, false);
					vfVec.push_back(abs(dot(&eCha, &eCha)));
					vfVec[iCha] = std::sqrt(vfVec[iCha]);

					GDEBUG("||e|| ch. %i  =  %e\n", iCha, vfVec[iCha]);
				}

				// how many channels are converged
				unsigned int iNom = 0;
				for (size_t iI = 0; iI < vfVec.size(); iI++) {
					if (vfVec[iI] > fEpsilon_) {
						iNom++;
					}
				}

				GDEBUG("number of non converged channels - %i\n", iNom);

				// if all channels converged -> stop calculation
				if (iNom == 0) {
					break;
				}

				//------------------------------------------------------------------------
				//---------------------------- constraints -------------------------------
				//------------------------------------------------------------------------

				// emphasize conjugate similarity - not used in 2D
				hoNDArray<std::complex<float> > hacfGradient_ESPReSSo = hacfRho;
				hacfGradient_ESPReSSo.fill(0.0);

				//----------------- gradient -------------------------
				// e: x-ky --> x-y
				hacfE_ifft = hacfE;
				Transform_KernelTransform_->FTransform(hacfE_ifft);

				// G = -conj(W).*IFFT(e)+Lambda.*Q
				fCalcGradient(hacfWWindowed, hacfE_ifft, GlobalVar::instance()->cfLambda_, hacfQ, GlobalVar::instance()->cfLambdaESPReSSo_, hacfGradient_ESPReSSo, hacfG);

				//------------------- cg beta - Polak-Ribiere -------------------------------
				#pragma omp parallel for
				for (int iCha = 0; iCha < iNChannels_; iCha++) {
					// fill sub array with data from higher order data array
					size_t tOffset = hacfG.get_size(0)*hacfG.get_size(1)*iCha;
					hoNDArray<std::complex<float> > hacfSubArrayG_old(hacfG.get_size(0), hacfG.get_size(1), hacfG_old.get_data_ptr()+ tOffset, false);
					hoNDArray<std::complex<float> > hacfSubArrayG(hacfG.get_size(0), hacfG.get_size(1), hacfG.get_data_ptr()+ tOffset, false);
					float fBetaCha = 0.0;
					float fNumerator = 0.0;
					float fDenominator = 0.0;

					// calculate nominator
					for (size_t iI = 0; iI < hacfSubArrayG.get_number_of_elements(); iI++) {
						fNumerator += std::pow(hacfSubArrayG.at(iI).real(), 2) + std::pow(hacfSubArrayG.at(iI).imag(), 2);		// = std::conj(hacfSubArrayG.at(iI))*hacfSubArrayG.at(iI)
					}

					// calculate denominator
					for (size_t iI = 0; iI < hacfSubArrayG.get_number_of_elements(); iI++) {
						fDenominator += std::pow(hacfSubArrayG_old.at(iI).real(), 2) + std::pow(hacfSubArrayG_old.at(iI).imag(), 2);		// = std::conj(hacfSubArrayG_old.at(iI))*hacfSubArrayG_old.at(iI)
					}

					if (abs(fDenominator) != 0) {
						fBetaCha = fNumerator / fDenominator;
					}

					// fill part of the 2D array
					#pragma omp parallel for
					for (size_t lI = 0; lI < hacfSubArrayG.get_number_of_elements(); lI++) {
						hacfBeta.at(lI+tOffset) = std::complex<float>(fBetaCha);
					}
				}
				//--------------------------------------------------------------------------

				// d = beta.*d - G and g_old = G
				multiply(hacfBeta, hacfD, hacfD);
				subtract(hacfD, hacfG, hacfD);
				hacfG_old = hacfG;

				// z = Phi.*FFT(W.*d) - x-ky-kz
				multiply(hacfWWindowed, hacfD, hacfZ);
				Transform_KernelTransform_->BTransform(hacfZ);
				multiply(hacfZ, hacfFullMask, hacfZ);

				//---------------------------- cg alpha -------------------------------------
				//alpha(:,:,:,c) = (z_helper(:)'*e_helper(:))/(z_helper(:)'*z_helper(:));
				#pragma omp parallel for
				for (int iCha = 0; iCha < iNChannels_; iCha++) {
					std::complex<float> fAlphaCha = 0.0;

					// fill sub array with data from higher order data array
					size_t tOffset = hacfE.get_size(0)*hacfE.get_size(1)*iCha;
					hoNDArray<std::complex<float> > hacfSubArrayE(hacfE.get_size(0), hacfE.get_size(1), hacfE.get_data_ptr()+ tOffset, false);
					hoNDArray<std::complex<float> > hacfSubArrayZ(hacfE.get_size(0), hacfE.get_size(1), hacfZ.get_data_ptr()+ tOffset, false);
					std::complex<float> fNumerator = 0.0;
					std::complex<float> fDenominator = 0.0;

					// calculate nominator
					for (size_t iI = 0; iI < hacfSubArrayE.get_number_of_elements(); iI++) {
						fNumerator += std::conj(hacfSubArrayZ.at(iI))*hacfSubArrayE.at(iI);
					}

					// calculate denominator
					for (size_t iI = 0; iI < hacfSubArrayZ.get_number_of_elements(); iI++) {
						fDenominator += std::conj(hacfSubArrayZ.at(iI))*hacfSubArrayZ.at(iI);
					}

					if (abs(fDenominator) != 0) {
						fAlphaCha = fNumerator / fDenominator;
					}

					// fill 3D alpha array
					#pragma omp parallel for
					for (size_t lI = 0; lI < hacfSubArrayE.get_number_of_elements(); lI++) {
						hacfAlpha.at(lI+tOffset) = fAlphaCha;
					}
				}
				//--------------------------------------------------------------------------

				// q = q + alpha.*d
				multiply(hacfAlpha, hacfD, hacfD);
				add(hacfQ, hacfD, hacfQ);

				// rho = W.*q
				multiply(hacfWWindowed, hacfQ, hacfRho);
			}
			catch(...) {
				GERROR("exception in inner loop..\n");
			}
		}

		// W = abs(rho).^p - p = .5 normalization
		hacfWWindowed = hacfRho;
		fAbsPowDivide(hacfWWindowed, fP_, hacfTotEnergy);
	}

	// rho = W.*q
	multiply(hacfWWindowed, hacfQ, hacfRho);

	//rho = kernelBTrafo(rho) -> x-y-z cart
	Transform_KernelTransform_->KernelBTransform(hacfRho);

	// output k-space for further k-space filtering or image
	if (Transform_fftAA_->get_active()) {
		Transform_fftAA_->BTransform(hacfRho);
	}

	// permute output - rho: y-x-c -> x-y-c
	vtDimOrder.clear();
	vtDimOrder.push_back(1);
	vtDimOrder.push_back(0);
	vtDimOrder.push_back(2);

	hacfRho = permute(hacfRho, vtDimOrder);

	vtDim_.clear();
	vtDim_ = *hacfRho.get_dimensions();

	// set output and return
	hacfRecon = hacfRho;

	GINFO("FOCUSS done..\n");

	return GADGET_OK;
}

//--------------------------------------------------------------------------
//---------------------------- windowing -----------------------------------
//--------------------------------------------------------------------------
void CS_FOCUSS_2D::fWindowing(hoNDArray<std::complex<float> > &hacfWWindowed)
{
	// array with mask
	hoNDArray<std::complex<float> > hacfMask2D(hacfWWindowed.get_dimensions());
	hacfMask2D.fill(std::complex<float>(0.0));

	// get calibration mask
	std::vector<size_t> vStart, vSize;
	for (int iI = 0; iI < 1; iI++) {
		if (viCalibrationSize_.at(iI) % 2) {
			vStart.push_back(std::floor((float)vtDim_[iI]/2)+std::ceil(-(float)viCalibrationSize_.at(iI)/2));
		} else {
			vStart.push_back(std::floor((float)vtDim_[iI]/2)+std::ceil(-(float)viCalibrationSize_.at(iI)/2));
		}
	}

	vStart.push_back(0);
	vStart.push_back(0);
	for (size_t iY = vStart.at(0); iY < vStart.at(0)+viCalibrationSize_.at(0); iY++) {
		for (size_t iX = vStart.at(1); iX < vStart.at(1)+viCalibrationSize_.at(1); iX++) {
			for (size_t iC = vStart.at(2); iC < vStart.at(2)+viCalibrationSize_.at(2); iC++) {
				hacfMask2D(iY, iX, iC) = std::complex<float>(1.0);
			}
		}
	}

	// windowing W
	hacfWWindowed *= hacfMask2D;

	// get kSpaceCenter
	habKSpaceCenter_.create(hacfWWindowed.get_dimensions());
	habKSpaceCenter_.fill(true);
	pbPtr_ = habKSpaceCenter_.get_data_ptr();
	pcfPtr_ = hacfMask2D.get_data_ptr();

	#pragma omp parallel for
	for (size_t lI = 0; lI < hacfMask2D.get_number_of_elements(); lI++) {
		if(pcfPtr_[lI] == std::complex<float>(0.0)) {
			pbPtr_[lI] = false;
		}
	}

	GINFO("data windowed for initial estimate and kSpaceCenter found..\n");
}

//--------------------------------------------------------------------------
//---------------------- get calibration size ------------------------------
//--------------------------------------------------------------------------
void CS_FOCUSS_2D::fGetCalibrationSize(const hoNDArray<bool> &habArray)
{
	size_t iSY = 2;
	bool bYflag = false;
	std::vector<size_t> vtDim = *habArray.get_dimensions();

	while(!bYflag) {
		if (!bYflag) {
			for (size_t iY = std::ceil((float)vtDim[0]/2)-iSY+1; iY < std::ceil((float)vtDim[0]/2)+iSY+1; iY++) {
				if (!habArray(iY, std::ceil((float)vtDim[1]/2))) {
					bYflag = true;
				} else {
					iSY++;
				}
			}
		}

		if (iSY == vtDim.at(0)) {
			bYflag = true;
		}
	}

	// push values on calibration size vector
	viCalibrationSize_.push_back(iSY);
	viCalibrationSize_.push_back(vtDim[1]);
	viCalibrationSize_.push_back(vtDim[2]);

	for (std::vector<int>::size_type i = 0; i != viCalibrationSize_.size(); i++) {
		GDEBUG("calibration size: %i..\n", viCalibrationSize_.at(i));
	}
}

GADGET_FACTORY_DECLARE(CS_FOCUSS_2D)

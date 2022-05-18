/*	
file name	: 	CS_AccumulatorGadget.cpp

author		: 	Martin Schwartz	(martin.schwartz@med.uni-tuebingen.de),
				Thomas Kuestner (thomas.kuestner@med.uni-tuebingen.de)

version		: 	1.

date		: 	v1.1: 17.02.2015
				v1.2: 02.02.2016
				v1.3: 06.12.2016
				v1.4: 31.01.2017

description	: 	implementation of the class "AccumulatorGadget.h"

references	:	original Gadgetron version 2.5 from 02-18-2014
				updated for Gadgetron version 3.6
*/

#include "CS_AccumulatorGadget.h"

using namespace Gadgetron;

// class constructor
CS_AccumulatorGadget::CS_AccumulatorGadget() : hacfBuffer_(0), image_counter_(0), image_series_(0)
{
	//clear CS_GlobalVar AcquisitionHeader vector
	GlobalVar::instance()->AcqVec_.clear();
}

// class destructor
CS_AccumulatorGadget::~CS_AccumulatorGadget()
{
	// free hacfBuffer_
	delete hacfBuffer_;
	hacfBuffer_ = NULL;
}

// read flexible data header
int CS_AccumulatorGadget::process_config(ACE_Message_Block *mb)
{
#ifdef __GADGETRON_VERSION_HIGHER_3_6__
	ISMRMRD::IsmrmrdHeader h;
	ISMRMRD::deserialize(mb->rd_ptr(),h);

	ISMRMRD::EncodingSpace e_space = h.encoding[0].encodedSpace;
	ISMRMRD::EncodingSpace r_space = h.encoding[0].reconSpace;
	ISMRMRD::EncodingLimits e_limits = h.encoding[0].encodingLimits;

	// get FOV
	vFOV_.push_back(r_space.fieldOfView_mm.x);
	vFOV_.push_back(e_space.fieldOfView_mm.y);
	vFOV_.push_back(e_space.fieldOfView_mm.z);

	// get matrix size
	vDim_.push_back(r_space.matrixSize.x);
	vDim_.push_back(e_space.matrixSize.y);
	vDim_.push_back(e_space.matrixSize.z);

	respiratory_phases_ = e_limits.slice? e_limits.slice->maximum+1 : 1;
	vDim_.push_back(respiratory_phases_);
#else
	// read xml header file
	boost::shared_ptr<ISMRMRD::ismrmrdHeader> cfg = parseIsmrmrdXMLHeader(std::string(mb->rd_ptr()));

	// create sequence encoding parameters
	ISMRMRD::ismrmrdHeader::encoding_sequence e_seq = cfg->encoding();
	ISMRMRD::encodingSpaceType e_space = (*e_seq.begin()).encodedSpace();
	ISMRMRD::encodingSpaceType r_space = (*e_seq.begin()).reconSpace();
	ISMRMRD::encodingLimitsType e_limits = (*e_seq.begin()).encodingLimits();

	// get FOV
	vFOV_.push_back(r_space.fieldOfView_mm().x());
	vFOV_.push_back(e_space.fieldOfView_mm().y());
	vFOV_.push_back(e_space.fieldOfView_mm().z());

	// get matrix size
	vDim_.push_back(r_space.matrixSize().x());
	vDim_.push_back(e_space.matrixSize().y());
	vDim_.push_back(e_space.matrixSize().z());

	// get no. of phases
	respiratory_phases_ = e_limits.phase().present() ? e_limits.phase().get().maximum() : 1;
	vDim_.push_back(respiratory_phases_);
#endif

	// read CS/ESPReSSo data and interpret integer values
	int iESPReSSoY = 0;
	int iESPReSSoZ = 0;
	iBodyRegion_ = 0;
	iVDMap_ = 0;
	iSamplingType_ = 0;
	fCSAcc_ = 0;
	fFullySa_ = 0;
	try {
#ifdef __GADGETRON_VERSION_HIGHER_3_6__
		if (h.encoding[0].trajectoryDescription) {
			GINFO("\n\nTrajectory description present!\n\n");
			ISMRMRD::TrajectoryDescription traj_desc = *h.encoding[0].trajectoryDescription;

			for (std::vector<ISMRMRD::UserParameterLong>::iterator i (traj_desc.userParameterLong.begin()); i != traj_desc.userParameterLong.end(); ++i) {
				if (i->name == "WIP_PF_y") {
					iESPReSSoY = i->value;
					GDEBUG("Phase ESPReSSo is %i \n", iESPReSSoY);
				} else if (i->name == "WIP_PF_z") {
					iESPReSSoZ = i->value;
					GDEBUG("Slice ESPReSSo is %i \n", iESPReSSoZ);
				} else if (i->name == "SamplingType") {
					iSamplingType_ = i->value;
					GDEBUG("Sampling Type is %i \n", iSamplingType_);
				} else if (i->name == "VDMap") {
					iVDMap_ = i->value;
				} else if (i->name == "BodyRegion") {
					iBodyRegion_ = i->value;
					GDEBUG("Body Region is %i \n", iBodyRegion_);
				} else if (i->name == "OuterIterations") {
					GlobalVar::instance()->iNOuter_ = i->value;
				} else if (i->name == "InnerIterations") {
					GlobalVar::instance()->iNInner_ = i->value;
				} else if (i->name == "fftSparseDim") {
					GlobalVar::instance()->iDimFFT_ = i->value;
				} else if (i->name == "dctSparseDim") {
					GlobalVar::instance()->iDimDCTSparse_ = i->value;
				} else if (i->name == "pcaSparseDim") {
					GlobalVar::instance()->iDimPCASparse_ = i->value;
				} else if (i->name == "kernelFftDim") {
					GlobalVar::instance()->iDimKernelFFT_ = i->value;
				} else if (i->name == "transformFftBaDim") {
					GlobalVar::instance()->iTransformFFTBA_ = i->value;
				} else if (i->name == "kSpaceOutDim") {
					GlobalVar::instance()->ikSpaceOut_ = i->value;
				} else if (i->name == "CSESPReSSo") {
					GlobalVar::instance()->bESPRActiveCS_ = i->value;
				}
			}

			for (std::vector<ISMRMRD::UserParameterDouble>::iterator i (traj_desc.userParameterDouble.begin()); i != traj_desc.userParameterDouble.end(); ++i) {
				if (i->name == "CS_Accel") {
					fCSAcc_ = i->value;
				} else if (i->name == "FullySampled") {
					GlobalVar::instance()->fFullySampled_ = i->value;
				} else if (i->name == "lambdaESPReSSo") {
					GlobalVar::instance()->cfLambdaESPReSSo_ = i->value;
				} else if (i->name == "lambda") {
					GlobalVar::instance()->cfLambda_ = i->value;
				}
			}
		}
#else
		if ((*e_seq.begin()).trajectoryDescription().present()) {
			GINFO("\n\nTrajectory description present!\n\n");
			ISMRMRD::trajectoryDescriptionType traj_desc = (*e_seq.begin()).trajectoryDescription().get();

			for (ISMRMRD::trajectoryDescriptionType::userParameterLong_sequence::iterator i (traj_desc.userParameterLong().begin ()); i != traj_desc.userParameterLong().end(); ++i) {
				if (std::strcmp(i->name().c_str(),"WIP_PF_y") == 0) {
					iESPReSSoY = i->value();
					GDEBUG("Phase ESPReSSo is %i \n", iESPReSSoY);
				} else if (std::strcmp(i->name().c_str(),"WIP_PF_z") == 0) {
					iESPReSSoZ = i->value();
					GDEBUG("Slice ESPReSSo is %i \n", iESPReSSoZ);
				} else if (std::strcmp(i->name().c_str(),"SamplingType") == 0) {
					iSamplingType_ = i->value();
					GDEBUG("Sampling Type is %i \n", iSamplingType_);
				} else if (std::strcmp(i->name().c_str(),"VDMap") == 0) {
					iVDMap_ = i->value();
				} else if (std::strcmp(i->name().c_str(),"BodyRegion") == 0) {
					iBodyRegion_ = i->value();
					GDEBUG("Body Region is %i \n", iBodyRegion_);
				} else if (std::strcmp(i->name().c_str(), "OuterIterations") == 0) {
					GlobalVar::instance()->iNOuter_ = i->value();
				} else if (std::strcmp(i->name().c_str(), "InnerIterations") == 0) {
					GlobalVar::instance()->iNInner_ = i->value();
				} else if (std::strcmp(i->name().c_str(), "fftSparseDim") == 0) {
					GlobalVar::instance()->iDimFFT_ = i->value();
				} else if (std::strcmp(i->name().c_str(), "dctSparseDim") == 0) {
					GlobalVar::instance()->iDimDCTSparse_ = i->value();
				} else if (std::strcmp(i->name().c_str(), "pcaSparseDim") == 0) {
					GlobalVar::instance()->iDimPCASparse_ = i->value();
				} else if (std::strcmp(i->name().c_str(), "kernelFftDim") == 0) {
					GlobalVar::instance()->iDimKernelFFT_ = i->value();
				} else if (std::strcmp(i->name().c_str(), "transformFftBaDim") == 0) {
					GlobalVar::instance()->iTransformFFTBA_ = i->value();
				} else if (std::strcmp(i->name().c_str(), "kSpaceOutDim") == 0) {
					GlobalVar::instance()->ikSpaceOut_ = i->value();
				} else if (std::strcmp(i->name().c_str(), "CSESPReSSo") == 0) {
					GlobalVar::instance()->bESPRActiveCS_ = i->value();
				}
			}

			for (ISMRMRD::trajectoryDescriptionType::userParameterDouble_sequence::iterator i (traj_desc.userParameterDouble().begin ()); i != traj_desc.userParameterDouble().end(); ++i) {
				if (std::strcmp(i->name().c_str(),"CS_Accel") == 0) {
					fCSAcc_ = i->value();
				} else if (std::strcmp(i->name().c_str(),"FullySampled") == 0) {
					GlobalVar::instance()->fFullySampled_ = i->value();
				} else if (std::strcmp(i->name().c_str(),"lambdaESPReSSo") == 0) {
					GlobalVar::instance()->cfLambdaESPReSSo_ = i->value();
				} else if (std::strcmp(i->name().c_str(),"lambda") == 0) {
					GlobalVar::instance()->cfLambda_ = i->value();					
				}
			}
		}
#endif
		else {
			GINFO("\n\nNo trajectory description present!\n\n");
		}

		//-------------------------------------------------------------------------
		//----------------------- Interpret Integer Data  -------------------------
		//-------------------------------------------------------------------------		
		switch (iBodyRegion_) {
		case 14:
			GDEBUG("Body region is none\n");
			break;
		case 15:
			GDEBUG("Body region is head\n");
			break;
		case 16:
			GDEBUG("Body region is thorax\n");
			break;
		case 17:
			GDEBUG("Body region is abdomen\n");
			break;
		case 18:
			GDEBUG("Body region is pelvis\n");
			break;
		}

		switch (iVDMap_) {
		case 1:
			GDEBUG("VDMap is none\n");
			break;
		case 2:
			GDEBUG("VDMap is point\n");
			break;
		case 3:
			GDEBUG("VDMap is block\n");
			break;
		case 4:
			GDEBUG("VDMap is ellipse\n");
			break;
		case 5:
			GDEBUG("VDMap is ring\n");
			break;
		}

		switch (iSamplingType_) {
		case 6:
			GDEBUG("sampling type is poisson\n");
			break;
		case 7:
			GDEBUG("sampling type is random\n");
			break;
		case 8:
			GDEBUG("sampling type is proba\n");
			break;
		}

		iESPReSSoDirection_ = 10;
		fPartialFourierVal_ = 1.0;

		if ((iESPReSSoY > 9 && iESPReSSoY < 14) || (iESPReSSoZ > 9 && iESPReSSoZ < 14)) {
			GINFO("Partial Fourier data..\n");
			GDEBUG("ESPReSSo Y: %f, ESPReSSo Z: %f\n", iESPReSSoY, iESPReSSoZ);

			// get Partial Fourier dimension
			if (iESPReSSoY > 9) {
				iESPReSSoDirection_ = 1;

				// get Partial Fourier value
				switch (iESPReSSoY) {
				case 10:
					fPartialFourierVal_ = 0.5;
					break;
				case 11:
					fPartialFourierVal_ = 0.625;
					break;
				case 12:
					fPartialFourierVal_ = 0.75;
					break;
				case 13:
					fPartialFourierVal_ = 0.875;
					break;
				default:
					fPartialFourierVal_ = 1.0;
					break;
				}
			} else if (iESPReSSoZ > 9) {
				iESPReSSoDirection_ = 2;

				// get Partial Fourier value
				switch (iESPReSSoZ) {
				case 10:
					fPartialFourierVal_ = 0.5;
					break;
				case 11:
					fPartialFourierVal_ = 0.625;
					break;
				case 12:
					fPartialFourierVal_ = 0.75;
					break;
				case 13:
					fPartialFourierVal_ = 0.875;
					break;
				default:
					fPartialFourierVal_ = 1.0;
					break;
				}
			}
		}

		GDEBUG("Partial Fourier is %f \n", fPartialFourierVal_);
		GDEBUG("ESPReSSo Direction is %i \n", iESPReSSoDirection_);
	} catch(...) {
		GERROR("Cannot find CS entries in trajectory description..\n");
	}

	return GADGET_OK;
}

// process(...): buffers the incoming data to a fully buffered k-space array
int CS_AccumulatorGadget::process(GadgetContainerMessage<ISMRMRD::AcquisitionHeader> *m1,GadgetContainerMessage<hoNDArray<std::complex<float> > > *m2)
{
	// create temporal buffer if not already exists
	if (!hacfBuffer_) {
		// get number of channels
		vDim_.push_back(m1->getObjectPtr()->active_channels); //GDEBUG("Number of receiver channels: %i\n", dimensionsIn_[4]);

		GDEBUG("Matrix size: %i, %i, %i, %i, %i\n", vDim_.at(0), vDim_.at(1), vDim_.at(2), vDim_.at(3), vDim_.at(4));

		// initialize buffer array for incoming data
		if (!(hacfBuffer_ = new hoNDArray< std::complex<float> >())) {
			GERROR("Failed create buffer\n");
			return GADGET_FAIL;
		}

		// adjust dimension in oversampled readout direction if oversampling was not already removed - only two-fold oversampling!!!
		int iNSamples =  m1->getObjectPtr()->number_of_samples;
		if (iNSamples > static_cast<int>(vDim_[0])) {
			GINFO("Input data is oversampled..adjust dim[0]..\n");
			vDim_[0] *= 2;
		}

		// create buffer array for incoming data
		try {
			hacfBuffer_->create(&vDim_);
		} catch (std::runtime_error &err) {
			GEXCEPTION(err,"Failed allocate buffer array\n");
			return GADGET_FAIL;
		}

		// read value for image_series
		//image_series_ = this->get_int_value("image_series");

		GINFO("receiving data...\n");  
	}

	// get pointers to the temporal buffer and the incoming data
	std::complex<float>* pcfBuffer	= hacfBuffer_->get_data_ptr();
	std::complex<float>* pcfData	= m2->getObjectPtr()->get_data_ptr();

	// get current loop counters
	int iSamples	= m1->getObjectPtr()->number_of_samples;
	int iLine		= m1->getObjectPtr()->idx.kspace_encode_step_1;
	int iPartition	= m1->getObjectPtr()->idx.kspace_encode_step_2;
	int iPhase		= m1->getObjectPtr()->idx.phase;

	// calculate the offset and copy the data for all the channels into the temporal buffer
	size_t tOffset = 0;
	for (int iC = 0; iC < m1->getObjectPtr()->active_channels; iC++) {
		tOffset = iC*vDim_[0]*vDim_[1]*vDim_[2]*vDim_[3] + iPhase*vDim_[0]*vDim_[1]*vDim_[2]+ iPartition*vDim_[0]*vDim_[1] + iLine*vDim_[0] + (vDim_[0]>>1)-m1->getObjectPtr()->center_sample;

		// copy samples
		memcpy(pcfBuffer+tOffset, pcfData+iC*iSamples, sizeof(std::complex<float>)*iSamples);
	}

	// get dimension indicating flags
#ifdef __GADGETRON_VERSION_HIGHER_3_6__
	bool bLast_encoding_step1 = ISMRMRD::FlagBit(ISMRMRD::ISMRMRD_ACQ_LAST_IN_ENCODE_STEP1).isSet(m1->getObjectPtr()->flags);
	bool bLast_in_measurement = ISMRMRD::FlagBit(ISMRMRD::ISMRMRD_ACQ_LAST_IN_MEASUREMENT).isSet(m1->getObjectPtr()->flags);
#else
	bool bLast_encoding_step1 = ISMRMRD::FlagBit(ISMRMRD::ACQ_LAST_IN_ENCODE_STEP1).isSet(m1->getObjectPtr()->flags);
	bool bLast_in_measurement = ISMRMRD::FlagBit(ISMRMRD::ACQ_LAST_IN_MEASUREMENT).isSet(m1->getObjectPtr()->flags);
#endif

	// copy header information for current slice to global variable
	if (bLast_encoding_step1) {
		GadgetContainerMessage<ISMRMRD::AcquisitionHeader>* GC_acq_tmp = new GadgetContainerMessage<ISMRMRD::AcquisitionHeader>();

		// copy header data
		fCopyAcqHeader(GC_acq_tmp, m1->getObjectPtr());

		// push header to global header vector
		GlobalVar::instance()->AcqVec_.push_back(GC_acq_tmp->getObjectPtr());
	}

	// copy data to a new GadgetContainer
	if (bLast_in_measurement) {
		GINFO("all data received - writing file and put on q..\n");
		fCopyData(m1, m2, pcfBuffer);
	}

	m1->release();
	return GADGET_OK;
}

// copy data to the new GadgetContainerMessage objects depending on incoming scan dimension and set Compressed Sensing values
int CS_AccumulatorGadget::fCopyData(GadgetContainerMessage<ISMRMRD::AcquisitionHeader> *GC_acq_hdr_m1, GadgetContainerMessage<hoNDArray<std::complex<float> > > *GC_img_m2, std::complex<float> *pcfBuffer)
{
	// create new image header
	GadgetContainerMessage<ISMRMRD::ImageHeader> *GC_img_hdr_m1 = new GadgetContainerMessage<ISMRMRD::ImageHeader>();

	// initialize the image header
	memset(GC_img_hdr_m1->getObjectPtr(),0,sizeof(ISMRMRD::ImageHeader));

	// initialize flags
	GC_img_hdr_m1->getObjectPtr()->flags = 0;

	// create new GadgetContainer
	GadgetContainerMessage<hoNDArray<std::complex<float> > > *GC_img_m2_new = new GadgetContainerMessage<hoNDArray<std::complex<float> > >();

	// concatenate data with header
	GC_img_hdr_m1->cont(GC_img_m2_new);
	
	try {
		GC_img_m2_new->getObjectPtr()->create(&vDim_);
	} catch (std::runtime_error &err) {
		GEXCEPTION(err,"Unable to allocate new image array\n");
		GC_img_hdr_m1->release();
		return -1;
	}

	// get data length and store dimension of incoming scan as user_int
	size_t tDataLength = vDim_.at(0)*vDim_.at(1)*vDim_.at(2)*vDim_.at(3)*vDim_.at(4);

	// copy data
	memcpy(GC_img_m2_new->getObjectPtr()->get_data_ptr(), pcfBuffer, sizeof(std::complex<float>)*tDataLength);

	// set header information
	GC_img_hdr_m1->getObjectPtr()->matrix_size[0]		= (uint16_t)vDim_[0];
	GC_img_hdr_m1->getObjectPtr()->matrix_size[1]		= (uint16_t)vDim_[1];
	GC_img_hdr_m1->getObjectPtr()->matrix_size[2]		= (uint16_t)vDim_[2];
	GC_img_hdr_m1->getObjectPtr()->field_of_view[0]		= vFOV_[0];
	GC_img_hdr_m1->getObjectPtr()->field_of_view[1]		= vFOV_[1];
	GC_img_hdr_m1->getObjectPtr()->field_of_view[2]		= vFOV_[2];
	GC_img_hdr_m1->getObjectPtr()->channels				= (uint16_t)GC_acq_hdr_m1->getObjectPtr()->active_channels;
	GC_img_hdr_m1->getObjectPtr()->slice				= GC_acq_hdr_m1->getObjectPtr()->idx.slice;
#ifdef __GADGETRON_VERSION_HIGHER_3_6__
	GC_img_hdr_m1->getObjectPtr()->data_type		= ISMRMRD::ISMRMRD_CXFLOAT;
#else
	GC_img_hdr_m1->getObjectPtr()->image_data_type		= ISMRMRD::DATA_COMPLEX_FLOAT;
#endif
	GC_img_hdr_m1->getObjectPtr()->image_index			= (uint16_t)(++image_counter_);
	GC_img_hdr_m1->getObjectPtr()->image_series_index	= (uint16_t)image_series_;
	memcpy(GC_img_hdr_m1->getObjectPtr()->position,GC_acq_hdr_m1->getObjectPtr()->position,sizeof(float)*3);
	memcpy(GC_img_hdr_m1->getObjectPtr()->read_dir,GC_acq_hdr_m1->getObjectPtr()->read_dir,sizeof(float)*3);
	memcpy(GC_img_hdr_m1->getObjectPtr()->phase_dir,GC_acq_hdr_m1->getObjectPtr()->phase_dir,sizeof(float)*3);
	memcpy(GC_img_hdr_m1->getObjectPtr()->slice_dir,GC_acq_hdr_m1->getObjectPtr()->slice_dir, sizeof(float)*3);
	memcpy(GC_img_hdr_m1->getObjectPtr()->patient_table_position,GC_acq_hdr_m1->getObjectPtr()->patient_table_position, sizeof(float)*3);
	
	set_number_of_gates(GC_img_hdr_m1->getObjectPtr()->user_int[0], 0, respiratory_phases_);
	GC_img_hdr_m1->getObjectPtr()->user_int[3]		= iVDMap_;
	GC_img_hdr_m1->getObjectPtr()->user_int[4]		= iESPReSSoDirection_;
	GC_img_hdr_m1->getObjectPtr()->user_int[6]		= iBodyRegion_;
	GC_img_hdr_m1->getObjectPtr()->user_float[0]	= fCSAcc_;
	GC_img_hdr_m1->getObjectPtr()->user_float[2]	= fPartialFourierVal_;
	GC_img_hdr_m1->getObjectPtr()->user_float[5]	= fFullySa_;
	GC_img_hdr_m1->getObjectPtr()->user_float[6]	= fLESPReSSo_;
	GC_img_hdr_m1->getObjectPtr()->user_float[7]	= fLQ_;

	// put on stream
	if (this->next()->putq(GC_img_hdr_m1) < 0) {
    	return GADGET_FAIL;
	}

	return GADGET_OK;
}

GADGET_FACTORY_DECLARE(CS_AccumulatorGadget)

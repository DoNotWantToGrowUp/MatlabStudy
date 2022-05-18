#include "CS_CombineGadget.h"

using namespace Gadgetron;

CS_CombineGadget::CS_CombineGadget() : combine_mode_(0), scale_(1.0), offset_(0.0), rep_avg_(false)
{
}

CS_CombineGadget::~CS_CombineGadget()
{
}

int CS_CombineGadget::process_config(ACE_Message_Block *mb)
{
	// get property values
#ifdef __GADGETRON_VERSION_HIGHER_3_6__
	ISMRMRD::IsmrmrdHeader h;
	ISMRMRD::deserialize(mb->rd_ptr(),h);

	if (h.userParameters) {
		for (size_t i = 0; i < h.userParameters->userParameterString.size(); i++) {
			std::string name = h.userParameters->userParameterString[i].name;
			std::string value = h.userParameters->userParameterString[i].value;

			if (name.substr(0,5) == std::string("Combi")) {
				combine_mode_ = std::atoi(name.substr(5,name.size()-5).c_str());

				if (combine_mode_ != 0 && combine_mode_ != 1) {
					GWARN("combine_mode_ false mode number!\n");
					// set default value
					combine_mode_ = 0;
				}
			} else if (name == std::string("Scale")) {
				scale_ = std::atoi(name.substr(5,name.size()-5).c_str());

				if (scale_ == 0.0) {
					scale_ = 1.0;
				}
			} else if (name == std::string("Offset")) {
				offset_ = std::atoi(name.substr(6,name.size()-6).c_str());
			} else if (name.substr(0,5) == std::string("Repet")) {
				rep_avg_ = std::atoi(name.substr(5,name.size()-5).c_str());
				GDEBUG("Repetition Averaging is: %i\n", rep_avg_);
			}
		}
	}
#else
	combine_mode_ = this->get_int_value("Combine Mode");
	if (combine_mode_ != 0 && combine_mode_ != 1) {
		GWARN("combine_mode_ false mode number!\n");
		// set default value
		combine_mode_ = 0;
	}

	// get scale and offset factor
	scale_ = this->get_double_value("Scale");
	if (scale_ == 0.0) {
		scale_ = 1.0;
	}

	offset_ = this->get_double_value("Offset");

	// get repetition averaging
	rep_avg_ = this->get_bool_value("Repetition Averaging");

	GDEBUG("Repetition Averaging is: %i\n", rep_avg_);
#endif

	return GADGET_OK;
}

int CS_CombineGadget::process(GadgetContainerMessage<ISMRMRD::ImageHeader> *m1, GadgetContainerMessage<hoNDArray<std::complex<float> > > *m2)
{
// get dimensions of incoming data
	size_t num_dims = m2->getObjectPtr()->get_number_of_dimensions();

	// debug output
	std::vector<size_t> di = *m2->getObjectPtr()->get_dimensions();
	for (size_t i = 0; i < di.size(); i++) {
		GDEBUG("d: %i\n", di.at(i));
	}

	std::vector<size_t> dimensions_;
	size_t nx = 0;
	size_t ny = 0;
	size_t nz = 0;
	size_t nt = 0;
	size_t nc = 0;
	GDEBUG("dims: %i\n", num_dims);

	// 2D, 3D,..
	if (num_dims == 4) {
		nx = m2->getObjectPtr()->get_size(0);
		ny = m2->getObjectPtr()->get_size(1);
		nz = m2->getObjectPtr()->get_size(2);
		nc = m2->getObjectPtr()->get_size(3);
		nt = 1;

		GDEBUG("dim 2D 3D - x: %i, y: %i, z: %i, c: %i, t: %i\n", nx,ny,nz,nc,nt);

		dimensions_.push_back(nx);
		dimensions_.push_back(ny);
		dimensions_.push_back(nz);
	} else if(num_dims == 5) {
		// 4D
		nx = m2->getObjectPtr()->get_size(0);
		ny = m2->getObjectPtr()->get_size(1);
		nz = m2->getObjectPtr()->get_size(2);
		nt = m2->getObjectPtr()->get_size(3);
		nc = m2->getObjectPtr()->get_size(4);

		GDEBUG("dim 4D - x: %i, y: %i, z: %i, c: %i, t: %i\n", nx,ny,nz,nc,nt);

		dimensions_.push_back(nx);
		dimensions_.push_back(ny);
		dimensions_.push_back(nz);
		dimensions_.push_back(nt);
	} else {
		GERROR("Error occured - dimension..\n");
	}

	hoNDArray<std::complex<float> > Data(dimensions_);
	std::complex<float> *d1 = m2->getObjectPtr()->get_data_ptr();
	std::complex<float> *d2 = Data.get_data_ptr();
	
	// Create a new message with an hoNDArray for the combined image
	GadgetContainerMessage<hoNDArray<std::complex<float> > > *m3 = new GadgetContainerMessage<hoNDArray<std::complex<float> > >();

	size_t img_block = nx*ny*nz*nt;

	switch (combine_mode_) {
	case 0:
		// Unweighted Complex Combination (Applies seperately for real part and imaginary part) - MS: added scale and offset
		for (size_t t = 0; t < nt; t++) {
			for (size_t z = 0; z < nz; z++) {
				for (size_t y = 0; y < ny; y++) {
					for (size_t x = 0; x < nx; x++) {
						float mag = 0;
						float phase = 0;
						size_t offset = x + y*nx + z*nx*ny + t*nx*ny*nz;

						for (size_t c = 0; c < nc; c++) {
							float mag_tmp = norm(d1[offset + c*img_block]);
							phase += mag_tmp*arg(d1[offset + c*img_block]);
							mag += mag_tmp;
						}

						d2[offset] = std::polar(std::sqrt(mag),phase)*std::complex<float>(scale_, scale_) + std::complex<float>(offset_,offset_);
					}
				}
			}
		}
		break;
	case 1:
		// Absolute Selfweighted Combine Mode (Applying for real source data) - Martin Schwartz
		for (size_t t = 0; t < nt; t++) {
			for (size_t z = 0; z < nz; z++) {
				for (size_t y = 0; y < ny; y++) {
					for (size_t x = 0; x < nx; x++) {
						float real_data = 0;
						size_t offset = x + y*nx + z*nx*ny + t*nx*ny*nz;

						for (size_t c = 0; c < nc; c++) {
							float real_data_tmp = norm(d1[offset + c*img_block]);
							real_data += real_data_tmp;
						}

						d2[offset] = std::sqrt(real_data);
					}
				}
			}
		}
		break;
	}
	
	// repetition averaging
	// 2Dt
	if (num_dims == 4 && rep_avg_) {
		try{
			m3->getObjectPtr()->create(dimensions_[0], dimensions_[1], 1);
		} catch (std::runtime_error &err) {
			GEXCEPTION(err, "Failed to allocate new array\n");
			return -1;
		}
		std::complex<float> *Ptr = m3->getObjectPtr()->get_data_ptr();

		// add repetitions
		img_block = nx*ny;
		for (size_t z = 0; z < nz; z++) {
			for (size_t y = 0; y < ny; y++) {
				for (size_t x = 0; x < nx; x++) {
					size_t offset = x + y*nx;
					Ptr[offset] += d2[offset + z*img_block];
				}
			}
		}
		
		// divide by number of repetitions
		for (size_t i = 0; i < m3->getObjectPtr()->get_number_of_elements(); i++) {
			Ptr[i] /= nt;
		}

		// set dim
		m1->getObjectPtr()->matrix_size[2] = 1;
		m1->getObjectPtr()->user_int[0] = 1;
	} else if (num_dims == 5 && rep_avg_) {
		// 3Dt
		try{
			m3->getObjectPtr()->create(dimensions_[0], dimensions_[1], dimensions_[2]);
		} catch (std::runtime_error &err) {
			GEXCEPTION(err, "Failed to allocate new array\n");
			return -1;
		}
		std::complex<float> *Ptr = m3->getObjectPtr()->get_data_ptr();

		// add repetitions
		img_block = nx*ny*nz;
		for (size_t t = 0; t < nt; t++) {
			for (size_t z = 0; z < nz; z++) {
				for (size_t y = 0; y < ny; y++) {
					for (size_t x = 0; x < nx; x++) {
						size_t offset = x + y*nx + z*nx*ny;
						Ptr[offset] += d2[offset + t*img_block];
					}
				}
			}
		}

		// divide by number of repetitions
		for (size_t i = 0; i < m3->getObjectPtr()->get_number_of_elements(); i++) {
			Ptr[i] /= nt;
		}

		// set dim
		m1->getObjectPtr()->user_int[0] = 5;
	} else {
		try {
			m3->getObjectPtr()->create(&dimensions_);
		} catch (std::runtime_error &err) {
			GEXCEPTION(err, "CS_CombineGadget, failed to allocate new array\n");
			return -1;
		}

		// fill output array
		std::complex<float> *Ptr = m3->getObjectPtr()->get_data_ptr();
		for (size_t i = 0; i < m3->getObjectPtr()->get_number_of_elements(); i++) {
			Ptr[i] = d2[i];
		}
	}

	GDEBUG("num dims new array: %i\n", m3->getObjectPtr()->get_number_of_dimensions());

	// Modify header to match the size and change the type to real
	// WARNING: "channels" are now phases (gates)
	m1->getObjectPtr()->channels = nt;

	// Now add the new array to the outgoing message
	m1->cont(m3);

	// Release the old data
	m2->release();

	return this->next()->putq(m1);
}

GADGET_FACTORY_DECLARE(CS_CombineGadget)

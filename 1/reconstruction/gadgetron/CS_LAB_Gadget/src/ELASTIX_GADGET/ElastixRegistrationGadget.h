/*	
file name	: 	ElastixRegistrationGadget.h

author		: 	Martin Schwartz	(martin.schwartz@med.uni-tuebingen.de)
				Thomas Kuestner (thomas.kuestner@med.uni-tuebingen.de)

version		: 	1.2

date		: 	13.10.2015
				23.03.2017
				23.01.2018

description	: 	Elastix-based image registration
*/

#ifndef ELASTIXREGISTRATIONGADGET_H
#define ELASTIXREGISTRATIONGADGET_H

#include "gadgetron_messages.h"

#pragma once
#include "CS_LAB_export.h"
#include "Gadget.h"
#include "hoNDArray.h"
#include <ismrmrd.h>
#include "GadgetIsmrmrdReadWrite.h"

#include "elastixlib.h"				// elastix library
#include "itkParameterFileParser.h" // read parameter file from disk
#include "itkImage.h"				// itk image class
#include "itkImportImageFilter.h"	// itk image filter class

typedef elastix::ELASTIX::ParameterMapType RegistrationParametersType;
typedef itk::ParameterFileParser ParserType;
typedef itk::Image<float, 3> ImageType;
typedef itk::ImportImageFilter<float, 3> ImportFilterType;

namespace Gadgetron
{
	class EXPORTCSLAB ElastixRegistrationGadget : public Gadget2<ISMRMRD::ImageHeader, hoNDArray<float> >
	{
	public:
		ElastixRegistrationGadget();
		~ElastixRegistrationGadget();

		GADGET_DECLARE(ElastixRegistrationGadget);

	protected:
		int process(GadgetContainerMessage<ISMRMRD::ImageHeader> *m1, GadgetContainerMessage<hoNDArray<float> > *m2);
		int process_config(ACE_Message_Block *mb);

	private:
		template<size_t DIM> bool read_itk_to_hondarray(hoNDArray<float> &deformation_field, const char *deformation_field_file_name, const size_t pixels_per_image) {
			using PixelType = itk::Vector<float, DIM>;
			using ImageType = itk::Image<PixelType, DIM>;

			typename itk::ImageFileReader<ImageType>::Pointer reader = itk::ImageFileReader<ImageType>::New();
			reader->SetFileName(deformation_field_file_name);

			try {
				reader->Update();
			} catch (itk::ExceptionObject& e) {
				std::cerr << e.GetDescription() << std::endl;
				return false;
			}

			// read image
			typename ImageType::Pointer inputImage = reader->GetOutput();

			// copy image from ITK to array
			memcpy(deformation_field.get_data_ptr(), inputImage->GetBufferPointer(), DIM*pixels_per_image*sizeof(float));

			return true;
		}

	private:
		bool log_output_;
		std::string sPathParam_;
		std::string sPathLog_;

#ifdef __GADGETRON_VERSION_HIGHER_3_6__
		GADGET_PROPERTY(LogOutput, bool, "LogOutput", false);
		GADGET_PROPERTY(PathParam, std::string, "PathParam", "");
		GADGET_PROPERTY(PathLog, std::string, "PathLog", "");
#endif
	};
}
#endif //ELASTIXREGISTRATIONGADGET_H

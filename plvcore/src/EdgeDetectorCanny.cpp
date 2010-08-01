#include <QDebug>

#include "EdgeDetectorCanny.h"
#include "Pin.h"
#include "OpenCVImage.h"
#include <opencv/cv.h>

using namespace plv;

#define INPUT_PIN_NAME "input"
#define OUTPUT_PIN_NAME "output"

EdgeDetectorCanny::EdgeDetectorCanny() :
        m_apertureSize(3),
        m_thresholdLow(0.1),
        m_thresholdHigh(1.0)
{
    m_inputPin = createInputPin<OpenCVImage>( INPUT_PIN_NAME, this );
    m_outputPin = createOutputPin<OpenCVImage>( OUTPUT_PIN_NAME, this );
}

EdgeDetectorCanny::~EdgeDetectorCanny()
{
}

void EdgeDetectorCanny::init() throw (PipelineException)
{
}

bool EdgeDetectorCanny::isReadyForProcessing() const
{
    return m_inputPin->hasData();
}

void EdgeDetectorCanny::process()
{
    assert(m_inputPin != 0);
    assert(m_outputPin != 0);

    RefPtr<OpenCVImage> img = m_inputPin->get();
    if(img->getDepth() != IPL_DEPTH_8U)
    {
        throw std::runtime_error("format not yet supported");
    }

    // temporary image with extra room (depth)
    RefPtr<OpenCVImage> tmp = OpenCVImageFactory::instance()->get(
            img->getWidth(), img->getHeight(), IPL_DEPTH_8U , 1 );

    RefPtr<OpenCVImage> img2 = OpenCVImageFactory::instance()->get(
            img->getWidth(), img->getHeight(), img->getDepth(), img->getNumChannels() );


    // open for reading
    const IplImage* iplImg1 = img->getImage();

    // open image for writing
    IplImage* iplImg2 = img2->getImageForWriting();
    IplImage* iplTmp = tmp->getImageForWriting();

    // INPUT REQUIRED TO BE GRAYSCALED! take the first channel as grayscale image
    cvSplit(iplImg1,iplTmp,NULL,NULL,NULL);

    // do a canny edge detection operator of the image
    cvCanny( iplTmp, iplTmp, m_thresholdLow, m_thresholdHigh, m_apertureSize);

    // convert the image back to 8bit depth
//    cvConvertScale(iplTmp, iplImg2, 1, 0);
    cvMerge( iplTmp, iplTmp, iplTmp, NULL, iplImg2 );

    // publish the new image
    m_outputPin->put( img2.getPtr() );
}


void EdgeDetectorCanny::setApertureSize(int i)
{
    //aperture size must be odd and positive, min 3, max 7 (but that is already way too much for sensible results)
    if (i < 3) i = 3;
    if (i > 7) i = 7;
    if (i%2 == 0)
    {   //even: determine appropriate new odd value
        if (i > m_apertureSize) i++; //we were increasing -- increase to next odd value
        else i--;                    //we were decreasing -- decrease to next odd value
    }
    m_apertureSize = i;
    emit(apertureSizeChanged(m_apertureSize));
}

#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

namespace enc = sensor_msgs::image_encodings;

static const char WINDOW[] = "Image window";
static bool shutdown;
static const unsigned int imgcount = 300;

class ImageConverter
{
  ros::NodeHandle nh_;
  image_transport::ImageTransport it_;
  image_transport::Subscriber image_sub_;
  image_transport::Publisher image_pub_;

  // per-pixel sums
  double* auxs;
  double* means;
  unsigned int numpixels;
  unsigned int total;
  bool init;
  
public:
  ImageConverter()
    : it_(nh_), init(false)
  {
    image_sub_ = it_.subscribe("in", 1, &ImageConverter::imageCb, this);

    cv::namedWindow(WINDOW);
  }

  ~ImageConverter()
  {
    delete[] auxs;
    delete[] means;
    cv::destroyWindow(WINDOW);
  }

  void imageCb(const sensor_msgs::ImageConstPtr& msg)
  {
    if(shutdown)
      return;

    cv_bridge::CvImagePtr cv_ptr;
    try
    {
      cv_ptr = cv_bridge::toCvCopy(msg, enc::BGR8);
    }
    catch (cv_bridge::Exception& e)
    {
      ROS_ERROR("cv_bridge exception: %s", e.what());
      return;
    }

    //if (cv_ptr->image.rows > 60 && cv_ptr->image.cols > 60)
     // cv::circle(cv_ptr->image, cv::Point(50, 50), 10, CV_RGB(255,0,0));

    if (!init)
    {
      numpixels = cv_ptr->image.rows*cv_ptr->image.cols;
      auxs = new double[numpixels];
      memset(auxs,0,numpixels*sizeof(double));
      means = new double[numpixels];
      memset(means,0,numpixels*sizeof(double));
      total = 0;

      init = true;
    }

    // Online algorithm for mean and variance, adapted to work per-pixel
    // http://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Online_algorithm
    total += 1;
    for (int i=0; i<cv_ptr->image.rows; i++)
    {
      for(int j = 0; j < cv_ptr->image.cols; j++)
      {
        unsigned idx = j*cv_ptr->image.rows+i;
        unsigned char pixel = cv_ptr->image.at<unsigned char>(i,j);
        double delta = pixel - means[idx];
        means[idx] += delta/total;
        auxs[idx] += delta*(pixel - means[idx]);
      }
    }
    fprintf(stderr, "processed image %u/%u\n", total, imgcount);

    if (total >= imgcount)
    {
      shutdown = true;
      return;
    }

    cv::imshow(WINDOW, cv_ptr->image);
    cv::waitKey(3);
  }

  void printStats()
  {
    printf("# idx mean stddev\n");
    double stddev_sum = 0.0;
    double stddev_min = std::numeric_limits<double>::max();
    double stddev_max = std::numeric_limits<double>::min();
    for(unsigned int i=0; i<numpixels; i++)
    {
      // Take sqrt to go from variance to stddev.  Divide by 256 to end up in
      // the range [0,1], which is how we'll specify stddev in noise models.
      double stddev = sqrt(auxs[i]/(total-1))/256.0;
      stddev_sum += stddev;
      if (stddev < stddev_min)
        stddev_min = stddev;
      if (stddev > stddev_max)
        stddev_max = stddev;
      printf("%u %f %f\n", i, means[i], stddev);
    }
    printf("# min stddev: %f\n", stddev_min);
    printf("# max stddev: %f\n", stddev_max);
    printf("# mean stddev: %f\n", stddev_sum / numpixels);
  }
};

int main(int argc, char** argv)
{
  ros::init(argc, argv, "image_converter");
  ImageConverter ic;
  while (ros::ok() && !shutdown)
    ros::spinOnce();
  ic.printStats();
  return 0;
}

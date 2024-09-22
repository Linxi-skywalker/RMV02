#include<opencv2/opencv.hpp>
#include<vector>
#include<iostream>

using namespace std;
using namespace cv;


        
    	Size rotateSize(const Size& size, float angle)                                    // 计算旋转后图像所需的新边界尺寸
    	{
    	float radian = angle * CV_PI / 180.0;                                             // 角度转换为弧度
    	float width = abs(size.width *cos(radian)) + abs(size.height * sin(radian));      // 计算原图宽对应的旋转后的对角长度
    	float height = abs(size.width * sin(radian)) + abs(size.height * cos(radian));    // 计算原图高对应的旋转后的对角长度
    	return Size(static_cast<int>(ceil(width)), static_cast<int>(ceil(height)));       // 返回新图像的尺寸（向上取整）
    	}


int main(int argc, char** argv)
{
    
	Mat Image = imread("/home/linxi/opencv_project/resources/test_image.png");    //从项目目录中获取测试图片
	if (Image.empty()){
        cerr<<"Could not open or find the image!"<<endl;
        return -1;                                                                    // 检测图像是否成功加载
	}

//图像颜色空间转化

	Mat grayImage;                                   //创建存储灰度图的对象
	cvtColor(Image,grayImage,COLOR_BGR2GRAY);         //转化为灰度图

	Mat hsvImage;                                    //创建存储HSV图像的对象
	cvtColor(Image,hsvImage,COLOR_BGR2HSV);          //转化为HSV图像

//应用各种滤波操作

	Mat blurred_mean;
	blur(Image,blurred_mean,Size(5,5));                               //使用5*5的核进行均值滤波             
	
	Mat blurred_gaussian;	                   
	GaussianBlur(Image,blurred_gaussian, Size(5,5),0);                 //使用5*5的核进行高斯滤波，标准差为0


//特征提取


    Scalar lower_red1(0, 100, 50);
    Scalar upper_red1(10, 255, 255);
    Scalar lower_red2(155, 120, 70);
    Scalar upper_red2(175, 255, 255);                                                     // 定义红色的 HSV 范围

    Mat mask9, mask8, mask;
    inRange(hsvImage, lower_red1, upper_red1, mask9);
    inRange(hsvImage, lower_red2, upper_red2, mask8);
    bitwise_or(mask9, mask8, mask);                                                       // 创建掩膜
  
    Mat redArea;
    bitwise_and(Image, Image, redArea, mask);                                             // 应用掩膜以提取红色区域
 
    vector<vector<Point>> contours;
    findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);                     // 寻找轮廓
     
    for (size_t i = 0; i < contours.size(); i++) {
        double area = contourArea(contours[i]);
        cout << "Contour " << i << " area: " << area << endl;                             // 遍历轮廓并计算面积
    }


    Mat imgContours = Image.clone();                                                      // 使用原图的副本以便保留颜色信息
    for (size_t i = 0; i < contours.size(); i++) {
        drawContours(imgContours, contours, (int)i, Scalar(0, 0, 255), 2);                // 绘制轮廓
        Rect boundingBox = boundingRect(contours[i]);
        rectangle(imgContours, boundingBox, Scalar(0, 0, 255), 2);                        //绘制bounding box
    }
    
//提取高亮颜色区域并进行图形学处理

// 定义高亮颜色的HSV范围
    	Scalar highlight_low(0, 0, 200);                                          // 低阈值：较亮的颜色
   	Scalar highlight_high(255, 255, 255);                                     // 高阈值：白色
 
   	Mat mask2;
   	inRange(hsvImage, highlight_low, highlight_high, mask2);                  // 创建掩膜（二值图像），只保留高亮颜色区域

    

// 对掩膜进行形态学操作：膨胀和腐蚀
    	
    	Mat dilated_mask;
    	Mat kernel2= getStructuringElement(MORPH_RECT, Size(3, 3));
   	dilate(mask2, dilated_mask, kernel2);                                   // 对掩膜进行形态学操作：膨胀

   	Mat eroded_mask;
   	erode(mask2, eroded_mask, kernel2);                                      // 对掩膜进行形态学操作：腐蚀

    	Mat closed_mask;
   	morphologyEx(mask2, closed_mask, MORPH_CLOSE, kernel2);                  // 对掩膜进行漫水处理（闭操作：先膨胀后腐蚀）

// 创建展示图像，将原始图像和处理后的掩膜叠加
    	Mat result_dilated, result_eroded, result_closed;
    	bitwise_and(Image,Image, result_dilated, dilated_mask);
    	bitwise_and(Image,Image, result_eroded, eroded_mask);
   	bitwise_and(Image,Image, result_closed, closed_mask);

    	Mat highlighted_area;
    	Image.copyTo(highlighted_area, mask2);                                   // 使用掩膜将原始图像中的高亮区域提取出来
    	Mat gray_highlighted_area;
    	cvtColor(highlighted_area, gray_highlighted_area, COLOR_BGR2GRAY);       //灰度化
//图像绘制

 	Mat image = Mat::zeros(480, 640, CV_8UC3);                              // 创建一个空白图像，尺寸为640x480，三通道（彩色）
	Scalar color = Scalar(255, 255, 255);                                  // 设置绘制颜色为白色

// 绘制圆形
    	int centerX = 250;         // 圆心x坐标
    	int centerY = 230;         // 圆心y坐标
    	int radius = 150;          // 半径
    	circle(image, Point(centerX, centerY), radius, color, -1);              // -1表示填充圆形

// 绘制方形
    	int topLeftX = 50;                       // 左上角x坐标
    	int topLeftY = 100;                      // 左上角y坐标
    	int bottomRightX = 450;                  // 右下角x坐标
    	int bottomRightY = 360;                  // 右下角y坐标
    	rectangle(image, Point(topLeftX, topLeftY), Point(bottomRightX, bottomRightY), color, -1);         // -1表示填充方形
// 绘制文字
    	string text = "Hello, OpenCV!";
    	int textX = 150;              // 文字起始x坐标
    	int textY = 450;              // 文字起始y坐标
   	putText(image, text, Point(textX, textY), FONT_HERSHEY_SIMPLEX, 1.0, color, 2);                     // 2表示文字线条
//对图像进行处理

    
    	float angle = 35.0;
    	Point2f center(Image.cols / 2.0, Image.rows / 2.0);	                                       // 设置旋转角度和旋转中心（图像中心）
    	Mat rotMat = getRotationMatrix2D(center, angle, 1.0);                                          // 获取旋转矩阵（2x3）
    	Size newSize = rotateSize(Image.size(), angle);                                                // 根据旋转角度计算新图像的尺寸
    	rotMat.at<float>(0, 2) += (newSize.width - Image.cols) / 2.0;                                  //调整旋转矩阵以适应新图像尺寸
    	rotMat.at<float>(1, 2) += (newSize.height - Image.rows) / 2.0;                                 //调整旋转矩阵以适应新图像尺寸


    	Mat rotated;
    	warpAffine(Image, rotated, rotMat, newSize);                                   // 进行图像旋转
    	
  	Rect cropRect(0, 0, Image.cols / 2,Image.rows / 2);
   	Mat cropped = Image(cropRect);                                                 // 裁剪图像为原图像左上角的1/4

//展示并保存
	namedWindow("Test Image", WINDOW_NORMAL);
	imshow("Test Image",Image);                                                    //显示测试图片原像
	namedWindow("Gray Image", WINDOW_NORMAL);
	imshow("Gray Image",grayImage);                                                //显示灰度图
	namedWindow("HSV", WINDOW_NORMAL);
	imshow("HSV",hsvImage);                                                        //显示HSV图像
	namedWindow("Blurred Image (Mean Filter)", WINDOW_NORMAL);
	imshow("Blurred Image (Mean Filter)",blurred_mean);                            //显示均值滤波后的图像
	namedWindow("Blurred Image (Gaussian Filter)", WINDOW_NORMAL);
	imshow("Blurred Image (Gaussian Filter)",blurred_gaussian);                    //显示高斯滤波后的图像
	namedWindow("Red Area", WINDOW_NORMAL);
	imshow("Red Area", redArea);                                                   //显示红色区域
	namedWindow("Detected Red Color", WINDOW_NORMAL);
	imshow("Detected Red Color", imgContours);                                     //显示轮廓和bounding box
	namedWindow("Highlighted Area", WINDOW_NORMAL);
	imshow("Highlighted Area", highlighted_area);                                  //展示高亮区域
	namedWindow("Gray Highlighted Area", WINDOW_NORMAL);
	imshow("Gray Highlighted Area", gray_highlighted_area);                        //展示高亮区域的灰度图
    	namedWindow("Mask", WINDOW_NORMAL);
    	imshow("Mask", mask2);                                                         //展示掩膜（二值化）
    	
	namedWindow("Dilated Mask", WINDOW_NORMAL);    
   	imshow("Dilated Mask", result_dilated);	// 展示提取高光后膨胀处理的图像
   	namedWindow("Eroded Mask", WINDOW_NORMAL);
    	imshow("Eroded Mask", result_eroded);   //展示提取高光后腐蚀处理的图像
    	namedWindow("Closed Mask", WINDOW_NORMAL);
    	imshow("Closed Mask", result_closed);   //展示提取高光后漫水处理的图像
    	namedWindow("Drawn Shapes and Text", WINDOW_NORMAL);
    	imshow("Drawn Shapes and Text", image);                                            //展示绘制的图形和文字
    	namedWindow("rotated_image.png", WINDOW_NORMAL);
    	imshow("rotated_image.png", rotated);                                               //展示旋转后的图像
    	namedWindow("cropped_image.png", WINDOW_NORMAL);
    	imshow("cropped_image.png", cropped);                                               //展示裁减后的图像
   

	imwrite("gray_image.png",grayImage);                                 //保存灰度图像

	bool isSaved = imwrite("hsvImage.png", hsvImage);                    //保存HSV图像
	imwrite("blurred_mean.png",blurred_mean);                            //保存均值滤波后的图像
	imwrite("blurred_gaussian.png",blurred_gaussian);                    //保存高斯滤波后的图像
	
       	bool isSaved2 = imwrite("red_area.png", redArea);                    //保存红色区域
	bool isSaved3 = imwrite("red_contours.png", imgContours);            //保存轮廓和bounding box
	imwrite("highlighted_area.png",highlighted_area);                    //保存高亮区域
	
	
	imwrite("gray_highlighted_area.png", gray_highlighted_area);         //保存高亮区域的灰度图
	imwrite("mask.png",mask2);                    //保存掩膜（二值化）
	imwrite("dilated_mask.png", result_dilated);  //保存提取高光后膨胀处理的图像
   	imwrite("eroded_mask.png", result_eroded);    // 保存提取高光后腐蚀处理的图像
    	imwrite("closed_mask.png", result_closed);    // 保存提取高光后漫水处理的图像
    	imwrite("drawn_shapes_and_text.png", image);  //保存绘制的圆形方形和文字
   	imwrite("rotated_image.png",rotated);         // 保存旋转后的图像
    	imwrite("cropped_image.png", cropped);        // 保存裁剪后的图像


	
	waitKey(0); //等待按键然后关闭窗口


	return 0;

 
 		
}


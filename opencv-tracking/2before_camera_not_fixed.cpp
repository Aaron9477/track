#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <iostream>

using namespace std;
using namespace cv;

Rect2d box;//矩形对象
bool drawing_box = false;//记录是否在画矩形对象
bool gotBB = false;

//bounding box mouse callback
void mouseHandler(int event, int x, int y, int flags, void *param){
  switch( event ){
    case CV_EVENT_LBUTTONDOWN://鼠标左键按下事件
	    drawing_box = true;//标志在画框
	    box = Rect2d( x, y, 0, 0 );//记录矩形的开始的点
    	break;
	case CV_EVENT_MOUSEMOVE://鼠标移动事件
		if (drawing_box){//如果左键一直按着，则表明在画矩形
			box.width = x-box.x;
			box.height = y-box.y;//更新长宽
		}
		break;
	case CV_EVENT_LBUTTONUP://鼠标左键松开事件
		drawing_box = false;//不在画矩形
		if( box.width < 0 ){//排除宽为负的情况，在这里判断是为了优化计算，不用再移动时每次更新都要计算长宽的绝对值
			box.x += box.width;//更新原点位置，使之始终符合左上角为原点
			box.width *= -1;//宽度取正
		}
		if( box.height < 0 ){//同上
			box.y += box.height;
			box.height *= -1;
		}
		gotBB = true;
		break;
	default:
		break;
  }
}

int main(int argc, char** argv){
	// can change to BOOSTING, MIL, KCF (OpenCV 3.1), TLD, MEDIANFLOW, or GOTURN (OpenCV 3.2)
	// Ptr<Tracker> tracker = Tracker::create("KCF"); 
	//TrackerKCF::Params::read("/usr/q.txt");
	Ptr<TrackerKCF> tracker = TrackerKCF::createTracker();
	VideoCapture video(0);	//用电脑自带摄像头比外置摄像头快很多
	if(!video.isOpened()){
		cerr << "cannot read video!" << endl;
		return -1;
	}
	Mat frame;
	video.read(frame);
	// namedWindow("Tracking");
	namedWindow("Tracking", 0);//WINDOW_NORMAL=0，在这个模式下可以调整窗口的大小。
	cvResizeWindow("Tracking", 960, 640);
	//cvResizeWindow("Tracking", 720, 480);
	setMouseCallback("Tracking", mouseHandler, &frame);

	while(!drawing_box){	//选框之前保持拍摄
		video.read(frame);
		imshow("Tracking", frame);
		waitKey(1);//维持imshow
	}

	while(!gotBB){
		//只要不再次按下鼠标左键触发事件,则程序显示的一直是if条件里面被矩形函数处理过的temp图像，如果再次按下鼠标左键就进入if，不断更新被画矩形函数处理过的temp，因为处理速度快所以看起来画矩形的过程是连续的没有卡顿，因为每次重新画都是在没有框的基础上画出新的框因为人眼的残影延迟所以不会有拖影现象。每次更新矩形框的传入数据是重新被img（没有框）的数据覆盖的temp（即img.data==temp.data）和通过回调函数更新了的Box记录的坐标点数据。
		waitKey(1);//维持imshow
		if(drawing_box){//不断更新正在画的矩形
			video.read(frame);
			// frame.copyTo(temp);//这句放在这里是保证了每次更新矩形框都是在没有原图的基础上更新矩形框。
			rectangle(frame, box, Scalar(255, 255, 255), 2, 1);
			imshow("Tracking", frame);//显示
		}
	}

	//Rect2d box(270, 120, 180, 260);
	tracker->init(frame, box);
	while(video.read(frame)){
		tracker->update(frame, box);	//tracker用的是Rect2d,不是Rect格式
		rectangle(frame, box, Scalar(255, 0, 0), 2, 1);
		imshow("Tracking", frame);
		int k=waitKey(1);
		if(k==27) break;
	}
}

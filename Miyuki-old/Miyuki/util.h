#pragma once
#include <QVector3D>
#include <QVector>
#include <QString>
#include <QImage>
#include <QPixmap>
#include <QEvent>
#include <QResizeEvent>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QCheckBox>
#include <QClipboard>


#include <CL/cl.h>
#include <ppl.h>
#pragma comment(lib,"OpenCL.lib")

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <set>
#include <thread>
#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <mutex>
#include <future>
#include <unordered_map>
#include "lib/fmt/format.h"
#include "lib/tiny_obj_loader/tiny_obj_loader.h"


const char *clGetErrorString(cl_int error);

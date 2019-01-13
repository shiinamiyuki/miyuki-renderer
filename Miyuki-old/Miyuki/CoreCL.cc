#include "CoreCL.h"

using namespace CoreCL;
cl_platform_id CoreCL::findPlatformContains(const std::string& s, cl_platform_id*list, int N) {
	auto name = s;
	std::transform(name.begin(), name.end(),name.begin(), ::toupper);
	for (int i = 0; i < N; i++) {
		char * m = new char[1024];
		clGetPlatformInfo(list[i], CL_PLATFORM_VENDOR, sizeof(char) * 1024, m, nullptr);
		std::string vendor = m;
		std::transform(vendor.begin(), vendor.end(), vendor.begin(), ::toupper);
		delete[]m;
		if (vendor.find(name) != std::string::npos)return list[i];
	}
	return nullptr;
}

bool Buffer::create(Context*ctx, cl_mem_flags flag, size_t size)
{
	if (allocated) { release(); }
	cl_int ret;
	object = clCreateBuffer(ctx->getContext(),
		flag,
		size,
		nullptr, &ret);
	if (ret != CL_SUCCESS) {
		ctx->log("Failed to allocated buffer: {}\n",clGetErrorString(ret));
		return false;
	}
	allocated = true; return true;
}

void CoreCL::Buffer::write(Context *ctx, size_t size, void * buffer)
{
	clEnqueueWriteBuffer(ctx->getCommandQueue(), object, CL_TRUE, 0,
		size, buffer, 0, nullptr, nullptr);
}

void CoreCL::Buffer::read(Context *ctx, size_t size, void * buffer)
{
	clEnqueueReadBuffer(ctx->getCommandQueue(),object, CL_TRUE, 0,
		size, buffer, 0, nullptr, nullptr); 
}

void Buffer::release()
{
	clReleaseMemObject(object);
}

void Context::create()
{
	log("Creating OpenCL Context\n");
	cl_platform_id platform_id = nullptr, platform_list[10];
	cl_device_id device_id = nullptr;
	cl_uint ret_num_devices;
	cl_uint ret_num_platforms;
	cl_int ret = clGetPlatformIDs(10, platform_list, &ret_num_platforms);


	if ((platform_id = CoreCL::findPlatformContains("NVidia", platform_list, 10)) == nullptr) {
		if ((platform_id = CoreCL::findPlatformContains("AMD", platform_list, 10)) == nullptr) {
			if ((platform_id = CoreCL::findPlatformContains("Intel", platform_list, 10)) == nullptr) {
				log("no available platform\n", 0);
			}
		}
	}
	
	ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1,
		&device_id, &ret_num_devices);
	char * m = new char[1024];
	clGetDeviceInfo(device_id, CL_DEVICE_NAME, sizeof(char) * 1024, m, nullptr);
	log("Running on {}\n", m);
	delete[]m;
	platform = platform_id;
	device = device_id;
	context = clCreateContext(nullptr,
		1,
		&device_id,
		nullptr,
		nullptr,
		&ret);

	// Create a command queue
	commandQueue = clCreateCommandQueueWithProperties(
		context,
		device_id,
		nullptr,
		&ret);
	if (ret != CL_SUCCESS) {
		log("cannot create command queue, error: {}", clGetErrorString(ret));
	}
}
void Kernel::buildProgram( const char * src, size_t size)
{
	cl_int ret;
	program = clCreateProgramWithSource(ctx->getContext(), 1,
		(const char **)&src, (const size_t *)&size, &ret);
	if (ret != CL_SUCCESS) {
		ctx->log("cannot create program, error: {}", clGetErrorString(ret));
		return;
	}
	auto device = ctx->getDevice();
	ret = clBuildProgram(program, 1, &device,
		"-I. -DGPU_KERNEL  -cl-single-precision-constant -Werror",
		nullptr, nullptr);
	if (ret != CL_SUCCESS) {
		ctx->log("Program Build failed\n");
		size_t length;
		char buffer[40960];
		clGetProgramBuildInfo(program, ctx->getDevice(), 
			CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &length);
		ctx->log("\"--- Build log ---\n{0}\n", buffer);
	}
	// Create the OpenCL kernel

}
#define MAX_SOURCE_SIZE 40960
void Kernel::loadProgram( const char * filename)
{
	FILE *fp;
	char *source_str;
	size_t source_size;

	fp = fopen(filename, "r");
	if (!fp) {
		ctx->log("Failed to load kernel \"%s\".\n", filename);
		return;
	}
	source_str = (char *)malloc(MAX_SOURCE_SIZE);
	source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
	fclose(fp);
	buildProgram(source_str, source_size);
}

void Kernel::createKernel( const char*file, const char* ker)
{
	succ = false;
	ctx->log("Creating kernel {}::{}\n", file, ker);
	loadProgram(file);
	cl_int ret;
	kernel = clCreateKernel(program, ker, &ret);
	if (ret != CL_SUCCESS) {
		ctx->log("error creating kernel {} \n", clGetErrorString(ret));
		return;
	}
	succ = true;
}

void CoreCL::Kernel::operator()()
{
	if (!this->succ)return;
	size_t local_item_size = localWorkSize;
	size_t  global_item_size = globalWorkSize;
	if (global_item_size % local_item_size != 0)
		global_item_size = (global_item_size / local_item_size + 1) * local_item_size;
	cl_int ret = clEnqueueNDRangeKernel(ctx->getCommandQueue(), kernel, 1, nullptr,
		&global_item_size, &local_item_size, 0, nullptr, nullptr);
	if (ret != CL_SUCCESS) {
		ctx->log("Cannot Enqueue NDRangeKernel error: {}\n", clGetErrorString(ret));
	}
	clFinish(ctx->getCommandQueue());
}

void CoreCL::Kernel::setArg(const Buffer & buffer, int i)
{
	clSetKernelArg(kernel, i, sizeof(cl_mem), buffer.getBuffer());
}

bool CoreCL::Image::create(Context *ctx, cl_mem_flags flag, const cl_image_format * format, const cl_image_desc * desc, void * hostptr)
{
	if (allocated) { release(); }
	cl_int ret;
	object = clCreateImage(ctx->getContext(),
		flag, format, desc, hostptr, &ret);
	if (ret != CL_SUCCESS) {
		ctx->log("Failed to allocated buffer: {}\n", clGetErrorString(ret));
		return false;
	}
	allocated = true; return true;
}

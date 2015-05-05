#include <stdio.h>
#include <string.h>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

char const* addKernel = "\n" 
"__kernel void add(                  \n" 
"   __global const uchar* a,         \n" 
"   __global const uchar* b,         \n" 
"   __global uchar* c,               \n" 
"   const unsigned int count)        \n" 
"{                                   \n" 
"   uint idx = get_global_id(0);     \n" 
"   if(idx < count)                  \n" 
"       c[idx] = a[idx] + b[idx];    \n" 
"}                                   \n" 
"\n";


void cl_assert(cl_int error, char const * const message) {
	static char const * const codes[] = {
		"CL_SUCCESS",
		"CL_DEVICE_NOT_FOUND",
		"CL_DEVICE_NOT_AVAILABLE",
		"CL_COMPILER_NOT_AVAILABLE",
		"CL_MEM_OBJECT_ALLOCATION_FAILURE",
		"CL_OUT_OF_RESOURCES",
		"CL_OUT_OF_HOST_MEMORY",
		"CL_PROFILING_INFO_NOT_AVAILABLE",
		"CL_MEM_COPY_OVERLAP",
		"CL_IMAGE_FORMAT_MISMATCH",
		"CL_IMAGE_FORMAT_NOT_SUPPORTED",
		"CL_BUILD_PROGRAM_FAILURE",
		"CL_MAP_FAILURE",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"CL_INVALID_VALUE",
		"CL_INVALID_DEVICE_TYPE",
		"CL_INVALID_PLATFORM",
		"CL_INVALID_DEVICE",
		"CL_INVALID_CONTEXT",
		"CL_INVALID_QUEUE_PROPERTIES",
		"CL_INVALID_COMMAND_QUEUE",
		"CL_INVALID_HOST_PTR",
		"CL_INVALID_MEM_OBJECT",
		"CL_INVALID_IMAGE_FORMAT_DESCRIPTOR",
		"CL_INVALID_IMAGE_SIZE",
		"CL_INVALID_SAMPLER",
		"CL_INVALID_BINARY",
		"CL_INVALID_BUILD_OPTIONS",
		"CL_INVALID_PROGRAM",
		"CL_INVALID_PROGRAM_EXECUTABLE",
		"CL_INVALID_KERNEL_NAME",
		"CL_INVALID_KERNEL_DEFINITION",
		"CL_INVALID_KERNEL",
		"CL_INVALID_ARG_INDEX",
		"CL_INVALID_ARG_VALUE",
		"CL_INVALID_ARG_SIZE",
		"CL_INVALID_KERNEL_ARGS",
		"CL_INVALID_WORK_DIMENSION",
		"CL_INVALID_WORK_GROUP_SIZE",
		"CL_INVALID_WORK_ITEM_SIZE",
		"CL_INVALID_GLOBAL_OFFSET",
		"CL_INVALID_EVENT_WAIT_LIST",
		"CL_INVALID_EVENT",
		"CL_INVALID_OPERATION",
		"CL_INVALID_GL_OBJECT",
		"CL_INVALID_BUFFER_SIZE",
		"CL_INVALID_MIP_LEVEL",
		"CL_INVALID_GLOBAL_WORK_SIZE",
	};
	
	if (error != CL_SUCCESS) {
		printf("%s (%s)\n", message, codes[-error]);
		exit(-1);
	}
}



int main(int argc, char** argv)
{
	char x_data[] = {70, 100, 100,   8, 100,  2, 80, 101, 100, 100, 50, 30, 0};
	char y_data[] = {2,    1,   8, 100,  11, 30,  7,  10,  14,   8, 50,  3, 0};
	size_t const count = sizeof(x_data);

	char *const a_data = malloc(count);
	char *const b_data = malloc(count);
	char *const c_data = malloc(count);

	memcpy(a_data, x_data, count);
	memcpy(b_data, y_data, count);
	memset(c_data,   0x00, count);

	cl_platform_id platforms[32];
	cl_uint num_platforms;
	char vendor[1024];
	cl_device_id devices[32];
	cl_uint num_devices;
	char deviceName[1024];
	cl_int err;
	
        err = clGetPlatformIDs(32, platforms, &num_platforms);
	cl_assert(err, "There was a problem getting the platforms");
	for(size_t p = 0; p < num_platforms; ++p) {
		cl_platform_id platform = platforms[p];
		clGetPlatformInfo (platform, CL_PLATFORM_VENDOR, sizeof(vendor), vendor, NULL);
		printf("Platform Vendor:\t%s\n", vendor);

		err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 32, devices, &num_devices);
		cl_assert(err, "There was a problem getting the device list");

		cl_context context = clCreateContext(NULL, num_devices, devices, NULL, NULL, &err);
		cl_assert(err, "There was a problem creating the context.");

		cl_program program = clCreateProgramWithSource(context, 1, &addKernel, NULL, &err);
		cl_assert(err, "There was a problem creating the program.");

		err = clBuildProgram(program, num_devices, devices, NULL, NULL, NULL);
		for(size_t d = 0; d < num_devices; ++d) {
			cl_device_id device = devices[d];
			char buffer[2048];
			size_t length = 0;
			clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 2048, buffer, &length);
			if (length > 1)
				printf("%s\n", buffer);
			cl_assert(err, "There was a problem building the program.");
		}


		cl_kernel kernel = clCreateKernel(program, "add", &err);
		cl_assert(err, "There was a problem getting the kernel.");

		cl_mem a_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,  
				sizeof(char) * count, a_data, &err);
		cl_assert(err, "There was a problem creating the a_buffer.");
		cl_mem b_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,  
				sizeof(char) * count, b_data, &err);
		cl_assert(err, "There was a problem creating the b_buffer.");
		cl_mem c_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, 
				sizeof(char) * count, c_data, &err);
		cl_assert(err, "There was a problem creating the c_buffer");


		err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &a_buffer);
		err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &b_buffer);
		err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &c_buffer);
		err |= clSetKernelArg(kernel, 3, sizeof(unsigned int), &count);
		cl_assert(err, "There was a problem setting the arguments.");

		for(size_t d = 0; d < num_devices; ++d) {
			cl_device_id device = devices[d];

			clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(deviceName), deviceName, NULL);
			printf("  Device Name:\t%s\n", deviceName);

			cl_command_queue_properties properties = CL_QUEUE_PROFILING_ENABLE;
			cl_command_queue commands = clCreateCommandQueue(context, device, properties, &err);
			cl_assert(err, "There was a problem creating the command queue");

			size_t local[]  = { count };
			size_t global[] = { count };
			cl_event event;
			err = clEnqueueNDRangeKernel(commands, kernel, 1, NULL, global, local, 0, NULL, &event);
			cl_assert(err, "There was a problem queueing the kernel.");

			err = clEnqueueReadBuffer(commands, c_buffer, CL_TRUE, 0, sizeof(char) * count, 
					c_data, 0, NULL, NULL);  
			cl_assert(err, "There was a problem reading the output buffer.");

			clFinish(commands);
			cl_ulong start, stop;
			err = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(start), &start, NULL);
			err |= clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(stop), &stop, NULL);
			cl_assert(err, "There was a problem getting profiling information.");
			printf("  Time: \t%lu ns.\n", stop - start);
			printf("  Output: \t%s\n", c_data);
			printf("\n");
			clReleaseCommandQueue(commands);

		}

		clReleaseMemObject(a_buffer);
		clReleaseMemObject(b_buffer);
		clReleaseMemObject(c_buffer);

		clReleaseProgram(program);
		clReleaseKernel(kernel);
		clReleaseContext(context);
	}

        free(a_data);
	free(b_data);
	free(c_data);

	return 0;
}

#include <cstdlib>
#include <string>
#include <fstream>
#include <streambuf>

#include "CL/cl.h"

std::string OpenclErrorCodeToString(const cl_int code)
{
  switch(code)
  {
    case CL_SUCCESS: return "CL_SUCCESS";
    case CL_DEVICE_NOT_FOUND: return "CL_DEVICE_NOT_FOUND";
    case CL_DEVICE_NOT_AVAILABLE: return "CL_DEVICE_NOT_AVAILABLE";
    case CL_COMPILER_NOT_AVAILABLE: return "CL_COMPILER_NOT_AVAILABLE";
    case CL_MEM_OBJECT_ALLOCATION_FAILURE: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
    case CL_OUT_OF_RESOURCES: return "CL_OUT_OF_RESOURCES";
    case CL_OUT_OF_HOST_MEMORY: return "CL_OUT_OF_HOST_MEMORY";
    case CL_PROFILING_INFO_NOT_AVAILABLE: return "CL_PROFILING_INFO_NOT_AVAILABLE";
    case CL_MEM_COPY_OVERLAP: return "CL_MEM_COPY_OVERLAP";
    case CL_IMAGE_FORMAT_MISMATCH: return "CL_IMAGE_FORMAT_MISMATCH";
    case CL_IMAGE_FORMAT_NOT_SUPPORTED: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
    case CL_BUILD_PROGRAM_FAILURE: return "CL_BUILD_PROGRAM_FAILURE";
    case CL_MAP_FAILURE: return "CL_MAP_FAILURE";
    case CL_INVALID_VALUE: return "CL_INVALID_VALUE";
    case CL_INVALID_DEVICE_TYPE: return "CL_INVALID_DEVICE_TYPE";
    case CL_INVALID_PLATFORM: return "CL_INVALID_PLATFORM";
    case CL_INVALID_DEVICE: return "CL_INVALID_DEVICE";
    case CL_INVALID_CONTEXT: return "CL_INVALID_CONTEXT";
    case CL_INVALID_QUEUE_PROPERTIES: return "CL_INVALID_QUEUE_PROPERTIES";
    case CL_INVALID_COMMAND_QUEUE: return "CL_INVALID_COMMAND_QUEUE";
    case CL_INVALID_HOST_PTR: return "CL_INVALID_HOST_PTR";
    case CL_INVALID_MEM_OBJECT: return "CL_INVALID_MEM_OBJECT";
    case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
    case CL_INVALID_IMAGE_SIZE: return "CL_INVALID_IMAGE_SIZE";
    case CL_INVALID_SAMPLER: return "CL_INVALID_SAMPLER";
    case CL_INVALID_BINARY: return "CL_INVALID_BINARY";
    case CL_INVALID_BUILD_OPTIONS: return "CL_INVALID_BUILD_OPTIONS";
    case CL_INVALID_PROGRAM: return "CL_INVALID_PROGRAM";
    case CL_INVALID_PROGRAM_EXECUTABLE: return "CL_INVALID_PROGRAM_EXECUTABLE";
    case CL_INVALID_KERNEL_NAME: return "CL_INVALID_KERNEL_NAME";
    case CL_INVALID_KERNEL_DEFINITION: return "CL_INVALID_KERNEL_DEFINITION";
    case CL_INVALID_KERNEL: return "CL_INVALID_KERNEL";
    case CL_INVALID_ARG_INDEX: return "CL_INVALID_ARG_INDEX";
    case CL_INVALID_ARG_VALUE: return "CL_INVALID_ARG_VALUE";
    case CL_INVALID_ARG_SIZE: return "CL_INVALID_ARG_SIZE";
    case CL_INVALID_KERNEL_ARGS: return "CL_INVALID_KERNEL_ARGS";
    case CL_INVALID_WORK_DIMENSION: return "CL_INVALID_WORK_DIMENSION";
    case CL_INVALID_WORK_GROUP_SIZE: return "CL_INVALID_WORK_GROUP_SIZE";
    case CL_INVALID_WORK_ITEM_SIZE: return "CL_INVALID_WORK_ITEM_SIZE";
    case CL_INVALID_GLOBAL_OFFSET: return "CL_INVALID_GLOBAL_OFFSET";
    case CL_INVALID_EVENT_WAIT_LIST: return "CL_INVALID_EVENT_WAIT_LIST";
    case CL_INVALID_EVENT: return "CL_INVALID_EVENT";
    case CL_INVALID_OPERATION: return "CL_INVALID_OPERATION";
    case CL_INVALID_GL_OBJECT: return "CL_INVALID_GL_OBJECT";
    case CL_INVALID_BUFFER_SIZE: return "CL_INVALID_BUFFER_SIZE";
    case CL_INVALID_MIP_LEVEL: return "CL_INVALID_MIP_LEVEL";
    case CL_INVALID_GLOBAL_WORK_SIZE: return "CL_INVALID_GLOBAL_WORK_SIZE";

    default: return "UNKNOWN CL ERROR CODE";
  }
}

void CheckOpenclCall(const cl_int status, std::string operation)
{
  if(status != CL_SUCCESS)
  {
    printf("%s returned error: %s.\n", operation.c_str(), OpenclErrorCodeToString(status).c_str());
  }
}

std::string LoadFile(const std::string &filename)
{
  std::ifstream f(filename);
  std::string result((std::istreambuf_iterator<char>(f)),
                     std::istreambuf_iterator<char>());
  return result;
}

int main()
{
  cl_uint platformId = 0;
  cl_uint deviceId = 0;
  std::string programSourceFileName = "skeincoin.cl";
  std::string kernelName = "search";

  printf("Start.\n");

  std::string programSource = LoadFile(programSourceFileName);

  cl_int status;

  cl_uint platformsCnt;
  CheckOpenclCall(clGetPlatformIDs(0, nullptr, &platformsCnt), "clGetPlatformIDs get number of platforms");

  cl_platform_id *platforms = new cl_platform_id[platformsCnt];
  CheckOpenclCall(clGetPlatformIDs(platformsCnt, platforms, nullptr), "clGetPlatformIDs get platforms");
  cl_platform_id platform = platforms[platformId];

  cl_uint devicesCnt;
  CheckOpenclCall(clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, nullptr, &devicesCnt), "clGetDeviceIDs. Getting number of devices on platform");
  cl_device_id *devices = new cl_device_id[devicesCnt];
  CheckOpenclCall(clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, devicesCnt, devices, nullptr), "clGetDeviceIDs. Getting devices");
  cl_device_id device = devices[deviceId];

  cl_context_properties contextProperties[3] = {CL_CONTEXT_PLATFORM, cl_context_properties(platform), 0};
  cl_context context = clCreateContext(contextProperties, devicesCnt, devices, nullptr, nullptr, &status);
  CheckOpenclCall(status, "clCreateContext");

  const char *programSourceStrings = programSource.c_str();
  cl_program program = clCreateProgramWithSource(context, 1, &programSourceStrings, nullptr, &status);
  CheckOpenclCall(status, "clCreateProgramWithSource");

  std::string compilerOptions = "-D WORKSIZE=64";
  status = clBuildProgram(program, 1, &device, compilerOptions.c_str(), nullptr, nullptr);
  if(status != CL_SUCCESS)
  {
    static char buf[0x10000] = { 0 };
    CheckOpenclCall(clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0x10000, buf, nullptr), "clGetProgramBuildInfo");
    printf("clBuildProgram error log: %s", buf);
    CheckOpenclCall(status, "clBuildProgram"); // show error
  }

  cl_kernel kernel = clCreateKernel(program, kernelName.c_str(), &status);
  CheckOpenclCall(status, "clCreateKernel OpenclKernel");

  printf("I'm done.\n");

  clReleaseKernel(kernel);
  clReleaseProgram(program);
  clReleaseContext(context);

  for(unsigned int i = 0; i < devicesCnt; i++)
  {
    clReleaseDevice(devices[i]);
  }
  delete[] devices;

  delete[] platforms;

  return 0;
}

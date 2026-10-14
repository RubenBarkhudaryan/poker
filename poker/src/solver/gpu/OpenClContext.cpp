#include "src/solver/gpu/OpenClContext.h"
#include "include/tools/tinyformat.h"
#include <stdexcept>

void clCheck(cl_int err, const char* what) {
    if (err != CL_SUCCESS)
        throw runtime_error(tfm::format("OpenCL error %s in %s", err, what));
}

namespace {

string deviceString(cl_device_id device, cl_device_info info) {
    size_t size = 0;
    clCheck(clGetDeviceInfo(device, info, 0, nullptr, &size), "clGetDeviceInfo");
    string value(size, '\0');
    clCheck(clGetDeviceInfo(device, info, size, &value[0], nullptr), "clGetDeviceInfo");
    while (!value.empty() && value.back() == '\0')
        value.pop_back();
    return value;
}

template <typename T>
T deviceValue(cl_device_id device, cl_device_info info) {
    T value{};
    clCheck(clGetDeviceInfo(device, info, sizeof(T), &value, nullptr), "clGetDeviceInfo");
    return value;
}

// Higher is better: GPUs first, discrete before integrated, then by memory.
long double deviceScore(cl_device_id device) {
    cl_device_type type = deviceValue<cl_device_type>(device, CL_DEVICE_TYPE);
    cl_bool unified = deviceValue<cl_bool>(device, CL_DEVICE_HOST_UNIFIED_MEMORY);
    cl_ulong memory = deviceValue<cl_ulong>(device, CL_DEVICE_GLOBAL_MEM_SIZE);
    long double score = (long double)memory;
    if (type & CL_DEVICE_TYPE_GPU)
        score += 1e20L;
    if (!unified)
        score += 1e19L;
    return score;
}

} // namespace

OpenClContext::OpenClContext() {
    cl_uint platform_count = 0;
    if (clGetPlatformIDs(0, nullptr, &platform_count) != CL_SUCCESS || platform_count == 0)
        throw runtime_error("no OpenCL platform found");
    vector<cl_platform_id> platforms(platform_count);
    clCheck(clGetPlatformIDs(platform_count, platforms.data(), nullptr), "clGetPlatformIDs");

    long double best_score = -1;
    for (cl_platform_id platform : platforms) {
        cl_uint device_count = 0;
        if (clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, nullptr, &device_count) != CL_SUCCESS)
            continue;
        vector<cl_device_id> devices(device_count);
        clCheck(clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, device_count, devices.data(), nullptr),
                "clGetDeviceIDs");
        for (cl_device_id device : devices) {
            if (!deviceValue<cl_bool>(device, CL_DEVICE_AVAILABLE))
                continue;
            long double score = deviceScore(device);
            if (score > best_score) {
                best_score = score;
                device_ = device;
            }
        }
    }
    if (device_ == nullptr)
        throw runtime_error("no OpenCL device found");

    cl_int err;
    context_ = clCreateContext(nullptr, 1, &device_, nullptr, nullptr, &err);
    clCheck(err, "clCreateContext");
    queue_ = clCreateCommandQueue(context_, device_, 0, &err);
    clCheck(err, "clCreateCommandQueue");

    global_mem_size_ = deviceValue<cl_ulong>(device_, CL_DEVICE_GLOBAL_MEM_SIZE);
    max_alloc_size_ = deviceValue<cl_ulong>(device_, CL_DEVICE_MAX_MEM_ALLOC_SIZE);
    local_mem_size_ = deviceValue<cl_ulong>(device_, CL_DEVICE_LOCAL_MEM_SIZE);
    max_work_group_size_ = deviceValue<size_t>(device_, CL_DEVICE_MAX_WORK_GROUP_SIZE);
    description_ = tfm::format("%s (%s, %s MB)", deviceString(device_, CL_DEVICE_NAME),
                               deviceString(device_, CL_DEVICE_VERSION), global_mem_size_ / (1024 * 1024));
}

OpenClContext::~OpenClContext() {
    if (queue_ != nullptr)
        clReleaseCommandQueue(queue_);
    if (context_ != nullptr)
        clReleaseContext(context_);
}

cl_program OpenClContext::buildProgram(const char* source, const string& options) {
    cl_int err;
    cl_program program = clCreateProgramWithSource(context_, 1, &source, nullptr, &err);
    clCheck(err, "clCreateProgramWithSource");
    err = clBuildProgram(program, 1, &device_, options.c_str(), nullptr, nullptr);
    if (err != CL_SUCCESS) {
        size_t size = 0;
        clGetProgramBuildInfo(program, device_, CL_PROGRAM_BUILD_LOG, 0, nullptr, &size);
        string log(size, '\0');
        clGetProgramBuildInfo(program, device_, CL_PROGRAM_BUILD_LOG, size, &log[0], nullptr);
        clReleaseProgram(program);
        throw runtime_error(tfm::format("OpenCL program build failed (%s):\n%s", err, log));
    }
    return program;
}

cl_kernel OpenClContext::createKernel(cl_program program, const char* name) {
    cl_int err;
    cl_kernel kernel = clCreateKernel(program, name, &err);
    clCheck(err, name);
    return kernel;
}

cl_mem OpenClContext::createBuffer(size_t bytes, const void* host_data) {
    if (bytes > max_alloc_size_)
        throw runtime_error(tfm::format("OpenCL buffer of %s MB exceeds the device limit of %s MB",
                                        bytes / (1024 * 1024), max_alloc_size_ / (1024 * 1024)));
    cl_int err;
    cl_mem_flags flags = CL_MEM_READ_WRITE | (host_data != nullptr && bytes > 0 ? CL_MEM_COPY_HOST_PTR : 0);
    cl_mem buffer = clCreateBuffer(context_, flags, bytes > 0 ? bytes : 1,
                                   bytes > 0 ? const_cast<void*>(host_data) : nullptr, &err);
    clCheck(err, "clCreateBuffer");
    return buffer;
}

void OpenClContext::write(cl_mem buffer, const void* data, size_t bytes, size_t offset, bool blocking) {
    if (bytes == 0)
        return;
    clCheck(clEnqueueWriteBuffer(queue_, buffer, blocking ? CL_TRUE : CL_FALSE, offset, bytes, data, 0, nullptr,
                                 nullptr),
            "clEnqueueWriteBuffer");
}

void OpenClContext::read(cl_mem buffer, void* data, size_t bytes, size_t offset) {
    if (bytes == 0)
        return;
    clCheck(clEnqueueReadBuffer(queue_, buffer, CL_TRUE, offset, bytes, data, 0, nullptr, nullptr),
            "clEnqueueReadBuffer");
}

void OpenClContext::finish() {
    clCheck(clFinish(queue_), "clFinish");
}

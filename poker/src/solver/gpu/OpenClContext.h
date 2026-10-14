#ifndef TEXASSOLVER_OPENCLCONTEXT_H
#define TEXASSOLVER_OPENCLCONTEXT_H

#include <CL/cl.h>
#include <cstddef>
#include <string>
#include <vector>

using namespace std;

void clCheck(cl_int err, const char* what);

// Owns an OpenCL context and in-order queue on the best available device (a discrete GPU when there is one).
class OpenClContext {
public:
    OpenClContext();
    ~OpenClContext();
    OpenClContext(const OpenClContext&) = delete;
    OpenClContext& operator=(const OpenClContext&) = delete;

    cl_program buildProgram(const char* source, const string& options);
    cl_kernel createKernel(cl_program program, const char* name);
    // Allocates a buffer of at least one byte, so empty levels still have a valid argument.
    cl_mem createBuffer(size_t bytes, const void* host_data = nullptr);
    void write(cl_mem buffer, const void* data, size_t bytes, size_t offset = 0, bool blocking = true);
    void read(cl_mem buffer, void* data, size_t bytes, size_t offset = 0);
    void finish();

    const string& description() const {
        return description_;
    }

    cl_command_queue queue() const {
        return queue_;
    }

    cl_ulong globalMemSize() const {
        return global_mem_size_;
    }

    cl_ulong maxAllocSize() const {
        return max_alloc_size_;
    }

    cl_ulong localMemSize() const {
        return local_mem_size_;
    }

    size_t maxWorkGroupSize() const {
        return max_work_group_size_;
    }

private:
    cl_device_id device_ = nullptr;
    cl_context context_ = nullptr;
    cl_command_queue queue_ = nullptr;
    string description_;
    cl_ulong global_mem_size_ = 0;
    cl_ulong max_alloc_size_ = 0;
    cl_ulong local_mem_size_ = 0;
    size_t max_work_group_size_ = 0;
};

// Sets every argument of a kernel in order.
template <typename... Args>
void setKernelArgs(cl_kernel kernel, const Args&... args) {
    cl_uint index = 0;
    (clCheck(clSetKernelArg(kernel, index++, sizeof(Args), &args), "clSetKernelArg"), ...);
}

#endif //TEXASSOLVER_OPENCLCONTEXT_H

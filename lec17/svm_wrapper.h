/*
 * Author: Vivek Kumar
 * vivekk@iiitd.ac.in
 */

/*
 * This wrapper is to avoid explicit typecasting of the SVM pointer in host side. 
 * Otherwise, the Boost APIs require following steps over Intel's integrated CPU-GPU:
 *
 * 1) Allocating/deallocating integer array of size 'N':
 * compute::svm_ptr<cl_int> array = compute::svm_alloc<cl_int>(context, N);
 * compute::svm_free(context, array);
 * 2) Accessing this svm pointer in host side:
 * for(int i = 0; i<N; i ++) {
 *   static_cast<cl_int*>(array.get())[i] = i;
 * }
 * 3) Setting kernel args:
 * kernel.set_arg(0, array)
 */
namespace my_svm {
  template <typename T>
  T* alloc(boost::compute::context context, size_t size) {
    return (T*) clSVMAlloc(context, CL_MEM_READ_WRITE, size*sizeof(T), 0);
  }
  template<typename T>
  void free(boost::compute::context context, T* ptr) {
    clSVMFree(context, ptr);
  }
}

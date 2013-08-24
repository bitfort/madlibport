// port of MADlib to Imapala
// author: victor bittorf (bittorf@cs.wisc.edu)

namespace madlib {
namespace port {
namespace dbconn {

/*! \brief a basic array type
 * MADlib expects an array type that can be treated as a POD. The Metaport
 * of MADlib expects this type by name with accessor functions, below. MADlib
 * uses macros to access this structure, Metaport defines these macros as calls
 * to the accessor functions. 
 */
struct PortArrayType {
  size_t len; //!< the length in bytes of the array 
  size_t ndims; //!< the number of dimensions; valid entires in dims[]
  size_t dims[10]; //!< dims[i] is the lenght of the dimension 
  void* ptr; //!< pointer to backing memory
};

/*! \brief This is used by MADlib to get and set the length of dimmensions
 */
size_t* ArrayDims(const PortArrayType *a) {
  // XXX TODO FIXME ugly comptability hack to work with MADlib
  PortArrayType *a_nonc = const_cast<PortArrayType*>(a);
  return a_nonc->dims;
}
/*! \brief Used by MADlib to get the number of dimensions
 */
size_t ArrayNDims(const PortArrayType *a) {
  return a->ndims;
}

/*! \brief Gets the base pointer of the array
 */
void* ArrayPointer(const PortArrayType *a) {
  return a->ptr;
}

} // namespace dbconn
} // namespace port
} // namespace madlib

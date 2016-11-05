// include/checkvec : vector class with checked limits
#ifndef CHECKVEC_H 
#define CHECKVEC_H

#include<cassert>
#include<vector>

using std::vector;


template<class T>
class checkedVector : public vector<T>   // inherit from std::vector<T>
{
   public:
      // type names like iterator etc. are also inherited
      checkedVector()
      {}

      checkedVector(typename checkedVector::size_type n, const T& value = T())
      : vector<T>(n, value)
      {}

      checkedVector(typename checkedVector::iterator i, typename checkedVector::iterator j)
      : vector<T>(i, j)
      {}

	typename checkedVector::reference operator[](typename checkedVector::difference_type index)
      {
         assert(index >=0 
             && index < static_cast<typename checkedVector::difference_type>(size()));
         return vector<T>::operator[](index);
      }

	typename checkedVector::const_reference operator[](typename checkedVector::difference_type index) const
      {
         assert(index >=0 
             && index < static_cast<typename checkedVector::difference_type>(size()));
         return vector<T>::operator[](index);
      }
};
#endif

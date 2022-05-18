#include "hoNDArray_operators.h"
#include "hoArmadillo.h"

namespace Gadgetron{

    // Private utility to verify array dimensions. 
    // It "replaces" NDArray::dimensions_equal() to support batch mode.
    // There is an identical function for all array instances (currently hoNDArray, cuNDArray, hoCuNDAraay)
    // !!! Remember to fix any bugs in all versions !!!
    //
    template<class T,class S> static bool compatible_dimensions( const hoNDArray<T> &x, const hoNDArray<S> &y )
    {
        return ((x.get_number_of_elements()%y.get_number_of_elements())==0);
    }

    template<class T> hoNDArray<T>& operator+= (hoNDArray<T> &x, const hoNDArray<T> &y)
    {
        if( compatible_dimensions<T,T>(x,y) ){
            arma::Col<typename stdType<T>::Type> aY = as_arma_col(y);
            size_t num_batches = x.get_number_of_elements()/y.get_number_of_elements();
            for( size_t batch=0; batch<num_batches; batch++ ){	
                hoNDArray<T> tmp;
                tmp.create( y.get_dimensions(), x.get_data_ptr()+batch*y.get_number_of_elements() );
                arma::Col<typename stdType<T>::Type> aRes = as_arma_col(tmp);
                aRes += aY;
            }
            return x;    
        } 
        else {
            throw std::runtime_error("hoNDArray::operator+=: Incompatible array dimensions");
        }
    }

    template<class T> hoNDArray< std::complex<T> >& operator+= (hoNDArray< std::complex<T> > &x, const hoNDArray<T> &y)
    {
        if( compatible_dimensions<std::complex<T>,T>(x,y) ){
            arma::Col< std::complex<T> > aY( as_arma_col(y), arma::Col<T>(y.get_number_of_elements()).zeros() );
            size_t num_batches = x.get_number_of_elements()/y.get_number_of_elements();
            for( size_t batch=0; batch<num_batches; batch++ ){	
                hoNDArray< std::complex<T> > tmp;
                tmp.create( y.get_dimensions(), x.get_data_ptr()+batch*y.get_number_of_elements() );
                arma::Col< std::complex<T> > aRes = as_arma_col(tmp);
                aRes += aY;
            }
            return x;
        } 
        else {
            throw std::runtime_error("hoNDArray::operator+=: Incompatible array dimensions");
        }
    }

    template<class T> hoNDArray< complext<T> >& operator+= (hoNDArray< complext<T> > &x, const hoNDArray<T> &y)
    {
        if( compatible_dimensions<complext<T>,T>(x,y) ){
            arma::Col< std::complex<T> > aY( as_arma_col(y), arma::Col<T>(y.get_number_of_elements()).zeros() );
            size_t num_batches = x.get_number_of_elements()/y.get_number_of_elements();
            for( size_t batch=0; batch<num_batches; batch++ ){	
                hoNDArray< complext<T> > tmp;
                tmp.create( y.get_dimensions(), x.get_data_ptr()+batch*y.get_number_of_elements() );
                arma::Col< std::complex<T> > aRes = as_arma_col(tmp);
                aRes += aY;
            }
            return x;
        } 
        else {
            throw std::runtime_error("hoNDArray::operator+=: Incompatible array dimensions");
        }
    }

    template<class T> hoNDArray<T>& operator+= (hoNDArray<T> &x, const T &y)
    {
        arma::Col<typename stdType<T>::Type> aRes = as_arma_col(x);
        typename stdType<T>::Type aY = *((typename stdType<T>::Type*)&y);
        aRes += aY;
        return x;  
    }

    template<class T> hoNDArray< std::complex<T> >& operator+= (hoNDArray< std::complex<T> > &x, const T &y)
    {
        arma::Col< std::complex<T> > aRes = as_arma_col(x);
        std::complex<T> aY( y, T(0) );
        aRes += aY;
        return x;  
    }

    template<class T> hoNDArray< complext<T> >& operator+= (hoNDArray< complext<T> > &x, const T &y)
    {
        arma::Col< std::complex<T> > aRes = as_arma_col(x);
        std::complex<T> aY( y, T(0) );
        aRes += aY;
        return x;  
    }

    template<class T> hoNDArray<T>& operator-= (hoNDArray<T> &x, const hoNDArray<T> &y)
    {
        if( compatible_dimensions<T,T>(x,y) ){
            arma::Col<typename stdType<T>::Type> aY = as_arma_col(y);
            size_t num_batches = x.get_number_of_elements()/y.get_number_of_elements();
            for( size_t batch=0; batch<num_batches; batch++ ){	
                hoNDArray<T> tmp;
                tmp.create( y.get_dimensions(), x.get_data_ptr()+batch*y.get_number_of_elements() );
                arma::Col<typename stdType<T>::Type> aRes = as_arma_col(tmp);
                aRes -= aY;
            }
            return x;    
        } 
        else {
            throw std::runtime_error("hoNDArray::operator-=: Incompatible array dimensions");
        }
    }

    template<class T> hoNDArray< std::complex<T> >& operator-= (hoNDArray< std::complex<T> > &x, const hoNDArray<T> &y)
    {
        if( compatible_dimensions<std::complex<T>,T>(x,y) ){
            arma::Col< std::complex<T> > aY( as_arma_col(y), arma::Col<T>(y.get_number_of_elements()).zeros() );
            size_t num_batches = x.get_number_of_elements()/y.get_number_of_elements();
            for( size_t batch=0; batch<num_batches; batch++ ){	
                hoNDArray< std::complex<T> > tmp;
                tmp.create( y.get_dimensions(), x.get_data_ptr()+batch*y.get_number_of_elements() );
                arma::Col< std::complex<T> > aRes = as_arma_col(tmp);
                aRes -= aY;
            }
            return x;
        } 
        else {
            throw std::runtime_error("hoNDArray::operator-=: Incompatible array dimensions");
        }
    }

    template<class T> hoNDArray< complext<T> >& operator-= (hoNDArray< complext<T> > &x, const hoNDArray<T> &y)
    {
        if( compatible_dimensions<complext<T>,T>(x,y) ){
            arma::Col< std::complex<T> > aY( as_arma_col(y), arma::Col<T>(y.get_number_of_elements()).zeros() );
            size_t num_batches = x.get_number_of_elements()/y.get_number_of_elements();
            for( size_t batch=0; batch<num_batches; batch++ ){	
                hoNDArray< complext<T> > tmp;
                tmp.create( y.get_dimensions(), x.get_data_ptr()+batch*y.get_number_of_elements() );
                arma::Col< std::complex<T> > aRes = as_arma_col(tmp);
                aRes -= arma::Col< std::complex<T> >( as_arma_col(y), arma::Col<T>(y.get_number_of_elements()).zeros() );
            }
            return x;
        } 
        else {
            throw std::runtime_error("hoNDArray::operator-=: Incompatible array dimensions");
        }
    }

    template<class T> hoNDArray<T>& operator-= (hoNDArray<T> &x, const T &y)
    {
        arma::Col<typename stdType<T>::Type> aRes = as_arma_col(x);
        typename stdType<T>::Type aY = *((typename stdType<T>::Type*)&y);
        aRes -= aY;
        return x;  
    }

    template<class T> hoNDArray< std::complex<T> >& operator-= (hoNDArray< std::complex<T> > &x, const T &y)
    {
        arma::Col< std::complex<T> > aRes = as_arma_col(x);
        std::complex<T> aY( y, T(0) );
        aRes -= aY;
        return x;  
    }

    template<class T> hoNDArray< complext<T> >& operator-= (hoNDArray< complext<T> > &x, const T &y)
    {
        arma::Col< std::complex<T> > aRes = as_arma_col(x);
        std::complex<T> aY( y, T(0) );
        aRes -= aY;
        return x;  
    }

    template<class T> hoNDArray<T>& operator*= (hoNDArray<T> &x, const hoNDArray<T> &y)
    {
        if( compatible_dimensions<T,T>(x,y) ){
            arma::Col<typename stdType<T>::Type> aY = as_arma_col(y);
            size_t num_batches = x.get_number_of_elements()/y.get_number_of_elements();
            for( size_t batch=0; batch<num_batches; batch++ ){	
                hoNDArray<T> tmp;
                tmp.create( y.get_dimensions(), x.get_data_ptr()+batch*y.get_number_of_elements() );
                arma::Col<typename stdType<T>::Type> aRes = as_arma_col(tmp);
                aRes %= aY;
            }
            return x;
        } 
        else {
            throw std::runtime_error("hoNDArray::operator*=: Incompatible array dimensions");
        }
    }

    template<class T> hoNDArray< std::complex<T> >& operator*= (hoNDArray< std::complex<T> > &x, const hoNDArray<T> &y)
    {
        if( compatible_dimensions<std::complex<T>,T>(x,y) ){
            arma::Col< std::complex<T> > aY( as_arma_col(y), arma::Col<T>(y.get_number_of_elements()).zeros() );
            size_t num_batches = x.get_number_of_elements()/y.get_number_of_elements();
            for( size_t batch=0; batch<num_batches; batch++ ){	
                hoNDArray< std::complex<T> > tmp;
                tmp.create( y.get_dimensions(), x.get_data_ptr()+batch*y.get_number_of_elements() );
                arma::Col< std::complex<T> > aRes = as_arma_col(tmp);
                aRes %= aY;
            }
            return x;
        } 
        else {
            throw std::runtime_error("hoNDArray::operator*=: Incompatible array dimensions");
        }
    }

    template<class T> hoNDArray< complext<T> >& operator*= (hoNDArray< complext<T> > &x, const hoNDArray<T> &y)
    {
        if( compatible_dimensions<complext<T>,T>(x,y) ){
            arma::Col< std::complex<T> > aY( as_arma_col(y), arma::Col<T>(y.get_number_of_elements()).zeros() );
            size_t num_batches = x.get_number_of_elements()/y.get_number_of_elements();
            for( size_t batch=0; batch<num_batches; batch++ ){	
                hoNDArray< complext<T> > tmp;
                tmp.create( y.get_dimensions(), x.get_data_ptr()+batch*y.get_number_of_elements() );
                arma::Col< std::complex<T> > aRes = as_arma_col(tmp);
                aRes %= arma::Col< std::complex<T> >( as_arma_col(y), arma::Col<T>(y.get_number_of_elements()).zeros() );
            }
            return x;
        } 
        else {
            throw std::runtime_error("hoNDArray::operator*=: Incompatible array dimensions");
        }
    }

    template<class T> hoNDArray<T>& operator*= (hoNDArray<T> &x, const T &y)
    {
        arma::Col<typename stdType<T>::Type> aRes = as_arma_col(x);
        typename stdType<T>::Type aY = *((typename stdType<T>::Type*)&y);
        aRes *= aY;
        return x;  
    }

    template<class T> hoNDArray< std::complex<T> >& operator*= (hoNDArray< std::complex<T> > &x, const T &y)
    {
        arma::Col< std::complex<T> > aRes = as_arma_col(x);
        std::complex<T> aY( y, T(0) );
        aRes *= aY;
        return x;  
    }

    template<class T> hoNDArray< complext<T> >& operator*= (hoNDArray< complext<T> > &x, const T &y)
    {
        arma::Col< std::complex<T> > aRes = as_arma_col(x);
        std::complex<T> aY( y, T(0) );
        aRes *= aY;
        return x;  
    }

    template<class T> hoNDArray<T>& operator/= (hoNDArray<T> &x, const hoNDArray<T> &y)
    {
        if( compatible_dimensions<T,T>(x,y) ){
            arma::Col<typename stdType<T>::Type> aY = as_arma_col(y);
            size_t num_batches = x.get_number_of_elements()/y.get_number_of_elements();
            for( size_t batch=0; batch<num_batches; batch++ ){	
                hoNDArray<T> tmp;
                tmp.create( y.get_dimensions(), x.get_data_ptr()+batch*y.get_number_of_elements() );
                arma::Col<typename stdType<T>::Type> aRes = as_arma_col(tmp);
                aRes /= aY;
            }
            return x;
        } 
        else {
            throw std::runtime_error("hoNDArray::operator/=: Incompatible array dimensions");
        }
    }

    template<class T> hoNDArray< std::complex<T> >& operator/= (hoNDArray< std::complex<T> > &x, const hoNDArray<T> &y)
    {
        if( compatible_dimensions<std::complex<T>,T>(x,y) ){
            arma::Col< std::complex<T> > aY( as_arma_col(y), arma::Col<T>(y.get_number_of_elements()).zeros() );
            size_t num_batches = x.get_number_of_elements()/y.get_number_of_elements();
            for( size_t batch=0; batch<num_batches; batch++ ){	
                hoNDArray< std::complex<T> > tmp;
                tmp.create( y.get_dimensions(), x.get_data_ptr()+batch*y.get_number_of_elements() );
                arma::Col< std::complex<T> > aRes = as_arma_col(tmp);
                aRes /= aY;
            }
            return x;
        } 
        else {
            throw std::runtime_error("hoNDArray::operator/=: Incompatible array dimensions");
        }
    }

    template<class T> hoNDArray< complext<T> >& operator/= (hoNDArray< complext<T> > &x, const hoNDArray<T> &y)
    {
        if( compatible_dimensions<complext<T>,T>(x,y) ){
            arma::Col< std::complex<T> > aY( as_arma_col(y), arma::Col<T>(y.get_number_of_elements()).zeros() );
            size_t num_batches = x.get_number_of_elements()/y.get_number_of_elements();
            for( size_t batch=0; batch<num_batches; batch++ ){	
                hoNDArray< complext<T> > tmp;
                tmp.create( y.get_dimensions(), x.get_data_ptr()+batch*y.get_number_of_elements() );
                arma::Col< std::complex<T> > aRes = as_arma_col(tmp);
                aRes /= arma::Col< std::complex<T> >( as_arma_col(y), arma::Col<T>(y.get_number_of_elements()).zeros() );
            }
            return x;
        } 
        else {
            throw std::runtime_error("hoNDArray::operator/=: Incompatible array dimensions");
        }
    }

    template<class T> hoNDArray<T>& operator/= (hoNDArray<T> &x, const T &y)
    {
        arma::Col<typename stdType<T>::Type> aRes = as_arma_col(x);
        typename stdType<T>::Type aY = *((typename stdType<T>::Type*)&y);
        aRes /= aY;
        return x;  
    }

    template<class T> hoNDArray< std::complex<T> >& operator/= (hoNDArray< std::complex<T> > &x, const T &y)
    {
        arma::Col< std::complex<T> > aRes = as_arma_col(x);
        std::complex<T> aY( y, T(0) );
        aRes /= aY;
        return x;  
    }

    template<class T> hoNDArray< complext<T> >& operator/= (hoNDArray< complext<T> > &x, const T &y)
    {
        arma::Col< std::complex<T> > aRes = as_arma_col(x);
        std::complex<T> aY( y, T(0) );
        aRes /= aY;
        return x;  
    }

    //
    // Instantiation
    //

    template EXPORTCSLAB hoNDArray<float>& operator+=<float>(hoNDArray<float>&, const hoNDArray<float>&);
    template EXPORTCSLAB hoNDArray<float>& operator+=<float>(hoNDArray<float>&, const float&);
    template EXPORTCSLAB hoNDArray<float>& operator-=<float>(hoNDArray<float>&, const hoNDArray<float>&);
    template EXPORTCSLAB hoNDArray<float>& operator-=<float>(hoNDArray<float>&, const float&);
    template EXPORTCSLAB hoNDArray<float>& operator*=<float>(hoNDArray<float>&, const hoNDArray<float>&);
    template EXPORTCSLAB hoNDArray<float>& operator*=<float>(hoNDArray<float>&, const float&);
    template EXPORTCSLAB hoNDArray<float>& operator/=<float>(hoNDArray<float>&, const hoNDArray<float>&);
    template EXPORTCSLAB hoNDArray<float>& operator/=<float>(hoNDArray<float>&, const float&);

    template EXPORTCSLAB hoNDArray<double>& operator+=<double>(hoNDArray<double>&, const hoNDArray<double>&);
    template EXPORTCSLAB hoNDArray<double>& operator+=<double>(hoNDArray<double>&, const double&);
    template EXPORTCSLAB hoNDArray<double>& operator-=<double>(hoNDArray<double>&, const hoNDArray<double>&);
    template EXPORTCSLAB hoNDArray<double>& operator-=<double>(hoNDArray<double>&, const double&);
    template EXPORTCSLAB hoNDArray<double>& operator*=<double>(hoNDArray<double>&, const hoNDArray<double>&);
    template EXPORTCSLAB hoNDArray<double>& operator*=<double>(hoNDArray<double>&, const double&);
    template EXPORTCSLAB hoNDArray<double>& operator/=<double>(hoNDArray<double>&, const hoNDArray<double>&);
    template EXPORTCSLAB hoNDArray<double>& operator/=<double>(hoNDArray<double>&, const double&);

    template EXPORTCSLAB hoNDArray< std::complex<float> >& operator+=< std::complex<float> > 
        (hoNDArray< std::complex<float> >&, const hoNDArray< std::complex<float> >&);
    template EXPORTCSLAB hoNDArray< std::complex<float> >& operator+=< std::complex<float> > 
        (hoNDArray< std::complex<float> >&, const std::complex<float>&);
    template EXPORTCSLAB hoNDArray< std::complex<float> >& operator-=< std::complex<float> > 
        (hoNDArray< std::complex<float> >&, const hoNDArray< std::complex<float> >&);
    template EXPORTCSLAB hoNDArray< std::complex<float> >& operator-=< std::complex<float> > 
        (hoNDArray< std::complex<float> >&, const std::complex<float>&);
    template EXPORTCSLAB hoNDArray< std::complex<float> >& operator*=< std::complex<float> >
        (hoNDArray< std::complex<float> >&, const hoNDArray< std::complex<float> >&);
    template EXPORTCSLAB hoNDArray< std::complex<float> >& operator*=< std::complex<float> >
        (hoNDArray< std::complex<float> >&, const std::complex<float>&);
    template EXPORTCSLAB hoNDArray< std::complex<float> >& operator/=< std::complex<float> > 
        (hoNDArray< std::complex<float> >&, const hoNDArray< std::complex<float> >&);
    template EXPORTCSLAB hoNDArray< std::complex<float> >& operator/=< std::complex<float> > 
        (hoNDArray< std::complex<float> >&, const std::complex<float>&);

    template EXPORTCSLAB hoNDArray< complext<float> >& operator+=< complext<float> > 
        (hoNDArray< complext<float> >&, const hoNDArray< complext<float> >&);
    template EXPORTCSLAB hoNDArray< complext<float> >& operator+=< complext<float> > 
        (hoNDArray< complext<float> >&, const complext<float>&);
    template EXPORTCSLAB hoNDArray< complext<float> >& operator-=< complext<float> > 
        (hoNDArray< complext<float> >&, const hoNDArray< complext<float> >&);
    template EXPORTCSLAB hoNDArray< complext<float> >& operator-=< complext<float> > 
        (hoNDArray< complext<float> >&, const complext<float>&);
    template EXPORTCSLAB hoNDArray< complext<float> >& operator*=< complext<float> >
        (hoNDArray< complext<float> >&, const hoNDArray< complext<float> >&);
    template EXPORTCSLAB hoNDArray< complext<float> >& operator*=< complext<float> >
        (hoNDArray< complext<float> >&, const complext<float>&);
    template EXPORTCSLAB hoNDArray< complext<float> >& operator/=< complext<float> > 
        (hoNDArray< complext<float> >&, const hoNDArray< complext<float> >&);
    template EXPORTCSLAB hoNDArray< complext<float> >& operator/=< complext<float> > 
        (hoNDArray< complext<float> >&, const complext<float>&);

    template EXPORTCSLAB hoNDArray< std::complex<float> >& operator+=<float>(hoNDArray< std::complex<float> >&, const hoNDArray<float>&);
    template EXPORTCSLAB hoNDArray< std::complex<float> >& operator-=<float>(hoNDArray< std::complex<float> >&, const hoNDArray<float>&);
    template EXPORTCSLAB hoNDArray< std::complex<float> >& operator*=<float>(hoNDArray< std::complex<float> >&, const hoNDArray<float>&);
    template EXPORTCSLAB hoNDArray< std::complex<float> >& operator/=<float>(hoNDArray< std::complex<float> >&, const hoNDArray<float>&);

    template EXPORTCSLAB hoNDArray< complext<float> >& operator+=<float>(hoNDArray< complext<float> >&, const hoNDArray<float>&);
    template EXPORTCSLAB hoNDArray< complext<float> >& operator-=<float>(hoNDArray< complext<float> >&, const hoNDArray<float>&);
    template EXPORTCSLAB hoNDArray< complext<float> >& operator*=<float>(hoNDArray< complext<float> >&, const hoNDArray<float>&);
    template EXPORTCSLAB hoNDArray< complext<float> >& operator/=<float>(hoNDArray< complext<float> >&, const hoNDArray<float>&);

    template EXPORTCSLAB hoNDArray< std::complex<float> >& operator+=<float>(hoNDArray< std::complex<float> >&, const float&);
    template EXPORTCSLAB hoNDArray< std::complex<float> >& operator-=<float>(hoNDArray< std::complex<float> >&, const float&);
    template EXPORTCSLAB hoNDArray< std::complex<float> >& operator*=<float>(hoNDArray< std::complex<float> >&, const float&);
    template EXPORTCSLAB hoNDArray< std::complex<float> >& operator/=<float>(hoNDArray< std::complex<float> >&, const float&);

    template EXPORTCSLAB hoNDArray< complext<float> >& operator+=<float>(hoNDArray< complext<float> >&, const float&);
    template EXPORTCSLAB hoNDArray< complext<float> >& operator-=<float>(hoNDArray< complext<float> >&, const float&);
    template EXPORTCSLAB hoNDArray< complext<float> >& operator*=<float>(hoNDArray< complext<float> >&, const float&);
    template EXPORTCSLAB hoNDArray< complext<float> >& operator/=<float>(hoNDArray< complext<float> >&, const float&);

    template EXPORTCSLAB hoNDArray< std::complex<double> >& operator+=< std::complex<double> > 
        (hoNDArray< std::complex<double> >&, const hoNDArray< std::complex<double> >&);
    template EXPORTCSLAB hoNDArray< std::complex<double> >& operator+=< std::complex<double> > 
        (hoNDArray< std::complex<double> >&, const std::complex<double>&);
    template EXPORTCSLAB hoNDArray< std::complex<double> >& operator-=< std::complex<double> > 
        (hoNDArray< std::complex<double> >&, const hoNDArray< std::complex<double> >&);
    template EXPORTCSLAB hoNDArray< std::complex<double> >& operator-=< std::complex<double> > 
        (hoNDArray< std::complex<double> >&, const std::complex<double>&);
    template EXPORTCSLAB hoNDArray< std::complex<double> >& operator*=< std::complex<double> >
        (hoNDArray< std::complex<double> >&, const hoNDArray< std::complex<double> >&);
    template EXPORTCSLAB hoNDArray< std::complex<double> >& operator*=< std::complex<double> >
        (hoNDArray< std::complex<double> >&, const std::complex<double>&);
    template EXPORTCSLAB hoNDArray< std::complex<double> >& operator/=< std::complex<double> > 
        (hoNDArray< std::complex<double> >&, const hoNDArray< std::complex<double> >&);
    template EXPORTCSLAB hoNDArray< std::complex<double> >& operator/=< std::complex<double> > 
        (hoNDArray< std::complex<double> >&, const std::complex<double>&);

    template EXPORTCSLAB hoNDArray< complext<double> >& operator+=< complext<double> > 
        (hoNDArray< complext<double> >&, const hoNDArray< complext<double> >&);
    template EXPORTCSLAB hoNDArray< complext<double> >& operator+=< complext<double> > 
        (hoNDArray< complext<double> >&, const complext<double>&);
    template EXPORTCSLAB hoNDArray< complext<double> >& operator-=< complext<double> > 
        (hoNDArray< complext<double> >&, const hoNDArray< complext<double> >&);
    template EXPORTCSLAB hoNDArray< complext<double> >& operator-=< complext<double> > 
        (hoNDArray< complext<double> >&, const complext<double>&);
    template EXPORTCSLAB hoNDArray< complext<double> >& operator*=< complext<double> >
        (hoNDArray< complext<double> >&, const hoNDArray< complext<double> >&);
    template EXPORTCSLAB hoNDArray< complext<double> >& operator*=< complext<double> >
        (hoNDArray< complext<double> >&, const complext<double>&);
    template EXPORTCSLAB hoNDArray< complext<double> >& operator/=< complext<double> > 
        (hoNDArray< complext<double> >&, const hoNDArray< complext<double> >&);
    template EXPORTCSLAB hoNDArray< complext<double> >& operator/=< complext<double> > 
        (hoNDArray< complext<double> >&, const complext<double>&);

    template EXPORTCSLAB hoNDArray< std::complex<double> >& operator+=<double>(hoNDArray< std::complex<double> >&, const hoNDArray<double>&);
    template EXPORTCSLAB hoNDArray< std::complex<double> >& operator-=<double>(hoNDArray< std::complex<double> >&, const hoNDArray<double>&);
    template EXPORTCSLAB hoNDArray< std::complex<double> >& operator*=<double>(hoNDArray< std::complex<double> >&, const hoNDArray<double>&);
    template EXPORTCSLAB hoNDArray< std::complex<double> >& operator/=<double>(hoNDArray< std::complex<double> >&, const hoNDArray<double>&);

    template EXPORTCSLAB hoNDArray< complext<double> >& operator+=<double>(hoNDArray< complext<double> >&, const hoNDArray<double>&);
    template EXPORTCSLAB hoNDArray< complext<double> >& operator-=<double>(hoNDArray< complext<double> >&, const hoNDArray<double>&);
    template EXPORTCSLAB hoNDArray< complext<double> >& operator*=<double>(hoNDArray< complext<double> >&, const hoNDArray<double>&);
    template EXPORTCSLAB hoNDArray< complext<double> >& operator/=<double>(hoNDArray< complext<double> >&, const hoNDArray<double>&);

    template EXPORTCSLAB hoNDArray< std::complex<double> >& operator+=<double>(hoNDArray< std::complex<double> >&, const double&);
    template EXPORTCSLAB hoNDArray< std::complex<double> >& operator-=<double>(hoNDArray< std::complex<double> >&, const double&);
    template EXPORTCSLAB hoNDArray< std::complex<double> >& operator*=<double>(hoNDArray< std::complex<double> >&, const double&);
    template EXPORTCSLAB hoNDArray< std::complex<double> >& operator/=<double>(hoNDArray< std::complex<double> >&, const double&);

    template EXPORTCSLAB hoNDArray< complext<double> >& operator+=<double>(hoNDArray< complext<double> >&, const double&);
    template EXPORTCSLAB hoNDArray< complext<double> >& operator-=<double>(hoNDArray< complext<double> >&, const double&);
    template EXPORTCSLAB hoNDArray< complext<double> >& operator*=<double>(hoNDArray< complext<double> >&, const double&);
    template EXPORTCSLAB hoNDArray< complext<double> >& operator/=<double>(hoNDArray< complext<double> >&, const double&);
}


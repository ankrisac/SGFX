#pragma once
#include <vector>
#include <iostream>

template<typename T, size_t N, size_t M> class Mat{
    T data[N * M];
public:
    Mat(const T value = T()){
        for(size_t i = 0; i < N * M; ++i){
            data[i] = value;
        }
    }
    Mat(const Mat& other){
        for(size_t i = 0; i < N * M; ++i){
            data[i] = other.data[i];
        }
    }
    Mat(std::initializer_list<T> lst){
        if(lst.size() == N * M){
            size_t i = 0;

            for(auto it: lst){
                data[i] = it;
                i++;
            }
        }
        else{
            throw std::out_of_range("Matrix size mismatch!");
        }
    }
    Mat(std::vector<T> lst){
        if(lst.size() == N * M){
            for(size_t i = 0; i < N * M; ++i){
                data[i] = lst[i];
            }
        }
        else{
            throw std::out_of_range("Matrix size mismatch!");
        }
    }
    ~Mat(){}

    T& operator[](const size_t i){
        if(i < N*M){
            return data[i];
        }
        throw std::out_of_range("Matrix index out of range!");
    }
    const T& operator[](const size_t i) const{
        if(i < N*M){
            return data[i];
        }
        throw std::out_of_range("Matrix index out of range!");
    }

    T& operator()(const size_t i, const size_t j){
        if(i < N && j < M){
            return data[i * M + j];
        }
        throw std::out_of_range("Matrix index out of range!");
    }
    const T& operator()(const size_t i, const size_t j) const{
        if(i < N && j < M){
            return data[i * M + j];
        }
        throw std::out_of_range("Matrix index out of range!");
    }

    T* getData(){
        return data;
    }

    Mat& operator+=(const Mat<T, N, M>& other){
        for(size_t i = 0; i < N * M; ++i){
            data[i] += other.data[i];
        }
        return *this;
    }
    Mat& operator-=(const Mat<T, N, M>& other){
        for(size_t i = 0; i < N * M; ++i){
            data[i] -= other.data[i];
        }
        return *this;
    }
    Mat<T, N, M> transpose() const{
        Mat<T, M, N> out;

        for(size_t i = 0; i < N; ++i){
            for(size_t j = 0; j < M; ++j){
                out(j, i) = this->operator()(i, j); 
            }
        }

        return out;
    }

    size_t getRows() const{
        return N;
    }
    size_t getCols() const{
        return M;
    }
};

template<typename T, size_t N, size_t M>  
Mat<T, N, M>& operator+(Mat<T, N, M>& lhs, const Mat<T, N, M>& rhs){
    return lhs += rhs;
}
template<typename T, size_t N, size_t M> 
Mat<T, N, M>& operator-(Mat<T, N, M>& lhs, const Mat<T, N, M>& rhs){
    return lhs -= rhs;
}
template<typename T, size_t N, size_t K, size_t M> 
Mat<T, N, M> operator*(const Mat<T, N, K> lhs, const Mat<T, K, M> rhs){
    Mat<T, N, M> out;

    for(size_t i = 0; i < N; ++i){
        for(size_t j = 0; j < M; ++j){
            float S = 0;

            for(size_t k = 0; k < K; ++k){
                S += lhs(i, k) * rhs(k, j);
            }

            out(i, j) = S;
        }
    }

    return out;
}
template<typename T, size_t N, size_t M>
std::ostream& operator<<(std::ostream& out, const Mat<T, N, M>& obj){
    out << N << "x" << M << "matrix\n";
    for(size_t i = 0; i < N; ++i){
        out << "[ ";
        for(size_t j = 0; j < M; ++j){
            out << obj(i, j) << " ";
        }
        out << "]\n";
    }

    return out;
}

template<typename T, size_t N> using colVec = Mat<T, N, 1>;
template<typename T, size_t N> using rowVec = Mat<T, 1, N>;
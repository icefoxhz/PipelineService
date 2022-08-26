//
// Created by saber on 2022/5/5.
//

#ifndef HZUTILS_NUMBERUTIL_HPP
#define HZUTILS_NUMBERUTIL_HPP

#include "iostream"
#include <iomanip>
#include <sstream>

using namespace std;

namespace hzNumberUtil {
    /**
     * 保留n位小数
     * @tparam T
     * @param f
     * @param bits
     */
    template<typename T>
    static void keepDecimal(T& num, int bits=2)
    {
        std::stringstream ss;
        ss<<setiosflags(ios::fixed)<<setprecision(bits)<<num;
        ss >> num;
    }

} // hz

#endif //HZUTILS_NUMBERUTIL_HPP

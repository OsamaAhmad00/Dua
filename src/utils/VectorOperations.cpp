#include <utils/VectorOperators.h>

strings operator+(const strings& v1, const strings& v2) {
    strings result(v1);
    result.insert(result.end(), v2.begin(), v2.end());
    return result;
}

strings operator+(const strings& v, const std::string& s) {
    strings result(v.size());
    for (size_t i = 0; i < v.size(); i++)
        result[i] = v[i] + s;
    return result;
}

strings operator+(const std::string& s, const strings& v) {
    strings result(v.size());
    for (size_t i = 0; i < v.size(); i++)
        result[i] = s + v[i];
    return result;
}

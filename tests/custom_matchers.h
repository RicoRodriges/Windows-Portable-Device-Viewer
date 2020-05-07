#include <codecvt>

#include "fakeit.hpp"


// Matcher for wchar_t* and std::wstring
template<typename T>
struct EqSTRCreator : public fakeit::TypedMatcherCreator<T> {

    const std::wstring expected;

    virtual ~EqSTRCreator() = default;

    EqSTRCreator(const std::wstring &expected)
            : fakeit::TypedMatcherCreator<T>(),
              expected(expected) {
    }

    struct Matcher : public fakeit::TypedMatcher<T> {
        const std::wstring expected;

        Matcher(const std::wstring &expected) : expected(expected) {}

        std::string format() const override {
            using convert_typeX = std::codecvt_utf8<wchar_t>;
            std::wstring_convert<convert_typeX, wchar_t> converterX;
            return converterX.to_bytes(expected);
        }

        bool matches(const T &actual) const override {
            return actual == expected;
        }
    };

    fakeit::TypedMatcher<T> *createMatcher() const override {
        return new Matcher(expected);
    }

};

template<typename T>
EqSTRCreator<T> EqSTR(const std::wstring &arg) {
    return EqSTRCreator<T>{arg};
}


// Matcher for T* and T
template<typename T, typename E = typename std::remove_pointer<T>::type>
struct EqPTRCreator : public fakeit::TypedMatcherCreator<T> {

    const E expected;

    virtual ~EqPTRCreator() = default;

    EqPTRCreator(const E &expected)
            : fakeit::TypedMatcherCreator<T>(),
              expected(expected) {
    }

    struct Matcher : public fakeit::TypedMatcher<T> {
        const E expected;

        Matcher(const E &expected) : expected(expected) {}

        std::string format() const override {
            return TypeFormatter<E>::format(expected);
        }

        bool matches(const T &actual) const override {
            return *actual == expected;
        }
    };

    fakeit::TypedMatcher<T> *createMatcher() const override {
        return new Matcher(expected);
    }

};

template<typename T>
EqPTRCreator<T> EqPTR(const typename std::remove_pointer<T>::type &arg) {
    return EqPTRCreator<T>{arg};
}
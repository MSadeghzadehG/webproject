
#pragma once

#include <new>
#include <utility>
#include "common/assert.h"
#include "common/bit_field.h"
#include "common/common_funcs.h"
#include "common/common_types.h"



enum class ErrorDescription : u32 {
    Success = 0,
    RemoteProcessDead = 301,
    InvalidOffset = 6061,
    InvalidLength = 6062,
};


enum class ErrorModule : u32 {
    Common = 0,
    Kernel = 1,
    FS = 2,
    NvidiaTransferMemory = 3,
    NCM = 5,
    DD = 6,
    LR = 8,
    Loader = 9,
    CMIF = 10,
    HIPC = 11,
    PM = 15,
    NS = 16,
    HTC = 18,
    SM = 21,
    RO = 22,
    SDMMC = 24,
    SPL = 26,
    ETHC = 100,
    I2C = 101,
    Settings = 105,
    NIFM = 110,
    Display = 114,
    NTC = 116,
    FGM = 117,
    PCIE = 120,
    Friends = 121,
    SSL = 123,
    Account = 124,
    Mii = 126,
    AM = 128,
    PlayReport = 129,
    PCV = 133,
    OMM = 134,
    NIM = 137,
    PSC = 138,
    USB = 140,
    BTM = 143,
    ERPT = 147,
    APM = 148,
    NPNS = 154,
    ARP = 157,
    BOOT = 158,
    NFC = 161,
    UserlandAssert = 162,
    UserlandCrash = 168,
    HID = 203,
    Capture = 206,
    TC = 651,
    GeneralWebApplet = 800,
    WifiWebAuthApplet = 809,
    WhitelistedApplet = 810,
    ShopN = 811,
};

union ResultCode {
    u32 raw;

    BitField<0, 9, ErrorModule> module;
    BitField<9, 13, u32> description;

            BitField<31, 1, u32> is_error;

    constexpr explicit ResultCode(u32 raw) : raw(raw) {}

    constexpr ResultCode(ErrorModule module, ErrorDescription description)
        : ResultCode(module, static_cast<u32>(description)) {}

    constexpr ResultCode(ErrorModule module_, u32 description_)
        : raw(module.FormatValue(module_) | description.FormatValue(description_)) {}

    constexpr ResultCode& operator=(const ResultCode& o) {
        raw = o.raw;
        return *this;
    }

    constexpr bool IsSuccess() const {
        return raw == 0;
    }

    constexpr bool IsError() const {
        return raw != 0;
    }
};

constexpr bool operator==(const ResultCode& a, const ResultCode& b) {
    return a.raw == b.raw;
}

constexpr bool operator!=(const ResultCode& a, const ResultCode& b) {
    return a.raw != b.raw;
}


constexpr ResultCode RESULT_SUCCESS(0);


template <typename T>
class ResultVal {
public:
            ResultVal(ResultCode error_code = ResultCode(-1)) : result_code(error_code) {
        ASSERT(error_code.IsError());
    }

    
    template <typename... Args>
    static ResultVal WithCode(ResultCode success_code, Args&&... args) {
        ResultVal<T> result;
        result.emplace(success_code, std::forward<Args>(args)...);
        return result;
    }

    ResultVal(const ResultVal& o) : result_code(o.result_code) {
        if (!o.empty()) {
            new (&object) T(o.object);
        }
    }

    ResultVal(ResultVal&& o) : result_code(o.result_code) {
        if (!o.empty()) {
            new (&object) T(std::move(o.object));
        }
    }

    ~ResultVal() {
        if (!empty()) {
            object.~T();
        }
    }

    ResultVal& operator=(const ResultVal& o) {
        if (!empty()) {
            if (!o.empty()) {
                object = o.object;
            } else {
                object.~T();
            }
        } else {
            if (!o.empty()) {
                new (&object) T(o.object);
            }
        }
        result_code = o.result_code;

        return *this;
    }

    
    template <typename... Args>
    void emplace(ResultCode success_code, Args&&... args) {
        ASSERT(success_code.IsSuccess());
        if (!empty()) {
            object.~T();
        }
        new (&object) T(std::forward<Args>(args)...);
        result_code = success_code;
    }

        bool empty() const {
        return result_code.IsError();
    }

        bool Succeeded() const {
        return result_code.IsSuccess();
    }
        bool Failed() const {
        return empty();
    }

    ResultCode Code() const {
        return result_code;
    }

    const T& operator*() const {
        return object;
    }
    T& operator*() {
        return object;
    }
    const T* operator->() const {
        return &object;
    }
    T* operator->() {
        return &object;
    }

        template <typename U>
    T ValueOr(U&& value) const {
        return !empty() ? object : std::move(value);
    }

        T& Unwrap() & {
        ASSERT_MSG(Succeeded(), "Tried to Unwrap empty ResultVal");
        return **this;
    }

    T&& Unwrap() && {
        ASSERT_MSG(Succeeded(), "Tried to Unwrap empty ResultVal");
        return std::move(**this);
    }

private:
            union {
        T object;
    };
    ResultCode result_code;
};


template <typename T, typename... Args>
ResultVal<T> MakeResult(Args&&... args) {
    return ResultVal<T>::WithCode(RESULT_SUCCESS, std::forward<Args>(args)...);
}


template <typename Arg>
ResultVal<std::remove_reference_t<Arg>> MakeResult(Arg&& arg) {
    return ResultVal<std::remove_reference_t<Arg>>::WithCode(RESULT_SUCCESS,
                                                             std::forward<Arg>(arg));
}


#define CASCADE_RESULT(target, source)                                                             \
    auto CONCAT2(check_result_L, __LINE__) = source;                                               \
    if (CONCAT2(check_result_L, __LINE__).Failed())                                                \
        return CONCAT2(check_result_L, __LINE__).Code();                                           \
    target = std::move(*CONCAT2(check_result_L, __LINE__))


#define CASCADE_CODE(source)                                                                       \
    auto CONCAT2(check_result_L, __LINE__) = source;                                               \
    if (CONCAT2(check_result_L, __LINE__).IsError())                                               \
        return CONCAT2(check_result_L, __LINE__);

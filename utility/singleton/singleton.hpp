#pragma once

namespace utility
{
    template<typename T>
    class Singleton
    {
    public:
        static T * Instance()
        {
            static T instance;
            return &instance;
        }
    private:
        Singleton() = default;
        Singleton(const Singleton<T> &) = delete;
        Singleton<T> & operator = (const Singleton<T> &) = delete;
        ~Singleton() = default;
    };

    #define SINGLETON(ClassName)                                            \
        friend class Singleton<ClassName>;                                  \
        public:                                                             \
            static ClassName* Instance(){static ClassName ins;return &ins;} \
        private:                                                            \
            ClassName() = default;                                          \
            ClassName(const ClassName &) = delete;                          \
            ClassName & operator = (const ClassName &) = delete;            \
            ~ClassName() = default
}

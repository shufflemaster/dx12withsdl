#pragma once

//Runtime hashed string. Numerically equivalent to entt::hashed_string
namespace GAL
{
    using StringHash = std::uint64_t;

    class HashedString final
    {
        static constexpr std::uint64_t offset = 14695981039346656037ull;
        static constexpr std::uint64_t prime = 1099511628211ull;

    public:
        HashedString() : m_str(""), m_hash(0) {}
        HashedString(const char *str) { SetString(str); }
        HashedString(const std::string& str) { SetString(str.c_str()); }
        ~HashedString() {};

        void SetString(const char *str)
        {
            m_str = str;
            m_hash = CalcHash(str);
        }

        static StringHash CalcHash(const char * str)
        {
            if (!str) {
                return 0;
            }

            StringHash retVal = offset;
            while (char val = *str)
            {
                retVal = (retVal ^ val) * prime;
                ++str;
            }

            return retVal;
        }

        const char * GetStringCstr() const { return m_str.c_str(); }
        const std::string& GetString() const { return m_str; }
        StringHash GetHash() const { return m_hash; }

    private:
        std::string m_str;
        StringHash m_hash;
    };

}; // namespace GAL
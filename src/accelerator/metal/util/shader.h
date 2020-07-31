#pragma once

#include <memory>
#include <string>
#include <type_traits>

namespace caspar { namespace accelerator { namespace metal {

class shader final
{
    shader(const shader&);
    shader& operator=(const shader&);

  public:
    shader(const std::string& vertex_source_str, const std::string& fragment_source_str);
    ~shader();

    void set(const std::string& name, bool value);
    void set(const std::string& name, int value);
    void set(const std::string& name, float value);
    void set(const std::string& name, double value0, double value1);
    void set(const std::string& name, double value);

    template <typename E>
    typename std::enable_if<std::is_enum<E>::value, void>::type set(const std::string& name, E value)
    {
        set(name, static_cast<typename std::underlying_type<E>::type>(value));
    }

    void use() const;

    int id() const;

  private:
    struct impl;
    std::unique_ptr<impl> impl_;
};

}}} // namespace caspar::accelerator::metal

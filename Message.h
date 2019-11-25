#pragma once
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <vector>

namespace ipc {

namespace detail {

  template <bool B, typename T = void>
  using enable_if_t = typename std::enable_if<B, T>::type;

  template <typename T>
  using remove_const_t = typename std::remove_const<T>::type;

  template <typename T>
  using remove_cvref_t = typename std::remove_reference<T>::type;

  template <typename T>
  using is_arithmetic_t = std::is_arithmetic<T>;

  template <typename T> using value_type_t = typename T::value_type;

  template <typename T> using iterator_t = typename T::iterator;

  struct nonesuch {
    nonesuch() = delete;
    ~nonesuch() = delete;
    nonesuch(nonesuch const&) = delete;
    nonesuch(nonesuch const&&) = delete;
    void operator=(nonesuch const&) = delete;
    void operator=(nonesuch&&) = delete;
  };

  template <typename... Ts> struct make_void {
    using type = void;
  };

  template <typename... Ts> using void_t = typename make_void<Ts...>::type;

  template <class Default, class AlwaysVoid, template <class...> class Op,
      class... Args>
  struct detector {
    using value_t = std::false_type;
    using type = Default;
  };

  template <class Default, template <class...> class Op, class... Args>
  struct detector<Default, void_t<Op<Args...>>, Op, Args...> {
    using value_t = std::true_type;
    using type = Op<Args...>;
  };

  template <template <class...> class Op, class... Args>
  using is_detected = typename detector<nonesuch, void, Op, Args...>::value_t;

  template <typename CompatibleArrayType, typename = void>
  struct is_compatible_array_type_impl : std::false_type {
  };

  template <typename CompatibleArrayType>
  struct is_compatible_array_type_impl<CompatibleArrayType,
      enable_if_t<is_detected<value_type_t, CompatibleArrayType>::value
          and is_detected<iterator_t, CompatibleArrayType>::value>>
      : std::true_type {
  };

  template <typename CompatibleArrayType>
  struct is_compatible_array_type
      : is_compatible_array_type_impl<CompatibleArrayType> {
  };

} // namespace detail

using namespace detail;

class message;

template <typename T, typename = void> struct Serializer {
  static inline void serialize(message& m, const T& thing)
  {
    to_message(m, thing);
  }

  static inline void deserialize(message& m, T& thing)
  {
    from_message(m, thing);
  }
};

template <template <typename...> class Container, class T>
struct Serializer<Container<T>,
    enable_if_t<is_compatible_array_type<Container<T>>::value>>;

template <typename Arithmetic>
struct Serializer<Arithmetic, enable_if_t<is_arithmetic_t<Arithmetic>::value>>;

class message {
  private:
  std::vector<uint8_t> m_data;
  unsigned m_readPos;

  public:
  message(const uint32_t m_structHash, const uint32_t m_methodHash)
      : m_data()
      , m_readPos(0)
  {
    *this << m_structHash << m_methodHash;
  }

  inline const uint8_t* const data() const { return m_data.data(); }

  inline size_t size() const { return m_data.size(); }

  inline void write(const uint8_t byte) { m_data.push_back(byte); }

  inline uint8_t read()
  {
    return m_readPos + 8 < m_data.size() ? m_data[8 + m_readPos++] : 0;
  }

  template <typename T> inline message& operator<<(T&& thing)
  {
    Serializer<remove_const_t<remove_cvref_t<T>>>::serialize(*this, thing);
    return *this;
  }

  template <typename T> inline message& operator>>(T& thing)
  {
    Serializer<remove_const_t<remove_cvref_t<T>>>::deserialize(*this, thing);
    return *this;
  }
};

template <template <typename...> class Container, class T>
struct Serializer<Container<T>,
    enable_if_t<is_compatible_array_type<Container<T>>::value>> {
  static inline void serialize(message& m, const Container<T>& thing)
  {
    m << thing.size();
    for (auto el : thing)
      m << el;
  }

  static inline void deserialize(message& m, Container<T>& thing)
  {
    size_t size;
    m >> size;
    thing.clear();
    thing.reserve(size);
    for (size_t i = 0; i < size; i++) {
      thing.push_back(T());
      m >> thing.back();
    }
  }
};

template <typename Arithmetic>
struct Serializer<Arithmetic, enable_if_t<is_arithmetic_t<Arithmetic>::value>> {
  static inline void serialize(message& m, const Arithmetic& thing)
  {
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&thing);
    for (size_t i = 0; i < sizeof(Arithmetic); ++i)
      m.write(bytes[i]);
  }

  static inline void deserialize(message& m, Arithmetic& thing)
  {
    uint8_t* bytes = reinterpret_cast<uint8_t*>(&thing);
    for (size_t i = 0; i < sizeof(Arithmetic); ++i)
      bytes[i] = m.read();
  }
};

template <class RawArray, std::size_t N>
struct Serializer<RawArray[N],
    enable_if_t<!std::is_pointer<RawArray>::value>> {
  static inline void serialize(message& m, const RawArray (&thing)[N])
  {
    m << N;
    for (size_t i = 0; i < N; ++i)
      m << thing[i];
  }

  static inline void deserialize(message& m, RawArray (&thing)[N])
  {
    size_t size;
    m >> size;
    size = std::min(N, size);
    for (size_t i = 0; i < size; i++) {
      m >> thing[i];
    }
  }
};

template <> struct Serializer<const char*&> {
  static inline void serialize(message& m, const char* const& thing)
  {
    size_t size = strlen(thing);
    m << size;
    for (size_t i = 0; i < size; ++i)
      m << thing[i];
  }
};

} // namespace ipc

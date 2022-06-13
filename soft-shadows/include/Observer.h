#include <functional>

template <class T>
inline void HashCombine(std::size_t& s, const T& v)
{
	std::hash<T> h;
	s ^= h(v) + 0x9e3779b9 + (s << 6) + (s >> 2);
}

template <typename T>
struct Hash
{
	std::size_t operator()(const T& desc) const
	{
		static_assert(false, "You must write specialization for your type");
	}
};

template <typename T>
class Observer
{
public:
	Observer(T value) : m_value{ value }, m_hashValue{ 0 } {}
	T& Get() { UpdateHash(); return m_value; }
	const T& Get() const { return m_value; }
	bool IsChanged() const { return m_hashValue != CalculateHash(); }

private:
	void UpdateHash()
	{
		m_hashValue = CalculateHash();
	}

	size_t CalculateHash() const
	{
		const size_t val = m_hash(m_value);
		return val;
	}

private:
	T m_value;
	Hash<T> m_hash;
	size_t m_hashValue;
};